#include "crow.h"
#include "json.hpp"
#include "database.h"
#include "jwt_util.h"
#include "snowflake.h"
#include "response.h"
#include "models.h"
#include "auth_middleware.h"
#include <ctime>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

namespace controllers {

static std::string getCurrentTime() {
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void setupCheckRoutes(crow::SimpleApp& app) {
    
    // 访客签到
    CROW_ROUTE(app, "/api/check/in").methods("POST"_method)
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
        
        // 只有保安和管理员可以操作签到
        if (payload->role != "security" && !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限操作").dump();
            return res;
        }
        
        try {
            json body = json::parse(req.body);
            
            std::string appointment_id = body.value("appointment_id", std::string(""));
            std::string gate_location = body.value("gate_location", std::string("正门"));
            std::string remarks = body.value("remarks", std::string(""));
            
            if (appointment_id.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "预约ID不能为空").dump();
                return res;
            }
            
            auto& db = db::Database::getInstance();
            auto apt = db.getAppointmentById(appointment_id);
            
            if (!apt) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "预约不存在").dump();
                return res;
            }
            
            if (apt->status != "approved") {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "该预约未通过审批，无法签到").dump();
                return res;
            }
            
            // 检查是否已签到
            auto existingRecord = db.getCheckRecordByAppointmentId(appointment_id);
            if (existingRecord) {
                res.body = utils::errorResponse(utils::ErrorCode::ALREADY_EXISTS, "该预约已签到").dump();
                return res;
            }
            
            // 创建签到记录
            models::CheckRecord record;
            record.id = utils::getIdGenerator().nextIdWithPrefix("CR");
            record.appointment_id = appointment_id;
            record.check_in_time = getCurrentTime();
            record.security_id = payload->user_id;
            record.gate_location = gate_location;
            record.remarks = remarks;
            
            if (db.createCheckRecord(record) && db.updateAppointmentStatus(appointment_id, "checked_in")) {
                json data = {
                    {"record_id", record.id},
                    {"check_in_time", record.check_in_time}
                };
                res.body = utils::successResponse(data, "签到成功").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "签到失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 访客签退
    CROW_ROUTE(app, "/api/check/out").methods("POST"_method)
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
        
        if (payload->role != "security" && !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限操作").dump();
            return res;
        }
        
        try {
            json body = json::parse(req.body);
            
            std::string appointment_id = body.value("appointment_id", std::string(""));
            
            if (appointment_id.empty()) {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "预约ID不能为空").dump();
                return res;
            }
            
            auto& db = db::Database::getInstance();
            auto apt = db.getAppointmentById(appointment_id);
            
            if (!apt) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "预约不存在").dump();
                return res;
            }
            
            if (apt->status != "checked_in") {
                res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "该预约未签到或已签退").dump();
                return res;
            }
            
            auto record = db.getCheckRecordByAppointmentId(appointment_id);
            if (!record) {
                res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "签到记录不存在").dump();
                return res;
            }
            
            // 更新签退时间
            record->check_out_time = getCurrentTime();
            
            if (db.updateCheckRecord(*record) && db.updateAppointmentStatus(appointment_id, "checked_out")) {
                json data = {
                    {"check_out_time", record->check_out_time}
                };
                res.body = utils::successResponse(data, "签退成功").dump();
            } else {
                res.body = utils::errorResponse(utils::ErrorCode::INTERNAL_ERROR, "签退失败").dump();
            }
        } catch (const std::exception& e) {
            res.body = utils::errorResponse(utils::ErrorCode::INVALID_PARAMS, "请求参数错误").dump();
        }
        
        return res;
    });
    
    // 获取签到记录列表
    CROW_ROUTE(app, "/api/check/records").methods("GET"_method)
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
        
        // 只有保安和管理员可以查看签到记录
        if (payload->role != "security" && !middleware::isAdmin(payload->role)) {
            res.code = 403;
            res.body = utils::errorResponse(utils::ErrorCode::FORBIDDEN, "无权限查看").dump();
            return res;
        }
        
        int page = 1, pageSize = 20;
        auto page_param = req.url_params.get("page");
        auto size_param = req.url_params.get("pageSize");
        
        if (page_param) page = std::stoi(page_param);
        if (size_param) pageSize = std::stoi(size_param);
        
        auto& db = db::Database::getInstance();
        auto records = db.getCheckRecords(page, pageSize);
        
        json list = json::array();
        for (const auto& record : records) {
            // 获取预约和访客信息
            auto apt = db.getAppointmentById(record.appointment_id);
            std::string visitor_name = "";
            std::string host_name = "";
            
            if (apt) {
                auto visitor = db.getUserById(apt->visitor_id);
                auto host = db.getUserById(apt->host_id);
                if (visitor) visitor_name = visitor->real_name;
                if (host) host_name = host->real_name;
            }
            
            json item = {
                {"id", record.id},
                {"appointment_id", record.appointment_id},
                {"visitor_name", visitor_name},
                {"host_name", host_name},
                {"check_in_time", record.check_in_time},
                {"check_out_time", record.check_out_time},
                {"gate_location", record.gate_location},
                {"remarks", record.remarks},
                {"created_at", record.created_at}
            };
            list.push_back(item);
        }
        
        res.body = utils::successResponse(list).dump();
        return res;
    });
    
    // 通过预约ID获取签到记录
    CROW_ROUTE(app, "/api/check/record/<string>").methods("GET"_method)
    ([](const crow::request& req, const std::string& appointment_id) {
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
        auto record = db.getCheckRecordByAppointmentId(appointment_id);
        
        if (!record) {
            res.body = utils::errorResponse(utils::ErrorCode::NOT_FOUND, "签到记录不存在").dump();
            return res;
        }
        
        json data = {
            {"id", record->id},
            {"appointment_id", record->appointment_id},
            {"check_in_time", record->check_in_time},
            {"check_out_time", record->check_out_time},
            {"gate_location", record->gate_location},
            {"remarks", record->remarks},
            {"created_at", record->created_at}
        };
        
        res.body = utils::successResponse(data).dump();
        return res;
    });
}

} // namespace controllers
