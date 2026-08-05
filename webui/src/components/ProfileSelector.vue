<template>
  <div class="md3-card" style="padding: 14px 16px; margin-bottom: 12px;">
    <div style="display: flex; align-items: center; justify-content: space-between;">
      <div style="display: flex; align-items: center; gap: 10px;">
        <div class="icon-badge">
          <Icons name="star" :size="18" />
        </div>
        <div>
          <div style="font-size: 13px; font-weight: 600; color: var(--on-surface);">Smart adaptive engine</div>
          <div style="font-size: 11px; color: var(--on-surface-variant); margin-top: 1px;">Automatic profile switching</div>
        </div>
      </div>
      <span class="badge-pill purple" style="font-size: 10px; padding: 3px 10px; font-weight: 600;">
        {{ engineStatus }}
      </span>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useHyperStore } from '@/stores/hyper'
import Icons from '@/components/icons/Icons.vue'

const store = useHyperStore()

const engineStatus = computed(() => {
  if (!store.isRunning) return 'Inactive'
  const profile = store.activeProfile || 'Interactive'
  if (profile.includes('Gaming')) return profile
  return profile
})
</script>
