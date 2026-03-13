#include "crow.h"
#include "json.hpp"
#include "database.h"
#include "jwt_util.h"
#include "password.h"
#include "snowflake.h"
#include "response.h"
#include "models.h"
#include "auth_middleware.h"

using json = nlohmann::json;

namespace controllers {

void setupUserRoutes(crow::SimpleApp& app) {
    
    // 获取用户列表
    CROW_ROUTE(app, "/api/users").methods("GET"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        // 验证权限
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限访问").dump();
            return res;
        }
        
        // 获取查询参数
        int page = 1, pageSize = 20;
        std::string role = "";
        
        auto page_param = req.url_params.get("page");
        auto size_param = req.url_params.get("pageSize");
        auto role_param = req.url_params.get("role");
        
        if (page_param) page = std::stoi(page_param);
        if (size_param) pageSize = std::stoi(size_param);
        if (role_param) role = role_param;
        
        auto& db = db::Database::getInstance();
        auto users = db.getAllUsers(page, pageSize, role);
        int total = db.getUserCount(role);
        
        json list = json::array();
        for (const auto& user : users) {
            json item = {
                {"id", user.id},
                {"username", user.username},
                {"role", user.role},
                {"real_name", user.real_name},
                {"phone", user.phone},
                {"email", user.email},
                {"department", user.department},
                {"status", user.status},
                {"created_at", user.created_at},
                {"updated_at", user.updated_at}
            };
            list.push_back(item);
        }
        
        res.body = utils::paginatedResponse(list, total, page, pageSize).dump();
        return res;
    });
    
    // 获取单个用户
    CROW_ROUTE(app, "/api/users/<string>").methods("GET"_method)
    ([](const crow::request& req, const std::string& id) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload) {
            res.code = 401;
            res.body = utils::errorResponse(utils::ErrorCode::UNAUTHORIZED, "未授权").dump();
            return res;
        }
        
        // 只有管理员或自己可以查看
        if (!middleware::isAdmin(payload->role) && payload->user_id != id) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限访问").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        auto user = db.getUserById(id);
        
        if (!user) {
            res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "用户不存在").dump();
            return res;
        }
        
        json data = {
            {"id", user->id},
            {"username", user->username},
            {"role", user->role},
            {"real_name", user->real_name},
            {"phone", user->phone},
            {"email", user->email},
            {"department", user->department},
            {"status", user->status},
            {"created_at", user->created_at},
            {"updated_at", user->updated_at}
        };
        
        res.body = utils::successResponse(data).dump();
        return res;
    });
    
    // 创建用户 (管理员)
    CROW_ROUTE(app, "/api/users").methods("POST"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限操作").dump();
            return res;
        }
        
        try {
            json body = json::parse(req.body);
            
            std::string username = body.value("username", std::string(""));
            std::string password = body.value("password", std::string("123456"));  // 默认密码
            std::string role = body.value("role", std::string("employee"));
            std::string real_name = body.value("real_name", std::string(""));
            std::string phone = body.value("phone", std::string(""));
            std::string email = body.value("email", std::string(""));
            std::string department = body.value("department", std::string(""));
            
            if (username.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "用户名不能为空").dump();
                return res;
            }
            
            auto& db = db::Database::getInstance();
            
            // 检查用户名是否已存在
            if (db.getUserByUsername(username)) {
                res.body = utils::errorResponse(utils::ErrorCode::ALREADY_EXISTS, "用户名已存在").dump();
                return res;
            }
            
            models::User user;
            user.id = utils::getIdGenerator().nextIdWithPrefix(models::getRolePrefix(role));
            user.username = username;
            user.password_hash = utils::Password::hash(password);
            user.role = role;
            user.real_name = real_name;
            user.phone = phone;
            user.email = email;
            user.department = department;
            user.status = 1;
            
            if (db.createUser(user)) {
                json data = {
                    {"id", user.id},
                    {"username", user.username},
                    {"role", user.role}
                };
                res.body = utils::successResponse(data, "用户创建成功").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "创建失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 更新用户
    CROW_ROUTE(app, "/api/users/<string>").methods("PUT"_method)
    ([](const crow::request& req, const std::string& id) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload) {
            res.code = 401;
            res.body = utils::errorResponse(utils::ErrorCode::UNAUTHORIZED, "未授权").dump();
            return res;
        }
        
        // 只有管理员或自己可以修改
        if (!middleware::isAdmin(payload->role) && payload->user_id != id) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限操作").dump();
            return res;
        }
        
        try {
            json body = json::parse(req.body);
            
            auto& db = db::Database::getInstance();
            auto user = db.getUserById(id);
            
            if (!user) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "用户不存在").dump();
                return res;
            }
            
            // 更新字段
            if (body.contains("real_name")) user->real_name = body["real_name"].get<std::string>();
            if (body.contains("phone")) user->phone = body["phone"].get<std::string>();
            if (body.contains("email")) user->email = body["email"].get<std::string>();
            if (body.contains("department")) user->department = body["department"].get<std::string>();
            
            // 只有管理员可以修改角色和状态
            if (middleware::isAdmin(payload->role)) {
                if (body.contains("role")) user->role = body["role"].get<std::string>();
                if (body.contains("status")) user->status = body["status"].get<int>();
            }
            
            if (db.updateUser(*user)) {
                res.body = utils::successResponse(nullptr, "更新成功").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "更新失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 删除用户
    CROW_ROUTE(app, "/api/users/<string>").methods("DELETE"_method)
    ([](const crow::request& req, const std::string& id) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限操作").dump();
            return res;
        }
        
        // 不能删除自己
        if (payload->user_id == id) {
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "不能删除自己").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        
        if (db.deleteUser(id)) {
            res.body = utils::successResponse(nullptr, "删除成功").dump();
        } else {
            res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "删除失败").dump();
        }
        
        return res;
    });
    
    // 获取可被访问的人员列表 (管理员、员工 - 用于预约时选择被访问人)
    CROW_ROUTE(app, "/api/employees").methods("GET"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload) {
            res.code = 401;
            res.body = utils::errorResponse(utils::ErrorCode::UNAUTHORIZED, "未授权").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        
        // 获取所有内部人员 (superadmin, admin, employee)
        json list = json::array();
        
        // 获取超级管理员
        auto superadmins = db.getAllUsers(1, 100, "superadmin");
        for (const auto& user : superadmins) {
            if (user.status == 1) {  // 只返回启用的用户
                list.push_back({
                    {"id", user.id},
                    {"username", user.username},
                    {"real_name", user.real_name},
                    {"department", user.department.empty() ? "管理层" : user.department}
                });
            }
        }
        
        // 获取管理员
        auto admins = db.getAllUsers(1, 100, "admin");
        for (const auto& user : admins) {
            if (user.status == 1) {
                list.push_back({
                    {"id", user.id},
                    {"username", user.username},
                    {"real_name", user.real_name},
                    {"department", user.department.empty() ? "管理层" : user.department}
                });
            }
        }
        
        // 获取员工
        auto employees = db.getAllUsers(1, 1000, "employee");
        for (const auto& user : employees) {
            if (user.status == 1) {
                list.push_back({
                    {"id", user.id},
                    {"username", user.username},
                    {"real_name", user.real_name},
                    {"department", user.department}
                });
            }
        }
        
        res.body = utils::successResponse(list).dump();
        return res;
    });
}

} // namespace controllers
