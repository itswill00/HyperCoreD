<template>
  <transition name="disclaimer-fade">
    <div v-if="visible" class="disclaimer-overlay">
      <div class="disclaimer-card">
        <div class="header-icon">
          <Icons name="alert" :size="22" style="color: var(--primary);" />
        </div>

        <h2 class="title">Terms of use & disclaimer</h2>
        <p class="subtitle">Tanzanite hypercore architecture</p>

        <div class="content-box" ref="contentBox" @scroll="handleScroll">
          <p>
            <strong>1. Scope of system modifications</strong><br />
            By using <strong>Tanzanite HyperCore</strong>, you acknowledge that this software applies low-level kernel optimizations, custom GPU Operating Performance Points (OPP), MediaTek GED DVFS parameters, and dynamic thermal charging policies on your MediaTek Helio G99 Ultra (MT6789) device.
          </p>

          <p>
            <strong>2. Complete developer release of liability</strong><br />
            All software routines, binaries, and kernel tunings are provided strictly on an <em>"as-is"</em> and <em>"as-available"</em> basis without warranties of any kind. The lead developer (<strong>@itswill</strong>) is explicitly released from any and all liability, claims, damages, data loss, soft reboots, bootloops, battery degradation, thermal throttling, or hardware malfunction arising directly or indirectly from the installation, operation, or misuse of this software.
          </p>

          <p>
            <strong>3. User risk acceptance & root operation</strong><br />
            Operating root-level kernel tuners carries inherent risks. You certify that you are operating this module voluntarily, understand the technical changes being applied, and accept 100% personal responsibility for your device's stability and hardware health.
          </p>

          <p>
            <strong>4. Safety rules & kernel requirements</strong><br />
            This module requires Linux Kernel 5.10.x and strictly enforces system-background thread pinning on Little cores 0-3 to preserve Android Binder dispatcher stability. Modifying or overriding these safety rules manually may result in system watchdog soft reboots.
          </p>

          <div class="scroll-completion-marker">
            <span>End of terms. You may now accept and proceed.</span>
          </div>
        </div>

        <p v-if="!hasScrolledToBottom" class="scroll-hint">
          Please scroll to the bottom to continue
        </p>

        <div class="option-row" @click="dontShowAgain = !dontShowAgain">
          <div class="checkbox-box" :class="{ checked: dontShowAgain }">
            <Icons v-if="dontShowAgain" name="check" :size="12" style="color: var(--on-primary);" />
          </div>
          <span>Don't show again</span>
        </div>

        <button
          class="agree-btn"
          :class="{ disabled: !hasScrolledToBottom }"
          :disabled="!hasScrolledToBottom"
          @click="acceptDisclaimer"
        >
          <span>I agree & continue</span>
          <Icons name="check" :size="16" />
        </button>
      </div>
    </div>
  </transition>
</template>

<script setup>
import { ref, onMounted, inject } from 'vue'
import Icons from '@/components/icons/Icons.vue'

const visible = ref(false)
const dontShowAgain = ref(true)
const hasScrolledToBottom = ref(false)
const contentBox = ref(null)
const toast = inject('toast')

onMounted(() => {
  window.resetDisclaimer = () => {
    localStorage.removeItem('hypercore_disclaimer_agreed')
    hasScrolledToBottom.value = false
    visible.value = true
  }
  window.addEventListener('reset-disclaimer', window.resetDisclaimer)
  const agreed = localStorage.getItem('hypercore_disclaimer_agreed')
  if (agreed !== '1') {
    visible.value = true
  }
})

function handleScroll(e) {
  if (hasScrolledToBottom.value) return
  const target = e.target
  const isBottom = target.scrollTop + target.clientHeight >= target.scrollHeight - 15
  if (isBottom) {
    hasScrolledToBottom.value = true
  }
}

function acceptDisclaimer() {
  if (!hasScrolledToBottom.value) return
  if (dontShowAgain.value) {
    localStorage.setItem('hypercore_disclaimer_agreed', '1')
  }
  visible.value = false
  if (toast) {
    toast('Terms accepted. Welcome!')
  }
}
</script>

<style scoped>
.disclaimer-overlay {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.78);
  backdrop-filter: blur(12px);
  -webkit-backdrop-filter: blur(12px);
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
}

.disclaimer-card {
  width: 100%;
  max-width: 400px;
  background: var(--surface-container-high);
  border: 1px solid var(--surface-bright);
  border-radius: 24px;
  padding: 24px;
  box-shadow: 0 12px 32px rgba(0, 0, 0, 0.6);
  display: flex;
  flex-direction: column;
  align-items: center;
  text-align: center;
}

.header-icon {
  width: 44px;
  height: 44px;
  border-radius: 14px;
  background: var(--primary-container);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 14px;
}

.title {
  font-size: 17px;
  font-weight: 600;
  color: var(--on-surface);
  margin: 0;
  line-height: 1.3;
}

.subtitle {
  font-size: 11px;
  font-weight: 500;
  color: var(--on-surface-variant);
  margin: 3px 0 14px 0;
  letter-spacing: 0.3px;
}

.content-box {
  background: var(--surface-container);
  border: 1px solid var(--surface-container-highest);
  border-radius: 16px;
  padding: 14px 16px;
  text-align: left;
  max-height: 200px;
  overflow-y: auto;
  margin-bottom: 10px;
  display: flex;
  flex-direction: column;
  gap: 12px;
  scrollbar-width: thin;
}

.content-box p {
  font-size: 12px;
  line-height: 1.5;
  color: var(--on-surface-variant);
  margin: 0;
}

.content-box strong {
  color: var(--on-surface);
}

.scroll-completion-marker {
  font-size: 11px;
  color: var(--primary);
  font-weight: 500;
  text-align: center;
  padding: 8px 0 4px 0;
  border-top: 1px dashed var(--surface-bright);
}

.scroll-hint {
  font-size: 11px;
  color: var(--error, #ffb4ab);
  margin: 2px 0 8px 0;
  font-weight: 500;
}

.option-row {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 4px 4px 14px 4px;
  cursor: pointer;
  user-select: none;
  touch-action: manipulation;
  -webkit-tap-highlight-color: transparent;
}

.checkbox-box {
  width: 18px;
  height: 18px;
  border-radius: 5px;
  border: 1.5px solid var(--on-surface-variant);
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.15s ease;
}

.checkbox-box.checked {
  background: var(--primary);
  border-color: var(--primary);
}

.option-row span {
  font-size: 12.5px;
  color: var(--on-surface);
  font-weight: 500;
}

.agree-btn {
  width: 100%;
  height: 46px;
  border-radius: 23px;
  background: var(--primary);
  color: var(--on-primary);
  border: none;
  font-size: 13.5px;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  cursor: pointer;
  touch-action: manipulation;
  -webkit-tap-highlight-color: transparent;
  transition: opacity 0.2s ease, transform 0.15s ease, background 0.2s ease;
}

.agree-btn.disabled {
  opacity: 0.45;
  cursor: not-allowed;
  background: var(--surface-container-highest);
  color: var(--on-surface-variant);
}

.agree-btn:not(.disabled):active {
  opacity: 0.85;
  transform: scale(0.98);
}

.disclaimer-fade-enter-active,
.disclaimer-fade-leave-active {
  transition: opacity 0.22s cubic-bezier(0.2, 0, 0, 1);
}

.disclaimer-fade-enter-from,
.disclaimer-fade-leave-to {
  opacity: 0;
}
</style>
