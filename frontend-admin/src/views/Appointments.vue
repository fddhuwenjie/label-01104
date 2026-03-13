<template>
  <div class="appointments-page">
    <div class="page-header">
      <h1 class="page-title">预约管理</h1>
    </div>
    
    <!-- 流程说明 -->
    <div class="workflow-guide">
      <h3 class="workflow-title">📋 预约流程说明</h3>
      <div class="workflow-steps">
        <div class="workflow-step">
          <div class="step-num">1</div>
          <div class="step-info">
            <span class="step-label">访客提交</span>
            <span class="badge badge-pending">待审批</span>
          </div>
        </div>
        <div class="workflow-arrow">→</div>
        <div class="workflow-step">
          <div class="step-num">2</div>
          <div class="step-info">
            <span class="step-label">管理员审批</span>
            <span class="badge badge-approved">已批准</span>
          </div>
        </div>
        <div class="workflow-arrow">→</div>
        <div class="workflow-step">
          <div class="step-num">3</div>
          <div class="step-info">
            <span class="step-label">访客到访签到</span>
            <span class="badge badge-checked_in">已签到</span>
          </div>
        </div>
        <div class="workflow-arrow">→</div>
        <div class="workflow-step">
          <div class="step-num">4</div>
          <div class="step-info">
            <span class="step-label">访客离开签退</span>
            <span class="badge badge-checked_out">已签退</span>
          </div>
        </div>
      </div>
    </div>
    
    <!-- 筛选 -->
    <div class="filter-bar">
      <select v-model="filter.status" class="select" @change="loadAppointments">
        <option value="">全部状态</option>
        <option value="pending">待审批</option>
        <option value="approved">已批准</option>
        <option value="rejected">已拒绝</option>
        <option value="checked_in">已签到</option>
        <option value="checked_out">已签退</option>
        <option value="cancelled">已取消</option>
      </select>
    </div>
    
    <!-- 预约列表 -->
    <div class="card">
      <div class="table-container">
        <table class="table">
          <thead>
            <tr>
              <th>访客</th>
              <th>被访人</th>
              <th>访问日期</th>
              <th>时间段</th>
              <th>访问目的</th>
              <th>状态</th>
              <th>创建时间</th>
              <th>操作</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="apt in appointments" :key="apt.id">
              <td>
                <div class="visitor-info">
                  <span class="visitor-name">{{ apt.visitor_name || '-' }}</span>
                  <span class="visitor-phone">{{ apt.visitor_phone }}</span>
                </div>
              </td>
              <td>
                <div class="host-info">
                  <span class="host-name">{{ apt.host_name || '-' }}</span>
                  <span class="host-dept">{{ apt.host_department }}</span>
                </div>
              </td>
              <td>{{ apt.visit_date }}</td>
              <td>{{ apt.visit_time_start }} - {{ apt.visit_time_end }}</td>
              <td>{{ apt.purpose || '-' }}</td>
              <td>
                <span :class="['badge', `badge-${apt.status}`]">{{ getStatusName(apt.status) }}</span>
              </td>
              <td>{{ formatDate(apt.created_at) }}</td>
              <td>
                <div class="action-btns">
                  <template v-if="apt.status === 'pending'">
                    <button @click="handleApprove(apt)" class="btn btn-sm btn-success">批准</button>
                    <button @click="handleReject(apt)" class="btn btn-sm btn-danger">拒绝</button>
                  </template>
                  <template v-else-if="apt.status === 'approved'">
                    <button @click="handleCheckIn(apt)" class="btn btn-sm btn-primary">签到</button>
                  </template>
                  <template v-else-if="apt.status === 'checked_in'">
                    <button @click="handleCheckOut(apt)" class="btn btn-sm btn-secondary">签退</button>
                  </template>
                  <span v-else class="text-sm text-gray-500">-</span>
                </div>
              </td>
            </tr>
          </tbody>
        </table>
        
        <div v-if="appointments.length === 0" class="empty-state">
          <div class="empty-state-icon">📅</div>
          <p>暂无预约数据</p>
        </div>
      </div>
      
      <!-- 分页 -->
      <div v-if="total > pageSize" class="pagination">
        <button :disabled="page === 1" @click="page--; loadAppointments()">上一页</button>
        <span class="text-sm text-gray-500">第 {{ page }} 页 / 共 {{ Math.ceil(total / pageSize) }} 页</span>
        <button :disabled="page * pageSize >= total" @click="page++; loadAppointments()">下一页</button>
      </div>
    </div>
    
    <!-- 拒绝原因模态框 -->
    <div v-if="showRejectModal" class="modal-overlay" @click.self="showRejectModal = false">
      <div class="modal">
        <div class="modal-header">
          <h3 class="modal-title">拒绝预约</h3>
          <button class="modal-close" @click="showRejectModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label class="form-label">拒绝原因</label>
            <textarea v-model="rejectReason" class="input" rows="3" placeholder="请输入拒绝原因（可选）"></textarea>
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-secondary" @click="showRejectModal = false">取消</button>
          <button class="btn btn-danger" @click="confirmReject">确认拒绝</button>
        </div>
      </div>
    </div>
    
    <!-- 签到模态框 -->
    <div v-if="showCheckInModal" class="modal-overlay" @click.self="showCheckInModal = false">
      <div class="modal">
        <div class="modal-header">
          <h3 class="modal-title">访客签到</h3>
          <button class="modal-close" @click="showCheckInModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label class="form-label">签到位置</label>
            <select v-model="checkInForm.gate_location" class="select">
              <option value="正门">正门</option>
              <option value="东门">东门</option>
              <option value="西门">西门</option>
              <option value="北门">北门</option>
            </select>
          </div>
          <div class="form-group">
            <label class="form-label">备注</label>
            <input v-model="checkInForm.remarks" class="input" placeholder="备注信息（可选）" />
          </div>
        </div>
        <div class="modal-footer">
          <button class="btn btn-secondary" @click="showCheckInModal = false">取消</button>
          <button class="btn btn-primary" @click="confirmCheckIn">确认签到</button>
        </div>
      </div>
    </div>
    
    <!-- 通用确认弹窗 -->
    <div v-if="showConfirmModal" class="modal-overlay" @click.self="showConfirmModal = false">
      <div class="modal confirm-modal">
        <div class="modal-header">
          <h3 class="modal-title">{{ confirmModal.title }}</h3>
          <button class="modal-close" @click="showConfirmModal = false">&times;</button>
        </div>
        <div class="modal-body">
          <div class="confirm-icon">{{ confirmModal.icon }}</div>
          <p class="confirm-message">{{ confirmModal.message }}</p>
        </div>
        <div class="modal-footer">
          <button class="btn btn-secondary" @click="showConfirmModal = false">取消</button>
          <button :class="['btn', confirmModal.confirmBtnClass || 'btn-primary']" @click="executeConfirm">
            {{ confirmModal.confirmText || '确认' }}
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, inject } from 'vue'
import { appointmentApi, checkApi } from '../api'

