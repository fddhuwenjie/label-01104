<template>
  <div class="page">
    <div class="container">
      <div class="page-header">
        <h1 class="page-title">我的预约</h1>
        <router-link to="/new-appointment" class="btn btn-primary">
          + 新建预约
        </router-link>
      </div>
      
      <div class="card">
        <div v-if="loading" class="loading">
          <div class="spinner"></div>
        </div>
        
        <div v-else-if="appointments.length === 0" class="empty-state">
          <div class="empty-state-icon">📅</div>
          <p>暂无预约记录</p>
          <router-link to="/new-appointment" class="btn btn-primary mt-4">
            创建第一个预约
          </router-link>
        </div>
        
        <div v-else class="appointment-list">
          <div v-for="apt in appointments" :key="apt.id" class="appointment-item">
            <div class="apt-main">
              <div class="apt-header">
                <span :class="['badge', `badge-${apt.status}`]">{{ getStatusName(apt.status) }}</span>
                <span class="apt-date">{{ apt.visit_date }}</span>
              </div>
              <div class="apt-info">
                <p class="apt-host">
                  <strong>被访人:</strong> {{ apt.host_name || '-' }}
                  <span v-if="apt.host_department" class="apt-dept">（{{ apt.host_department }}）</span>
                </p>
                <p class="apt-time">
                  <strong>时间:</strong> {{ apt.visit_time_start }} - {{ apt.visit_time_end }}
                </p>
                <p v-if="apt.purpose" class="apt-purpose">
                  <strong>目的:</strong> {{ apt.purpose }}
                </p>
                <p v-if="apt.reject_reason" class="apt-reject">
                  <strong>拒绝原因:</strong> {{ apt.reject_reason }}
                </p>
              </div>
            </div>
            <div class="apt-actions">
              <button 
                v-if="apt.status === 'pending'" 
                @click="handleCancel(apt)" 
                class="btn btn-secondary"
              >
                取消预约
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
    
    <!-- 取消预约确认弹窗 -->
    <div v-if="showCancelModal" class="modal-overlay" @click.self="showCancelModal = false">
      <div class="modal confirm-modal">
        <div class="modal-header">
          <h3 class="modal-title">取消预约</h3>
          <button class="modal-close" @click="showCancelModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <div class="confirm-icon">⚠️</div>
          <p class="confirm-message">确定取消此预约吗？</p>
          <p class="confirm-sub">取消后需要重新提交预约申请</p>
        </div>
        <div class="modal-footer">
          <button class="btn btn-secondary" @click="showCancelModal = false">返回</button>
          <button class="btn btn-danger" @click="confirmCancel">确认取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, inject } from 'vue'
import { appointmentApi } from '../api'

const showToast = inject('showToast')

const appointments = ref([])
const loading = ref(true)

// 取消预约
const showCancelModal = ref(false)
const cancelingApt = ref(null)

const loadAppointments = async () => {
  loading.value = true
  try {
    const res = await appointmentApi.list({ page: 1, pageSize: 50 })
    appointments.value = res.data.list
  } catch (err) {
    showToast('error', '加载失败')
  } finally {
    loading.value = false
  }
}

const handleCancel = (apt) => {
  cancelingApt.value = apt
  showCancelModal.value = true
}

const confirmCancel = async () => {
  try {
    await appointmentApi.cancel(cancelingApt.value.id)
    showToast('success', '已取消')
    showCancelModal.value = false
    loadAppointments()
  } catch (err) {
    showToast('error', err.message || '取消失败')
  }
}

const getStatusName = (status) => {
  const statuses = {
    pending: '待审批',
    approved: '已批准',
    rejected: '已拒绝',
    checked_in: '已签到',
    checked_out: '已签退',
    cancelled: '已取消'
  }
  return statuses[status] || status
}

onMounted(loadAppointments)
</script>

<style scoped>
.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 1.5rem;
}

.loading {
  display: flex;
  justify-content: center;
  padding: 3rem;
}

.spinner {
  width: 2rem;
  height: 2rem;
  border: 3px solid var(--gray-800);
  border-top-color: var(--primary-500);
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.appointment-list {
  padding: 1rem;
}

.appointment-item {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  padding: 1.25rem;
  border-bottom: 1px solid var(--gray-800);
}

.appointment-item:last-child {
  border-bottom: none;
}

.apt-header {
  display: flex;
  align-items: center;
  gap: 1rem;
  margin-bottom: 0.75rem;
}

.apt-date {
  font-size: 0.875rem;
  color: var(--gray-400);
}

.apt-info p {
  margin-bottom: 0.375rem;
  font-size: 0.9375rem;
  color: var(--gray-300);
}

.apt-info strong {
  color: var(--gray-500);
  font-weight: 500;
}

.apt-dept {
  color: var(--gray-500);
  font-size: 0.875rem;
}

.apt-reject {
  color: var(--error-400) !important;
}

.apt-actions {
  flex-shrink: 0;
}

/* 确认弹窗样式 */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.7);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  animation: fadeIn 0.2s ease;
}

@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}

.modal {
  background: var(--gray-900);
  border: 1px solid var(--gray-700);
  border-radius: var(--radius-xl);
  width: 90%;
  max-width: 400px;
  animation: slideUp 0.3s ease;
}

@keyframes slideUp {
  from { transform: translateY(20px); opacity: 0; }
  to { transform: translateY(0); opacity: 1; }
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.25rem 1.5rem;
  border-bottom: 1px solid var(--gray-800);
}

.modal-title {
  font-size: 1.125rem;
  font-weight: 600;
  color: var(--gray-100);
}

.modal-close {
  background: none;
  border: none;
  font-size: 1.5rem;
  color: var(--gray-400);
  cursor: pointer;
  padding: 0;
  line-height: 1;
}

.modal-close:hover {
  color: var(--gray-200);
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
  color: var(--gray-300);
  line-height: 1.6;
}

.confirm-sub {
  font-size: 0.875rem;
  color: var(--gray-500);
  margin-top: 0.5rem;
}

.modal-footer {
  display: flex;
  justify-content: flex-end;
  gap: 0.75rem;
  padding: 1rem 1.5rem;
  border-top: 1px solid var(--gray-800);
}

.btn-danger {
  background: linear-gradient(135deg, var(--error-500) 0%, var(--error-600) 100%);
  color: white;
}

.btn-danger:hover {
  opacity: 0.9;
}
</style>
