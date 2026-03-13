#pragma once

#include <string>

namespace config {
    // 服务器配置
    constexpr int SERVER_PORT = 8080;
    constexpr const char* SERVER_HOST = "0.0.0.0";
    
    // 数据库配置
    constexpr const char* DB_PATH = "./data/visitor.db";
    
    // JWT配置
    constexpr const char* JWT_SECRET = "visitor_system_secret_key_2024";
    constexpr int JWT_EXPIRE_HOURS = 24;
    
    // 雪花算法配置
    constexpr int SNOWFLAKE_WORKER_ID = 1;
    constexpr int SNOWFLAKE_DATACENTER_ID = 1;
}
