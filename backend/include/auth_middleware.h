#pragma once

#include "crow.h"
#include "jwt_util.h"
#include <vector>
#include <string>

namespace middleware {

// 认证中间件
struct AuthMiddleware {
    struct context {
        std::string user_id;
        std::string username;
        std::string role;
        bool authenticated = false;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx);
};

// 检查是否需要认证的路径
bool requiresAuth(const std::string& path);

// 检查角色权限
bool hasPermission(const std::string& role, const std::string& required_role);

// 检查是否是管理员角色
bool isAdmin(const std::string& role);

} // namespace middleware
