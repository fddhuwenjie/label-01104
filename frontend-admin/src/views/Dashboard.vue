<template>
  <div class="dashboard">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="welcome-section">
          <h1 class="page-title">
            {{ getGreeting() }}，{{ user?.real_name || user?.username }}
          </h1>
          <p class="page-subtitle">欢迎回到访客预约管理系统</p>
        </div>
        <div class="header-date">
          <span class="date-icon">📅</span>
          <span>{{ currentDate }}</span>
        </div>
      </div>
    </div>
    
    <!-- 统计卡片 -->
    <div class="stats-grid">
      <div class="stat-card" v-for="(stat, index) in statCards" :key="index">
        <div class="stat-icon" :class="stat.color">{{ stat.icon }}</div>
        <div class="stat-content">
          <div class="stat-value">{{ stats[stat.key] || 0 }}</div>
          <div class="stat-label">{{ stat.label }}</div>
        </div>
        <div class="stat-trend" v-if="stat.trend">
          <span :class="stat.trend > 0 ? 'trend-up' : 'trend-down'">
            {{ stat.trend > 0 ? '↑' : '↓' }} {{ Math.abs(stat.trend) }}%
          </span>
        </div>
      </div>
    </div>
    
    <!-- 双列布局 -->
    <div class="dashboard-grid">
      <!-- 左侧 - 用户分布 -->
      <div class="card">
        <div class="card-header">
          <h3 class="card-title">
            <span class="title-icon">👥</span>
            用户角色分布
          </h3>
        </div>
        <div class="card-body">
          <div class="distribution-list">
            <div class="distribution-item" v-for="role in roleDistribution" :key="role.key">
              <div class="distribution-info">
                <span class="distribution-dot" :style="{ background: role.color }"></span>
                <span class="distribution-name">{{ role.name }}</span>
              </div>
              <div class="distribution-bar-wrapper">
                <div class="distribution-bar" :style="{ 
                  width: getPercentage(stats[role.key], stats.total_users) + '%',
                  background: role.color 
                }"></div>
              </div>
              <span class="distribution-count">{{ stats[role.key] || 0 }}</span>
            </div>
          </div>
        </div>
      </div>
      
      <!-- 右侧 - 预约状态 -->
      <div class="card">
        <div class="card-header">
          <h3 class="card-title">
            <span class="title-icon">📊</span>
            预约状态统计
          </h3>
        </div>
        <div class="card-body">
          <div class="status-grid">
            <div class="status-card" v-for="status in statusCards" :key="status.key">
              <div class="status-icon" :style="{ background: status.bgColor }">
                {{ status.icon }}
              </div>
              <div class="status-info">
                <div class="status-value">{{ stats[status.key] || 0 }}</div>
                <div class="status-label">{{ status.label }}</div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    
    <!-- 快捷操作 -->
    <div class="card">
      <div class="card-header">
        <h3 class="card-title">
          <span class="title-icon">⚡</span>
          快捷操作
        </h3>
      </div>
      <div class="card-body">
        <div class="quick-actions">
          <router-link to="/users" class="action-card">
            <div class="action-icon blue">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M16 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2"/>
                <circle cx="8.5" cy="7" r="4"/>
                <line x1="20" y1="8" x2="20" y2="14"/>
                <line x1="23" y1="11" x2="17" y2="11"/>
              </svg>
            </div>
            <div class="action-content">
              <span class="action-label">添加用户</span>
              <span class="action-desc">创建新的系统用户</span>
            </div>
            <span class="action-arrow">→</span>
          </router-link>
          
          <router-link to="/appointments" class="action-card">
            <div class="action-icon orange">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M9 11l3 3L22 4"/>
                <path d="M21 12v7a2 2 0 01-2 2H5a2 2 0 01-2-2V5a2 2 0 012-2h11"/>
              </svg>
            </div>
            <div class="action-content">
              <span class="action-label">审批预约</span>
              <span class="action-desc">处理待审批的预约请求</span>
            </div>
            <span class="action-arrow">→</span>
          </router-link>
          
          <router-link to="/check-records" class="action-card">
            <div class="action-icon green">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M14 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V8z"/>
                <polyline points="14,2 14,8 20,8"/>
                <line x1="16" y1="13" x2="8" y2="13"/>
                <line x1="16" y1="17" x2="8" y2="17"/>
              </svg>
            </div>
            <div class="action-content">
              <span class="action-label">查看记录</span>
              <span class="action-desc">浏览访客签到签退记录</span>
            </div>
            <span class="action-arrow">→</span>
          </router-link>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { statsApi } from '../api'

const user = ref(null)
const stats = ref({})

const currentDate = computed(() => {
  const now = new Date()
  const options = { year: 'numeric', month: 'long', day: 'numeric', weekday: 'long' }
  return now.toLocaleDateString('zh-CN', options)
})

const statCards = [
  { key: 'total_users', label: '总用户数', icon: '👥', color: 'blue' },
  { key: 'total_appointments', label: '总预约数', icon: '📅', color: 'green' },
  { key: 'pending_appointments', label: '待审批', icon: '⏳', color: 'orange' },
  { key: 'today_appointments', label: '今日预约', icon: '📆', color: 'purple' }
]

const roleDistribution = [
  { key: 'superadmin_count', name: '管理员', color: '#6366f1' },
  { key: 'employee_count', name: '员工', color: '#10b981' },
  { key: 'security_count', name: '保安', color: '#f97316' },
  { key: 'visitor_count', name: '访客', color: '#8b5cf6' }
]

