<template>
  <div style="height: 100%; display: flex; flex-direction: column;">
    
    <div class="page-header">
      <div>
        <div class="page-header-title">Detected Games</div>
        <div class="page-header-sub">Installed games that trigger Gaming mode</div>
      </div>
      <button class="btn-md3 btn-md3-primary" style="padding: 6px 12px; font-size: 11px;" @click="showPicker = true">
        <Icons name="plus" :size="14" />
        <span>Add game</span>
      </button>
    </div>

    <div class="content-area">
      <GameList @open-picker="showPicker = true" />
    </div>

    <AppPicker v-if="showPicker" @close="showPicker = false" @pick="onPick" />
  </div>
</template>

<script setup>
import { ref, inject } from 'vue'
import { useHyperStore } from '@/stores/hyper'
import GameList from '@/components/GameList.vue'
import AppPicker from '@/components/AppPicker.vue'
import Icons from '@/components/icons/Icons.vue'

const store = useHyperStore()
const toast = inject('toast')
const showPicker = ref(false)

async function onPick(pkg) {
  showPicker.value = false
  const msg = await store.addGame(pkg)
  if (msg && toast) toast(msg)
}
</script>
