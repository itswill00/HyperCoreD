<template>
  <div>
    
    <div style="display: flex; align-items: center; gap: 8px; margin-bottom: 12px;">
      <input
        v-model="search"
        type="text"
        class="input-md3"
        placeholder="Search configured games..."
        style="flex: 1; border-radius: 20px; padding: 6px 14px; font-size: 11px;"
      >
      <span class="badge-pill purple" style="flex-shrink: 0; padding: 4px 10px;">
        {{ displayGames.length }} {{ displayGames.length === 1 ? 'app' : 'apps' }}
      </span>
    </div>

    <div class="md3-list-group">
      
      <div v-if="displayGames.length === 0 && !loading" style="padding: 32px 16px; text-align: center;">
        <div class="icon-badge secondary" style="width: 44px; height: 44px; margin: 0 auto 10px auto;">
          <Icons name="rocket" :size="20" />
        </div>
        <div style="font-size: 13px; font-weight: 600; color: var(--on-surface);">
          {{ search ? 'No matching games found' : 'No games configured' }}
        </div>
        <div style="font-size: 11px; color: var(--on-surface-variant); margin-top: 2px; max-width: 260px; margin-left: auto; margin-right: auto;">
          Add installed games to automatically trigger high performance profiles when playing
        </div>
        <button class="btn-md3 btn-md3-primary" style="margin-top: 14px; font-size: 11px;" @click="$emit('open-picker')">
          <Icons name="plus" :size="14" />
          <span>Add game</span>
        </button>
      </div>

      <div v-else-if="loading" style="padding: 24px 16px; text-align: center; font-size: 12px; color: var(--on-surface-variant);">
        Scanning configured games...
      </div>

      <TransitionGroup name="list-fade">
        <div
          v-for="item in displayGames"
          :key="item.pkg"
          class="md3-list-row game-list-item"
          style="padding: 10px 14px;"
        >
          <div class="row-left" style="gap: 10px; min-width: 0;">
            <div class="icon-frame">
              <img
                v-if="iconFor(item.pkg) && !brokenIcons[item.pkg]"
                :src="iconFor(item.pkg)"
                style="width: 36px; height: 36px; border-radius: 10px; object-fit: cover;"
                @error="brokenIcons[item.pkg] = true"
                loading="lazy"
              >
              <div v-else class="icon-fallback">
                <Icons name="rocket" :size="18" />
              </div>
            </div>
            <div class="row-meta" style="min-width: 0;">
              <div
                class="row-title"
                style="font-size: 13px; font-weight: 600; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;"
                :title="item.name"
              >
                {{ item.name }}
              </div>
              <div
                class="row-sub"
                style="font-family: var(--font-mono); font-size: 10px; opacity: 0.75; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;"
                :title="item.pkg"
              >
                {{ item.pkg }}
              </div>
            </div>
          </div>

          <div style="display: flex; align-items: center; gap: 8px; flex-shrink: 0; margin-left: 6px;">
            
            <select
              v-model="item.profile"
              @change="changeProfile(item.pkg, item.profile)"
              class="select-md3"
            >
              <option value="GAMING">Gaming</option>
              <option value="INTERACTIVE">Balanced</option>
              <option value="SLEEP">Saver</option>
            </select>

            <button
              class="btn-md3 btn-md3-primary"
              style="padding: 5px 10px; font-size: 11px; height: 30px;"
              @click="launch(item.pkg)"
            >
              <Icons name="rocket" :size="13" />
              <span>Launch</span>
            </button>
            <button
              class="btn-md3 btn-md3-danger btn-icon-only"
              style="width: 30px; height: 30px;"
              title="Remove game"
              @click="remove(item.pkg)"
            >
              <Icons name="trash" :size="13" />
            </button>
          </div>
        </div>
      </TransitionGroup>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, inject } from 'vue'
import { useHyperStore } from '@/stores/hyper'
import { listInstalledApps, getIconUrl } from '@/helpers/shell'
import Icons from '@/components/icons/Icons.vue'

defineEmits(['open-picker'])

const store = useHyperStore()
const toast = inject('toast')
const search = ref('')
const installedMap = ref({})
const brokenIcons = ref({})
const loading = ref(true)

async function loadInstalled() {
  loading.value = true
  try {
    const list = await listInstalledApps()
    const map = {}
    list.forEach(a => {
      map[a.pkg] = a.name
    })

    if (store.games && store.games.length > 0 && typeof ksu !== 'undefined' && typeof ksu.getPackagesInfo === 'function') {
      try {
        const pkgs = store.games.map(g => g.pkg)
        const infos = JSON.parse(ksu.getPackagesInfo(JSON.stringify(pkgs)))
        if (Array.isArray(infos)) {
          infos.forEach(info => {
            if (info && info.packageName && info.appLabel) {
              map[info.packageName] = info.appLabel
            }
          })
        }
      } catch {}
    }

    installedMap.value = map
  } catch {}
  loading.value = false
}

onMounted(() => {
  loadInstalled()
})

const displayGames = computed(() => {
  if (!store.games) return []

  const parsed = store.games.map(item => {
    const appName = installedMap.value[item.pkg]

    return {
      pkg: item.pkg,
      profile: item.profile,
      name: appName || item.pkg
    }
  })

  if (search.value.trim()) {
    const q = search.value.toLowerCase()
    return parsed.filter(item =>
      item.name.toLowerCase().includes(q) || item.pkg.toLowerCase().includes(q)
    )
  }

  return parsed
})

function iconFor(pkg) {
  return getIconUrl(pkg)
}

async function remove(pkg) {
  const msg = store.removeGame(pkg)
  if (msg && toast) toast(msg)
}

async function launch(pkg) {
  const msg = store.launchGame(pkg)
  if (msg && toast) toast(msg)
}

async function changeProfile(pkg, profile) {
  const msg = store.updateGameProfile(pkg, profile)
  if (msg && toast) toast(msg)
}
</script>

<style scoped>
.icon-frame {
  width: 36px;
  height: 36px;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}

.icon-fallback {
  width: 36px;
  height: 36px;
  border-radius: 10px;
  background: var(--surface-container-high);
  color: var(--primary);
  display: flex;
  align-items: center;
  justify-content: center;
}

.select-md3 {
  background: var(--surface-container-high);
  color: var(--on-surface);
  border: 1px solid var(--surface-container-highest);
  border-radius: 8px;
  padding: 5px 8px;
  font-size: 11px;
  outline: none;
  cursor: pointer;
  font-family: inherit;
  transition: all 0.2s ease;
}

.select-md3:focus {
  border-color: var(--primary);
}
</style>
