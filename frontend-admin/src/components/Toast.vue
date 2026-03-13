<template>
  <div :class="['toast', `toast-${type}`]">
    {{ message }}
  </div>
</template>

<script setup>
defineProps({
  type: {
    type: String,
    default: 'success'
  },
  message: {
    type: String,
    required: true
  }
})
</script>
