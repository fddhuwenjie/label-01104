# 访客预约系统 - 后端 Dockerfile
# 支持 ARM64 和 AMD64 架构

FROM debian:bookworm-slim AS builder

# 安装编译依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    unzip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 下载 SQLite 源码
RUN wget -q https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip \
    && unzip -q sqlite-amalgamation-3450000.zip \
    && rm sqlite-amalgamation-3450000.zip

# 复制项目文件
COPY . .

# 使用下载的 SQLite 源码替换占位文件
RUN cp sqlite-amalgamation-3450000/sqlite3.c third_party/sqlite/ \
    && cp sqlite-amalgamation-3450000/sqlite3.h third_party/sqlite/

# 编译项目
RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc)

# 运行时镜像
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    wget \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 复制编译好的可执行文件
COPY --from=builder /app/build/visitor_system /app/

# 创建数据目录
RUN mkdir -p /app/data

# 暴露端口
EXPOSE 8080

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 \
    CMD wget -q -O /dev/null http://127.0.0.1:8080/health || exit 1

# 启动服务
CMD ["./visitor_system"]
