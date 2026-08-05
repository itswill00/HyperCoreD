import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { execCommand, sanitize, isKSU } from '@/helpers/shell'

const MOD = '/data/adb/modules/hypercore'
const LOG = '/sdcard/Android/hypercore.log'
const GL_MOD = `${MOD}/gamelist.txt`
const GL_SD = '/sdcard/Android/gamelist.txt'

export const useHyperStore = defineStore('hyper', () => {
  
  const daemonPid = ref('')
  const activeProfile = ref('—')
  const thermalTier = ref('—')

  const cpuLittle = ref('—')
  const cpuBig = ref('—')
  const cpuGov = ref('—')
  const cpuCores = ref('—')
  const gpuInfo = ref('—')
  const sysLoad = ref('—')

  const ramUsage = ref('—')
  const ramPercent = ref(0)
  const zramUsage = ref('—')
  const zramPercent = ref(0)
  const ioInfo = ref('—')
  const vmInfo = ref('—')

  const cpuTemp = ref(0)
  const batTemp = ref(0)
  const gpuTemp = ref(0)
  const chgTemp = ref(0)
  const batStatus = ref('—')
  const batLevel = ref('')
  const batRate = ref('—')
  const batVolt = ref('—')
  const batteryCycles = ref(0)
  const batHealth = ref('Good')
  const batCapFull = ref('5350 mAh')
  const batTech = ref('Li-poly')

  const thermalGuardState = computed(() => {
    if (batTemp.value >= 42) return 'Trickle Guard (1.0A)'
    if (batTemp.value >= 37) return 'Safety Balance (1.8A)'
    return 'Fast Charge (2.5A)'
  })

  const moduleVersion = ref('v4.3')
  const kernelVersion = ref('—')
  const chipset = ref('MediaTek Helio G99 Ultra (MT6789)')
  const androidSdk = ref('—')
  const selinux = ref('—')
  const uptime = ref('—')
  const uptimeSec = ref(0)
  let uptimeInterval = null

  const games = ref([])
  const logs = ref('Loading...')
  const loading = ref(false)
  const isLogsActive = ref(false)
  let isRefreshing = false

  function setLogsActive(active) {
    isLogsActive.value = active
  }

  const isRunning = computed(() => daemonPid.value.length > 0)

  const thermalColor = computed(() => {
    if (thermalTier.value.includes('3')) return 'c-red'
    if (thermalTier.value.includes('2') || thermalTier.value.includes('1')) return 'c-amber'
    return 'c-green'
  })

  const tempColor = computed(() => {
    if (cpuTemp.value >= 75 || batTemp.value >= 53) return 'c-red'
    if (cpuTemp.value >= 58 || batTemp.value >= 44) return 'c-amber'
    return ''
  })

  const batColor = computed(() => {
    if (batStatus.value === 'Charging') return 'c-green'
    if (parseInt(batLevel.value) <= 15) return 'c-red'
    return ''
  })

  function normTemp(raw) {
    const v = parseInt(raw || 0)
    if (v > 1000) return Math.round(v / 1000)
    if (v > 100) return Math.round(v / 10)
    return v
  }

  function fmtFreq(raw) {
    return Math.round(parseInt(raw || 0) / 1000)
  }

  function fmtUptime(totalSec) {
    const s = Math.floor(totalSec || 0)
    if (!s) return '—'
    const d = Math.floor(s / 86400)
    const h = Math.floor((s % 86400) / 3600)
    const m = Math.floor((s % 3600) / 60)
    const sec = Math.floor(s % 60)

    if (d > 0) return `${d}d ${h}h ${m}m ${sec}s`
    if (h > 0) return `${h}h ${m}m ${sec}s`
    return `${m}m ${sec}s`
  }

  function startUptimeTicker() {
    if (uptimeInterval) return
    uptimeInterval = setInterval(() => {
      if (uptimeSec.value > 0) {
        uptimeSec.value++
        uptime.value = fmtUptime(uptimeSec.value)
      }
    }, 1000)
  }

  function stopUptimeTicker() {
    if (uptimeInterval) {
      clearInterval(uptimeInterval)
      uptimeInterval = null
    }
  }

  async function fetchDeviceInfo() {
    try {
      const [kv, sdk, se] = await Promise.all([
        execCommand('uname -r -m 2>/dev/null'),
        execCommand('getprop ro.build.version.sdk 2>/dev/null'),
        execCommand('getenforce 2>/dev/null')
      ])
      if (kv) kernelVersion.value = kv.trim()
      if (sdk) androidSdk.value = `API ${sdk.trim()}`
      if (se) selinux.value = se.trim()
    } catch {}
  }

  async function refresh() {
    if (isRefreshing) return
    isRefreshing = true

    const cmdList = [
      `MOD="/data/adb/modules/hypercore"`,
      `if [ ! -d "$MOD" ]; then MOD="/data/adb/modules/tanzanite_hypercore"; fi`,
      `echo "IPC:$(echo GET_STATUS | nc -U $MOD/hypercore.sock 2>/dev/null || true)"`,
      `PID=$(pidof libhypercore.so || pidof hypercore || pgrep -f libhypercore.so || pgrep -f hypercore 2>/dev/null); echo "PID:\${PID}"`,
      `echo "CL0:$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq 2>/dev/null):$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq 2>/dev/null):$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 2>/dev/null)"`,
      `echo "CL1:$(cat /sys/devices/system/cpu/cpu6/cpufreq/scaling_cur_freq 2>/dev/null):$(cat /sys/devices/system/cpu/cpu6/cpufreq/scaling_min_freq 2>/dev/null):$(cat /sys/devices/system/cpu/cpu6/cpufreq/scaling_max_freq 2>/dev/null)"`,
      `echo "GOV:$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null)"`,
      `echo "CORES:$(cat /sys/devices/system/cpu/online 2>/dev/null)"`,
      `echo "GPU:$(cat /sys/module/ged/parameters/gpu_loading 2>/dev/null):$(cat /sys/module/ged/parameters/gpu_bottom_freq 2>/dev/null)"`,
      `echo "LOAD:$(cat /proc/loadavg 2>/dev/null | cut -d' ' -f1)"`,
      `echo "MEM:$(cat /proc/meminfo 2>/dev/null | grep -E '^(MemTotal|MemAvailable|SwapTotal|SwapFree):' | tr '\n' ' ')"`,
      `echo "CT:$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null)"`,
      `echo "BT:$(cat /sys/class/power_supply/battery/temp 2>/dev/null)"`,
      `echo "BS:$(cat /sys/class/power_supply/battery/status 2>/dev/null)"`,
      `echo "BL:$(cat /sys/class/power_supply/battery/capacity 2>/dev/null)"`,
      `echo "BC:$(cat /sys/class/power_supply/battery/current_now 2>/dev/null)"`,
      `echo "BV:$(cat /sys/class/power_supply/battery/voltage_now 2>/dev/null)"`,
      `echo "BH:$(cat /sys/class/power_supply/battery/health 2>/dev/null || cat /sys/class/power_supply/bms/health 2>/dev/null)"`,
      `echo "BFC:$(cat /sys/class/power_supply/battery/charge_full 2>/dev/null || cat /sys/class/power_supply/bms/charge_full 2>/dev/null)"`,
      `echo "BTEC:$(cat /sys/class/power_supply/battery/technology 2>/dev/null || cat /sys/class/power_supply/bms/technology 2>/dev/null)"`,
      `BLK=$(ls /sys/block/ 2>/dev/null | grep -E '^(sd|mmcblk|ufs|dm-)' | head -1)`,
      `echo "IO:$(cat /sys/block/$BLK/queue/scheduler 2>/dev/null | grep -oP '\\[\\K[^]]+'):$(cat /sys/block/$BLK/queue/read_ahead_kb 2>/dev/null)"`,
      `echo "SW:$(cat /proc/sys/vm/swappiness 2>/dev/null):$(cat /proc/sys/vm/vfs_cache_pressure 2>/dev/null)"`,
      `echo "UP:$(cat /proc/uptime 2>/dev/null | cut -d' ' -f1)"`,
      `echo "KV:$(uname -r 2>/dev/null)"`,
      `DESC=$(grep '^description=' $MOD/module.prop 2>/dev/null); echo "DESC:$DESC"`,
      `VER=$(grep '^version=' $MOD/module.prop 2>/dev/null | cut -d'=' -f2); echo "VER:$VER"`,
      `echo "===GL==="`,
      `cat $MOD/gamelist.txt 2>/dev/null || cat /sdcard/Android/gamelist.txt 2>/dev/null || true`
    ]

    if (isLogsActive.value) {
      cmdList.push(`echo "===LOG==="`, `tail -n 35 ${LOG} 2>/dev/null || true`)
    }

    const cmd = cmdList.join('\n')

    try {
      const res = await execCommand(cmd)
      if (!res) return

      const sections = res.split('===GL===')
      const kvBlock = sections[0] || ''
      const rest = (sections[1] || '').split('===LOG===')
      const gameBlock = rest[0] || ''
      const logBlock = rest[1] || ''

      const kv = {}
      kvBlock.trim().split('\n').forEach(line => {
        const i = line.indexOf(':')
        if (i > 0) kv[line.substring(0, i)] = line.substring(i + 1)
      })

      if (kv.VER) moduleVersion.value = kv.VER.trim()
      if (kv.KV && kv.KV.trim().length > 0) kernelVersion.value = kv.KV.trim()

      let ipcSetThermal = false
      if (kv.IPC && kv.IPC.includes('"status":"ok"')) {
        try {
          const ipcData = JSON.parse(kv.IPC)
          daemonPid.value = String(ipcData.pid || '')
          activeProfile.value = ipcData.profile || '—'
          thermalTier.value = `Tier ${ipcData.thermal_tier}`
          if (ipcData.battery_cycles > 0) batteryCycles.value = ipcData.battery_cycles
          if (ipcData.gpu_temp > 0) gpuTemp.value = ipcData.gpu_temp
          if (ipcData.chg_temp > 0) chgTemp.value = ipcData.chg_temp
          ipcSetThermal = true
        } catch {}
      } else {
        
        daemonPid.value = (kv.PID || '').trim()

        const desc = kv.DESC || ''
        const m = desc.match(/\[Active:\s*([^\]]+)\]/)
        if (m) activeProfile.value = m[1].trim()
      }

      if (kv.CL0) {
        const p = kv.CL0.split(':')
        cpuLittle.value = `${fmtFreq(p[0])} MHz [${fmtFreq(p[1])}–${fmtFreq(p[2])}]`
      }
      if (kv.CL1) {
        const p = kv.CL1.split(':')
        cpuBig.value = `${fmtFreq(p[0])} MHz [${fmtFreq(p[1])}–${fmtFreq(p[2])}]`
      }
      cpuGov.value = (kv.GOV || 'schedutil').trim()
      cpuCores.value = (kv.CORES || '0-7').trim()

      if (kv.GPU) {
        const p = kv.GPU.split(':')
        gpuInfo.value = `${p[0] || '0'}% / ${fmtFreq(p[1])} MHz floor`
      }

      sysLoad.value = kv.LOAD || '0.00'
      if (kv.UP) {
        const parsedSec = Math.floor(parseFloat(kv.UP.trim()))
        if (parsedSec > 0) {
          uptimeSec.value = parsedSec
          uptime.value = fmtUptime(parsedSec)
          startUptimeTicker()
        }
      }

      if (kv.MEM) {
        const memMatch = kv.MEM.match(/MemTotal:\s*(\d+)\s*kB.*MemAvailable:\s*(\d+)\s*kB/)
        const swapMatch = kv.MEM.match(/SwapTotal:\s*(\d+)\s*kB.*SwapFree:\s*(\d+)\s*kB/)

        if (memMatch) {
          const tot = Math.round(parseInt(memMatch[1]) / 1024)
          const avail = Math.round(parseInt(memMatch[2]) / 1024)
          const used = tot - avail
          ramPercent.value = Math.round((used / tot) * 100)
          ramUsage.value = `${(used / 1024).toFixed(1)} GB / ${(tot / 1024).toFixed(1)} GB (${ramPercent.value}%)`
        }

        if (swapMatch) {
          const tot = Math.round(parseInt(swapMatch[1]) / 1024)
          const free = Math.round(parseInt(swapMatch[2]) / 1024)
          const used = tot - free
          zramPercent.value = tot > 0 ? Math.round((used / tot) * 100) : 0
          zramUsage.value = `${(used / 1024).toFixed(1)} GB / ${(tot / 1024).toFixed(1)} GB (${zramPercent.value}%)`
        }
      }

      cpuTemp.value = normTemp(kv.CT)
      batTemp.value = normTemp(kv.BT)

      batStatus.value = (kv.BS || 'Unknown').trim()
      batLevel.value = (kv.BL || '').trim()

      const rawCurr = Math.abs(parseInt(kv.BC || 0))
      const rawVolt = parseInt(kv.BV || 0)
      if (rawCurr > 0) {
        const ma = Math.round(rawCurr > 10000 ? rawCurr / 1000 : rawCurr)
        const sign = batStatus.value === 'Charging' ? '+' : '-'
        batRate.value = `${sign}${ma} mA`
      } else {
        batRate.value = '—'
      }

      if (rawVolt > 0) {
        const v = (rawVolt > 10000 ? rawVolt / 1000000 : rawVolt / 1000).toFixed(2)
        batVolt.value = `${v} V`
      } else {
        batVolt.value = '—'
      }

      if (kv.BH) batHealth.value = kv.BH.trim() || 'Good'
      if (kv.BTEC) batTech.value = kv.BTEC.trim() || 'Li-poly'
      if (kv.BFC) {
        const rawFc = parseInt(kv.BFC || 0)
        if (rawFc > 0) {
          const mah = Math.round(rawFc > 10000 ? rawFc / 1000 : rawFc)
          batCapFull.value = `${mah} mAh`
        }
      }

      if (kv.IO) {
        const p = kv.IO.split(':')
        ioInfo.value = `${p[0] || 'none'} / ${p[1] || '256'} KB`
      }
      if (kv.SW) {
        const p = kv.SW.split(':')
        vmInfo.value = `${p[0] || '15'} / ${p[1] || '100'}`
      }

      if (!ipcSetThermal) {
        const logLines = logBlock.trim().split('\n')
        let tier = '—'
        for (let i = logLines.length - 1; i >= 0; i--) {
          if (logLines[i].indexOf('Thermal Tier:') !== -1) {
            if (logLines[i].indexOf('T3') !== -1) tier = 'Tier 3 (Protection)'
            else if (logLines[i].indexOf('T2') !== -1) tier = 'Tier 2 (Warm)'
            else if (logLines[i].indexOf('T1') !== -1) tier = 'Tier 1 (Normal)'
            else tier = 'Tier 0 (Optimal)'
            break
          }
        }
        if (tier === '—' && cpuTemp.value > 0) {
          if (cpuTemp.value >= 75) tier = 'Tier 3 (Protection)'
          else if (cpuTemp.value >= 65) tier = 'Tier 2 (Warm)'
          else if (cpuTemp.value >= 58) tier = 'Tier 1 (Normal)'
          else tier = 'Tier 0 (Optimal)'
        }
        thermalTier.value = tier
      }

      const parsedPkgs = []
      gameBlock.trim().split('\n').forEach(line => {
        const trimmed = line.trim().replace(/\r/g, '')
        if (trimmed.length > 0 && trimmed[0] !== '#') {
          const parts = trimmed.split(':')
          const pkg = parts[0].trim()
          const profile = parts[1] ? parts[1].trim() : 'GAMING'
          if (pkg && !parsedPkgs.some(p => p.pkg === pkg)) {
            parsedPkgs.push({ pkg, profile })
          }
        }
      })
      games.value = parsedPkgs

      const trimmed = logBlock.trim()
      if (trimmed) logs.value = trimmed

    } catch (err) {
      console.error('refresh:', err)
    } finally {
      isRefreshing = false
    }
  }

  async function flushRam() {
    loading.value = true
    try {
      await execCommand('sync; echo 3 > /proc/sys/vm/drop_caches; (echo 1 > /proc/sys/vm/compact_memory &)')
      await new Promise(resolve => setTimeout(resolve, 600))
      await refresh()
      return `RAM cleared (${ramUsage.value})`
    } catch (e) {
      return 'Failed to clear RAM'
    } finally {
      loading.value = false
    }
  }

  async function restartDaemon() {
    loading.value = true
    
    await execCommand(`pkill -9 -x libhypercore.so 2>/dev/null; pkill -9 -x hypercore 2>/dev/null; sleep 1; nohup ${MOD}/system/bin/libhypercore.so >/dev/null 2>&1 &`)
    loading.value = false
    setTimeout(refresh, 1500)
    return 'Daemon restarted'
  }

  async function exportLogs() {
    loading.value = true
    await execCommand(`cp ${LOG} /sdcard/HyperCore_log.txt 2>/dev/null`)
    loading.value = false
    return 'Log saved to /sdcard/HyperCore_log.txt'
  }

  async function clearLogs() {
    loading.value = true
    await execCommand(`> ${LOG} 2>/dev/null || true; > /sdcard/Android/hypercore.log 2>/dev/null || true`)
    logs.value = ''
    loading.value = false
    return 'Log cleared'
  }

  function createShortcut() {
    if (isKSU() && typeof ksu.createShortcut === 'function') {
      ksu.createShortcut()
      return 'Shortcut requested'
    }
    return 'Shortcut feature unavailable'
  }

  function addGame(rawPkg, profile = 'GAMING') {
    const pkg = sanitize(rawPkg).split(':')[0].trim()
    if (!pkg) return

    if (!games.value.some(g => g.pkg === pkg)) {
      games.value.push({ pkg, profile })
    }

    import('@/helpers/shell').then(m => m.listInstalledApps(true)).catch(() => {})

    const cmd = [
      `for f in ${GL_MOD} ${GL_SD}; do`,
      `  touch "$f" 2>/dev/null`,
      `  if [ -f "$f" ]; then`,
      `    grep -vE '^${pkg}(:|$)' "$f" > "$f.tmp" 2>/dev/null`,
      `    echo '${pkg}:${profile}' >> "$f.tmp"`,
      `    mv "$f.tmp" "$f" 2>/dev/null`,
      `  fi`,
      `done`
    ].join('\n')
    execCommand(cmd)
    setTimeout(refresh, 400)
    return `Added ${pkg} (${profile})`
  }

  function removeGame(rawPkg) {
    const pkg = sanitize(rawPkg).split(':')[0].trim()
    if (!pkg) return

    games.value = games.value.filter(g => g.pkg !== pkg)

    const cmd = [
      `for f in ${GL_MOD} ${GL_SD}; do`,
      `  if [ -f "$f" ]; then`,
      `    grep -vE '^${pkg}(:|$)' "$f" > "$f.tmp" 2>/dev/null`,
      `    mv "$f.tmp" "$f" 2>/dev/null`,
      `  fi`,
      `done`
    ].join('\n')
    execCommand(cmd)
    setTimeout(refresh, 400)
    return `Removed ${pkg}`
  }

  function updateGameProfile(pkg, profile) {
    const existing = games.value.find(g => g.pkg === pkg)
    if (existing) {
      existing.profile = profile
    }

    const cmd = [
      `for f in ${GL_MOD} ${GL_SD}; do`,
      `  if [ -f "$f" ]; then`,
      `    grep -vE '^${pkg}(:|$)' "$f" > "$f.tmp" 2>/dev/null`,
      `    echo '${pkg}:${profile}' >> "$f.tmp"`,
      `    mv "$f.tmp" "$f" 2>/dev/null`,
      `  fi`,
      `done`
    ].join('\n')
    execCommand(cmd)
    setTimeout(refresh, 400)
    return `Updated ${pkg} to ${profile}`
  }

  function launchGame(rawPkg) {
    const clean = sanitize(rawPkg).split(':')[0].trim()
    if (!clean) return
    execCommand(`monkey -p ${clean} -c android.intent.category.LAUNCHER 1 >/dev/null 2>&1`)
    return `Launched ${clean}`
  }

  return {
    daemonPid, activeProfile, thermalTier, cpuLittle, cpuBig, cpuGov, cpuCores, gpuInfo,
    sysLoad, ramUsage, ramPercent, zramUsage, zramPercent, ioInfo, vmInfo,
    cpuTemp, batTemp, gpuTemp, chgTemp, batStatus, batLevel, batRate, batVolt, batteryCycles,
    batHealth, batCapFull, batTech, thermalGuardState,
    games, logs, loading,
    moduleVersion, kernelVersion, chipset, androidSdk, selinux, uptime,
    isRunning, thermalColor, tempColor, batColor,
    fetchDeviceInfo, refresh, flushRam, restartDaemon, exportLogs, clearLogs, createShortcut,
    addGame, removeGame, updateGameProfile, launchGame, setLogsActive, stopUptimeTicker
  }
})
