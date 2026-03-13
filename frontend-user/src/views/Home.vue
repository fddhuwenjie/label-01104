<template>
  <div class="home">
    <!-- Hero Section -->
    <section class="hero">
      <div class="hero-bg">
        <div class="hero-gradient"></div>
        <div class="hero-grid"></div>
        <div class="hero-glow glow-1"></div>
        <div class="hero-glow glow-2"></div>
      </div>
      
      <div class="container hero-content">
        <div class="hero-badge">
          <span class="badge-dot"></span>
          企业级访客管理平台
        </div>
        <h1 class="hero-title">
          智能访客<br/>
          <span class="gradient-text">预约系统</span>
        </h1>
        <p class="hero-subtitle">
          便捷高效的访客管理解决方案，一键预约，轻松到访。
          提供完整的访客登记、审批、签到签退全流程管理。
        </p>
        <div class="hero-actions">
          <router-link v-if="!user" to="/register" class="btn btn-primary btn-lg">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="btn-icon">
              <path d="M16 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2"/>
              <circle cx="8.5" cy="7" r="4"/>
              <line x1="20" y1="8" x2="20" y2="14"/>
              <line x1="23" y1="11" x2="17" y2="11"/>
            </svg>
            立即注册
          </router-link>
          <router-link v-else to="/new-appointment" class="btn btn-primary btn-lg">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="btn-icon">
              <rect x="3" y="4" width="18" height="18" rx="2" ry="2"/>
              <line x1="16" y1="2" x2="16" y2="6"/>
              <line x1="8" y1="2" x2="8" y2="6"/>
              <line x1="3" y1="10" x2="21" y2="10"/>
            </svg>
            立即预约
          </router-link>
          <router-link v-if="!user" to="/login" class="btn btn-secondary btn-lg">
            登录账户
          </router-link>
          <router-link v-else to="/appointments" class="btn btn-secondary btn-lg">
            我的预约
          </router-link>
        </div>

        <!-- Stats -->
        <div class="hero-stats">
          <div class="stat-item">
            <div class="stat-number">99.9%</div>
            <div class="stat-text">系统可用率</div>
          </div>
          <div class="stat-divider"></div>
          <div class="stat-item">
            <div class="stat-number">3s</div>
            <div class="stat-text">快速签到</div>
          </div>
          <div class="stat-divider"></div>
          <div class="stat-item">
            <div class="stat-number">24/7</div>
            <div class="stat-text">全天候服务</div>
          </div>
        </div>
      </div>
    </section>

    <!-- Features Section -->
    <section class="features">
      <div class="container">
        <div class="section-header">
          <span class="section-tag">核心功能</span>
          <h2 class="section-title">为什么选择我们？</h2>
          <p class="section-desc">全方位的访客管理解决方案，让访客接待更高效</p>
        </div>
        
        <div class="feature-grid">
          <div class="feature-card" v-for="(feature, index) in features" :key="index">
            <div class="feature-icon" :style="{ background: feature.bgColor }">
              {{ feature.icon }}
            </div>
            <h3 class="feature-title">{{ feature.title }}</h3>
            <p class="feature-desc">{{ feature.desc }}</p>
          </div>
        </div>
      </div>
    </section>

    <!-- How it works -->
    <section class="process">
      <div class="container">
        <div class="section-header">
          <span class="section-tag">使用流程</span>
          <h2 class="section-title">简单三步，完成预约</h2>
        </div>

        <div class="process-grid">
          <div class="process-item" v-for="(step, index) in processSteps" :key="index">
            <div class="process-number">{{ String(index + 1).padStart(2, '0') }}</div>
            <div class="process-content">
              <h3 class="process-title">{{ step.title }}</h3>
              <p class="process-desc">{{ step.desc }}</p>
            </div>
          </div>
        </div>
      </div>
    </section>

    <!-- CTA Section -->
    <section class="cta">
      <div class="container">
        <div class="cta-card">
          <div class="cta-content">
            <h2 class="cta-title">准备好开始了吗？</h2>
            <p class="cta-desc">立即注册账户，体验便捷的访客预约服务</p>
          </div>
          <router-link v-if="!user" to="/register" class="btn btn-primary btn-lg">
            免费注册
          </router-link>
          <router-link v-else to="/appointments" class="btn btn-primary btn-lg">
            查看预约
          </router-link>
        </div>
      </div>
    </section>
  </div>
