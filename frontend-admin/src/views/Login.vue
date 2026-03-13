<template>
  <div class="login-page">
    <!-- 动态背景 -->
    <div class="bg-layer">
      <div class="bg-gradient"></div>
      <div class="bg-grid"></div>
      <div class="bg-glow glow-1"></div>
      <div class="bg-glow glow-2"></div>
      <div class="bg-glow glow-3"></div>
    </div>

    <!-- 登录卡片 -->
    <div class="login-container">
      <div class="login-card" :class="{ 'setup-mode': needSetup }">
        <!-- Logo区域 -->
        <div class="login-header">
          <div class="logo-wrapper">
            <div class="logo">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <path d="M3 21h18M5 21V7l7-4 7 4v14M9 21v-6h6v6" stroke-linecap="round" stroke-linejoin="round"/>
                <path d="M9 9h.01M15 9h.01M9 13h.01M15 13h.01" stroke-linecap="round"/>
              </svg>
            </div>
            <div class="logo-glow"></div>
          </div>
          <h1 class="title">{{ needSetup ? '系统初始化' : '访客预约系统' }}</h1>
          <p class="subtitle">{{ needSetup ? '创建您的管理员账户以开始使用' : '管理后台' }}</p>
        </div>

        <!-- 初始化表单 -->
        <form v-if="needSetup" @submit.prevent="handleSetup" class="login-form">
          <div class="form-group">
            <label class="form-label">管理员用户名</label>
            <div class="input-wrapper">
              <input 
                v-model="setupForm.username" 
                type="text" 
                class="input" 
                placeholder="设置管理员用户名"
                required
                autocomplete="username"
              />
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2" stroke-linecap="round"/>
                  <circle cx="12" cy="7" r="4"/>
                </svg>
              </span>
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">设置密码</label>
            <div class="input-wrapper">
              <input 
                v-model="setupForm.password" 
                type="password" 
                class="input" 
                placeholder="至少6位密码"
                required
                autocomplete="new-password"
              />
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>
                  <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
                </svg>
              </span>
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">真实姓名 <span class="optional">(可选)</span></label>
            <div class="input-wrapper">
              <input 
                v-model="setupForm.real_name" 
                type="text" 
                class="input" 
                placeholder="您的姓名"
                autocomplete="name"
              />
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/>
                  <circle cx="9" cy="7" r="4"/>
                  <path d="M23 21v-2a4 4 0 0 0-3-3.87M16 3.13a4 4 0 0 1 0 7.75"/>
                </svg>
              </span>
            </div>
          </div>

          <button type="submit" class="btn btn-primary login-btn" :disabled="loading">
            <span v-if="loading" class="btn-spinner"></span>
            <span v-else>初始化系统</span>
          </button>

          <p v-if="error" class="error-message">{{ error }}</p>
        </form>

        <!-- 登录表单 -->
        <form v-else @submit.prevent="handleLogin" class="login-form">
          <div class="form-group">
            <label class="form-label">用户名</label>
            <div class="input-wrapper">
              <input 
                v-model="form.username" 
                type="text" 
                class="input" 
                placeholder="请输入用户名"
                required
                autocomplete="username"
              />
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2" stroke-linecap="round"/>
                  <circle cx="12" cy="7" r="4"/>
                </svg>
              </span>
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">密码</label>
            <div class="input-wrapper">
              <input 
                v-model="form.password" 
                type="password" 
                class="input" 
                placeholder="请输入密码"
                required
                autocomplete="current-password"
              />
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>
                  <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
                </svg>
              </span>
            </div>
          </div>

          <button type="submit" class="btn btn-primary login-btn" :disabled="loading">
            <span v-if="loading" class="btn-spinner"></span>
            <span v-else>登 录</span>
          </button>

          <p v-if="error" class="error-message">{{ error }}</p>
        </form>

        <!-- 底部信息 -->
        <div class="login-footer">
          <p>Visitor Appointment System v1.0</p>
        </div>
      </div>

      <!-- 装饰元素 -->
      <div class="floating-shapes">
        <div class="shape shape-1"></div>
        <div class="shape shape-2"></div>
        <div class="shape shape-3"></div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, inject, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { authApi } from '../api'

