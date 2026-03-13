import { createApp } from 'vue'
import { createRouter, createWebHistory } from 'vue-router'
import App from './App.vue'
import './style.css'

// 路由配置
const routes = [
  { path: '/login', name: 'Login', component: () => import('./views/Login.vue') },
  { 
    path: '/', 
    component: () => import('./views/Layout.vue'),
    children: [
      { path: '', name: 'Dashboard', component: () => import('./views/Dashboard.vue') },
      { path: 'users', name: 'Users', component: () => import('./views/Users.vue') },
      { path: 'appointments', name: 'Appointments', component: () => import('./views/Appointments.vue') },
      { path: 'check-records', name: 'CheckRecords', component: () => import('./views/CheckRecords.vue') },
    ]
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

// 路由守卫
router.beforeEach((to, from, next) => {
  const token = localStorage.getItem('token')
  if (to.path !== '/login' && !token) {
    next('/login')
  } else {
    next()
  }
})

const app = createApp(App)
app.use(router)
app.mount('#app')