const statusCards = [
  { key: 'pending_appointments', label: '待审批', icon: '⏳', bgColor: 'rgba(245, 158, 11, 0.15)' },
  { key: 'approved_appointments', label: '已批准', icon: '✓', bgColor: 'rgba(16, 185, 129, 0.15)' },
  { key: 'checked_in_count', label: '已签到', icon: '📍', bgColor: 'rgba(99, 102, 241, 0.15)' },
  { key: 'checked_out_count', label: '已签退', icon: '🚪', bgColor: 'rgba(113, 113, 122, 0.15)' }
]

const getGreeting = () => {
  const hour = new Date().getHours()
  if (hour < 12) return '早上好'
  if (hour < 18) return '下午好'
  return '晚上好'
}

const getPercentage = (value, total) => {
  if (!total || !value) return 0
  return Math.round((value / total) * 100)
}

onMounted(async () => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    user.value = JSON.parse(userStr)
  }
  
  try {
    const res = await statsApi.overview()
    stats.value = res.data
    // 计算管理员总数
    stats.value.superadmin_count = (res.data.superadmin_count || 0) + (res.data.admin_count || 0)
  } catch (err) {
    console.error('Failed to load stats:', err)
  }
})
</script>

<style scoped>
.dashboard {
  animation: fadeIn 0.4s ease-out;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(10px); }
  to { opacity: 1; transform: translateY(0); }
}

/* 页面头部 */
.page-header {
  margin-bottom: 2rem;
}

.header-content {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
}

.page-title {
  font-size: 1.75rem;
  font-weight: 700;
  color: var(--gray-50);
  margin-bottom: 0.25rem;
  letter-spacing: -0.025em;
}

.page-subtitle {
  color: var(--gray-400);
  font-size: 0.9375rem;
}

.header-date {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.625rem 1rem;
  background: var(--gray-900);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-lg);
  font-size: 0.875rem;
  color: var(--gray-400);
}

.date-icon {
  font-size: 1rem;
}

/* 统计网格 */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 1rem;
  margin-bottom: 1.5rem;
}

@media (max-width: 1024px) {
  .stats-grid {
    grid-template-columns: repeat(2, 1fr);
  }
}

@media (max-width: 640px) {
  .stats-grid {
    grid-template-columns: 1fr;
  }
}

/* 仪表板网格 */
.dashboard-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 1.5rem;
  margin-bottom: 1.5rem;
}

@media (max-width: 1024px) {
  .dashboard-grid {
    grid-template-columns: 1fr;
  }
}

/* 卡片标题 */
.title-icon {
  margin-right: 0.5rem;
}

/* 分布列表 */
.distribution-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.distribution-item {
  display: flex;
  align-items: center;
  gap: 1rem;
}

.distribution-info {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  width: 80px;
  flex-shrink: 0;
}

.distribution-dot {
  width: 0.5rem;
  height: 0.5rem;
  border-radius: 50%;
}

.distribution-name {
  font-size: 0.875rem;
  color: var(--gray-400);
}

.distribution-bar-wrapper {
  flex: 1;
  height: 8px;
  background: var(--gray-800);
  border-radius: 4px;
  overflow: hidden;
}

.distribution-bar {
  height: 100%;
  border-radius: 4px;
  transition: width 0.6s cubic-bezier(0.4, 0, 0.2, 1);
}

.distribution-count {
  font-size: 0.875rem;
  font-weight: 600;
  color: var(--gray-300);
  width: 32px;
  text-align: right;
}

/* 状态网格 */
.status-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 1rem;
}

.status-card {
  display: flex;
  align-items: center;
  gap: 0.875rem;
  padding: 1rem;
  background: var(--gray-900);
  border-radius: var(--radius-lg);
  border: 1px solid var(--gray-800);
}

.status-icon {
  width: 2.5rem;
  height: 2.5rem;
  border-radius: var(--radius);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1rem;
}

.status-value {
  font-size: 1.25rem;
  font-weight: 700;
  color: var(--gray-100);
}

.status-label {
  font-size: 0.75rem;
  color: var(--gray-500);
}

/* 快捷操作 */
.quick-actions {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 1rem;
}

@media (max-width: 768px) {
  .quick-actions {
    grid-template-columns: 1fr;
  }
}

.action-card {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1.25rem;
  background: var(--gray-900);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-xl);
  text-decoration: none;
  transition: all 0.2s ease;
}

.action-card:hover {
  background: var(--gray-800);
  border-color: var(--gray-700);
  transform: translateY(-2px);
}

.action-icon {
  width: 3rem;
  height: 3rem;
  border-radius: var(--radius-lg);
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.action-icon svg {
  width: 1.25rem;
  height: 1.25rem;
}

.action-icon.blue {
  background: linear-gradient(135deg, rgba(99, 102, 241, 0.2) 0%, rgba(99, 102, 241, 0.1) 100%);
  color: var(--primary-400);
}

.action-icon.orange {
  background: linear-gradient(135deg, rgba(249, 115, 22, 0.2) 0%, rgba(249, 115, 22, 0.1) 100%);
  color: var(--accent-400);
}

.action-icon.green {
  background: linear-gradient(135deg, rgba(16, 185, 129, 0.2) 0%, rgba(16, 185, 129, 0.1) 100%);
  color: var(--success-400);
}

.action-content {
  flex: 1;
  display: flex;
  flex-direction: column;
}

.action-label {
  font-size: 0.9375rem;
  font-weight: 600;
  color: var(--gray-200);
}

.action-desc {
  font-size: 0.75rem;
  color: var(--gray-500);
  margin-top: 0.125rem;
}

.action-arrow {
  color: var(--gray-600);
  font-size: 1.125rem;
  transition: transform 0.2s ease;
}

.action-card:hover .action-arrow {
  transform: translateX(4px);
  color: var(--gray-400);
}
</style>
