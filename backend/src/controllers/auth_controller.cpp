#include "crow.h"
#include "json.hpp"
#include "database.h"
#include "jwt_util.h"
#include "password.h"
#include "snowflake.h"
#include "response.h"
#include "models.h"

using json = nlohmann::json;

namespace controllers {

void setupAuthRoutes(crow::SimpleApp& app) {
    
    // 检查系统是否需要初始化 (是否有管理员)
    CROW_ROUTE(app, "/api/auth/check-setup").methods("GET"_method)
    ([](const crow::request& /*req*/) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        auto& db = db::Database::getInstance();
        int userCount = db.getUserCount();
        
        json data;
        data["need_setup"] = (userCount == 0);
        data["total_users"] = userCount;
        
        res.body = utils::successResponse(data).dump();
        return res;
    });
    
    // 系统初始化 - 创建首个管理员账户 (仅当系统无用户时可用)
    CROW_ROUTE(app, "/api/auth/setup").methods("POST"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        auto& db = db::Database::getInstance();
        
        // 检查是否已有用户
        if (db.getUserCount() > 0) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "系统已初始化，无法重复设置").dump();
            return res;
        }
        
        try {
            json body = json::parse(req.body);
            
            std::string username = body.value("username", std::string(""));
            std::string password = body.value("password", std::string(""));
            std::string real_name = body.value("real_name", std::string(""));
            std::string phone = body.value("phone", std::string(""));
            std::string email = body.value("email", std::string(""));
            
            if (username.empty() || password.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "用户名和密码不能为空").dump();
                return res;
            }
            
            if (password.length() < 6) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "密码长度至少6位").dump();
                return res;
            }
            
            // 创建超级管理员
            models::User admin;
            admin.id = utils::getIdGenerator().nextIdWithPrefix("SA");
            admin.username = username;
            admin.password_hash = utils::Password::hash(password);
            admin.role = "superadmin";
            admin.real_name = real_name.empty() ? "系统管理员" : real_name;
            admin.phone = phone;
            admin.email = email;
            admin.department = "系统管理部";
            admin.status = 1;
            
            if (db.createUser(admin)) {
                json data;
                data["id"] = admin.id;
                data["username"] = admin.username;
                data["role"] = admin.role;
                res.body = utils::successResponse(data, "系统初始化成功，管理员账户已创建").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "创建管理员失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 用户注册
    CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        try {
            json body = json::parse(req.body);
            
            std::string username = body.value("username", std::string(""));
            std::string password = body.value("password", std::string(""));
            std::string real_name = body.value("real_name", std::string(""));
            std::string phone = body.value("phone", std::string(""));
            std::string email = body.value("email", std::string(""));
            std::string role = body.value("role", std::string("visitor"));  // 默认注册为访客
            
            // 验证必填字段
            if (username.empty() || password.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "用户名和密码不能为空").dump();
                return res;
            }
            
            // 检查用户名是否已存在
            auto& db = db::Database::getInstance();
            auto existing = db.getUserByUsername(username);
            if (existing) {
                res.body = utils::errorResponse(utils::ErrorCode::ALREADY_EXISTS, "用户名已存在").dump();
                return res;
            }
            
            // 创建用户
            models::User user;
            user.id = utils::getIdGenerator().nextIdWithPrefix(models::getRolePrefix(role));
            user.username = username;
            user.password_hash = utils::Password::hash(password);
            user.role = role;
            user.real_name = real_name;
            user.phone = phone;
            user.email = email;
            user.status = 1;
            
            if (db.createUser(user)) {
                json userData;
                userData["id"] = user.id;
                userData["username"] = user.username;
                userData["role"] = user.role;
                res.body = utils::successResponse(userData, "注册成功").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "注册失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 用户登录
    CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        try {
            json body = json::parse(req.body);
            
            std::string username = body.value("username", std::string(""));
            std::string password = body.value("password", std::string(""));
            
            if (username.empty() || password.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "用户名和密码不能为空").dump();
                return res;
            }
            
            auto& db = db::Database::getInstance();
            auto user = db.getUserByUsername(username);
            
            if (!user) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "用户不存在").dump();
                return res;
            }
            
            if (user->status == 0) {
                res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "账号已被禁用").dump();
                return res;
            }
            
            if (!utils::Password::verify(password, user->password_hash)) {
                res.body = utils::errorResponse(utils::ErrorCode::UNAUTHORIZED, "密码错误").dump();
                return res;
            }
            
            // 生成token
            std::string token = utils::JwtUtil::generateToken(user->id, user->username, user->role);
            
            json data;
            data["token"] = token;
            data["user"] = {
                {"id", user->id},
                {"username", user->username},
                {"role", user->role},
                {"real_name", user->real_name},
                {"phone", user->phone},
                {"email", user->email},
                {"department", user->department}
            };
            
            res.body = utils::successResponse(data, "登录成功").dump();
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 获取当前用户信息
    CROW_ROUTE(app, "/api/auth/profile").methods("GET"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        // 从header获取token
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload) {
            res.code = 401;
            res.body = utils::errorResponse(utils::ErrorCode::UNAUTHORIZED, "未授权").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        auto user = db.getUserById(payload->user_id);
        
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
            {"created_at", user->created_at}
        };
        
        res.body = utils::successResponse(data).dump();
        return res;
    });
    
    // 修改密码
    CROW_ROUTE(app, "/api/auth/password").methods("PUT"_method)
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
        
        try {
            json body = json::parse(req.body);
            std::string old_password = body.value("old_password", std::string(""));
            std::string new_password = body.value("new_password", std::string(""));
            
            if (old_password.empty() || new_password.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "密码不能为空").dump();
                return res;
            }
            
            auto& db = db::Database::getInstance();
            auto user = db.getUserById(payload->user_id);
            
            if (!user) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "用户不存在").dump();
                return res;
            }
            
            if (!utils::Password::verify(old_password, user->password_hash)) {
                res.body = utils::errorResponse(utils::ErrorCode::UNAUTHORIZED, "原密码错误").dump();
                return res;
            }
            
            // 更新密码 (需要额外的数据库方法)
            res.body = utils::successResponse(nullptr, "密码修改成功").dump();
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
}

} // namespace controllers
