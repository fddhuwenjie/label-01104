# 访客预约管理系统

## 1 How to Run

### 使用 Docker Compose (推荐)

```bash
# 1. 进入项目目录
cd /path/to/project

# 2. 构建并启动所有服务
docker-compose up --build -d

# 3. 查看服务状态
docker-compose ps

# 4. 查看日志
docker-compose logs -f

# 5. 停止服务
docker-compose down
```

### 服务访问地址

启动成功后，通过以下地址访问：

- **管理后台**: http://localhost:8081
- **用户端**: http://localhost:8082
- **后端API**: http://localhost:8080

### ⚠️ 首次运行 - 系统初始化

首次启动系统时，**没有默认账户**。您需要：

1. 访问管理后台 http://localhost:8081
2. 系统会自动检测到尚未初始化，显示"系统初始化"页面
3. 设置管理员用户名、密码和姓名
4. 点击"初始化系统"完成首个管理员账户的创建
5. 使用创建的管理员账户登录

> **安全提示**: 系统初始化接口仅在数据库无用户时可用，创建首个管理员后该接口自动禁用。

---

## 2 Services

| 服务名 | 端口 | 描述 |
|--------|------|------|
| backend | 8080 | C++ 后端 REST API 服务 |
| frontend-admin | 8081 | Vue.js 管理后台 |
| frontend-user | 8082 | Vue.js 用户端 |

---

## 3 测试账号

系统不预置默认账户，请按以下步骤创建账户：

### 创建管理员账户（管理后台）

1. 首次访问 http://localhost:8081
2. 在系统初始化页面创建管理员账户
3. 使用创建的账户登录管理后台

### 注册访客账户（用户端）

1. 访问 http://localhost:8082
2. 点击"创建新账户"进行注册
3. 注册完成后登录使用

### 创建其他角色账户

管理员登录后可在"用户管理"页面创建：
- 管理员 (admin)
- 员工 (employee)
- 保安 (security)
- 访客 (visitor)

---

## 4 题目内容
用C++帮我开发一个访客预约系统，系统功能要完善，系统要分角色，有自己的ID生成系统

### 项目名称
访客预约管理系统

### 技术栈
- **后端**: C++ (自定义 HTTP 服务器 + SQLite)
- **前端**: Vue.js 3 + Vite
- **数据库**: SQLite
- **容器化**: Docker + Docker Compose

### 功能模块

#### 1. 用户管理
- 多角色系统（超级管理员、管理员、员工、保安、访客）
- 用户注册/登录（JWT认证）
- 用户信息管理（CRUD）
- 密码加密存储（SHA-256 + Salt）

#### 2. 自定义ID生成系统
- 基于雪花算法的分布式ID生成器
- 支持不同实体类型的ID前缀
- ID格式: `{角色前缀}-{18位雪花ID}`
  - SA: 超级管理员
  - AD: 管理员
  - EM: 员工
  - SE: 保安
  - VI: 访客
  - AP: 预约
  - CR: 签到记录

#### 3. 访客预约
- 访客提交预约申请
- 员工邀请访客
- 管理员审批预约
- 预约状态流转（待审批→已批准/已拒绝→已签到→已签退）

#### 4. 签到签退
- 保安验证访客身份
- 签到/签退时间记录
- 签到位置记录

#### 5. 数据统计
- 用户统计
- 预约统计
- 访问记录统计

---

## 项目结构

```
visitor-system/
├── backend/                    # C++ 后端
│   ├── include/               # 头文件
│   ├── src/                   # 源代码
│   │   ├── controllers/       # 控制器
│   │   ├── database/          # 数据库操作
│   │   ├── middleware/        # 中间件
│   │   └── utils/             # 工具类
│   ├── third_party/           # 第三方库
│   ├── CMakeLists.txt
│   └── Dockerfile
├── frontend-admin/            # 管理后台
│   ├── src/
│   │   ├── api/              # API 接口
│   │   ├── components/       # 组件
│   │   └── views/            # 页面
│   ├── Dockerfile
│   └── nginx.conf
├── frontend-user/             # 用户端
│   ├── src/
│   │   ├── api/              # API 接口
│   │   ├── components/       # 组件
│   │   └── views/            # 页面
│   ├── Dockerfile
│   └── nginx.conf
├── docker-compose.yml         # Docker Compose 配置
├── .gitignore
└── README.md
```

---

## API 接口

### 认证接口
- `GET /api/auth/check-setup` - 检查系统是否需要初始化
- `POST /api/auth/setup` - 系统初始化（创建首个管理员）
- `POST /api/auth/register` - 用户注册
- `POST /api/auth/login` - 用户登录
- `GET /api/auth/profile` - 获取当前用户信息

### 用户接口
- `GET /api/users` - 获取用户列表
- `GET /api/users/:id` - 获取用户详情
- `POST /api/users` - 创建用户
- `PUT /api/users/:id` - 更新用户
- `DELETE /api/users/:id` - 删除用户
- `GET /api/employees` - 获取员工列表

### 预约接口
- `GET /api/appointments` - 获取预约列表
- `GET /api/appointments/:id` - 获取预约详情
- `POST /api/appointments` - 创建预约
- `POST /api/appointments/:id/approve` - 批准预约
- `POST /api/appointments/:id/reject` - 拒绝预约
- `POST /api/appointments/:id/cancel` - 取消预约

### 签到接口
- `POST /api/check/in` - 访客签到
- `POST /api/check/out` - 访客签退
- `GET /api/check/records` - 获取签到记录

### 统计接口
- `GET /api/stats/overview` - 获取概览统计
- `GET /api/stats/visitors` - 获取访客统计
- `GET /api/stats/appointments` - 获取预约统计

---

## 许可证

MIT License
