<template>
  <div class="records-page">
    <div class="page-header">
      <h1 class="page-title">签到记录</h1>
    </div>
    
    <!-- 记录列表 -->
    <div class="card">
      <div class="table-container">
        <table class="table">
          <thead>
            <tr>
              <th>记录ID</th>
              <th>访客姓名</th>
              <th>被访人</th>
              <th>签到时间</th>
              <th>签退时间</th>
              <th>签到位置</th>
              <th>停留时长</th>
              <th>备注</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="record in records" :key="record.id">
              <td><code class="record-id">{{ record.id.substring(0, 15) }}...</code></td>
              <td>{{ record.visitor_name || '-' }}</td>
              <td>{{ record.host_name || '-' }}</td>
              <td>{{ formatDateTime(record.check_in_time) }}</td>
              <td>
                <span v-if="record.check_out_time">{{ formatDateTime(record.check_out_time) }}</span>
                <span v-else class="badge badge-checked_in">在访中</span>
              </td>
              <td>{{ record.gate_location || '-' }}</td>
              <td>{{ calculateDuration(record.check_in_time, record.check_out_time) }}</td>
              <td>{{ record.remarks || '-' }}</td>
            </tr>
          </tbody>
        </table>
        
        <div v-if="records.length === 0" class="empty-state">
          <div class="empty-state-icon">✅</div>
          <p>暂无签到记录</p>
        </div>
      </div>
      
      <!-- 分页 -->
      <div v-if="records.length > 0" class="pagination">
        <button :disabled="page === 1" @click="page--; loadRecords()">上一页</button>
        <span class="text-sm text-gray-500">第 {{ page }} 页</span>
        <button :disabled="records.length < pageSize" @click="page++; loadRecords()">下一页</button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, inject } from 'vue'
import { checkApi } from '../api'

const showToast = inject('showToast')

const records = ref([])
const page = ref(1)
const pageSize = ref(20)

const loadRecords = async () => {
  try {
    const res = await checkApi.records({
      page: page.value,
      pageSize: pageSize.value
    })
    records.value = res.data || []
  } catch (err) {
    showToast('error', '加载失败')
  }
}

const formatDateTime = (dateStr) => {
  if (!dateStr) return '-'
  return dateStr.replace('T', ' ').substring(0, 19)
}

const calculateDuration = (checkIn, checkOut) => {
  if (!checkIn) return '-'
  
  const start = new Date(checkIn)
  const end = checkOut ? new Date(checkOut) : new Date()
  
  const diffMs = end - start
  const diffMins = Math.floor(diffMs / 60000)
  
  if (diffMins < 60) {
    return `${diffMins} 分钟`
  }
  
  const hours = Math.floor(diffMins / 60)
  const mins = diffMins % 60
  return `${hours} 小时 ${mins} 分钟`
}

onMounted(() => {
  loadRecords()
})
</script>

<style scoped>
.records-page {
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

.record-id {
  font-size: 0.75rem;
  background: var(--gray-100);
  padding: 0.125rem 0.375rem;
  border-radius: 0.25rem;
}
</style>
