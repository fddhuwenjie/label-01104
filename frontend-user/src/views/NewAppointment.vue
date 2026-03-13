<template>
  <div class="page">
    <div class="container">
      <h1 class="page-title">新建预约</h1>
      
      <div class="form-card">
        <form @submit.prevent="handleSubmit">
          <div class="form-group">
            <label class="form-label">选择被访人 *</label>
            <select v-model="form.host_id" class="select" required :disabled="employees.length === 0">
              <option value="">{{ employees.length === 0 ? '暂无可选被访人' : '请选择被访人' }}</option>
              <option v-for="emp in employees" :key="emp.id" :value="emp.id">
                {{ emp.real_name || emp.username }} - {{ emp.department || '未设置部门' }}
              </option>
            </select>
            <p v-if="employees.length === 0 && !loadingEmployees" class="field-hint">
              暂无可访问的人员，请联系管理员添加员工
            </p>
          </div>
          
          <div class="form-group">
            <label class="form-label">访问日期 *</label>
            <div class="date-input-wrapper">
              <input 
                v-model="form.visit_date" 
                type="date" 
                class="input date-input" 
                required 
                :min="minDate"
                @click="$event.target.showPicker && $event.target.showPicker()"
              />
              <span class="date-icon">📅</span>
            </div>
          </div>
          
          <div class="form-row">
            <div class="form-group">
              <label class="form-label">开始时间</label>
              <input v-model="form.visit_time_start" type="time" class="input" />
            </div>
            <div class="form-group">
              <label class="form-label">结束时间</label>
              <input v-model="form.visit_time_end" type="time" class="input" />
            </div>
          </div>
          
          <div class="form-group">
            <label class="form-label">访问目的</label>
            <textarea v-model="form.purpose" class="input" rows="3" placeholder="请简要描述您的访问目的"></textarea>
          </div>
          
          <div class="form-actions">
            <router-link to="/appointments" class="btn btn-secondary">取消</router-link>
            <button type="submit" class="btn btn-primary" :disabled="loading">
              {{ loading ? '提交中...' : '提交预约' }}
            </button>
          </div>
          
          <p v-if="error" class="error-msg">{{ error }}</p>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, inject } from 'vue'
import { useRouter } from 'vue-router'
import { appointmentApi, userApi } from '../api'

const router = useRouter()
const showToast = inject('showToast')

const employees = ref([])
const loading = ref(false)
const loadingEmployees = ref(true)
const error = ref('')

const form = ref({
  host_id: '',
  visit_date: '',
  visit_time_start: '09:00',
  visit_time_end: '18:00',
  purpose: ''
})

const minDate = computed(() => {
  const today = new Date()
  return today.toISOString().split('T')[0]
})

const loadEmployees = async () => {
  loadingEmployees.value = true
  try {
    const res = await userApi.employees()
    employees.value = res.data || []
  } catch (err) {
    showToast('error', '加载被访人列表失败')
    employees.value = []
  } finally {
    loadingEmployees.value = false
  }
}

const handleSubmit = async () => {
  loading.value = true
  error.value = ''
  
  try {
    await appointmentApi.create(form.value)
    showToast('success', '预约提交成功，等待审批')
    router.push('/appointments')
  } catch (err) {
    error.value = err.message || '提交失败'
  } finally {
    loading.value = false
  }
}

onMounted(loadEmployees)
</script>

<style scoped>
.page {
  display: flex;
  justify-content: center;
  padding: 3rem 1rem;
}

.page .container {
  width: 100%;
  max-width: 640px;
}

.page-title {
  margin-bottom: 1.5rem;
}

.form-card {
  background: rgba(39, 39, 42, 0.8);
  backdrop-filter: blur(20px);
  border: 1px solid var(--gray-800);
  border-radius: var(--radius-xl);
  padding: 2rem;
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

textarea.input {
  resize: vertical;
}

.form-actions {
  display: flex;
  justify-content: flex-end;
  gap: 1rem;
  margin-top: 1.5rem;
}

.form-actions .btn {
  min-width: 120px;
  padding: 0.75rem 1.5rem;
}

/* 日期输入样式 */
.date-input-wrapper {
  position: relative;
}

.date-input {
  cursor: pointer;
  padding-right: 3rem;
}

.date-input::-webkit-calendar-picker-indicator {
  position: absolute;
  right: 0;
  top: 0;
  width: 100%;
  height: 100%;
  opacity: 0;
  cursor: pointer;
}

.date-icon {
  position: absolute;
  right: 1rem;
  top: 50%;
  transform: translateY(-50%);
  font-size: 1.125rem;
  pointer-events: none;
}

.error-msg {
  color: var(--error-400);
  font-size: 0.875rem;
  text-align: center;
  margin-top: 1rem;
  padding: 0.75rem;
  background: rgba(244, 63, 94, 0.1);
  border-radius: var(--radius);
  border: 1px solid rgba(244, 63, 94, 0.2);
}

.field-hint {
  font-size: 0.8125rem;
  color: var(--warning-400);
  margin-top: 0.5rem;
}

.select:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}
</style>
