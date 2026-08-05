<template>
  <nav class="md3-navbar">
    <router-link
      v-for="item in navItems"
      :key="item.name"
      :to="item.path"
      class="nav-item"
      :class="{ 'active': isActive(item) }"
    >
      <div class="nav-icon-wrapper">
        <Icons :name="item.icon" :size="20" />
      </div>
      <span class="nav-label">{{ item.label }}</span>
    </router-link>
  </nav>
</template>

<script setup>
import { useRoute } from 'vue-router'
import Icons from '@/components/icons/Icons.vue'

const route = useRoute()

const navItems = [
  { name: 'Dashboard', path: '/', label: 'Dashboard', icon: 'home' },
  { name: 'Games', path: '/games', label: 'Games', icon: 'games' },
  { name: 'Logs', path: '/logs', label: 'Logs', icon: 'logs' }
]

const isActive = (item) => {
  if (item.path === '/') return route.path === '/'
  return route.path.startsWith(item.path)
}
</script>