const showToast = inject('showToast')

const appointments = ref([])
const total = ref(0)
const page = ref(1)
const pageSize = ref(10)
const filter = ref({ status: '' })

const showRejectModal = ref(false)
const rejectReason = ref('')
const rejectingApt = ref(null)

const showCheckInModal = ref(false)
const checkingInApt = ref(null)
const checkInForm = ref({
  gate_location: '正门',
  remarks: ''
})

// 通用确认弹窗
const showConfirmModal = ref(false)
const confirmModal = ref({
  title: '',
  message: '',
  icon: '⚠️',
  confirmText: '确认',
  confirmBtnClass: 'btn-primary',
  onConfirm: null
})

const openConfirm = (options) => {
  confirmModal.value = { ...confirmModal.value, ...options }
  showConfirmModal.value = true
}

const executeConfirm = async () => {
  if (confirmModal.value.onConfirm) {
    await confirmModal.value.onConfirm()
  }
  showConfirmModal.value = false
}

const loadAppointments = async () => {
  try {
    const res = await appointmentApi.list({
      page: page.value,
      pageSize: pageSize.value,
      status: filter.value.status
    })
    appointments.value = res.data.list
    total.value = res.data.total
  } catch (err) {
    showToast('error', '加载失败')
  }
}

const handleApprove = (apt) => {
  openConfirm({
    title: '批准预约',
    message: `确定批准 ${apt.visitor_name || '该访客'} 的预约申请吗？`,
    icon: '✅',
    confirmText: '批准',
    confirmBtnClass: 'btn-success',
    onConfirm: async () => {
      try {
        await appointmentApi.approve(apt.id)
        showToast('success', '已批准')
        loadAppointments()
      } catch (err) {
        showToast('error', err.message || '操作失败')
      }
    }
  })
}

