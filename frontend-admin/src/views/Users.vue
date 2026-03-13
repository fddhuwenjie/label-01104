<template>
  <div class="users-page">
    <div class="page-header">
      <h1 class="page-title">用户管理</h1>
      <button @click="openModal()" class="btn btn-primary">
        <span>+ 添加用户</span>
      </button>
    </div>
    
    <!-- 筛选 -->
    <div class="filter-bar">
      <select v-model="filter.role" class="select" @change="loadUsers">
        <option value="">全部角色</option>
        <option value="superadmin">超级管理员</option>
        <option value="admin">管理员</option>
        <option value="employee">员工</option>
        <option value="security">保安</option>
        <option value="visitor">访客</option>
      </select>
    </div>
    
    <!-- 用户列表 -->
    <div class="card">
      <div class="table-container">
        <table class="table">
          <thead>
            <tr>
              <th>用户名</th>
              <th>姓名</th>
              <th>角色</th>
              <th>部门</th>
              <th>电话</th>
              <th>状态</th>
              <th>创建时间</th>
              <th>操作</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="user in users" :key="user.id">
              <td>{{ user.username }}</td>
              <td>{{ user.real_name || '-' }}</td>
              <td><span :class="['badge', `badge-role-${user.role}`]">{{ getRoleName(user.role) }}</span></td>
              <td>{{ user.department || '-' }}</td>
              <td>{{ user.phone || '-' }}</td>
              <td>
                <span :class="['badge', user.status === 1 ? 'badge-approved' : 'badge-rejected']">
                  {{ user.status === 1 ? '启用' : '禁用' }}
                </span>
              </td>
              <td>{{ formatDate(user.created_at) }}</td>
              <td>
                <div class="action-btns">
                  <button @click="openModal(user)" class="btn btn-sm btn-secondary">编辑</button>
                  <button @click="handleDelete(user)" class="btn btn-sm btn-danger">删除</button>
                </div>
              </td>
            </tr>
          </tbody>
        </table>
        
        <div v-if="users.length === 0" class="empty-state">
          <div class="empty-state-icon">👥</div>
          <p>暂无用户数据</p>
        </div>
      </div>
      
      <!-- 分页 -->
      <div v-if="total > pageSize" class="pagination">
        <button :disabled="page === 1" @click="page--; loadUsers()">上一页</button>
        <span class="text-sm text-gray-500">第 {{ page }} 页 / 共 {{ Math.ceil(total / pageSize) }} 页</span>
        <button :disabled="page * pageSize >= total" @click="page++; loadUsers()">下一页</button>
      </div>
    </div>
    
    <!-- 添加/编辑用户模态框 -->
    <div v-if="showModal" class="modal-overlay" @click.self="showModal = false">
      <div class="modal">
        <div class="modal-header">
          <h3 class="modal-title">{{ editingUser ? '编辑用户' : '添加用户' }}</h3>
          <button class="modal-close" @click="showModal = false">&times;</button>
        </div>
        <form @submit.prevent="handleSubmit" class="modal-body">
          <div class="form-group">
            <label class="form-label">用户名 *</label>
            <input v-model="form.username" type="text" class="input" required :disabled="!!editingUser" />
          </div>
          <div v-if="!editingUser" class="form-group">
            <label class="form-label">密码</label>
            <input v-model="form.password" type="password" class="input" placeholder="默认: 123456" />
          </div>
          <div class="form-group">
            <label class="form-label">角色 *</label>
            <select v-model="form.role" class="select" required>
              <option value="admin">管理员</option>
              <option value="employee">员工</option>
              <option value="security">保安</option>
              <option value="visitor">访客</option>
            </select>
          </div>
          <div class="form-group">
            <label class="form-label">姓名</label>
            <input v-model="form.real_name" type="text" class="input" />
          </div>
          <div class="form-group">
            <label class="form-label">电话</label>
            <input v-model="form.phone" type="text" class="input" />
          </div>
          <div class="form-group">
            <label class="form-label">邮箱</label>
            <input v-model="form.email" type="email" class="input" />
          </div>
          <div class="form-group">
            <label class="form-label">部门</label>
            <input v-model="form.department" type="text" class="input" />
          </div>
          <div v-if="editingUser" class="form-group">
            <label class="form-label">状态</label>
            <select v-model="form.status" class="select">
              <option :value="1">启用</option>
              <option :value="0">禁用</option>
            </select>
          </div>
        </form>
        <div class="modal-footer">
          <button type="button" class="btn btn-secondary" @click="showModal = false">取消</button>
          <button type="submit" class="btn btn-primary" @click="handleSubmit">保存</button>
        </div>
      </div>
    </div>
    
    <!-- 删除确认弹窗 -->
    <div v-if="showDeleteModal" class="modal-overlay" @click.self="showDeleteModal = false">
      <div class="modal confirm-modal">
        <div class="modal-header">
          <h3 class="modal-title">删除用户</h3>
          <button class="modal-close" @click="showDeleteModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <div class="confirm-icon">⚠️</div>
          <p class="confirm-message">确定删除用户 <strong>"{{ deletingUser?.username }}"</strong> 吗？</p>
          <p class="confirm-warning">此操作不可恢复！</p>
        </div>
        <div class="modal-footer">
          <button class="btn btn-secondary" @click="showDeleteModal = false">取消</button>
          <button class="btn btn-danger" @click="confirmDelete">确认删除</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, inject } from 'vue'