</template>

<script setup>
import { inject } from 'vue'

const user = inject('user')

const features = [
  {
    icon: '📝',
    title: '便捷预约',
    desc: '在线提交访问申请，无需电话沟通，随时随地完成预约',
    bgColor: 'rgba(99, 102, 241, 0.15)'
  },
  {
    icon: '⚡',
    title: '快速审批',
    desc: '管理员在线审批，实时通知结果，高效处理访问请求',
    bgColor: 'rgba(249, 115, 22, 0.15)'
  },
  {
    icon: '🔒',
    title: '安全可靠',
    desc: '完善的身份验证机制，保障企业访客管理安全',
    bgColor: 'rgba(16, 185, 129, 0.15)'
  },
  {
    icon: '📊',
    title: '记录追溯',
    desc: '完整的访问记录，便于查询和统计分析',
    bgColor: 'rgba(168, 85, 247, 0.15)'
  }
]

const processSteps = [
  {
    title: '注册账户',
    desc: '填写基本信息，创建您的访客账户'
  },
  {
    title: '提交预约',
    desc: '选择被访人员和访问时间，提交预约申请'
  },
  {
    title: '到访签到',
    desc: '预约审批通过后，到访时完成签到登记'
  }
]
</script>

<style scoped>
.home {
  overflow-x: hidden;
}

/* Hero Section */
.hero {
  position: relative;
  min-height: 90vh;
  display: flex;
  align-items: center;
  padding: 6rem 0;
}

.hero-bg {
  position: absolute;
  inset: 0;
  z-index: 0;
  overflow: hidden;
}

.hero-gradient {
  position: absolute;
  inset: 0;
  background: 
    radial-gradient(ellipse 60% 50% at 50% 0%, rgba(99, 102, 241, 0.15) 0%, transparent 60%),
    var(--gray-950);
}

.hero-grid {
  position: absolute;
  inset: 0;
  background-image: 
    linear-gradient(rgba(99, 102, 241, 0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(99, 102, 241, 0.03) 1px, transparent 1px);
  background-size: 80px 80px;
  mask-image: radial-gradient(ellipse at center, black 0%, transparent 70%);
}

.hero-glow {
  position: absolute;
  border-radius: 50%;
  filter: blur(100px);
  animation: pulse 8s ease-in-out infinite;
}

.glow-1 {
  width: 600px;
  height: 600px;
  background: rgba(99, 102, 241, 0.2);
  top: -200px;
  left: 50%;
  transform: translateX(-50%);
}

.glow-2 {
  width: 400px;
  height: 400px;
  background: rgba(168, 85, 247, 0.15);
  bottom: -100px;
  right: 10%;
  animation-delay: -4s;
}

@keyframes pulse {
  0%, 100% { opacity: 0.5; transform: translateX(-50%) scale(1); }
  50% { opacity: 0.8; transform: translateX(-50%) scale(1.1); }
}

.hero-content {
  position: relative;
  z-index: 1;
  text-align: center;
  animation: fadeUp 0.8s cubic-bezier(0.16, 1, 0.3, 1);
}

@keyframes fadeUp {
  from { opacity: 0; transform: translateY(30px); }
  to { opacity: 1; transform: translateY(0); }
}

.hero-badge {
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.5rem 1rem;
  background: rgba(99, 102, 241, 0.1);
  border: 1px solid rgba(99, 102, 241, 0.2);
  border-radius: 9999px;
  font-size: 0.8125rem;
  font-weight: 500;
  color: var(--primary-300);
  margin-bottom: 2rem;
}

.badge-dot {
  width: 0.5rem;
  height: 0.5rem;
  background: var(--primary-400);
  border-radius: 50%;
  animation: blink 2s ease-in-out infinite;
}

@keyframes blink {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.4; }
}

.hero-title {
  font-size: clamp(2.5rem, 8vw, 4.5rem);
  font-weight: 800;
  color: var(--gray-50);
  line-height: 1.1;
  letter-spacing: -0.04em;
  margin-bottom: 1.5rem;
}

