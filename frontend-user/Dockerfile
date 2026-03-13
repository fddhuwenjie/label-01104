# 访客预约系统 - 用户端 Dockerfile
# 支持 ARM64 和 AMD64 架构

FROM --platform=$BUILDPLATFORM node:20-alpine AS builder

WORKDIR /app

COPY package.json ./

RUN npm install --registry=https://registry.npmmirror.com

COPY . .

RUN npm run build

FROM --platform=$TARGETPLATFORM nginx:alpine

COPY --from=builder /app/dist /usr/share/nginx/html

COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD wget -q --spider http://localhost:80/ || exit 1

CMD ["nginx", "-g", "daemon off;"]
