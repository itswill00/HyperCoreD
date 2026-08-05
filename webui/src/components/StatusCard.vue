<template>
  <div class="status-banner">
    <div class="status-banner-left">
      <div class="status-title">
        HyperCore
        <span class="developer-sub">by aerichandesu@noticesa</span>
      </div>
      <div class="status-sub">
        {{ store.isRunning ? `Daemon active • PID ${store.daemonPid}` : 'Service inactive' }}
      </div>
    </div>
    <div class="status-banner-right">
      <div class="profile-hero-badge" :class="heroBadgeClass">
        <span class="profile-text">{{ heroBadgeText }}</span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useHyperStore } from '@/stores/hyper'

const store = useHyperStore()

const heroBadgeText = computed(() => {
  if (!store.isRunning) return 'Stopped'
  return store.activeProfile || 'Interactive'
})

const heroBadgeClass = computed(() => {
  if (!store.isRunning) return 'is-stopped'
  const prof = (store.activeProfile || 'interactive').toLowerCase()
  if (prof.includes('game') || prof.includes('gaming')) return 'is-gaming'
  if (prof.includes('sleep') || prof.includes('saver')) return 'is-sleep'
  return 'is-interactive'
})
</script>

<style scoped>
.status-banner {
  background: var(--surface-container-high, #26272e);
  border: 1px solid var(--outline-variant, #383a42);
  border-radius: 16px;
  margin-bottom: 14px;
  padding: 14px 16px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
}

.status-banner-left {
  display: flex;
  flex-direction: column;
  min-width: 0;
}

.status-title {
  font-size: 15px;
  font-weight: 700;
  color: var(--on-surface, #f0f2f5);
  letter-spacing: -0.2px;
  display: flex;
  align-items: baseline;
  flex-wrap: wrap;
  gap: 4px;
}

.developer-sub {
  font-size: 10px;
  font-weight: 500;
  color: var(--primary, #a0c4ff);
  opacity: 0.85;
}

.status-sub {
  font-size: 11px;
  color: var(--on-surface-variant, #b0b4c0);
  opacity: 0.8;
  margin-top: 2px;
  font-weight: 400;
}

.status-banner-right {
  display: flex;
  align-items: center;
  flex-shrink: 0;
}

.profile-hero-badge {
  display: inline-flex;
  align-items: center;
  font-size: 11px;
  font-weight: 600;
  padding: 4px 12px;
  border-radius: 20px;
  transition: all 0.25s cubic-bezier(0.34, 1.56, 0.64, 1);
  user-select: none;
}

.profile-hero-badge.is-interactive {
  background: rgba(160, 178, 198, 0.14);
  color: #a0b2c6;
  border: 1px solid rgba(160, 178, 198, 0.24);
}

.profile-hero-badge.is-gaming {
  background: rgba(184, 179, 232, 0.16);
  color: #b8b3e8;
  border: 1px solid rgba(184, 179, 232, 0.3);
}

.profile-hero-badge.is-sleep {
  background: rgba(210, 200, 176, 0.14);
  color: #d2c8b0;
  border: 1px solid rgba(210, 200, 176, 0.22);
}

.profile-hero-badge.is-stopped {
  background: rgba(229, 115, 115, 0.14);
  color: #e57373;
  border: 1px solid rgba(229, 115, 115, 0.22);
}
</style>
