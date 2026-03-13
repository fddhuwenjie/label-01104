#include "crow.h"
#include "config.h"
#include "database.h"
#include <iostream>

// 声明路由设置函数
namespace controllers {
    void setupAuthRoutes(crow::SimpleApp& app);
    void setupUserRoutes(crow::SimpleApp& app);
    void setupAppointmentRoutes(crow::SimpleApp& app);
    void setupCheckRoutes(crow::SimpleApp& app);
    void setupStatsRoutes(crow::SimpleApp& app);
}

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "   访客预约管理系统 (Visitor System)" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // 初始化数据库
    auto& db = db::Database::getInstance();
    if (!db.initialize(config::DB_PATH)) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return 1;
    }
    
    // 创建Crow应用
    crow::SimpleApp app;
    
    // 启用CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .headers("Content-Type", "Authorization", "X-Requested-With")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method);
    
    // 健康检查
    CROW_ROUTE(app, "/health")
    ([](const crow::request& /*req*/) {
        return crow::response(200, R"({"status":"ok","service":"visitor-system"})");
    });
    
    // 根路径
    CROW_ROUTE(app, "/")
    ([](const crow::request& /*req*/) {
        return crow::response(200, R"({
            "name": "访客预约管理系统 API",
            "version": "1.0.0",
            "endpoints": {
                "auth": "/api/auth/*",
                "users": "/api/users/*",
                "appointments": "/api/appointments/*",
                "check": "/api/check/*",
                "stats": "/api/stats/*"
            }
        })");
    });
    
    // 设置路由
    controllers::setupAuthRoutes(app);
    controllers::setupUserRoutes(app);
    controllers::setupAppointmentRoutes(app);
    controllers::setupCheckRoutes(app);
    controllers::setupStatsRoutes(app);
    
    // 启动服务器
    std::cout << "Starting server on " << config::SERVER_HOST << ":" << config::SERVER_PORT << std::endl;
    app.bindaddr(config::SERVER_HOST)
       .port(config::SERVER_PORT)
       .multithreaded()
       .run();
    
    // 关闭数据库
    db.close();
    
    return 0;
}
