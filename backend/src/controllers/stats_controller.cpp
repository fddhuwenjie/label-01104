#include "crow.h"
#include "json.hpp"
#include "database.h"
#include "jwt_util.h"
#include "response.h"
#include "auth_middleware.h"
#include <ctime>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

namespace controllers {

static std::string getDateString(int days_offset = 0) {
    auto now = std::time(nullptr);
    now += days_offset * 24 * 60 * 60;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d");
    return ss.str();
}

void setupStatsRoutes(crow::SimpleApp& app) {
    
    // 获取概览统计
    CROW_ROUTE(app, "/api/stats/overview").methods("GET"_method)
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
        
        // 只有管理员可以查看统计
        if (!middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限查看").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        json stats = db.getOverviewStats();
        
        res.body = utils::successResponse(stats).dump();
        return res;
    });
    
    // 获取访客统计
    CROW_ROUTE(app, "/api/stats/visitors").methods("GET"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限查看").dump();
            return res;
        }
        
        // 默认查询最近7天
        std::string start_date = getDateString(-6);
        std::string end_date = getDateString(0);
        
        auto start_param = req.url_params.get("start_date");
        auto end_param = req.url_params.get("end_date");
        
        if (start_param) start_date = start_param;
        if (end_param) end_date = end_param;
        
        auto& db = db::Database::getInstance();
        json stats = db.getVisitorStats(start_date, end_date);
        
        res.body = utils::successResponse(stats).dump();
        return res;
    });
    
    // 获取预约统计
    CROW_ROUTE(app, "/api/stats/appointments").methods("GET"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限查看").dump();
            return res;
        }
        
        std::string start_date = getDateString(-30);
        std::string end_date = getDateString(0);
        
        auto start_param = req.url_params.get("start_date");
        auto end_param = req.url_params.get("end_date");
        
        if (start_param) start_date = start_param;
        if (end_param) end_date = end_param;
        
        auto& db = db::Database::getInstance();
        json stats = db.getAppointmentStats(start_date, end_date);
        
        res.body = utils::successResponse(stats).dump();
        return res;
    });
}

} // namespace controllers
