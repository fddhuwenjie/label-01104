#include "auth_middleware.h"
#include <algorithm>

namespace middleware {

// 不需要认证的路径列表
static const std::vector<std::string> public_paths = {
    "/api/auth/login",
    "/api/auth/register",
    "/health",
    "/"
};

bool requiresAuth(const std::string& path) {
    for (const auto& p : public_paths) {
        if (path == p) {
            return false;
        }
    }
    return true;
}

bool hasPermission(const std::string& role, const std::string& required_role) {
    // 超级管理员拥有所有权限
    if (role == "superadmin") {
        return true;
    }
    
    // 管理员权限
    if (role == "admin") {
        return required_role == "admin" || 
               required_role == "employee" || 
               required_role == "security" ||
               required_role == "visitor";
    }
    
    // 其他角色只能访问自己的权限
    return role == required_role;
}

bool isAdmin(const std::string& role) {
    return role == "superadmin" || role == "admin";
}

void AuthMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    // 检查是否需要认证
    if (!requiresAuth(req.url)) {
        ctx.authenticated = true;
        return;
    }
    
    // 获取Authorization头
    std::string auth_header = req.get_header_value("Authorization");
    if (auth_header.empty()) {
        res.code = 401;
        res.set_header("Content-Type", "application/json");
        res.body = R"({"code":1002,"message":"未授权访问","data":null})";
        res.end();
        return;
    }
    
    // 提取并验证token
    std::string token = utils::JwtUtil::extractToken(auth_header);
    auto payload = utils::JwtUtil::verifyToken(token);
    
    if (!payload) {
        res.code = 401;
        res.set_header("Content-Type", "application/json");
        res.body = R"({"code":1002,"message":"Token无效或已过期","data":null})";
        res.end();
        return;
    }
    
    // 设置上下文
    ctx.user_id = payload->user_id;
    ctx.username = payload->username;
    ctx.role = payload->role;
    ctx.authenticated = true;
}

void AuthMiddleware::after_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/) {
    // 后处理（如日志记录等）
}

} // namespace middleware
