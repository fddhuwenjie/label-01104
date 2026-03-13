<template>
  <div class="auth-page">
    <!-- 动态背景 -->
    <div class="bg-layer">
      <div class="bg-gradient"></div>
      <div class="bg-shapes">
        <div class="shape shape-1"></div>
        <div class="shape shape-2"></div>
        <div class="shape shape-3"></div>
      </div>
    </div>

    <div class="container">
      <div class="auth-card">
        <!-- 装饰线 -->
        <div class="card-accent"></div>
        
        <div class="auth-header">
          <div class="auth-logo">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M3 21h18M5 21V7l7-4 7 4v14M9 21v-6h6v6" stroke-linecap="round" stroke-linejoin="round"/>
            </svg>
          </div>
          <h1 class="auth-title">欢迎回来</h1>
          <p class="auth-subtitle">登录您的账户，开始预约访问</p>
        </div>

        <form @submit.prevent="handleLogin" class="auth-form">
          <div class="form-group">
            <label class="form-label">用户名</label>
            <div class="input-group">
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M20 21v-2a4 4 0 00-4-4H8a4 4 0 00-4 4v2"/>
                  <circle cx="12" cy="7" r="4"/>
                </svg>
              </span>
              <input 
                v-model="form.username" 
                type="text" 
                class="input input-with-icon" 
                placeholder="请输入用户名" 
                required 
              />
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">密码</label>
            <div class="input-group">
              <span class="input-icon">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>
                  <path d="M7 11V7a5 5 0 0110 0v4"/>
                </svg>
              </span>
              <input 
                v-model="form.password" 
                type="password" 
                class="input input-with-icon" 
                placeholder="请输入密码" 
                required 
              />
            </div>
          </div>

          <button type="submit" class="btn btn-primary btn-lg btn-block" :disabled="loading">
            <span v-if="loading" class="btn-spinner"></span>
            <span v-else>登 录</span>
          </button>

          <p v-if="error" class="error-msg">{{ error }}</p>
        </form>

        <div class="auth-divider">
          <span>或</span>
        </div>

        <div class="auth-footer">
          <p>还没有账户？</p>
          <router-link to="/register" class="btn btn-outline btn-block">
            创建新账户
          </router-link>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, inject } from 'vue'
import { useRouter } from 'vue-router'
import { authApi } from '../api'

const router = useRouter()
const showToast = inject('showToast')

const form = ref({ username: '', password: '' })
const loading = ref(false)
const error = ref('')

