param([string]$ProxyExecutable = 'bin/udp_proxy.exe')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$gateway = New-Object System.Net.Sockets.UdpClient(0)
$px4 = New-Object System.Net.Sockets.UdpClient(0)
$other = New-Object System.Net.Sockets.UdpClient(0)
$reservation = New-Object System.Net.Sockets.UdpClient(0)
$listenPort = $reservation.Client.LocalEndPoint.Port
$reservation.Close()
$gateway.Client.ReceiveTimeout = 2000
$px4.Client.ReceiveTimeout = 2000
$process = $null
try {
    $gatewayPort = $gateway.Client.LocalEndPoint.Port
    $process = Start-Process -FilePath (Join-Path $root $ProxyExecutable) -ArgumentList @($listenPort, '127.0.0.1', $gatewayPort, '127.0.0.1') -WindowStyle Hidden -PassThru
    Start-Sleep -Milliseconds 400
    if ($process.HasExited) { throw 'Proxy exited during startup' }
    $target = New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Loopback, $listenPort)
    $source = New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any, 0)
    foreach ($length in @(256, 60000, 0)) {
        $payload = New-Object byte[] $length
        for ($i = 0; $i -lt $length; $i++) { $payload[$i] = $i % 256 }
        [void]$px4.Send($payload, $payload.Length, $target)
        $received = $gateway.Receive([ref]$source)
        if ([Convert]::ToBase64String($received) -cne [Convert]::ToBase64String($payload)) { throw 'Uplink mismatch' }
        [byte[]]$reply = @(253, 0, 128, 255, 42)
        [void]$gateway.Send($reply, $reply.Length, $source)
        $returnSource = New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any, 0)
        $received = $px4.Receive([ref]$returnSource)
        if ([Convert]::ToBase64String($received) -cne [Convert]::ToBase64String($reply)) { throw 'Downlink mismatch' }
        if ($returnSource.Port -ne $listenPort) { throw 'Reply used incorrect source port' }
    }
    [byte[]]$foreign = @(1, 2, 3)
    [void]$other.Send($foreign, $foreign.Length, $target)
    $gateway.Client.ReceiveTimeout = 300
    $timedOut = $false
    try { [void]$gateway.Receive([ref]$source) }
    catch [System.Net.Sockets.SocketException] {
        if ($_.Exception.SocketErrorCode -ne [System.Net.Sockets.SocketError]::TimedOut) { throw }
        $timedOut = $true
    }
    if (!$timedOut) { throw 'Unrecognized PX4 sender was forwarded' }
    Write-Output 'PASS: binary/large/empty datagrams, bidirectional forwarding, reply source port, foreign PX4 rejection'
} finally {
    if ($process -and !$process.HasExited) { Stop-Process -Id $process.Id }
    $gateway.Close()
    $px4.Close()
    $other.Close()
}
