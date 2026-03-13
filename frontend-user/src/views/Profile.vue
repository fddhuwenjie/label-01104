<template>
  <div class="page">
    <div class="container">
      <h1 class="page-title">个人中心</h1>
      
      <div class="profile-card">
        <div class="profile-header">
          <div class="avatar">{{ user?.real_name?.charAt(0) || user?.username?.charAt(0) || 'U' }}</div>
          <div class="profile-info">
            <h2>{{ user?.real_name || user?.username }}</h2>
            <p class="profile-role">{{ getRoleName(user?.role) }}</p>
          </div>
        </div>
        
        <div class="profile-details">
          <div class="detail-item">
            <span class="detail-label">用户名</span>
            <span class="detail-value">{{ user?.username }}</span>
          </div>
          <div class="detail-item">
            <span class="detail-label">用户ID</span>
            <span class="detail-value">{{ user?.id }}</span>
          </div>
          <div class="detail-item">
            <span class="detail-label">手机号码</span>
            <span class="detail-value">{{ user?.phone || '未设置' }}</span>
          </div>
          <div class="detail-item">
            <span class="detail-label">邮箱</span>
            <span class="detail-value">{{ user?.email || '未设置' }}</span>
          </div>
          <div class="detail-item">
            <span class="detail-label">部门</span>
            <span class="detail-value">{{ user?.department || '未设置' }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'

const user = ref(null)

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

onMounted(() => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    user.value = JSON.parse(userStr)
  }
})
</script>

<style scoped>
.page {
  display: flex;
  justify-content: center;
  padding: 3rem 1rem;
}

.page .container {
  width: 100%;
  max-width: 560px;
}

.page-title {
  text-align: center;
  margin-bottom: 2rem;
}

.profile-card {
  background: rgba(39, 39, 42, 0.8);
  backdrop-filter: blur(20px);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-xl);
  overflow: hidden;
}

.profile-header {
  display: flex;
  align-items: center;
  gap: 1.5rem;
  padding: 2rem;
  background: linear-gradient(135deg, var(--primary-600) 0%, var(--primary-800) 100%);
  color: white;
}

.avatar {
  width: 5rem;
  height: 5rem;
  background: rgba(255,255,255,0.2);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 2rem;
  font-weight: 700;
}

.profile-info h2 {
  font-size: 1.5rem;
  margin-bottom: 0.25rem;
}

.profile-role {
  opacity: 0.9;
}

.profile-details {
  padding: 1.5rem 2rem;
}

.detail-item {
  display: flex;
  justify-content: space-between;
  padding: 1rem 0;
  border-bottom: 1px solid var(--gray-800);
}

.detail-item:last-child {
  border-bottom: none;
}

.detail-label {
  color: var(--gray-500);
}

.detail-value {
  font-weight: 500;
  color: var(--gray-200);
}
</style>
