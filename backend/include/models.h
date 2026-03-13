#pragma once

#include <string>
#include <ctime>

namespace models {

// 用户角色枚举
enum class UserRole {
    SUPERADMIN,
    ADMIN,
    EMPLOYEE,
    SECURITY,
    VISITOR
};

// 预约状态枚举
enum class AppointmentStatus {
    PENDING,
    APPROVED,
    REJECTED,
    CHECKED_IN,
    CHECKED_OUT,
    CANCELLED
};

// 用户模型
struct User {
    std::string id;
    std::string username;
    std::string password_hash;
    std::string role;
    std::string real_name;
    std::string phone;
    std::string email;
    std::string department;
    int status = 1;
    std::string created_at;
    std::string updated_at;
};

// 预约模型
struct Appointment {
    std::string id;
    std::string visitor_id;
    std::string host_id;
    std::string purpose;
    std::string visit_date;
    std::string visit_time_start;
    std::string visit_time_end;
    std::string status = "pending";
    std::string approver_id;
    std::string approved_at;
    std::string reject_reason;
    std::string created_at;
    std::string updated_at;
};

// 签到记录模型
struct CheckRecord {
    std::string id;
    std::string appointment_id;
    std::string check_in_time;
    std::string check_out_time;
    std::string security_id;
    std::string gate_location;
    std::string remarks;
    std::string created_at;
};

// 操作日志模型
struct OperationLog {
    std::string id;
    std::string user_id;
    std::string action;
    std::string target_type;
    std::string target_id;
    std::string details;
    std::string ip_address;
    std::string created_at;
};

// 角色前缀转换
inline std::string getRolePrefix(const std::string& role) {
    if (role == "superadmin") return "SA";
    if (role == "admin") return "AD";
    if (role == "employee") return "EM";
    if (role == "security") return "SE";
    if (role == "visitor") return "VI";
    return "XX";
}

// 状态字符串转换
inline std::string statusToString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::PENDING: return "pending";
        case AppointmentStatus::APPROVED: return "approved";
        case AppointmentStatus::REJECTED: return "rejected";
        case AppointmentStatus::CHECKED_IN: return "checked_in";
        case AppointmentStatus::CHECKED_OUT: return "checked_out";
        case AppointmentStatus::CANCELLED: return "cancelled";
        default: return "unknown";
    }
}

} // namespace models
