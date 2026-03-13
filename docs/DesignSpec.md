# 访客预约系统 - 设计规范

## 1. 设计理念

**主题**: 专业、简洁、高效  
**风格**: 现代企业级管理系统  
**色调**: 深蓝主色调 + 白色背景 + 橙色强调

---

## 2. 色彩规范

### 主色板
```css
:root {
  /* 主色 - 深蓝 */
  --primary-50: #eff6ff;
  --primary-100: #dbeafe;
  --primary-200: #bfdbfe;
  --primary-300: #93c5fd;
  --primary-400: #60a5fa;
  --primary-500: #3b82f6;
  --primary-600: #2563eb;
  --primary-700: #1d4ed8;
  --primary-800: #1e40af;
  --primary-900: #1e3a8a;

  /* 强调色 - 橙色 */
  --accent-400: #fb923c;
  --accent-500: #f97316;
  --accent-600: #ea580c;

  /* 成功色 - 绿色 */
  --success-500: #22c55e;
  --success-600: #16a34a;

  /* 警告色 - 黄色 */
  --warning-500: #eab308;
  --warning-600: #ca8a04;

  /* 错误色 - 红色 */
  --error-500: #ef4444;
  --error-600: #dc2626;

  /* 中性色 */
  --gray-50: #f9fafb;
  --gray-100: #f3f4f6;
  --gray-200: #e5e7eb;
  --gray-300: #d1d5db;
  --gray-400: #9ca3af;
  --gray-500: #6b7280;
  --gray-600: #4b5563;
  --gray-700: #374151;
  --gray-800: #1f2937;
  --gray-900: #111827;
}
```

---

## 3. 字体规范

```css
:root {
  /* 字体家族 */
  --font-sans: 'Inter', 'PingFang SC', 'Microsoft YaHei', sans-serif;
  --font-mono: 'JetBrains Mono', 'Fira Code', monospace;

  /* 字号 */
  --text-xs: 0.75rem;    /* 12px */
  --text-sm: 0.875rem;   /* 14px */
  --text-base: 1rem;     /* 16px */
  --text-lg: 1.125rem;   /* 18px */
  --text-xl: 1.25rem;    /* 20px */
  --text-2xl: 1.5rem;    /* 24px */
  --text-3xl: 1.875rem;  /* 30px */
  --text-4xl: 2.25rem;   /* 36px */

  /* 字重 */
  --font-normal: 400;
  --font-medium: 500;
  --font-semibold: 600;
  --font-bold: 700;
}
```

---

## 4. 间距规范

```css
:root {
  --space-1: 0.25rem;   /* 4px */
  --space-2: 0.5rem;    /* 8px */
  --space-3: 0.75rem;   /* 12px */
  --space-4: 1rem;      /* 16px */
  --space-5: 1.25rem;   /* 20px */
  --space-6: 1.5rem;    /* 24px */
  --space-8: 2rem;      /* 32px */
  --space-10: 2.5rem;   /* 40px */
  --space-12: 3rem;     /* 48px */
  --space-16: 4rem;     /* 64px */
}
```

---

## 5. 圆角规范

```css
:root {
  --radius-sm: 0.25rem;   /* 4px */
  --radius-md: 0.375rem;  /* 6px */
  --radius-lg: 0.5rem;    /* 8px */
  --radius-xl: 0.75rem;   /* 12px */
  --radius-2xl: 1rem;     /* 16px */
  --radius-full: 9999px;
}
```

---

## 6. 阴影规范

```css
:root {
  --shadow-sm: 0 1px 2px 0 rgb(0 0 0 / 0.05);
  --shadow-md: 0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1);
  --shadow-lg: 0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1);
  --shadow-xl: 0 20px 25px -5px rgb(0 0 0 / 0.1), 0 8px 10px -6px rgb(0 0 0 / 0.1);
}
```

---

## 7. 组件规范

### 7.1 按钮

| 类型 | 背景色 | 文字色 | 用途 |
|------|--------|--------|------|
| Primary | --primary-600 | white | 主要操作 |
| Secondary | --gray-100 | --gray-700 | 次要操作 |
| Success | --success-600 | white | 确认/成功 |
| Danger | --error-600 | white | 删除/危险 |
| Ghost | transparent | --primary-600 | 轻量操作 |

### 7.2 表单输入

- 边框: 1px solid --gray-300
- 圆角: --radius-lg
- 内边距: --space-3 --space-4
- 聚焦: 2px solid --primary-500

### 7.3 卡片

- 背景: white
- 圆角: --radius-xl
- 阴影: --shadow-md
- 内边距: --space-6

### 7.4 表格

- 表头背景: --gray-50
- 行悬停: --primary-50
- 边框: 1px solid --gray-200

---

## 8. 页面布局

### 8.1 管理后台布局

```
┌──────────────────────────────────────────────────┐
│  Logo    导航菜单              用户头像 ▼        │  <- Header (64px)
├──────────┬───────────────────────────────────────┤
│          │                                       │
│  侧边栏   │            主内容区                   │
│  菜单     │                                       │
│          │                                       │
│  (240px) │                                       │
│          │                                       │
└──────────┴───────────────────────────────────────┘
```

### 8.2 用户端布局

```
┌──────────────────────────────────────────────────┐
│  Logo           导航            登录/注册        │  <- Header (64px)
├──────────────────────────────────────────────────┤
│                                                  │
│                   主内容区                        │
│                                                  │
│                                                  │
├──────────────────────────────────────────────────┤
│                   Footer                         │
└──────────────────────────────────────────────────┘
```

---

## 9. 状态颜色映射

| 预约状态 | 颜色 | 标签样式 |
|----------|------|----------|
| pending (待审批) | --warning-500 | 黄色背景 |
| approved (已批准) | --success-500 | 绿色背景 |
| rejected (已拒绝) | --error-500 | 红色背景 |
| checked_in (已签到) | --primary-500 | 蓝色背景 |
| checked_out (已签退) | --gray-500 | 灰色背景 |
| cancelled (已取消) | --gray-400 | 浅灰背景 |

---

## 10. 响应式断点

```css
/* 移动端 */
@media (max-width: 640px) { }

/* 平板 */
@media (min-width: 641px) and (max-width: 1024px) { }

/* 桌面 */
@media (min-width: 1025px) { }
```
