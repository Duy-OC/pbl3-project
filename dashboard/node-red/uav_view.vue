<template>
  <section class="px4-world">
    <header>
      <h2>Vị trí UAV trong Gazebo</h2>
      <span class="px4-state" :class="{ live: tracking }" role="status">{{ status }}</span>
    </header>

    <svg :viewBox="viewBox" preserveAspectRatio="xMidYMid meet"
         role="img" aria-label="Mặt phẳng world X–Y theo mét và vị trí UAV">
      <path d="M -46 0 H 107 M 0 47 V -47" class="px4-axis" />
      <g v-for="x in [-40, -20, 0, 20, 40, 60, 80, 100]" :key="'x' + x">
        <path :d="'M ' + x + ' -1 v 2'" class="px4-axis" />
        <text :x="x" y="7" text-anchor="middle">{{ x }}</text>
      </g>
      <g v-for="y in [-40, -20, 20, 40]" :key="'y' + y">
        <path :d="'M -1 ' + -y + ' h 2'" class="px4-axis" />
        <text x="-3" :y="-y + 1" text-anchor="end">{{ y }}</text>
      </g>
      <text x="105" y="-3">X (m)</text>
      <text x="3" y="-45">Y (m)</text>
      <g v-for="(x, index) in [0, 60]" :key="'gateway' + x" :class="index ? 'px4-b' : 'px4-a'">
        <circle :cx="x" cy="0" r="40" class="px4-zone" />
        <circle :cx="x" cy="0" r="1.4" class="px4-gateway" />
        <text :x="x + 3" y="-4">Gateway {{ index ? 'B' : 'A' }}</text>
      </g>
      <polyline v-if="trail.length > 1" :points="trailPoints" class="px4-trail" />
      <g v-if="lastWorld" :transform="'translate(' + lastWorld.x_m + ' ' + -lastWorld.y_m + ')'"
         :opacity="tracking ? 1 : 0.38">
        <circle r="3" class="px4-uav" />
        <path d="M -1.6 -1.6 L 1.6 1.6 M -1.6 1.6 L 1.6 -1.6 M 0 -2.2 L 0 0" stroke="white" stroke-width=".65" />
        <text x="4" y="-3">UAV {{ lastWorld.sysid }}</text>
      </g>
    </svg>

    <dl>
      <div><dt>SYSID</dt><dd>{{ latest?.sysid ?? '—' }}</dd></div>
      <div><dt>Tuổi mẫu NED</dt><dd>{{ ageLabel }}</dd></div>
      <div><dt>N / E / D (m)</dt><dd>{{ triple(latest, ['ned_n_m', 'ned_e_m', 'ned_d_m']) }}</dd></div>
      <div><dt>World X / Y / Z (m)</dt><dd>{{ triple(lastWorld, ['x_m', 'y_m', 'z_m']) }}</dd></div>
      <div><dt>Cao độ world Z (m)</dt><dd>{{ number(lastWorld?.z_m) }}</dd></div>
      <div><dt>Transform</dt><dd>{{ latest ? (latest.transform_valid ? 'Đã cấu hình' : 'Chưa cấu hình') : '—' }}</dd></div>
    </dl>
    <p v-if="lastWorld && !tracking">Đang hiển thị vị trí world cuối đã biết.</p>
  </section>
</template>

