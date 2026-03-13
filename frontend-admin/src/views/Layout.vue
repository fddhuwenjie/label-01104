<template>
  <div class="layout">
    <!-- 侧边栏 -->
    <aside class="sidebar">
      <div class="sidebar-header">
        <div class="logo">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <path d="M3 21h18M5 21V7l7-4 7 4v14M9 21v-6h6v6" stroke-linecap="round" stroke-linejoin="round"/>
            <path d="M9 9h.01M15 9h.01M9 13h.01M15 13h.01" stroke-linecap="round"/>
          </svg>
        </div>
        <div class="logo-text">
          <span class="logo-title">VAS</span>
          <span class="logo-subtitle">Admin</span>
        </div>
      </div>
      
      <nav class="sidebar-nav">
        <div class="nav-section">
          <span class="nav-section-title">概览</span>
          <router-link to="/" class="nav-item" :class="{ active: $route.path === '/' }">
            <span class="nav-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M3 9l9-7 9 7v11a2 2 0 01-2 2H5a2 2 0 01-2-2V9z"/>
                <path d="M9 22V12h6v10"/>
              </svg>
            </span>
            <span class="nav-text">数据中心</span>
          </router-link>
        </div>

        <div class="nav-section">
          <span class="nav-section-title">管理</span>
          <router-link to="/users" class="nav-item" :class="{ active: $route.path === '/users' }">
            <span class="nav-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M17 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2"/>
                <circle cx="9" cy="7" r="4"/>
                <path d="M23 21v-2a4 4 0 00-3-3.87M16 3.13a4 4 0 010 7.75"/>
              </svg>
            </span>
            <span class="nav-text">用户管理</span>
          </router-link>
          <router-link to="/appointments" class="nav-item" :class="{ active: $route.path === '/appointments' }">
            <span class="nav-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <rect x="3" y="4" width="18" height="18" rx="2" ry="2"/>
                <line x1="16" y1="2" x2="16" y2="6"/>
                <line x1="8" y1="2" x2="8" y2="6"/>
                <line x1="3" y1="10" x2="21" y2="10"/>
              </svg>
            </span>
            <span class="nav-text">预约管理</span>
            <span v-if="pendingCount > 0" class="nav-badge">{{ pendingCount }}</span>
          </router-link>
          <router-link to="/check-records" class="nav-item" :class="{ active: $route.path === '/check-records' }">
            <span class="nav-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M9 11l3 3L22 4"/>
                <path d="M21 12v7a2 2 0 01-2 2H5a2 2 0 01-2-2V5a2 2 0 012-2h11"/>
              </svg>
            </span>
            <span class="nav-text">签到记录</span>
          </router-link>
        </div>
      </nav>
      
      <div class="sidebar-footer">
        <div class="user-card">
          <div class="user-avatar">
            <span>{{ user?.real_name?.charAt(0) || user?.username?.charAt(0) || 'A' }}</span>
          </div>
          <div class="user-info">
            <p class="user-name">{{ user?.real_name || user?.username }}</p>
            <p class="user-role">{{ getRoleName(user?.role) }}</p>
          </div>
          <button @click="handleLogout" class="logout-btn" title="退出登录">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4"/>
              <polyline points="16,17 21,12 16,7"/>
              <line x1="21" y1="12" x2="9" y2="12"/>
            </svg>
          </button>
        </div>
      </div>
    </aside>
    
    <!-- 主内容区 -->
    <main class="main-content">
      <div class="content-wrapper">
        <router-view />
      </div>
    </main>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { statsApi } from '../api'

const router = useRouter()
const user = ref(null)
const pendingCount = ref(0)

onMounted(async () => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    user.value = JSON.parse(userStr)
  }
  
  // 获取待审批数量
  try {
    const res = await statsApi.overview()
    pendingCount.value = res.data?.pending_appointments || 0
  } catch (err) {
    console.error('Failed to load stats:', err)
  }
})

const getRoleName = (role) => {
  const roles = {
    superadmin: '超级管理员',
    admin: '管理员',
    employee: '员工',
    security: '保安',
    visitor: '访客'
  }
  return roles[role] || role
}

const handleLogout = () => {
  localStorage.removeItem('token')
  localStorage.removeItem('user')
  router.push('/login')
}
</script>

<style scoped>
.layout {
  display: flex;
  min-height: 100vh;
  background: var(--gray-950);
}

/* 侧边栏 */
.sidebar {
  width: 260px;
  background: var(--gray-900);
  border-right: 1px solid var(--gray-800);
  display: flex;
  flex-direction: column;
  position: fixed;
  top: 0;
  left: 0;
  bottom: 0;
  z-index: 100;
}

.sidebar-header {
  padding: 1.5rem;
  display: flex;
  align-items: center;
  gap: 0.875rem;
  border-bottom: 1px solid var(--gray-800);
}

