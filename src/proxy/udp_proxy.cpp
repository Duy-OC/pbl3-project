#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include "../common/envelope.h"
#include "../common/network.h"
#include "coverage_gate.h"
#include "coverage_model.h"
#include "handover_executor.h"
#include "link_monitor.h"
#include "position_tracker.h"
#include "proxy_config.h"
#include "rssi_model.h"
#include <iomanip>
#include <ws2tcpip.h>
#include <windows.h>

#include <array>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
volatile LONG stopping = 0;

BOOL WINAPI onControl(DWORD event) {
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&stopping, 1);
    return TRUE;
}

struct Socket {
    SOCKET value = INVALID_SOCKET;
    ~Socket() { Network::closeSocket(value); }
    Socket() = default;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
};

struct GatewayPeer {
    std::string id;
    std::string ip;
    unsigned short port = 0;
    sockaddr_in addr{};
    std::unique_ptr<Socket> sock = std::make_unique<Socket>();
    std::optional<GatewayZone> zone;
    double lastRssiDbm = -120.0;
    bool hasRssi = false;
    bool wasInRange = false;
    bool hadRangeState = false;
};

unsigned short port(const char* text) {
    const std::string value(text);
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        throw std::runtime_error("Port must be an integer from 1 to 65535");
    const auto number = std::stoul(value);
    if (number == 0 || number > 65535) throw std::runtime_error("Port out of range");
    return static_cast<unsigned short>(number);
}

sockaddr_in address(const char* ip, unsigned short number) {
    sockaddr_in result{};
    result.sin_family = AF_INET;
    result.sin_port = htons(number);
    unsigned long packed = 0;
    const std::string text(ip);
    std::size_t start = 0;
    for (int part = 0; part < 4; ++part) {
        const auto end = text.find('.', start);
        const auto token = text.substr(start, end == std::string::npos ? end : end - start);
        if (token.empty() || token.size() > 3 || token.find_first_not_of("0123456789") != std::string::npos ||
            (part < 3 && end == std::string::npos) || (part == 3 && end != std::string::npos))
            throw std::runtime_error(std::string("Invalid IPv4 address: ") + ip);
        const auto octet = std::stoul(token);
        if (octet > 255) throw std::runtime_error(std::string("Invalid IPv4 address: ") + ip);
        packed = (packed << 8) | octet;
        start = end + 1;
    }
    result.sin_addr.s_addr = htonl(packed);
    return result;
}

std::string endpoint(const sockaddr_in& addr) {
    const auto ip = ntohl(addr.sin_addr.s_addr);
    return std::to_string((ip >> 24) & 255) + "." + std::to_string((ip >> 16) & 255) + "." +
           std::to_string((ip >> 8) & 255) + "." + std::to_string(ip & 255) + ":" +
           std::to_string(ntohs(addr.sin_port));
}