<script>
export default {
  data() {
    return {
      latest: null,
      lastWorld: null,
      trail: [],
      arrivedAt: null,
      clock: 0,
      timer: null
    }
  },
  computed: {
    timedOut() {
      const timeout = Math.max(1500, 3 * (this.latest?.metrics_interval_ms ?? 100))
      return this.arrivedAt !== null && this.clock - this.arrivedAt >= timeout
    },
    tracking() {
      return !this.timedOut && this.latest?.position_status === 'VALID' &&
        this.latest.position_valid && this.latest.transform_valid
    },
    status() {
      if (this.timedOut) return 'MẤT CẬP NHẬT UDP'
      return this.latest?.position_status ?? 'ĐANG CHỜ PX4'
    },
    ageLabel() {
      if (this.latest?.position_age_ms == null) return '—'
      return Math.floor(this.latest.position_age_ms + this.clock - this.arrivedAt) + ' ms'
    },
    trailPoints() {
      return this.trail.map(point => point.x_m + ',' + -point.y_m).join(' ')
    },
    viewBox() {
      // Include both coverage circles and recent UAV positions at the same scale.
      let left = -50, right = 122, top = -51, bottom = 51
      for (const point of this.trail) {
        left = Math.min(left, point.x_m - 8)
        right = Math.max(right, point.x_m + 17)
        top = Math.min(top, -point.y_m - 8)
        bottom = Math.max(bottom, -point.y_m + 8)
      }
      return [left, top, right - left, bottom - top].join(' ')
    }
  },
  mounted() {
    // Subscribe to live messages so a cached Dashboard payload cannot look fresh.
    this.$socket.on('msg-input:' + this.id, this.receive)
    this.timer = setInterval(() => { this.clock = performance.now() }, 200)
  },
  unmounted() {
    this.$socket.off('msg-input:' + this.id, this.receive)
    clearInterval(this.timer)
  },
  methods: {
    receive({ payload }) {
      if (payload?.source !== 'PX4_PROXY') return
      if (this.latest && (this.latest.sysid !== payload.sysid ||
          payload.sequence < this.latest.sequence)) {
        this.lastWorld = null
        this.trail = []
      }
      this.latest = payload
      this.clock = this.arrivedAt = performance.now()
      if (!this.tracking) return
      const previous = this.lastWorld
      this.lastWorld = payload
      if (!previous || previous.x_m !== payload.x_m || previous.y_m !== payload.y_m) {
        this.trail.push(payload)
        if (this.trail.length > 240) this.trail.shift()
      }
    },
    number(value) {
      return Number.isFinite(value) ? value.toFixed(2) : '—'
    },
    triple(sample, keys) {
      return keys.map(key => this.number(sample?.[key])).join(' / ')
    }
  }
}
</script>

<style>
.px4-world {
  color: #16314a;
  background: #f7fafc;
  padding: clamp(12px, 2.5vw, 24px);
  border-radius: 12px;
}
.px4-world header { display: flex; flex-wrap: wrap; gap: 8px 16px; align-items: center; justify-content: space-between; }
.px4-world h2 { font-size: 1.25rem; margin: 0; }
.px4-world .px4-state { color: #823b13; background: #fff0da; padding: 4px 9px; border-radius: 6px; font-weight: 700; }
.px4-world .px4-state.live { color: #07533d; background: #d6f5e9; }
.px4-world svg { display: block; width: 100%; height: clamp(230px, 45vw, 540px); background: #ecf3f8; margin: 14px 0; border-radius: 8px; }
.px4-world svg text { font: 3px system-ui, sans-serif; fill: #18354c; }
.px4-world .px4-axis { stroke: #648096; stroke-width: .28; fill: none; }
.px4-world .px4-a { color: #116ab0; }
.px4-world .px4-b { color: #a95510; }
.px4-world .px4-zone { fill: currentColor; fill-opacity: .07; stroke: currentColor; stroke-width: .45; stroke-dasharray: 1.4 1; }
.px4-world .px4-gateway { fill: currentColor; }
.px4-world .px4-trail { fill: none; stroke: #098779; stroke-width: .55; }
.px4-world .px4-uav { fill: #08796e; stroke: white; stroke-width: .35; }
.px4-world dl { display: grid; gap: 12px; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); margin: 0; }
.px4-world dt { color: #436179; font-size: .85rem; }
.px4-world dd { font-variant-numeric: tabular-nums; font-weight: 650; margin: 2px 0 0; }
.px4-world p { margin: 12px 0 0; color: #823b13; }
</style>