.gradient-text {
  background: linear-gradient(135deg, var(--primary-400) 0%, var(--accent-400) 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.hero-subtitle {
  font-size: 1.125rem;
  color: var(--gray-400);
  max-width: 600px;
  margin: 0 auto 2.5rem;
  line-height: 1.7;
}

.hero-actions {
  display: flex;
  justify-content: center;
  gap: 1rem;
  margin-bottom: 4rem;
}

.btn-icon {
  width: 1.125rem;
  height: 1.125rem;
}

.hero-stats {
  display: inline-flex;
  align-items: center;
  gap: 2rem;
  padding: 1.5rem 2.5rem;
  background: rgba(39, 39, 42, 0.6);
  backdrop-filter: blur(20px);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-2xl);
}

.stat-item {
  text-align: center;
}

.stat-number {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--gray-50);
  margin-bottom: 0.25rem;
}

.stat-text {
  font-size: 0.8125rem;
  color: var(--gray-500);
}

.stat-divider {
  width: 1px;
  height: 40px;
  background: var(--gray-700);
}

/* Features Section */
.features {
  padding: 6rem 0;
}

.section-header {
  text-align: center;
  margin-bottom: 4rem;
}

.section-tag {
  display: inline-block;
  padding: 0.375rem 1rem;
  background: rgba(99, 102, 241, 0.1);
  color: var(--primary-400);
  font-size: 0.8125rem;
  font-weight: 600;
  border-radius: 9999px;
  margin-bottom: 1rem;
}

.section-title {
  font-size: 2.25rem;
  font-weight: 700;
  color: var(--gray-50);
  margin-bottom: 1rem;
  letter-spacing: -0.025em;
}

.section-desc {
  font-size: 1.0625rem;
  color: var(--gray-400);
}

.feature-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 1.5rem;
}

@media (max-width: 1024px) {
  .feature-grid {
    grid-template-columns: repeat(2, 1fr);
  }
}

@media (max-width: 640px) {
  .feature-grid {
    grid-template-columns: 1fr;
  }
}

.feature-card {
  background: rgba(39, 39, 42, 0.5);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-xl);
  padding: 2rem;
  transition: all 0.3s ease;
}

.feature-card:hover {
  background: rgba(39, 39, 42, 0.8);
  border-color: var(--gray-700);
  transform: translateY(-4px);
}

.feature-icon {
  width: 3.5rem;
  height: 3.5rem;
  border-radius: var(--radius-lg);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1.5rem;
  margin-bottom: 1.25rem;
}

.feature-title {
  font-size: 1.125rem;
  font-weight: 600;
  color: var(--gray-100);
  margin-bottom: 0.5rem;
}

.feature-desc {
  font-size: 0.9375rem;
  color: var(--gray-400);
  line-height: 1.6;
}

/* Process Section */
.process {
  padding: 6rem 0;
  background: var(--gray-900);
}

.process-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 2rem;
}

@media (max-width: 768px) {
  .process-grid {
    grid-template-columns: 1fr;
  }
}

.process-item {
  text-align: center;
}

.process-number {
  font-size: 3rem;
  font-weight: 800;
  color: var(--gray-800);
  margin-bottom: 1rem;
  font-variant-numeric: tabular-nums;
}

.process-title {
  font-size: 1.25rem;
  font-weight: 600;
  color: var(--gray-100);
  margin-bottom: 0.5rem;
}

.process-desc {
  font-size: 0.9375rem;
  color: var(--gray-500);
}

/* CTA Section */
.cta {
  padding: 6rem 0;
}

.cta-card {
  background: linear-gradient(135deg, rgba(99, 102, 241, 0.15) 0%, rgba(168, 85, 247, 0.1) 100%);
  border: 1px solid rgba(99, 102, 241, 0.2);
  border-radius: var(--radius-2xl);
  padding: 4rem;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 2rem;
}

@media (max-width: 768px) {
  .cta-card {
    flex-direction: column;
    text-align: center;
    padding: 3rem 2rem;
  }
}

.cta-title {
  font-size: 1.75rem;
  font-weight: 700;
  color: var(--gray-50);
  margin-bottom: 0.5rem;
}

.cta-desc {
  font-size: 1rem;
  color: var(--gray-400);
}
</style>
