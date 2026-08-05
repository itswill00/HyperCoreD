
let cbSeq = 0
let cachedInstalledApps = null

export function execCommand(cmd) {
  return new Promise((resolve, reject) => {
    if (typeof ksu !== 'undefined' && typeof ksu.exec === 'function') {
      const id = `_hc_${++cbSeq}_${Date.now()}`
      
      const timer = setTimeout(() => {
        if (window[id]) {
          delete window[id]
          resolve('')
        }
      }, 6000)

      window[id] = (errno, stdout, stderr) => {
        clearTimeout(timer)
        delete window[id]
        resolve(stdout || stderr || '')
      }

      try {
        ksu.exec(cmd, '{}', id)
      } catch (e) {
        clearTimeout(timer)
        delete window[id]
        reject(e)
      }
    } else if (typeof exec === 'function') {
      exec(cmd)
        .then(r => resolve(typeof r === 'object' ? (r.stdout || r.stderr || '') : String(r)))
        .catch(reject)
    } else {
      reject(new Error('No root manager bridge available'))
    }
  })
}

export function sanitize(s) {
  return String(s).replace(/[^a-zA-Z0-9._:\-]/g, '').trim()
}

export function isKSU() {
  return typeof ksu !== 'undefined'
}

export async function listInstalledApps(forceRefresh = false) {
  if (cachedInstalledApps && !forceRefresh) {
    return cachedInstalledApps
  }

  if (isKSU() && typeof ksu.listUserPackages === 'function') {
    try {
      const raw = ksu.listUserPackages()
      const pkgs = JSON.parse(raw)
      let infos = []
      if (typeof ksu.getPackagesInfo === 'function') {
        try { infos = JSON.parse(ksu.getPackagesInfo(raw)) } catch {}
      }
      const infoMap = {}
      if (Array.isArray(infos)) {
        infos.forEach(info => {
          if (info && info.packageName) {
            infoMap[info.packageName] = info.appLabel || info.packageName
          }
        })
      }
      cachedInstalledApps = pkgs.map(p => ({
        pkg: p,
        name: infoMap[p] || p
      })).sort((a, b) => a.name.localeCompare(b.name))

      return cachedInstalledApps
    } catch {}
  }

  try {
    const out = await execCommand('pm list packages -3 2>/dev/null')
    cachedInstalledApps = out.trim().split('\n')
      .map(l => { const p = l.replace('package:', '').trim(); return { pkg: p, name: p } })
      .filter(i => i.pkg.length > 0)
      .sort((a, b) => a.pkg.localeCompare(b.pkg))

    return cachedInstalledApps
  } catch {
    return []
  }
}

export function getIconUrl(pkg) {
  if (isKSU()) return `ksu://icon/${pkg}`
  return ''
}