const router = useRouter()
const showToast = inject('showToast')

const form = ref({ username: '', password: '' })
const setupForm = ref({ username: '', password: '', real_name: '' })
const loading = ref(false)
const error = ref('')
const needSetup = ref(false)

// 检查是否需要初始化
onMounted(async () => {
  try {
    const res = await authApi.checkSetup()
    needSetup.value = res.data.need_setup
  } catch (err) {
    console.error('Failed to check setup status:', err)
  }
})

const handleSetup = async () => {
  loading.value = true
  error.value = ''

  try {
    await authApi.setup(setupForm.value)
    showToast('success', '系统初始化成功，请使用新账户登录')
    needSetup.value = false
    form.value.username = setupForm.value.username
    setupForm.value = { username: '', password: '', real_name: '' }
  } catch (err) {
    error.value = err.message || '初始化失败'
  } finally {
    loading.value = false
  }
}

const handleLogin = async () => {
  loading.value = true
  error.value = ''

  try {
    const res = await authApi.login(form.value)
    localStorage.setItem('token', res.data.token)
    localStorage.setItem('user', JSON.stringify(res.data.user))
    showToast('success', '登录成功')
    router.push('/')
  } catch (err) {
    error.value = err.message || '登录失败'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.login-page {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
  overflow: hidden;
  padding: 2rem;
}

/* 背景层 */
.bg-layer {
  position: fixed;
  inset: 0;
  z-index: 0;
}

.bg-gradient {
  position: absolute;
  inset: 0;
  background: 
    radial-gradient(ellipse 80% 50% at 50% -20%, rgba(99, 102, 241, 0.15) 0%, transparent 50%),
    radial-gradient(ellipse 60% 40% at 100% 100%, rgba(168, 85, 247, 0.1) 0%, transparent 50%),
    linear-gradient(180deg, var(--gray-950) 0%, var(--gray-900) 100%);
}

.bg-grid {
  position: absolute;
  inset: 0;
  background-image: 
    linear-gradient(rgba(99, 102, 241, 0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(99, 102, 241, 0.03) 1px, transparent 1px);
  background-size: 60px 60px;
  mask-image: radial-gradient(ellipse at center, black 20%, transparent 70%);
}

.bg-glow {
  position: absolute;
  border-radius: 50%;
  filter: blur(80px);
  opacity: 0.5;
  animation: float 20s ease-in-out infinite;
}

.glow-1 {
  width: 400px;
  height: 400px;
  background: var(--primary-600);
  top: -100px;
  left: -100px;
}

.glow-2 {
  width: 300px;
  height: 300px;
  background: #a855f7;
  bottom: -50px;
  right: -50px;
  animation-delay: -7s;
}

.glow-3 {
  width: 200px;
  height: 200px;
  background: var(--accent-500);
  top: 50%;
  right: 20%;
  animation-delay: -14s;
}

@keyframes float {
  0%, 100% { transform: translate(0, 0) scale(1); }
  25% { transform: translate(30px, -30px) scale(1.1); }
  50% { transform: translate(-20px, 20px) scale(0.9); }
  75% { transform: translate(-30px, -20px) scale(1.05); }
}

/* 登录容器 */
.login-container {
  position: relative;
  z-index: 1;
  width: 100%;
  max-width: 520px;
  min-width: 400px;
}

@media (max-width: 560px) {
  .login-container {
    min-width: auto;
    margin: 0 1rem;
  }
}

/* 登录卡片 */
.login-card {
  background: rgba(24, 24, 27, 0.8);
  backdrop-filter: blur(40px);
  border: 1px solid rgba(99, 102, 241, 0.1);
  border-radius: 1.5rem;
  padding: 3rem;
  box-shadow: 
    0 0 0 1px rgba(255, 255, 255, 0.05) inset,
    0 25px 50px -12px rgba(0, 0, 0, 0.5),
    0 0 100px -20px rgba(99, 102, 241, 0.3);
  animation: cardAppear 0.6s cubic-bezier(0.16, 1, 0.3, 1);
}

@media (max-width: 560px) {
  .login-card {
    padding: 2rem;
  }
}

.login-card.setup-mode {
  border-color: rgba(249, 115, 22, 0.2);
  box-shadow: 
    0 0 0 1px rgba(255, 255, 255, 0.05) inset,
    0 25px 50px -12px rgba(0, 0, 0, 0.5),
    0 0 100px -20px rgba(249, 115, 22, 0.3);
}

@keyframes cardAppear {
  from {
    opacity: 0;
    transform: translateY(30px) scale(0.95);
  }
  to {
    opacity: 1;
    transform: translateY(0) scale(1);
  }
}

/* 头部 */
.login-header {
  text-align: center;
  margin-bottom: 2rem;
}

.logo-wrapper {
  position: relative;
  display: inline-flex;
  margin-bottom: 1.25rem;
}

.logo {
  width: 4rem;
  height: 4rem;
  background: linear-gradient(135deg, var(--primary-500) 0%, var(--primary-700) 100%);
  border-radius: 1rem;
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
  z-index: 1;
}

.setup-mode .logo {
  background: linear-gradient(135deg, var(--accent-500) 0%, var(--accent-600) 100%);
}

.logo svg {
  width: 2rem;
  height: 2rem;
  color: white;
}

.logo-glow {
  position: absolute;
  inset: -8px;
  background: linear-gradient(135deg, var(--primary-500) 0%, var(--primary-700) 100%);
  border-radius: 1.25rem;
  filter: blur(20px);
  opacity: 0.5;
  z-index: 0;
}

.setup-mode .logo-glow {
  background: linear-gradient(135deg, var(--accent-500) 0%, var(--accent-600) 100%);
}

.title {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--gray-50);
  margin-bottom: 0.375rem;
  letter-spacing: -0.025em;
}

.subtitle {
  color: var(--gray-400);
  font-size: 0.9375rem;
}

/* 表单 */
.login-form {
  display: flex;
  flex-direction: column;
  gap: 1.25rem;
}

.input-wrapper {
  position: relative;
}

.input-wrapper .input {
  padding-left: 3rem;
}

.input-icon {
  position: absolute;
  left: 1rem;
  top: 50%;
  transform: translateY(-50%);
  color: var(--gray-500);
  pointer-events: none;
  transition: color 0.2s;
}

.input-icon svg {
  width: 1.125rem;
  height: 1.125rem;
}

.input-wrapper .input:focus + .input-icon {
  color: var(--primary-400);
}

.optional {
  color: var(--gray-500);
  font-weight: 400;
  font-size: 0.75rem;
}

.login-btn {
  width: 100%;
  padding: 0.875rem 1.5rem;
  font-size: 1rem;
  margin-top: 0.5rem;
}

.btn-spinner {
  width: 1.25rem;
  height: 1.25rem;
  border: 2px solid rgba(255,255,255,0.3);
  border-top-color: white;
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

.error-message {
  color: var(--error-400);
  font-size: 0.875rem;
  text-align: center;
  padding: 0.75rem 1rem;
  background: rgba(244, 63, 94, 0.1);
  border-radius: var(--radius);
  border: 1px solid rgba(244, 63, 94, 0.2);
}

/* 底部 */
.login-footer {
  margin-top: 2rem;
  padding-top: 1.5rem;
  border-top: 1px solid var(--gray-800);
  text-align: center;
}

.login-footer p {
  font-size: 0.75rem;
  color: var(--gray-600);
  letter-spacing: 0.05em;
}

/* 浮动装饰 */
.floating-shapes {
  position: absolute;
  inset: -100px;
  pointer-events: none;
  z-index: -1;
}

.shape {
  position: absolute;
  border-radius: 50%;
  border: 1px solid rgba(99, 102, 241, 0.1);
  animation: orbit 30s linear infinite;
}

.shape-1 {
  width: 300px;
  height: 300px;
  top: -50px;
  left: -100px;
}

.shape-2 {
  width: 200px;
  height: 200px;
  bottom: -30px;
  right: -80px;
  animation-delay: -10s;
  animation-duration: 25s;
}

.shape-3 {
  width: 150px;
  height: 150px;
  top: 40%;
  right: -120px;
  animation-delay: -20s;
  animation-duration: 20s;
}

@keyframes orbit {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}
</style>
