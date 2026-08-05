<template>
  <transition name="disclaimer-fade">
    <div v-if="visible" class="disclaimer-overlay">
      <div class="disclaimer-card">
        <h2 class="title">Terms of use & disclaimer</h2>
        <p class="subtitle">Tanzanite hypercore architecture</p>

        <div class="content-box" ref="contentBox" @scroll="handleScroll">
          <div class="sec-block">
            <div class="sec-title">1. Scope of system modifications</div>
            <p>
              By using <strong>Tanzanite HyperCore</strong>, you acknowledge that this software applies low-level kernel optimizations, custom GPU Operating Performance Points (OPP), MediaTek GED DVFS parameters, CPU frequency governor limits, and dynamic thermal charging policies on your MediaTek Helio G99 Ultra (MT6789) device.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">2. Complete developer release of liability</div>
            <p>
              All software routines, C daemon binaries, and kernel tunings are provided strictly on an <em>"as-is"</em> and <em>"as-available"</em> basis without warranties of any kind. The lead developer (<strong>@itswill00 / @noticesa</strong>) is explicitly and permanently released from any and all liability, claims, financial losses, hardware degradation, data corruption, soft reboots, bootloops, battery wear, or unexpected device malfunction arising directly or indirectly from the installation, execution, or misuse of this software.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">3. GPU Devfreq & MediaTek GED policies</div>
            <p>
              HyperCore dynamically manages Mali-G57 GPU devfreq governors and GED frame boost parameters. While these adjustments eliminate vendor frequency caps and optimize frame delivery, operating the GPU at elevated performance states generates additional thermal output.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">4. Thermal boundaries & charging guard</div>
            <p>
              The daemon monitors battery temperature in real time during gaming sessions and adjusts charge current limits dynamically (2A, 4A, 8A). You acknowledge that thermal guard triggers are hardware protections and attempting to manually bypass or tamper with thermal guard nodes may cause thermal throttling or accelerated battery wear.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">5. System-background thread stability</div>
            <p>
              To preserve Android Binder dispatcher stability and prevent watchdog timeouts, system-background threads are strictly pinned to Little cores 0-3. Modifying these cgroup boundaries manually is unsupported and may trigger OS watchdog soft reboots.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">6. Asynchronous memory compaction</div>
            <p>
              Memory management routines rely exclusively on kernel <code>kcompactd</code> proactive compaction. Synchronous writes to <code>/proc/sys/vm/compact_memory</code> are disabled to prevent kernel <code>mmap_lock</code> contention across memory zones.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">7. Voluntary root operation & risk acceptance</div>
            <p>
              Executing root-level system daemons carries inherent risks. You certify that you are installing and operating this software voluntarily, understand the kernel-level changes being executed, and assume 100% personal responsibility for your device's operational stability and hardware integrity.
            </p>
          </div>

          <div class="sec-block">
            <div class="sec-title">8. Binding agreement confirmation</div>
            <p>
              By scrolling to the bottom of these terms and clicking "I agree & continue", you acknowledge that you have read, understood, and agreed to all 8 sections of this agreement.
            </p>
          </div>

          <div class="scroll-completion-marker" :class="{ completed: hasScrolledToBottom }">
            <Icons v-if="hasScrolledToBottom" name="check" :size="14" />
            <span>{{ hasScrolledToBottom ? 'All terms read. You may now accept and proceed.' : 'Scroll further to reach the end of terms.' }}</span>
          </div>
        </div>

        <div class="progress-bar-bg" v-if="!hasScrolledToBottom">
          <div class="progress-bar-fill" :style="{ width: scrollProgress + '%' }"></div>
        </div>

        <p v-if="!hasScrolledToBottom" class="scroll-hint">
          Scroll down to read all terms ({{ scrollProgress }}%)
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
const scrollProgress = ref(0)
const contentBox = ref(null)
const toast = inject('toast')

onMounted(() => {
  window.resetDisclaimer = () => {
    localStorage.removeItem('hypercore_disclaimer_agreed')
    hasScrolledToBottom.value = false
    scrollProgress.value = 0
    visible.value = true
  }
  window.addEventListener('reset-disclaimer', window.resetDisclaimer)
  const agreed = localStorage.getItem('hypercore_disclaimer_agreed')
  if (agreed !== '1') {
    visible.value = true
  }
})

function handleScroll(e) {
  const target = e.target
  const maxScroll = target.scrollHeight - target.clientHeight
  if (maxScroll <= 0) {
    scrollProgress.value = 100
    hasScrolledToBottom.value = true
    return
  }
  
  const current = target.scrollTop
  const pct = Math.min(100, Math.max(0, Math.round((current / maxScroll) * 100)))
  scrollProgress.value = pct

  if (current + target.clientHeight >= target.scrollHeight - 10) {
    hasScrolledToBottom.value = true
    scrollProgress.value = 100
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
  background: rgba(0, 0, 0, 0.82);
  backdrop-filter: blur(14px);
  -webkit-backdrop-filter: blur(14px);
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 16px;
}

.disclaimer-card {
  width: 100%;
  max-width: 420px;
  background: var(--surface-container-high);
  border: 1px solid var(--surface-bright);
  border-radius: 24px;
  padding: 24px 20px;
  box-shadow: 0 16px 36px rgba(0, 0, 0, 0.65);
  display: flex;
  flex-direction: column;
  align-items: center;
  text-align: center;
  max-height: 90vh;
}

.header-icon {
  width: 44px;
  height: 44px;
  border-radius: 14px;
  background: var(--primary-container);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 12px;
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
  max-height: 260px;
  overflow-y: auto;
  margin-bottom: 10px;
  display: flex;
  flex-direction: column;
  gap: 14px;
  scrollbar-width: thin;
  scrollbar-color: var(--outline-variant) transparent;
}

.content-box::-webkit-scrollbar {
  width: 5px;
}

.content-box::-webkit-scrollbar-track {
  background: transparent;
}

.content-box::-webkit-scrollbar-thumb {
  background: var(--outline-variant);
  border-radius: 4px;
}

.sec-block {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.sec-title {
  font-size: 12.5px;
  font-weight: 600;
  color: var(--primary);
  letter-spacing: -0.1px;
}

.content-box p {
  font-size: 12.5px;
  line-height: 1.6;
  color: var(--on-surface-variant);
  margin: 0;
}

.content-box strong {
  color: var(--on-surface);
}

.content-box code {
  font-family: var(--font-mono);
  font-size: 11px;
  background: rgba(255, 255, 255, 0.08);
  padding: 1px 4px;
  border-radius: 4px;
}

.scroll-completion-marker {
  font-size: 11px;
  color: var(--on-surface-variant);
  font-weight: 500;
  text-align: center;
  padding: 10px 0 4px 0;
  border-top: 1px dashed var(--surface-bright);
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 6px;
  transition: color 0.2s ease;
}

.scroll-completion-marker.completed {
  color: var(--primary);
  font-weight: 600;
}

.progress-bar-bg {
  width: 100%;
  height: 3px;
  background: var(--surface-container-highest);
  border-radius: 2px;
  overflow: hidden;
  margin-bottom: 6px;
}

.progress-bar-fill {
  height: 100%;
  background: var(--primary);
  transition: width 0.1s ease-out;
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
  opacity: 0.35;
  cursor: not-allowed;
  pointer-events: none;
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
