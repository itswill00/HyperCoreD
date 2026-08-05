<template>
  <Teleport to="body">
    <transition name="modal-fade" appear>
      <div class="modal-backdrop" @click.self="$emit('close')">
        <div class="modal-card">
          
          <div style="padding: 14px 16px; border-bottom: 1px solid var(--surface-container-high); display: flex; align-items: center; justify-content: space-between;">
          <div>
            <div style="font-size: 14px; font-weight: 600; color: var(--on-surface);">Select application</div>
            <div style="font-size: 11px; color: var(--on-surface-variant); margin-top: 1px;">Installed apps & custom packages</div>
          </div>
          <button class="btn-md3 btn-md3-secondary" style="padding: 4px 10px; font-size: 11px;" @click="$emit('close')">Close</button>
        </div>

        <div style="padding: 10px 14px; border-bottom: 1px solid var(--surface-container-high); background: var(--surface-container-low);">
          <input
            v-model="query"
            type="text"
            class="input-md3"
            placeholder="Search app name or package (e.g. com.game)..."
            ref="searchInput"
            @keydown.enter="addCustom"
          >
        </div>

        <div style="padding: 8px; overflow-y: auto; flex: 1; scrollbar-width: none;">
          <div v-if="loading" style="padding: 32px; text-align: center; font-size: 12px; color: var(--on-surface-variant);">
            Scanning installed packages...
          </div>

          <div v-else-if="filtered.length === 0" style="padding: 24px; text-align: center;">
            <div style="font-size: 12px; color: var(--on-surface-variant); margin-bottom: 12px;">
              No available apps matching "{{ query }}"
            </div>
            <button v-if="query.trim()" class="btn-md3 btn-md3-primary" style="font-size: 11px;" @click="addCustom">
              <Icons name="plus" :size="14" />
              <span>Add package "{{ query.trim() }}"</span>
            </button>
          </div>

          <div
            v-for="app in filtered"
            :key="app.pkg"
            class="md3-list-row clickable"
            style="border-radius: 12px; margin-bottom: 4px; border-bottom: none; padding: 10px 12px;"
            @click="$emit('pick', app.pkg)"
          >
            <div class="row-left">
              <div class="icon-frame">
                <img
                  v-if="iconFor(app.pkg) && !brokenIcons[app.pkg]"
                  :src="iconFor(app.pkg)"
                  style="width: 32px; height: 32px; border-radius: 10px; object-fit: cover;"
                  @error="brokenIcons[app.pkg] = true"
                  loading="lazy"
                >
                <div v-else class="icon-fallback">
                  <Icons name="rocket" :size="16" />
                </div>
              </div>
              <div class="row-meta">
                <div class="row-title" style="font-size: 12px;">{{ app.name }}</div>
                <div class="row-sub" style="font-family: var(--font-mono); font-size: 10px; opacity: 0.75;">{{ app.pkg }}</div>
              </div>
            </div>
            <button class="btn-md3 btn-md3-primary" style="padding: 4px 10px; font-size: 11px; flex-shrink: 0; margin-left: 8px;" @click.stop="$emit('pick', app.pkg)">
              Add
            </button>
          </div>
        </div>
      </div>
    </div>
  </transition>
</Teleport>
</template>

<script setup>
import { ref, computed, onMounted, nextTick } from 'vue'
import { useHyperStore } from '@/stores/hyper'
import { listInstalledApps, getIconUrl } from '@/helpers/shell'
import Icons from '@/components/icons/Icons.vue'

const emit = defineEmits(['close', 'pick'])

const store = useHyperStore()
const apps = ref([])
const query = ref('')
const loading = ref(true)
const brokenIcons = ref({})
const searchInput = ref(null)

const existingPkgs = computed(() => {
  if (!store.games) return []
  return store.games.map(g => g.pkg)
})

const filtered = computed(() => {
  
  const available = apps.value.filter(a => !existingPkgs.value.includes(a.pkg))

  if (!query.value.trim()) return available
  const q = query.value.toLowerCase()
  return available.filter(a =>
    a.name.toLowerCase().includes(q) || a.pkg.toLowerCase().includes(q)
  )
})

function iconFor(pkg) {
  return getIconUrl(pkg)
}

function addCustom() {
  if (query.value.trim()) {
    emit('pick', query.value.trim())
  }
}

onMounted(async () => {
  try {
    apps.value = await listInstalledApps()
  } catch {
    apps.value = []
  }
  loading.value = false
  await nextTick()
  if (searchInput.value) searchInput.value.focus()
})
</script>

<style scoped>
.icon-frame {
  width: 32px;
  height: 32px;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}

.icon-fallback {
  width: 32px;
  height: 32px;
  border-radius: 10px;
  background: var(--surface-container-highest);
  color: var(--primary);
  display: flex;
  align-items: center;
  justify-content: center;
}
</style>
