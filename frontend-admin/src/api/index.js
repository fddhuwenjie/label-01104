import axios from 'axios'

const api = axios.create({
  baseURL: '/api',
  timeout: 10000
})

// 请求拦截器
api.interceptors.request.use(config => {
  const token = localStorage.getItem('token')
  if (token) {
    config.headers.Authorization = `Bearer ${token}`
  }
  return config
})

// 响应拦截器
api.interceptors.response.use(
  response => {
    const data = response.data
    if (data.code === 0) {
      return data
    }
    return Promise.reject(data)
  },
  error => {
    if (error.response?.status === 401) {
      localStorage.removeItem('token')
      localStorage.removeItem('user')
      window.location.href = '/login'
    }
    return Promise.reject(error.response?.data || error)
  }
)

// 认证接口
export const authApi = {
  checkSetup: () => api.get('/auth/check-setup'),
  setup: (data) => api.post('/auth/setup', data),
  login: (data) => api.post('/auth/login', data),
  register: (data) => api.post('/auth/register', data),
  profile: () => api.get('/auth/profile')
}

// 用户接口
export const userApi = {
  list: (params) => api.get('/users', { params }),
  get: (id) => api.get(`/users/${id}`),
  create: (data) => api.post('/users', data),
  update: (id, data) => api.put(`/users/${id}`, data),
  delete: (id) => api.delete(`/users/${id}`),
  employees: () => api.get('/employees')
}

// 预约接口
export const appointmentApi = {
  list: (params) => api.get('/appointments', { params }),
  get: (id) => api.get(`/appointments/${id}`),
  create: (data) => api.post('/appointments', data),
  approve: (id) => api.post(`/appointments/${id}/approve`),
  reject: (id, reason) => api.post(`/appointments/${id}/reject`, { reason }),
  cancel: (id) => api.post(`/appointments/${id}/cancel`)
}

// 签到接口
export const checkApi = {
  checkIn: (data) => api.post('/check/in', data),
  checkOut: (data) => api.post('/check/out', data),
  records: (params) => api.get('/check/records', { params }),
  record: (appointmentId) => api.get(`/check/record/${appointmentId}`)
}

// 统计接口
export const statsApi = {
  overview: () => api.get('/stats/overview'),
  visitors: (params) => api.get('/stats/visitors', { params }),
  appointments: (params) => api.get('/stats/appointments', { params })
}

export default api