import { userApi } from '../api'

const showToast = inject('showToast')

const users = ref([])
const total = ref(0)
const page = ref(1)
const pageSize = ref(10)
const filter = ref({ role: '' })

const showModal = ref(false)
const editingUser = ref(null)
const form = ref({
  username: '',
  password: '',
  role: 'employee',
  real_name: '',
  phone: '',
  email: '',
  department: '',
  status: 1
})

// 删除确认
const showDeleteModal = ref(false)
const deletingUser = ref(null)

const loadUsers = async () => {
  try {
    const res = await userApi.list({
      page: page.value,
      pageSize: pageSize.value,
      role: filter.value.role
    })
    users.value = res.data.list
    total.value = res.data.total
  } catch (err) {
    showToast('error', '加载失败')
  }
}

const openModal = (user = null) => {
  editingUser.value = user
  if (user) {
    form.value = { ...user }
  } else {
    form.value = {
      username: '',
      password: '',
      role: 'employee',
      real_name: '',
      phone: '',
      email: '',
      department: '',
      status: 1
    }
  }
  showModal.value = true
}

const handleSubmit = async () => {
  try {
    if (editingUser.value) {
      await userApi.update(editingUser.value.id, form.value)
      showToast('success', '更新成功')
    } else {
      await userApi.create(form.value)
      showToast('success', '创建成功')
    }
    showModal.value = false
    loadUsers()
  } catch (err) {
    showToast('error', err.message || '操作失败')
  }
}

const handleDelete = (user) => {
  deletingUser.value = user
  showDeleteModal.value = true
}

const confirmDelete = async () => {
  try {
    await userApi.delete(deletingUser.value.id)
    showToast('success', '删除成功')
    showDeleteModal.value = false
    loadUsers()
  } catch (err) {
    showToast('error', err.message || '删除失败')
  }
}

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

const formatDate = (dateStr) => {
  if (!dateStr) return '-'
  return dateStr.substring(0, 16)
}

onMounted(() => {
  loadUsers()
})
</script>

<style scoped>
.users-page {
  max-width: 1400px;
}

.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 1.5rem;
}

.page-title {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--gray-800);
}

.filter-bar {
  display: flex;
  gap: 1rem;
  margin-bottom: 1rem;
}

.filter-bar .select {
  width: 180px;
}

.action-btns {
  display: flex;
  gap: 0.5rem;
}

.badge-role-superadmin,
.badge-role-admin {
  background: var(--primary-100);
  color: var(--primary-700);
}

.badge-role-employee {
  background: var(--success-100);
  color: var(--success-600);
}

.badge-role-security {
  background: #fff7ed;
  color: var(--accent-600);
}

.badge-role-visitor {
  background: var(--gray-100);
  color: var(--gray-600);
}

/* 确认弹窗样式 */
.confirm-modal {
  max-width: 400px;
}

.confirm-modal .modal-body {
  text-align: center;
  padding: 2rem 1.5rem;
}

.confirm-icon {
  font-size: 3rem;
  margin-bottom: 1rem;
}

.confirm-message {
  font-size: 1rem;
  color: var(--gray-600);
  line-height: 1.6;
}

.confirm-message strong {
  color: var(--gray-800);
}

.confirm-warning {
  font-size: 0.875rem;
  color: var(--error-500);
  margin-top: 0.75rem;
}
</style>