.logo {
  width: 2.5rem;
  height: 2.5rem;
  background: linear-gradient(135deg, var(--primary-500) 0%, var(--primary-700) 100%);
  border-radius: 0.625rem;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.logo svg {
  width: 1.25rem;
  height: 1.25rem;
  color: white;
}

.logo-text {
  display: flex;
  flex-direction: column;
}

.logo-title {
  font-size: 1.125rem;
  font-weight: 700;
  color: var(--gray-50);
  letter-spacing: -0.025em;
}

.logo-subtitle {
  font-size: 0.75rem;
  color: var(--gray-500);
  font-weight: 500;
}

/* 导航 */
.sidebar-nav {
  flex: 1;
  padding: 1rem 0;
  overflow-y: auto;
}

.nav-section {
  margin-bottom: 1rem;
}

.nav-section-title {
  display: block;
  padding: 0.5rem 1.5rem;
  font-size: 0.6875rem;
  font-weight: 600;
  color: var(--gray-500);
  text-transform: uppercase;
  letter-spacing: 0.1em;
}

.nav-item {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  padding: 0.75rem 1.5rem;
  margin: 0.125rem 0.5rem;
  color: var(--gray-400);
  text-decoration: none;
  font-size: 0.9375rem;
  font-weight: 500;
  border-radius: 0.5rem;
  transition: all 0.15s ease;
  position: relative;
}

.nav-item:hover {
  background: var(--gray-800);
  color: var(--gray-200);
}

.nav-item.active {
  background: linear-gradient(135deg, rgba(99, 102, 241, 0.15) 0%, rgba(99, 102, 241, 0.05) 100%);
  color: var(--primary-400);
}

.nav-item.active::before {
  content: '';
  position: absolute;
  left: 0;
  top: 50%;
  transform: translateY(-50%);
  width: 3px;
  height: 60%;
  background: var(--primary-500);
  border-radius: 0 2px 2px 0;
}

.nav-icon {
  display: flex;
  width: 1.25rem;
  height: 1.25rem;
}

.nav-icon svg {
  width: 100%;
  height: 100%;
}

.nav-text {
  flex: 1;
}

.nav-badge {
  background: var(--accent-500);
  color: white;
  font-size: 0.6875rem;
  font-weight: 600;
  padding: 0.125rem 0.5rem;
  border-radius: 9999px;
  min-width: 1.25rem;
  text-align: center;
}

/* 底部用户卡片 */
.sidebar-footer {
  padding: 1rem;
  border-top: 1px solid var(--gray-800);
}

.user-card {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  padding: 0.75rem;
  background: var(--gray-800);
  border-radius: 0.75rem;
}

.user-avatar {
  width: 2.25rem;
  height: 2.25rem;
  background: linear-gradient(135deg, var(--accent-500) 0%, var(--accent-600) 100%);
  border-radius: 0.5rem;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 600;
  font-size: 0.875rem;
  color: white;
  flex-shrink: 0;
}

.user-info {
  flex: 1;
  min-width: 0;
}

.user-name {
  font-size: 0.875rem;
  font-weight: 600;
  color: var(--gray-200);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.user-role {
  font-size: 0.75rem;
  color: var(--gray-500);
}

.logout-btn {
  width: 2rem;
  height: 2rem;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  border-radius: 0.375rem;
  color: var(--gray-500);
  cursor: pointer;
  transition: all 0.15s ease;
}

.logout-btn:hover {
  background: var(--gray-700);
  color: var(--error-400);
}

.logout-btn svg {
  width: 1.125rem;
  height: 1.125rem;
}

/* 主内容区 */
.main-content {
  flex: 1;
  margin-left: 260px;
  min-height: 100vh;
  background: 
    radial-gradient(ellipse 80% 50% at 50% -10%, rgba(99, 102, 241, 0.08) 0%, transparent 50%),
    var(--gray-950);
}

.content-wrapper {
  padding: 2rem;
  max-width: 1400px;
}

/* 响应式 */
@media (max-width: 768px) {
  .sidebar {
    width: 100%;
    position: fixed;
    bottom: 0;
    top: auto;
    height: auto;
    flex-direction: row;
    border-right: none;
    border-top: 1px solid var(--gray-800);
  }
  
  .sidebar-header,
  .sidebar-footer,
  .nav-section-title {
    display: none;
  }
  
  .sidebar-nav {
    display: flex;
    justify-content: space-around;
    padding: 0.5rem;
    width: 100%;
  }
  
  .nav-section {
    display: contents;
    margin: 0;
  }
  
  .nav-item {
    flex-direction: column;
    gap: 0.25rem;
    padding: 0.5rem;
    margin: 0;
    font-size: 0.75rem;
  }
  
  .nav-item.active::before {
    display: none;
  }
  
  .main-content {
    margin-left: 0;
    margin-bottom: 60px;
  }
  
  .content-wrapper {
    padding: 1rem;
  }
}
</style>