bool sendPacket(SOCKET socket, const char* data, int length, const sockaddr_in* target) {
    const int sent = target
        ? sendto(socket, data, length, 0, reinterpret_cast<const sockaddr*>(target), sizeof(*target))
        : send(socket, data, length, 0);
    if (sent == SOCKET_ERROR)
        std::cerr << "UDP send failed, packet dropped: " << WSAGetLastError() << '\n';
    else if (sent != length)
        std::cerr << "Incomplete UDP send, packet dropped\n";
    return sent == length;
}
} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: udp_proxy.exe [--config FILE [--gateway ID]] [listen_port [gateway_ipv4 [gateway_port [bind_ipv4]]]]\n"
                     "Defaults: 14600 127.0.0.1 14550 0.0.0.0\n"
                     "One PX4 per process; the first UDP sender is pinned until restart.\n";
        return 0;
    }
    try {
        proxy_config::Settings settings;
        int firstArg = 1;
        if (argc > 1 && std::string(argv[1]) == "--config") {
            if (argc < 3) throw std::runtime_error("--config requires a JSON file");
            firstArg = 3;
            std::string gatewayId = "A";
            if (argc > firstArg && std::string(argv[firstArg]) == "--gateway") {
                if (argc <= firstArg + 1) throw std::runtime_error("--gateway requires an ID");
                gatewayId = argv[firstArg + 1];
                firstArg += 2;
            }
            settings.load(argv[2], gatewayId);
        }
        const int countArgs = argc - firstArg;
        if (countArgs > 4) throw std::runtime_error("Too many arguments; use --help");
        if (countArgs > 0) settings.listenPort = port(argv[firstArg]);
        if (countArgs > 1) settings.gatewayIp = argv[firstArg + 1];
        if (countArgs > 2) settings.gatewayPort = port(argv[firstArg + 2]);
        if (countArgs > 3) settings.bindIp = argv[firstArg + 3];

        if (settings.gateways.empty()) {
            settings.gateways.push_back({settings.gatewayId, settings.gatewayIp, settings.gatewayPort});
        } else {
            for (auto& gw : settings.gateways) {
                if (gw.id == settings.gatewayId) {
                    gw.ip = settings.gatewayIp;
                    gw.port = settings.gatewayPort;
                }
            }
        }

        if (!settings.transform.valid())
            std::cerr << "NED-to-Gazebo calibration missing or invalid. "
                         "See docs/udp_proxy.md and scripts/calibrate_proxy_position.py.\n";

        Network::Runtime runtime;
        if (runtime.error() != 0) throw std::runtime_error("WSAStartup failed: " + std::to_string(runtime.error()));
        const auto local = address(settings.bindIp.c_str(), settings.listenPort);

        Socket px4Socket;
        px4Socket.value = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (px4Socket.value == INVALID_SOCKET)
            throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
        const BOOL exclusive = TRUE;
        if (setsockopt(px4Socket.value, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                       reinterpret_cast<const char*>(&exclusive), sizeof(exclusive)) == SOCKET_ERROR)
            throw std::runtime_error("SO_EXCLUSIVEADDRUSE failed: " + std::to_string(WSAGetLastError()));
        if (bind(px4Socket.value, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR)
            throw std::runtime_error("bind failed: " + std::to_string(WSAGetLastError()));
        if (!Network::setNonblocking(px4Socket.value))
            throw std::runtime_error("Cannot set nonblocking mode");

        std::vector<GatewayPeer> peers;
        GatewayPeer* activePeer = nullptr;
        for (const auto& cfgGw : settings.gateways) {
            GatewayPeer peer;
            peer.id = cfgGw.id;
            peer.ip = cfgGw.ip;
            peer.port = static_cast<unsigned short>(cfgGw.port);
            peer.addr = address(peer.ip.c_str(), peer.port);
            peer.sock->value = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (peer.sock->value == INVALID_SOCKET)
                throw std::runtime_error("gateway socket failed: " + std::to_string(WSAGetLastError()));
            if (connect(peer.sock->value, reinterpret_cast<const sockaddr*>(&peer.addr), sizeof(peer.addr)) == SOCKET_ERROR)
                throw std::runtime_error("UDP connect failed: " + std::to_string(WSAGetLastError()));
            if (!Network::setNonblocking(peer.sock->value))
                throw std::runtime_error("Cannot set gateway nonblocking mode");
            for (const auto& z : settings.zones) {
                if (z.id == peer.id) {
                    peer.zone = z;
                    break;
                }
            }
            peers.push_back(std::move(peer));
        }
        for (auto& peer : peers) {
            if (peer.id == settings.gatewayId) {
                activePeer = &peer;
                break;
            }
        }
        if (!activePeer) throw std::runtime_error("Active gateway not found: " + settings.gatewayId);

        if (!SetConsoleCtrlHandler(onControl, TRUE)) throw std::runtime_error("Cannot install Ctrl+C handler");
        std::cout << "PX4 -> " << endpoint(local) << " <-> gateway " << activePeer->id << ' ' << endpoint(activePeer->addr)
                  << "\nWaiting for PX4; Ctrl+C to stop.\n" << std::flush;

        sockaddr_in px4{};
        bool learned = false;
        PositionTracker positionTracker(static_cast<std::uint8_t>(settings.sysid));
        CoverageGate coverageGate(settings.coveragePolicy);
        RssiModel rssiModel(settings.rssiConfig);
        LinkMonitor linkMonitor(settings.probeConfig);
        HandoverExecutor handoverExecutor(settings.handoverMode == "rssi" || settings.handoverMode == "auto");

        using Clock = PositionTracker::Clock;
        const auto staleAfter = std::chrono::milliseconds(settings.staleMs);
        const auto worldPosition = [&](const PositionTracker::Snapshot& ned) -> std::optional<WorldPosition> {
            if (ned.status != PositionTracker::Status::Fresh) return std::nullopt;
            return settings.transform.apply(ned.position->x, ned.position->y, ned.position->z);
        };
        const auto peerStatus = [&](const GatewayPeer& peer, const std::optional<WorldPosition>& world) {
            if (!peer.zone) {
                CoverageModel::GatewayStatus st;
                st.gatewayId = peer.id;
                st.inRange = true;
                return st;
            }
            return CoverageModel::evaluateOne(*peer.zone, world);
        };

        auto nextLog = Clock::time_point::min();
        auto nextReport = Clock::time_point::min();
        std::string previousStatus;
        auto previousMotion = PositionTracker::Motion::Stationary;
        bool previousTransform = settings.transform.valid();
        std::string previousCoverage;
        std::uint64_t forwardedToGateway = 0, forwardedToPx4 = 0;
        std::uint64_t droppedToGateway = 0, droppedToPx4 = 0;
        std::array<char, 65535> buffer{};

        while (InterlockedCompareExchange(&stopping, 0, 0) == 0) {
            fd_set readable;
            FD_ZERO(&readable);
            FD_SET(px4Socket.value, &readable);
            for (const auto& peer : peers) {
                FD_SET(peer.sock->value, &readable);
            }
            timeval timeout{0, 50000};
            if (select(0, &readable, nullptr, nullptr, &timeout) == SOCKET_ERROR)
                throw std::runtime_error("select failed: " + std::to_string(WSAGetLastError()));

            const auto now = Clock::now();

            if (FD_ISSET(px4Socket.value, &readable)) {
                sockaddr_in source{};
                int sourceLength = sizeof(source);
                const int count = recvfrom(px4Socket.value, buffer.data(), static_cast<int>(buffer.size()), 0,
                    reinterpret_cast<sockaddr*>(&source), &sourceLength);
                if (count == SOCKET_ERROR) {
                    const int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK)
                        std::cerr << "UDP receive failed, packet dropped: " << error << '\n';
                } else {
                    if (!learned) {
                        px4 = source;
                        learned = true;
                        std::cout << "PX4 endpoint: " << endpoint(px4) << '\n' << std::flush;
                    }
                    if (Network::sameAddress(source, px4)) {
                        positionTracker.ingest(buffer.data(), static_cast<std::size_t>(count));
                        if (settings.transform.valid() && positionTracker.sawBootTimeRegression()) {
                            settings.transform.invalidate();
                            std::cout << "[POSITION] PX4 boot time moved backwards; verify NED origin before re-enabling transform.\n";
                        }
                        const auto world = worldPosition(positionTracker.snapshot(settings.sysid, now, staleAfter));
                        const auto activeSt = peerStatus(*activePeer, world);
                        if (coverageGate.allow(activeSt, CoverageGate::TrafficKind::Telemetry))
                            forwardedToGateway += sendPacket(activePeer->sock->value, buffer.data(), count, nullptr);
                        else
                            ++droppedToGateway;
                    }
                }
            }

            const auto ned = positionTracker.snapshot(settings.sysid, now, staleAfter);
            const auto world = worldPosition(ned);

            for (auto& peer : peers) {
                if (!FD_ISSET(peer.sock->value, &readable)) continue;
                const int count = recv(peer.sock->value, buffer.data(), static_cast<int>(buffer.size()), 0);
                if (count == SOCKET_ERROR) {
                    const int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK && error != WSAECONNRESET)
                        std::cerr << "Gateway UDP receive failed, packet dropped: " << error << '\n';
                    continue;
                }
                if (EnvelopeCodec::isEnvelope(buffer.data(), static_cast<std::size_t>(count))) {
                    EnvelopeCodec::Envelope env;
                    if (EnvelopeCodec::decode(buffer.data(), static_cast<std::size_t>(count), env)) {
                        if (env.type == EnvelopeCodec::MessageType::ProbePong) {
                            linkMonitor.onPong(env, now);
                        }
                    }
                    continue;
                }
                if (&peer == activePeer && learned) {
                    const auto activeSt = peerStatus(*activePeer, world);
                    if (coverageGate.allow(activeSt, CoverageGate::TrafficKind::Command))
                        forwardedToPx4 += sendPacket(px4Socket.value, buffer.data(), count, &px4);
                    else
                        ++droppedToPx4;
                }
            }

            // Probe, RSSI & coverage-change notifications per gateway.
            std::vector<HandoverExecutor::Candidate> handoverCandidates;
            handoverCandidates.reserve(peers.size());
            for (auto& peer : peers) {
                const auto st = peerStatus(peer, world);
                if (rssiModel.enabled() && world && peer.zone) {
                    peer.lastRssiDbm = rssiModel.estimateDbm(st.distance3dM);
                    peer.hasRssi = true;
                } else {
                    peer.hasRssi = false;
                }
                if (coverageGate.policy().enabled && peer.zone && world) {
                    if (!peer.hadRangeState || peer.wasInRange != st.inRange) {
                        peer.hadRangeState = true;
                        peer.wasInRange = st.inRange;
                        if (st.inRange) {
                            std::cout << "\n[COVERAGE] Phat hien vung kha dung: Gateway " << peer.id
                                      << " (dist=" << std::fixed << std::setprecision(2) << st.horizontalDistanceM << " m";
                            if (peer.hasRssi)
                                std::cout << ", RSSI=" << std::setprecision(1) << peer.lastRssiDbm << " dBm";
                            std::cout << ")\n" << std::flush;
                        } else {
                            std::cout << "\n[COVERAGE] Mat vung phu: Gateway " << peer.id << "\n" << std::flush;
                        }
                    }
                }
                handoverCandidates.push_back({peer.id, st.inRange, peer.lastRssiDbm, peer.hasRssi});

                const bool allowProbe = coverageGate.allow(st, CoverageGate::TrafficKind::Probe);
                const auto pings = linkMonitor.tickGateway(peer.id, static_cast<std::uint8_t>(settings.sysid), allowProbe, now);
                for (const auto& ping : pings) {
                    const auto wire = EnvelopeCodec::encode(ping);
                    sendPacket(peer.sock->value, reinterpret_cast<const char*>(wire.data()), static_cast<int>(wire.size()), nullptr);
                }
            }

            // RSSI-based handover (strongest RSSI wins; equal RSSI keeps current gateway).
            if (handoverExecutor.enabled()) {
                const auto decision = handoverExecutor.evaluate(activePeer->id, handoverCandidates);
                if (decision.switched) {
                    for (auto& peer : peers) {
                        if (peer.id == decision.toGateway) {
                            activePeer = &peer;
                            break;
                        }
                    }
                    std::cout << "\n[HANDOVER] Chuyen vung phuc vu: Gateway " << decision.fromGateway
                              << " -> Gateway " << decision.toGateway
                              << " (RSSI: " << std::fixed << std::setprecision(1)
                              << decision.fromRssiDbm << " -> " << decision.toRssiDbm << " dBm)\n"
                              << std::flush;

                    // Notify Central through the new active gateway so RouteTable updates immediately.
                    EnvelopeCodec::Envelope ack;
                    ack.type = EnvelopeCodec::MessageType::HandoverAck;
                    ack.sysid = static_cast<std::uint8_t>(settings.sysid);
                    ack.gatewayId = activePeer->id;
                    ack.targetGatewayId = activePeer->id;
                    ack.payload = EnvelopeCodec::encodeHandover({0u, EnvelopeCodec::HandoverStatus::Accepted});
                    const auto wire = EnvelopeCodec::encode(ack);
                    sendPacket(activePeer->sock->value, reinterpret_cast<const char*>(wire.data()), static_cast<int>(wire.size()), nullptr);
                }
            }

            // Periodically send LinkReport to each gateway when probe or RSSI is enabled.
            if ((linkMonitor.enabled() || rssiModel.enabled()) && now >= nextReport) {
                nextReport = now + std::chrono::milliseconds(500);
                for (const auto& peer : peers) {
                    const auto st = peerStatus(peer, world);
                    if (!coverageGate.allow(st, CoverageGate::TrafficKind::Probe)) continue;
                    const auto q = linkMonitor.qualityFor(peer.id);
                    EnvelopeCodec::LinkReportPayload rep;
                    rep.rssiDbm = peer.hasRssi ? static_cast<float>(peer.lastRssiDbm) : -120.0f;
                    rep.rttMs = static_cast<float>(q.lastRttMs);
                    rep.probeLossRatio = static_cast<float>(q.probeLossRatio);
                    rep.consecutiveTimeouts = q.consecutiveTimeouts;
                    rep.positionAgeMs = ned.position ? static_cast<std::uint32_t>(ned.age.count()) : 0u;
                    rep.inRange = st.inRange ? 1u : 0u;
                    rep.linkUp = (st.inRange && (!linkMonitor.enabled() || q.consecutiveTimeouts == 0)) ? 1u : 0u;

                    EnvelopeCodec::Envelope env;
                    env.type = EnvelopeCodec::MessageType::LinkReport;
                    env.sysid = static_cast<std::uint8_t>(settings.sysid);
                    env.gatewayId = peer.id;
                    env.payload = EnvelopeCodec::encodeLinkReport(rep);
                    const auto wire = EnvelopeCodec::encode(env);
                    sendPacket(peer.sock->value, reinterpret_cast<const char*>(wire.data()), static_cast<int>(wire.size()), nullptr);
                }
            }

            const bool transformValid = settings.transform.valid();
            const auto activeSt = peerStatus(*activePeer, world);
            const auto distance = activePeer->zone && world
                ? std::optional<double>(activeSt.horizontalDistanceM) : std::nullopt;
            const char* coverage = "OFF";
            if (coverageGate.policy().enabled && activePeer->zone) {
                if (!distance) coverage = "BLOCKED_NO_POSITION";
                else coverage = activeSt.inRange ? "IN_RANGE" : "OUT_OF_RANGE";
            }
            const char* positionStatus = "NO_POSITION";
            if (ned.status == PositionTracker::Status::Stale) {
                positionStatus = "STALE";
            } else if (ned.status == PositionTracker::Status::Fresh) {
                positionStatus = world ? "VALID" : "TRANSFORM_NOT_CONFIGURED";
            }
            const auto motion = ned.position ? ned.position->motion : PositionTracker::Motion::Stationary;
            if (now >= nextLog || previousStatus != positionStatus ||
                previousMotion != motion || previousTransform != transformValid || previousCoverage != coverage) {
                nextLog = now + std::chrono::seconds(1);
                previousStatus = positionStatus;
                previousMotion = motion;
                previousTransform = transformValid;
                previousCoverage = coverage;
                std::ostringstream out;
                out << std::fixed << std::setprecision(3)
                    << "\n==================== PROXY STATUS (SYSID=" << settings.sysid << ") ====================\n"
                    << "  Trang thai   : " << positionStatus
                    << " | " << (ned.position ? PositionTracker::motionName(motion) : "-")
                    << " | age=" << (ned.position ? std::to_string(ned.age.count()) + " ms" : "-") << "\n";
                if (ned.position)
                    out << "  Vi tri NED   : N=" << std::setw(8) << ned.position->x
                        << " m | E=" << std::setw(8) << ned.position->y
                        << " m | D=" << std::setw(8) << ned.position->z << " m\n";
                else
                    out << "  Vi tri NED   : -\n";
                if (world)
                    out << "  Vi tri World : X=" << std::setw(8) << world->x
                        << " m | Y=" << std::setw(8) << world->y
                        << " m | Z=" << std::setw(8) << world->z << " m\n";
                else
                    out << "  Vi tri World : -\n";

                out << "  Dang phuc vu : Gateway " << activePeer->id
                    << " (" << coverage << ")"
                    << " | Handover=" << (handoverExecutor.enabled() ? "RSSI_AUTO" : "MANUAL") << "\n"
                    << "  Cac vung GW  :\n";
                for (const auto& peer : peers) {
                    const auto st = peerStatus(peer, world);
                    const bool isAct = (&peer == activePeer);
                    const char* gwCov = "OFF";
                    if (coverageGate.policy().enabled && peer.zone) {
                        if (!world) gwCov = "NO_POS";
                        else gwCov = st.inRange ? "IN_RANGE" : "OUT_OF_RANGE";
                    }
                    out << (isAct ? "    * [" : "      [") << peer.id << "] "
                        << std::left << std::setw(12) << gwCov << std::right;
                    if (peer.zone && world)
                        out << " | dist=" << std::setw(6) << std::setprecision(2) << st.horizontalDistanceM << " m";
                    else
                        out << " | dist=     - m";
                    if (rssiModel.enabled() && peer.hasRssi)
                        out << " | RSSI=" << std::setw(6) << std::setprecision(1) << peer.lastRssiDbm << " dBm";
                    else
                        out << " | RSSI=     - dBm";
                    if (linkMonitor.enabled()) {
                        const auto q = linkMonitor.qualityFor(peer.id);
                        out << " | RTT=" << std::setw(5) << std::setprecision(2) << q.lastRttMs << " ms"
                            << " | loss=" << std::setw(5) << std::setprecision(1) << (q.probeLossRatio * 100.0) << "%";
                    }
                    if (isAct) out << "  <-- ACTIVE";
                    out << "\n";
                }
                out << "  Goi tin      : GW fwd=" << forwardedToGateway << " drop=" << droppedToGateway
                    << " | PX4 fwd=" << forwardedToPx4 << " drop=" << droppedToPx4 << "\n"
                    << "================================================================\n";
                std::cout << out.str() << std::flush;
            }
        }
        SetConsoleCtrlHandler(onControl, FALSE);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
