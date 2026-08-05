<template>
  <div>
    <div class="section-title">Quick Actions</div>
    <div class="md3-list-group">
      <div class="md3-list-row clickable" :class="{ 'disabled': store.loading }" @click="flush">
        <div class="row-left">
          <div class="icon-badge">
            <Icons name="memory" :size="18" />
          </div>
          <div class="row-meta">
            <div class="row-title">Clear RAM cache</div>
            <div class="row-sub">Compact memory & drop caches</div>
          </div>
        </div>
        <Icons name="chevron-right" :size="20" style="color: var(--on-surface-variant);" />
      </div>

      <div class="md3-list-row clickable" :class="{ 'disabled': store.loading }" @click="saveLog">
        <div class="row-left">
          <div class="icon-badge secondary">
            <Icons name="logs" :size="18" />
          </div>
          <div class="row-meta">
            <div class="row-title">Save log</div>
            <div class="row-sub">Export operational log to SD card</div>
          </div>
        </div>
        <Icons name="chevron-right" :size="20" style="color: var(--on-surface-variant);" />
      </div>

      <div class="md3-list-row clickable" :class="{ 'disabled': store.loading }" @click="restart">
        <div class="row-left">
          <div class="icon-badge tertiary">
            <Icons name="refresh" :size="18" />
          </div>
          <div class="row-meta">
            <div class="row-title">Restart daemon</div>
            <div class="row-sub">Relaunch background service</div>
          </div>
        </div>
        <Icons name="chevron-right" :size="20" style="color: var(--on-surface-variant);" />
      </div>

      <div class="md3-list-row clickable" @click="addShortcut">
        <div class="row-left">
          <div class="icon-badge">
            <Icons name="star" :size="18" />
          </div>
          <div class="row-meta">
            <div class="row-title">Create shortcut</div>
            <div class="row-sub">Add home screen launcher shortcut</div>
          </div>
        </div>
        <Icons name="chevron-right" :size="20" style="color: var(--on-surface-variant);" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { inject } from 'vue'
import { useHyperStore } from '@/stores/hyper'
import Icons from '@/components/icons/Icons.vue'

const store = useHyperStore()
const toast = inject('toast')

async function flush() {
  if (store.loading) return
  if (toast) toast('Clearing RAM cache...')
  const msg = await store.flushRam()
  if (toast) toast(msg)
}

async function restart() {
  if (store.loading) return
  if (toast) toast('Restarting daemon...')
  const msg = await store.restartDaemon()
  if (toast) toast(msg)
}

async function saveLog() {
  if (store.loading) return
  const msg = await store.exportLogs()
  if (toast) toast(msg)
}

function addShortcut() {
  const msg = store.createShortcut()
  if (toast) toast(msg)
}
</script>

<style scoped>
.disabled {
  opacity: 0.5;
  pointer-events: none;
}
</style>
