<template>
  <transition name="disclaimer-fade">
    <div v-if="visible" class="disclaimer-overlay">
      <div class="disclaimer-card">
        <div class="header-icon">
          <Icons name="alert" :size="24" style="color: var(--primary);" />
        </div>

        <h2 class="title">Terms of Use & Disclaimer</h2>
        <p class="subtitle">Tanzanite HyperCore Architecture</p>

        <div class="content-box">
          <p>
            By accessing and operating <strong>Tanzanite HyperCore</strong>, you acknowledge that you are applying system-level kernel optimizations, GPU frequency scaling configurations, and hardware thermal policies on your MediaTek MT6789 device.
          </p>          <p>
            All software routines and kernel tunings are provided on an <em>"as-is"</em> basis. The lead developer (<strong>@itswill</strong>) assumes zero liability or responsibility for any potential hardware damage, data loss, soft reboots, thermal throttling, or unexpected system behavior resulting from the use or misuse of this software.
          </p>
          <p>
            By proceeding, you confirm that you fully understand these risks and accept complete personal responsibility for operating this module on your device.
          </p>
        </div>

        <button class="agree-btn" @click="acceptDisclaimer">
          <span>I Agree & Continue</span>
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
const toast = inject('toast')

onMounted(() => {
  const agreed = localStorage.getItem('hypercore_disclaimer_agreed')
  if (agreed !== '1') {
    visible.value = true
  }
})

function acceptDisclaimer() {
  localStorage.setItem('hypercore_disclaimer_agreed', '1')
  visible.value = false
  if (toast) {
    toast('Terms accepted. Welcome to Tanzanite HyperCore!')
  }
}
</script>

<style scoped>
.disclaimer-overlay {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.78);
  backdrop-filter: blur(10px);
  -webkit-backdrop-filter: blur(10px);
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
}

.disclaimer-card {
  width: 100%;
  max-width: 420px;
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
  width: 48px;
  height: 48px;
  border-radius: 16px;
  background: var(--primary-container);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 16px;
}

.title {
  font-size: 18px;
  font-weight: 700;
  color: var(--on-surface);
  margin: 0;
  line-height: 1.3;
}

.subtitle {
  font-size: 12px;
  font-weight: 500;
  color: var(--on-surface-variant);
  margin: 4px 0 16px 0;
  letter-spacing: 0.5px;
  text-transform: uppercase;
}

.content-box {
  background: var(--surface-container);
  border: 1px solid var(--surface-container-highest);
  border-radius: 16px;
  padding: 16px;
  text-align: left;
  max-height: 240px;
  overflow-y: auto;
  margin-bottom: 20px;
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.content-box p {
  font-size: 13px;
  line-height: 1.5;
  color: var(--on-surface-variant);
  margin: 0;
}

.content-box strong {
  color: var(--on-surface);
}

.agree-btn {
  width: 100%;
  height: 48px;
  border-radius: 24px;
  background: var(--primary);
  color: var(--on-primary);
  border: none;
  font-size: 14px;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  cursor: pointer;
  touch-action: manipulation;
  -webkit-tap-highlight-color: transparent;
  transition: opacity 0.15s ease, transform 0.15s ease;
}

.agree-btn:active {
  opacity: 0.85;
  transform: scale(0.98);
}

.disclaimer-fade-enter-active,
.disclaimer-fade-leave-active {
  transition: opacity 0.25s cubic-bezier(0.2, 0, 0, 1);
}

.disclaimer-fade-enter-from,
.disclaimer-fade-leave-to {
  opacity: 0;
}
</style>
