<template>
  <router-view />
  <Toast v-if="toast.show" :type="toast.type" :message="toast.message" />
</template>

<script setup>
import { reactive, provide } from 'vue'
import Toast from './components/Toast.vue'

const toast = reactive({
  show: false,
  type: 'success',
  message: ''
})

const showToast = (type, message) => {
  toast.type = type
  toast.message = message
  toast.show = true
  setTimeout(() => {
    toast.show = false
  }, 3000)
}

provide('showToast', showToast)
</script>
