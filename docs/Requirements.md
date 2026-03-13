# 访客预约系统 - 需求文档

## 1. 项目概述

**项目名称**: 访客预约管理系统 (Visitor Appointment System)  
**技术栈**: C++ 后端 + Vue.js 前端  
**目标**: 实现企业访客预约、审批、签到的全流程管理

---

## 2. 角色定义

| 角色 | 角色ID前缀 | 权限描述 |
|------|-----------|----------|
| **超级管理员** (SuperAdmin) | `SA` | 系统最高权限，管理所有用户和配置 |
| **管理员** (Admin) | `AD` | 审批访客预约，管理员工 |
| **员工** (Employee) | `EM` | 邀请访客，查看自己的访客记录 |
| **保安** (Security) | `SE` | 验证访客身份，签到/签退操作 |
| **访客** (Visitor) | `VI` | 提交预约申请，查看预约状态 |

---

## 3. 功能模块

### 3.1 用户管理模块
- [x] 用户注册/登录（JWT认证）
- [x] 角色分配与权限控制
- [x] 用户信息管理（CRUD）
- [x] 密码加密存储

### 3.2 ID生成系统
- [x] 基于雪花算法的分布式ID生成器
- [x] 支持不同实体类型的ID前缀
- [x] ID格式: `{角色前缀}-{时间戳}-{序列号}-{机器ID}`

### 3.3 访客预约模块
- [x] 访客提交预约申请
- [x] 员工邀请访客
- [x] 管理员审批预约
- [x] 预约状态流转（待审批→已批准/已拒绝→已签到→已签退）

### 3.4 签到签退模块
- [x] 保安验证访客身份
- [x] 签到时间记录
- [x] 签退时间记录
- [x] 异常访问报警

### 3.5 数据统计模块
- [x] 访客统计（日/周/月）
- [x] 预约审批统计
- [x] 访问时长统计

---

## 4. 数据模型

### 4.1 用户表 (users)
```sql
CREATE TABLE users (
    id VARCHAR(32) PRIMARY KEY,      -- 自定义ID
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    role VARCHAR(20) NOT NULL,       -- superadmin/admin/employee/security/visitor
    real_name VARCHAR(50),
    phone VARCHAR(20),
    email VARCHAR(100),
    department VARCHAR(50),
    status INT DEFAULT 1,            -- 0:禁用 1:启用
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### 4.2 预约表 (appointments)
```sql
CREATE TABLE appointments (
    id VARCHAR(32) PRIMARY KEY,
    visitor_id VARCHAR(32) NOT NULL,
    host_id VARCHAR(32) NOT NULL,    -- 被访问的员工
    purpose VARCHAR(200),            -- 访问目的
    visit_date DATE NOT NULL,
    visit_time_start TIME,
    visit_time_end TIME,
    status VARCHAR(20) DEFAULT 'pending',  -- pending/approved/rejected/checked_in/checked_out/cancelled
    approver_id VARCHAR(32),
    approved_at DATETIME,
    reject_reason VARCHAR(200),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (visitor_id) REFERENCES users(id),
    FOREIGN KEY (host_id) REFERENCES users(id)
);
```

### 4.3 签到记录表 (check_records)
```sql
CREATE TABLE check_records (
    id VARCHAR(32) PRIMARY KEY,
    appointment_id VARCHAR(32) NOT NULL,
    check_in_time DATETIME,
    check_out_time DATETIME,
    security_id VARCHAR(32),         -- 操作的保安
    gate_location VARCHAR(50),       -- 签到门禁位置
    remarks VARCHAR(200),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (appointment_id) REFERENCES appointments(id)
);
```

### 4.4 操作日志表 (operation_logs)
```sql
CREATE TABLE operation_logs (
    id VARCHAR(32) PRIMARY KEY,
    user_id VARCHAR(32),
    action VARCHAR(50),
    target_type VARCHAR(50),
    target_id VARCHAR(32),
    details TEXT,
    ip_address VARCHAR(50),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

---

## 5. API 接口设计

### 5.1 认证接口
| 方法 | 路径 | 描述 |
|------|------|------|
| POST | /api/auth/register | 用户注册 |
| POST | /api/auth/login | 用户登录 |
| POST | /api/auth/logout | 用户登出 |
| GET | /api/auth/profile | 获取当前用户信息 |

### 5.2 用户管理接口
| 方法 | 路径 | 描述 |
|------|------|------|
| GET | /api/users | 获取用户列表 |
| GET | /api/users/:id | 获取用户详情 |
| POST | /api/users | 创建用户 |
| PUT | /api/users/:id | 更新用户 |
| DELETE | /api/users/:id | 删除用户 |

### 5.3 预约管理接口
| 方法 | 路径 | 描述 |
|------|------|------|
| GET | /api/appointments | 获取预约列表 |
| GET | /api/appointments/:id | 获取预约详情 |
| POST | /api/appointments | 创建预约 |
| PUT | /api/appointments/:id | 更新预约 |
| POST | /api/appointments/:id/approve | 审批预约 |
| POST | /api/appointments/:id/reject | 拒绝预约 |
| POST | /api/appointments/:id/cancel | 取消预约 |

### 5.4 签到管理接口
| 方法 | 路径 | 描述 |
|------|------|------|
| POST | /api/check/in | 访客签到 |
| POST | /api/check/out | 访客签退 |
| GET | /api/check/records | 获取签到记录 |

### 5.5 统计接口
| 方法 | 路径 | 描述 |
|------|------|------|
| GET | /api/stats/overview | 获取概览统计 |
| GET | /api/stats/visitors | 获取访客统计 |
| GET | /api/stats/appointments | 获取预约统计 |

---

## 6. 技术架构

```
┌─────────────────────────────────────────────────────────────┐
│                        Nginx (反向代理)                       │
├─────────────────────────────────────────────────────────────┤
│   frontend-admin (8081)  │   frontend-user (8082)           │
│      Vue.js 管理后台       │      Vue.js 用户端                │
├─────────────────────────────────────────────────────────────┤
│                    backend (8080)                            │
│              C++ REST API (Crow Framework)                   │
├─────────────────────────────────────────────────────────────┤
│                    SQLite Database                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 7. 非功能需求

- **性能**: API 响应时间 < 200ms
- **安全**: JWT 认证，密码 bcrypt 加密
- **可用性**: Docker 容器化部署
- **跨平台**: 支持 ARM64 和 AMD64 架构