const handleLogin = async () => {
  loading.value = true
  error.value = ''

  try {
    const res = await authApi.login(form.value)
    localStorage.setItem('token', res.data.token)
    localStorage.setItem('user', JSON.stringify(res.data.user))
    showToast('success', '登录成功')
    router.push('/appointments')
  } catch (err) {
    error.value = err.message || '登录失败'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.auth-page {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 2rem 0;
  position: relative;
  overflow: hidden;
}

/* 背景 */
.bg-layer {
  position: fixed;
  inset: 0;
  z-index: 0;
}

.bg-gradient {
  position: absolute;
  inset: 0;
  background: 
    radial-gradient(ellipse 80% 50% at 50% 0%, rgba(99, 102, 241, 0.12) 0%, transparent 50%),
    radial-gradient(ellipse 60% 40% at 100% 100%, rgba(168, 85, 247, 0.08) 0%, transparent 50%),
    var(--gray-950);
}

.bg-shapes {
  position: absolute;
  inset: 0;
  overflow: hidden;
}

.shape {
  position: absolute;
  border-radius: 50%;
  background: linear-gradient(135deg, rgba(99, 102, 241, 0.1) 0%, rgba(168, 85, 247, 0.05) 100%);
  animation: float 30s ease-in-out infinite;
}

.shape-1 {
  width: 400px;
  height: 400px;
  top: -100px;
  right: -100px;
}

.shape-2 {
  width: 300px;
  height: 300px;
  bottom: -50px;
  left: -100px;
  animation-delay: -10s;
}

.shape-3 {
  width: 200px;
  height: 200px;
  top: 40%;
  left: 10%;
  animation-delay: -20s;
}

@keyframes float {
  0%, 100% { transform: translate(0, 0) scale(1); }
  25% { transform: translate(20px, -20px) scale(1.05); }
  50% { transform: translate(-10px, 10px) scale(0.95); }
  75% { transform: translate(-20px, -10px) scale(1.02); }
}

/* 卡片 */
.auth-card {
  position: relative;
  z-index: 1;
  background: rgba(24, 24, 27, 0.8);
  backdrop-filter: blur(40px);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-2xl);
  padding: 3rem;
  width: 100%;
  max-width: 520px;
  min-width: 400px;
  box-shadow: 
    0 0 0 1px rgba(255, 255, 255, 0.03) inset,
    var(--shadow-xl);
  animation: cardAppear 0.5s cubic-bezier(0.16, 1, 0.3, 1);
}

@media (max-width: 560px) {
  .auth-card {
    min-width: auto;
    padding: 2rem;
    margin: 0 1rem;
  }
}

.card-accent {
  position: absolute;
  top: 0;
  left: 2rem;
  right: 2rem;
  height: 3px;
  background: linear-gradient(90deg, var(--primary-500), var(--accent-500));
  border-radius: 0 0 2px 2px;
}

@keyframes cardAppear {
  from { opacity: 0; transform: translateY(20px); }
  to { opacity: 1; transform: translateY(0); }
}

/* 头部 */
.auth-header {
  text-align: center;
  margin-bottom: 2rem;
}

.auth-logo {
  width: 4rem;
  height: 4rem;
  margin: 0 auto 1.25rem;
  background: linear-gradient(135deg, var(--primary-500) 0%, var(--primary-700) 100%);
  border-radius: var(--radius-xl);
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 8px 24px -4px rgba(99, 102, 241, 0.4);
}

.auth-logo svg {
  width: 2rem;
  height: 2rem;
  color: white;
}

.auth-title {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--gray-50);
  margin-bottom: 0.375rem;
  letter-spacing: -0.025em;
}

.auth-subtitle {
  color: var(--gray-400);
  font-size: 0.9375rem;
}

/* 表单 */
.auth-form {
  display: flex;
  flex-direction: column;
  gap: 1.25rem;
}

.input-group {
  position: relative;
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

.input-with-icon {
  padding-left: 3rem;
  padding-top: 0.875rem;
  padding-bottom: 0.875rem;
  font-size: 1rem;
}

.input-with-icon:focus + .input-icon,
.input-group:focus-within .input-icon {
  color: var(--primary-400);
}

.btn-block {
  width: 100%;
}

.btn-spinner {
  width: 1.25rem;
  height: 1.25rem;
  border: 2px solid rgba(255,255,255,0.3);
  border-top-color: white;
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.error-msg {
  color: var(--error-400);
  font-size: 0.875rem;
  text-align: center;
  padding: 0.75rem 1rem;
  background: rgba(244, 63, 94, 0.1);
  border-radius: var(--radius);
  border: 1px solid rgba(244, 63, 94, 0.2);
}

/* 分隔线 */
.auth-divider {
  display: flex;
  align-items: center;
  margin: 1.5rem 0;
}

.auth-divider::before,
.auth-divider::after {
  content: '';
  flex: 1;
  height: 1px;
  background: var(--gray-800);
}

.auth-divider span {
  padding: 0 1rem;
  color: var(--gray-500);
  font-size: 0.8125rem;
}

/* 底部 */
.auth-footer {
  text-align: center;
}

.auth-footer p {
  color: var(--gray-500);
  font-size: 0.875rem;
  margin-bottom: 0.75rem;
}
</style>
