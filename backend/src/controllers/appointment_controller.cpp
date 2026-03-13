#include "crow.h"
#include "json.hpp"
#include "database.h"
#include "jwt_util.h"
#include "snowflake.h"
#include "response.h"
#include "models.h"
#include "auth_middleware.h"

using json = nlohmann::json;

namespace controllers {

void setupAppointmentRoutes(crow::SimpleApp& app) {
    
    // 获取预约列表
    CROW_ROUTE(app, "/api/appointments").methods("GET"_method)
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
        
        int page = 1, pageSize = 20;
        std::string status = "", visitor_id = "", host_id = "";
        
        auto page_param = req.url_params.get("page");
        auto size_param = req.url_params.get("pageSize");
        auto status_param = req.url_params.get("status");
        
        if (page_param) page = std::stoi(page_param);
        if (size_param) pageSize = std::stoi(size_param);
        if (status_param) status = status_param;
        
        auto& db = db::Database::getInstance();
        
        // 根据角色过滤数据
        if (payload->role == "visitor") {
            visitor_id = payload->user_id;
        } else if (payload->role == "employee") {
            host_id = payload->user_id;
        }
        // 管理员和保安可以看所有预约
        
        auto appointments = db.getAppointments(page, pageSize, status, visitor_id, host_id);
        int total = db.getAppointmentCount(status, visitor_id, host_id);
        
        json list = json::array();
        for (const auto& apt : appointments) {
            // 获取访客和被访人信息
            auto visitor = db.getUserById(apt.visitor_id);
            auto host = db.getUserById(apt.host_id);
            
            json item = {
                {"id", apt.id},
                {"visitor_id", apt.visitor_id},
                {"visitor_name", visitor ? visitor->real_name : ""},
                {"visitor_phone", visitor ? visitor->phone : ""},
                {"host_id", apt.host_id},
                {"host_name", host ? host->real_name : ""},
                {"host_department", host ? host->department : ""},
                {"purpose", apt.purpose},
                {"visit_date", apt.visit_date},
                {"visit_time_start", apt.visit_time_start},
                {"visit_time_end", apt.visit_time_end},
                {"status", apt.status},
                {"approver_id", apt.approver_id},
                {"approved_at", apt.approved_at},
                {"reject_reason", apt.reject_reason},
                {"created_at", apt.created_at},
                {"updated_at", apt.updated_at}
            };
            list.push_back(item);
        }
        
        res.body = utils::paginatedResponse(list, total, page, pageSize).dump();
        return res;
    });
    
    // 获取单个预约
    CROW_ROUTE(app, "/api/appointments/<string>").methods("GET"_method)
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
        
        auto& db = db::Database::getInstance();
        auto apt = db.getAppointmentById(id);
        
        if (!apt) {
            res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "预约不存在").dump();
            return res;
        }
        
        // 检查权限
        if (payload->role == "visitor" && apt->visitor_id != payload->user_id) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限查看").dump();
            return res;
        }
        if (payload->role == "employee" && apt->host_id != payload->user_id) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限查看").dump();
            return res;
        }
        
        auto visitor = db.getUserById(apt->visitor_id);
        auto host = db.getUserById(apt->host_id);
        
        json data = {
            {"id", apt->id},
            {"visitor_id", apt->visitor_id},
            {"visitor_name", visitor ? visitor->real_name : ""},
            {"visitor_phone", visitor ? visitor->phone : ""},
            {"host_id", apt->host_id},
            {"host_name", host ? host->real_name : ""},
            {"host_department", host ? host->department : ""},
            {"purpose", apt->purpose},
            {"visit_date", apt->visit_date},
            {"visit_time_start", apt->visit_time_start},
            {"visit_time_end", apt->visit_time_end},
            {"status", apt->status},
            {"approver_id", apt->approver_id},
            {"approved_at", apt->approved_at},
            {"reject_reason", apt->reject_reason},
            {"created_at", apt->created_at},
            {"updated_at", apt->updated_at}
        };
        
        res.body = utils::successResponse(data).dump();
        return res;
    });
    
    // 创建预约
    CROW_ROUTE(app, "/api/appointments").methods("POST"_method)
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
            
            std::string host_id = body.value("host_id", std::string(""));
            std::string purpose = body.value("purpose", std::string(""));
            std::string visit_date = body.value("visit_date", std::string(""));
            std::string visit_time_start = body.value("visit_time_start", std::string("09:00"));
            std::string visit_time_end = body.value("visit_time_end", std::string("18:00"));
            
            if (host_id.empty() || visit_date.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "被访人和访问日期不能为空").dump();
                return res;
            }
            
            auto& db = db::Database::getInstance();
            
            // 检查被访人是否存在 (可以是员工、管理员或超级管理员)
            auto host = db.getUserById(host_id);
            if (!host || (host->role != "employee" && host->role != "admin" && host->role != "superadmin")) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "被访人不存在或不可访问").dump();
                return res;
            }
            
            // 确定访客ID
            std::string visitor_id = payload->user_id;
            if (payload->role == "employee") {
                // 员工邀请访客时，visitor_id从请求中获取
                if (body.contains("visitor_id") && !body["visitor_id"].empty()) {
                    visitor_id = body["visitor_id"].get<std::string>();
                }
            }
            
            models::Appointment apt;
            apt.id = utils::getIdGenerator().nextIdWithPrefix("AP");
            apt.visitor_id = visitor_id;
            apt.host_id = host_id;
            apt.purpose = purpose;
            apt.visit_date = visit_date;
            apt.visit_time_start = visit_time_start;
            apt.visit_time_end = visit_time_end;
            apt.status = "pending";
            
            if (db.createAppointment(apt)) {
                json data = {
                    {"id", apt.id},
                    {"status", apt.status}
                };
                res.body = utils::successResponse(data, "预约提交成功").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "预约失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 审批预约 - 批准
    CROW_ROUTE(app, "/api/appointments/<string>/approve").methods("POST"_method)
    ([](const crow::request& req, const std::string& id) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无审批权限").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        auto apt = db.getAppointmentById(id);
        
        if (!apt) {
            res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "预约不存在").dump();
            return res;
        }
        
        if (apt->status != "pending") {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "该预约不是待审批状态").dump();
            return res;
        }
        
        if (db.updateAppointmentStatus(id, "approved", payload->user_id)) {
            res.body = utils::successResponse(nullptr, "审批通过").dump();
        } else {
            res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "审批失败").dump();
        }
        
        return res;
    });
    
    // 审批预约 - 拒绝
    CROW_ROUTE(app, "/api/appointments/<string>/reject").methods("POST"_method)
    ([](const crow::request& req, const std::string& id) {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        std::string auth_header = req.get_header_value("Authorization");
        std::string token = utils::JwtUtil::extractToken(auth_header);
        auto payload = utils::JwtUtil::verifyToken(token);
        
        if (!payload || !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无审批权限").dump();
            return res;
        }
        
        auto& db = db::Database::getInstance();
        auto apt = db.getAppointmentById(id);
        
        if (!apt) {
            res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "预约不存在").dump();
            return res;
        }
        
        if (apt->status != "pending") {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "该预约不是待审批状态").dump();
            return res;
        }
        
        std::string reason = "";
        try {
            json body = json::parse(req.body);
            reason = body.value("reason", std::string(""));
        } catch (...) {}
        
        if (db.updateAppointmentStatus(id, "rejected", payload->user_id, reason)) {
            res.body = utils::successResponse(nullptr, "已拒绝").dump();
        } else {
            res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "操作失败").dump();
        }
        
        return res;
    });
    
    // 取消预约
    CROW_ROUTE(app, "/api/appointments/<string>/cancel").methods("POST"_method)
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
        
        auto& db = db::Database::getInstance();
        auto apt = db.getAppointmentById(id);
        
        if (!apt) {
            res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "预约不存在").dump();
            return res;
        }
        
        // 只有访客本人或管理员可以取消
        if (payload->role == "visitor" && apt->visitor_id != payload->user_id) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限取消").dump();
            return res;
        }
        
        if (apt->status == "checked_in" || apt->status == "checked_out") {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "该预约已签到，无法取消").dump();
            return res;
        }
        
        if (db.updateAppointmentStatus(id, "cancelled")) {
            res.body = utils::successResponse(nullptr, "已取消").dump();
        } else {
            res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "取消失败").dump();
        }
        
        return res;
    });
}

} // namespace controllers
