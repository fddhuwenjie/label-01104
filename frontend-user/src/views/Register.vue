<template>
  <div class="auth-page">
    <!-- 动态背景 -->
    <div class="bg-layer">
      <div class="bg-gradient"></div>
      <div class="bg-shapes">
        <div class="shape shape-1"></div>
        <div class="shape shape-2"></div>
      </div>
    </div>

    <div class="container">
      <div class="auth-card">
        <div class="card-accent"></div>

        <div class="auth-header">
          <div class="auth-logo">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M16 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2"/>
              <circle cx="8.5" cy="7" r="4"/>
              <line x1="20" y1="8" x2="20" y2="14"/>
              <line x1="23" y1="11" x2="17" y2="11"/>
            </svg>
          </div>
          <h1 class="auth-title">创建账户</h1>
          <p class="auth-subtitle">注册新账户，开始您的访问预约</p>
        </div>

        <form @submit.prevent="handleRegister" class="auth-form">
          <div class="form-row">
            <div class="form-group">
              <label class="form-label">用户名 <span class="required">*</span></label>
              <input 
                v-model="form.username" 
                type="text" 
                class="input" 
                placeholder="设置用户名" 
                required 
              />
            </div>

            <div class="form-group">
              <label class="form-label">密码 <span class="required">*</span></label>
              <input 
                v-model="form.password" 
                type="password" 
                class="input" 
                placeholder="设置密码" 
                required 
              />
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">真实姓名</label>
            <input 
              v-model="form.real_name" 
              type="text" 
              class="input" 
              placeholder="您的姓名（用于访问登记）" 
            />
          </div>

          <div class="form-row">
            <div class="form-group">
              <label class="form-label">手机号码</label>
              <input 
                v-model="form.phone" 
                type="tel" 
                class="input" 
                placeholder="联系电话" 
              />
            </div>

            <div class="form-group">
              <label class="form-label">邮箱</label>
              <input 
                v-model="form.email" 
                type="email" 
                class="input" 
                placeholder="电子邮箱" 
              />
            </div>
          </div>

          <button type="submit" class="btn btn-primary btn-lg btn-block" :disabled="loading">
            <span v-if="loading" class="btn-spinner"></span>
            <span v-else>创建账户</span>
          </button>

          <p v-if="error" class="error-msg">{{ error }}</p>
        </form>

        <div class="auth-footer">
          <p>已有账户？ <router-link to="/login" class="link">立即登录</router-link></p>
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

const form = ref({
  username: '',
  password: '',
  real_name: '',
  phone: '',
  email: '',
  role: 'visitor'
})
const loading = ref(false)
const error = ref('')

const handleRegister = async () => {
  loading.value = true
  error.value = ''

  try {
    await authApi.register(form.value)
    showToast('success', '注册成功，请登录')
    router.push('/login')
  } catch (err) {
    error.value = err.message || '注册失败'
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

.bg-layer {
  position: fixed;
  inset: 0;
  z-index: 0;
}

.bg-gradient {
  position: absolute;
  inset: 0;
  background: 
    radial-gradient(ellipse 80% 50% at 50% 0%, rgba(249, 115, 22, 0.08) 0%, transparent 50%),
    radial-gradient(ellipse 60% 40% at 0% 100%, rgba(99, 102, 241, 0.1) 0%, transparent 50%),
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
  background: linear-gradient(135deg, rgba(249, 115, 22, 0.08) 0%, rgba(99, 102, 241, 0.05) 100%);
  animation: float 30s ease-in-out infinite;
}

.shape-1 {
  width: 350px;
  height: 350px;
  top: -80px;
  left: -100px;
}

.shape-2 {
  width: 280px;
  height: 280px;
  bottom: -80px;
  right: -80px;
  animation-delay: -15s;
}

@keyframes float {
  0%, 100% { transform: translate(0, 0) rotate(0deg); }
  50% { transform: translate(20px, -20px) rotate(5deg); }
}

.auth-card {
  position: relative;
  z-index: 1;
  background: rgba(24, 24, 27, 0.8);
  backdrop-filter: blur(40px);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-2xl);
  padding: 3rem;
  width: 100%;
  max-width: 560px;
  min-width: 420px;
  box-shadow: var(--shadow-xl);
  animation: cardAppear 0.5s cubic-bezier(0.16, 1, 0.3, 1);
}

@media (max-width: 600px) {
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
  background: linear-gradient(90deg, var(--accent-500), var(--primary-500));
  border-radius: 0 0 2px 2px;
}

@keyframes cardAppear {
  from { opacity: 0; transform: translateY(20px); }
  to { opacity: 1; transform: translateY(0); }
}

.auth-header {
  text-align: center;
  margin-bottom: 2rem;
}

.auth-logo {
  width: 4rem;
  height: 4rem;
  margin: 0 auto 1.25rem;
  background: linear-gradient(135deg, var(--accent-500) 0%, var(--accent-600) 100%);
  border-radius: var(--radius-xl);
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 8px 24px -4px rgba(249, 115, 22, 0.4);
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

.auth-form {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1rem;
}

@media (max-width: 480px) {
  .form-row {
    grid-template-columns: 1fr;
  }
}

.required {
  color: var(--error-400);
}

.btn-block {
  width: 100%;
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

.auth-footer {
  text-align: center;
  margin-top: 1.5rem;
  padding-top: 1.5rem;
  border-top: 1px solid var(--gray-800);
}

.auth-footer p {
  color: var(--gray-500);
  font-size: 0.9375rem;
}

.link {
  color: var(--primary-400);
  text-decoration: none;
  font-weight: 500;
}

.link:hover {
  text-decoration: underline;
}
</style>
