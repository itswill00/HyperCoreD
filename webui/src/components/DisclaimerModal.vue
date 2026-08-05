<template>
  <transition name="disclaimer-fade">
    <div v-if="visible" class="disclaimer-overlay">
      <div class="disclaimer-card">
        <div class="header-icon">
          <Icons name="alert" :size="22" style="color: var(--primary);" />
        </div>

        <h2 class="title">Terms of use & disclaimer</h2>
        <p class="subtitle">Tanzanite hypercore architecture</p>

        <div class="content-box">
          <p>
            By using <strong>Tanzanite HyperCore</strong>, you acknowledge that you are applying system kernel tuning, GPU frequency scaling, and thermal policies on your MediaTek MT6789 device.
          </p>
          <p>
            All software routines and kernel tunings are provided on an <em>"as-is"</em> basis. The developer (<strong>@itswill</strong>) is not responsible for any hardware damage, data loss, soft reboots, or device instability resulting from the use of this module.
          </p>
          <p>
            You accept full personal responsibility for operating this module on your device.
          </p>
        </div>

        <div class="option-row" @click="dontShowAgain = !dontShowAgain">
          <div class="checkbox-box" :class="{ checked: dontShowAgain }">
            <Icons v-if="dontShowAgain" name="check" :size="12" style="color: var(--on-primary);" />
          </div>
          <span>Don't show again</span>
        </div>

        <button class="agree-btn" @click="acceptDisclaimer">
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
const toast = inject('toast')

onMounted(() => {
  const agreed = localStorage.getItem('hypercore_disclaimer_agreed')
  if (agreed !== '1') {
    visible.value = true
  }
})

function acceptDisclaimer() {
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
  background: rgba(0, 0, 0, 0.76);
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
  max-height: 220px;
  overflow-y: auto;
  margin-bottom: 16px;
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.content-box p {
  font-size: 12.5px;
  line-height: 1.5;
  color: var(--on-surface-variant);
  margin: 0;
}

.content-box strong {
  color: var(--on-surface);
}

.option-row {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 6px 4px 16px 4px;
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
  transition: opacity 0.15s ease, transform 0.15s ease;
}

.agree-btn:active {
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
