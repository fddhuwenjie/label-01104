<template>
  <div class="app">
    <header class="header">
      <div class="container header-content">
        <router-link to="/" class="logo">
          <span class="logo-icon">🏢</span>
          <span>访客预约</span>
        </router-link>
        
        <nav class="nav">
          <template v-if="user">
            <router-link to="/appointments" class="nav-link">我的预约</router-link>
            <router-link to="/new-appointment" class="nav-link">新建预约</router-link>
            <router-link to="/profile" class="nav-link">个人中心</router-link>
            <button @click="handleLogout" class="btn btn-outline">退出</button>
          </template>
          <template v-else>
            <router-link to="/login" class="btn btn-outline">登录</router-link>
            <router-link to="/register" class="btn btn-primary">注册</router-link>
          </template>
        </nav>
      </div>
    </header>
    
    <main>
      <router-view />
    </main>
    
    <footer class="footer">
      <div class="container">
        <p>© 2024 访客预约系统. All rights reserved.</p>
      </div>
    </footer>
    
    <Toast v-if="toast.show" :type="toast.type" :message="toast.message" />
  </div>
</template>

<script setup>
import { ref, reactive, provide, onMounted, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import Toast from './components/Toast.vue'

const router = useRouter()
const route = useRoute()
const user = ref(null)

const toast = reactive({
  show: false,
  type: 'success',
  message: ''
})

const showToast = (type, message) => {
  toast.type = type
  toast.message = message
  toast.show = true
  setTimeout(() => { toast.show = false }, 3000)
}

const loadUser = () => {
  const userStr = localStorage.getItem('user')
  user.value = userStr ? JSON.parse(userStr) : null
}

const handleLogout = () => {
  localStorage.removeItem('token')
  localStorage.removeItem('user')
  user.value = null
  router.push('/')
  showToast('success', '已退出登录')
}

provide('showToast', showToast)
provide('user', user)

onMounted(loadUser)
watch(route, loadUser)
</script>
