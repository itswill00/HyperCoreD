<template>
  <div
    class="app-shell"
    @touchstart="onTouchStart"
    @touchend="onTouchEnd"
  >
    
    <main style="flex: 1; overflow: hidden; position: relative;">
      <router-view v-slot="{ Component }">
        <transition :name="transitionName" mode="out-in">
          <component :is="Component" />
        </transition>
      </router-view>
    </main>

    <Navigation />

    <transition name="toast-slide">
      <div v-if="toastMsg" class="toast-pill">
        <Icons name="check" :size="14" style="color: var(--primary);" />
        <span>{{ toastMsg }}</span>
      </div>
    </transition>
  </div>
</template>

<script setup>
import { ref, provide, watch, onMounted, onUnmounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { useHyperStore } from '@/stores/hyper'
import Navigation from '@/components/Navigation.vue'
import Icons from '@/components/icons/Icons.vue'

const store = useHyperStore()
const route = useRoute()
const router = useRouter()

const toastMsg = ref('')
const transitionName = ref('slide-left')
let toastTimer = null
let refreshInterval = null

const routesOrder = ['/', '/games', '/logs']

let touchStartX = 0
let touchStartY = 0

function onTouchStart(e) {
  if (e.touches && e.touches.length === 1) {
    touchStartX = e.touches[0].clientX
    touchStartY = e.touches[0].clientY
  }
}

function onTouchEnd(e) {
  if (!e.changedTouches || e.changedTouches.length !== 1) return

  const touchEndX = e.changedTouches[0].clientX
  const touchEndY = e.changedTouches[0].clientY
  const deltaX = touchEndX - touchStartX
  const deltaY = touchEndY - touchStartY

  if (Math.abs(deltaX) > Math.abs(deltaY) * 1.4 && Math.abs(deltaX) > 55) {
    const currentIdx = routesOrder.indexOf(route.path)
    if (currentIdx === -1) return

    if (deltaX < 0 && currentIdx < routesOrder.length - 1) {
      
      transitionName.value = 'slide-left'
      router.push(routesOrder[currentIdx + 1])
    } else if (deltaX > 0 && currentIdx > 0) {
      
      transitionName.value = 'slide-right'
      router.push(routesOrder[currentIdx - 1])
    }
  }
}

watch(
  () => route.path,
  (to, from) => {
    const toIdx = routesOrder.indexOf(to)
    const fromIdx = routesOrder.indexOf(from)
    if (toIdx !== -1 && fromIdx !== -1) {
      transitionName.value = toIdx > fromIdx ? 'slide-left' : 'slide-right'
    }
  }
)

function toast(msg) {
  toastMsg.value = msg
  if (toastTimer) clearTimeout(toastTimer)
  toastTimer = setTimeout(() => { toastMsg.value = '' }, 2500)
}

provide('toast', toast)

onMounted(() => {
  store.refresh()
  refreshInterval = setInterval(() => store.refresh(), 3000)
})

onUnmounted(() => {
  if (refreshInterval) clearInterval(refreshInterval)
  store.stopUptimeTicker()
})
</script>

<style scoped>
.toast-pill {
  position: fixed;
  bottom: 76px;
  left: 50%;
  transform: translateX(-50%);
  background: var(--surface-container-highest);
  border: 1px solid var(--surface-bright);
  color: var(--on-surface);
  font-size: 12px;
  font-weight: 500;
  padding: 8px 16px;
  border-radius: 20px;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.4);
  z-index: 99;
  pointer-events: none;
  display: flex;
  align-items: center;
  gap: 8px;
}

.toast-slide-enter-active,
.toast-slide-leave-active {
  transition: all 0.2s cubic-bezier(0.2, 0, 0, 1);
}

.toast-slide-enter-from {
  opacity: 0;
  transform: translate(-50%, 10px);
}

.toast-slide-leave-to {
  opacity: 0;
  transform: translate(-50%, -6px);
}
</style>

<style>
.slide-left-enter-active,
.slide-left-leave-active,
.slide-right-enter-active,
.slide-right-leave-active {
  transition: all 0.22s cubic-bezier(0.2, 0, 0, 1);
  position: absolute;
  width: 100%;
  height: 100%;
}

.slide-left-enter-from {
  opacity: 0;
  transform: translateX(30px);
}
.slide-left-leave-to {
  opacity: 0;
  transform: translateX(-30px);
}

.slide-right-enter-from {
  opacity: 0;
  transform: translateX(-30px);
}
.slide-right-leave-to {
  opacity: 0;
  transform: translateX(30px);
}
</style>