const handleReject = (apt) => {
  rejectingApt.value = apt
  rejectReason.value = ''
  showRejectModal.value = true
}

const confirmReject = async () => {
  try {
    await appointmentApi.reject(rejectingApt.value.id, rejectReason.value)
    showToast('success', '已拒绝')
    showRejectModal.value = false
    loadAppointments()
  } catch (err) {
    showToast('error', err.message || '操作失败')
  }
}

const handleCheckIn = (apt) => {
  checkingInApt.value = apt
  checkInForm.value = {
    gate_location: '正门',
    remarks: ''
  }
  showCheckInModal.value = true
}

const confirmCheckIn = async () => {
  try {
    await checkApi.checkIn({
      appointment_id: checkingInApt.value.id,
      ...checkInForm.value
    })
    showToast('success', '签到成功')
    showCheckInModal.value = false
    loadAppointments()
  } catch (err) {
    showToast('error', err.message || '签到失败')
  }
}

const handleCheckOut = (apt) => {
  openConfirm({
    title: '访客签退',
    message: `确定为 ${apt.visitor_name || '该访客'} 办理签退吗？`,
    icon: '👋',
    confirmText: '确认签退',
    confirmBtnClass: 'btn-primary',
    onConfirm: async () => {
      try {
        await checkApi.checkOut({ appointment_id: apt.id })
        showToast('success', '签退成功')
        loadAppointments()
      } catch (err) {
        showToast('error', err.message || '签退失败')
      }
    }
  })
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

const formatDate = (dateStr) => {
  if (!dateStr) return '-'
  return dateStr.substring(0, 16)
}

onMounted(() => {
  loadAppointments()
})
</script>

<style scoped>
.appointments-page {
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

/* 流程说明样式 */
.workflow-guide {
  background: linear-gradient(135deg, var(--primary-50) 0%, white 100%);
  border: 1px solid var(--primary-100);
  border-radius: 0.75rem;
  padding: 1.25rem 1.5rem;
  margin-bottom: 1.5rem;
}

.workflow-title {
  font-size: 0.9375rem;
  font-weight: 600;
  color: var(--gray-700);
  margin-bottom: 1rem;
}

.workflow-steps {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  flex-wrap: wrap;
}

.workflow-step {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  background: white;
  border: 1px solid var(--gray-200);
  border-radius: 0.5rem;
  padding: 0.625rem 1rem;
}

.step-num {
  width: 1.5rem;
  height: 1.5rem;
  background: var(--primary-500);
  color: white;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 0.75rem;
  font-weight: 700;
}

.step-info {
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
}

.step-label {
  font-size: 0.8125rem;
  font-weight: 500;
  color: var(--gray-700);
}

.workflow-arrow {
  color: var(--gray-400);
  font-size: 1.25rem;
  font-weight: 700;
}

@media (max-width: 768px) {
  .workflow-steps {
    flex-direction: column;
    align-items: flex-start;
  }
  .workflow-arrow {
    transform: rotate(90deg);
    margin: 0.25rem 0 0.25rem 1.5rem;
  }
}

.filter-bar {
  display: flex;
  gap: 1rem;
  margin-bottom: 1rem;
}

.filter-bar .select {
  width: 180px;
}

.visitor-info, .host-info {
  display: flex;
  flex-direction: column;
  gap: 0.125rem;
}

.visitor-name, .host-name {
  font-weight: 500;
}

.visitor-phone, .host-dept {
  font-size: 0.75rem;
  color: var(--gray-500);
}

.action-btns {
  display: flex;
  gap: 0.5rem;
}

.action-btns .btn {
  min-width: 60px;
  justify-content: center;
}

textarea.input {
  resize: vertical;
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
</style>
