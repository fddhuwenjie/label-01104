#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "sqlite3.h"
#include "models.h"
#include "json.hpp"

namespace db {

using json = nlohmann::json;

class Database {
public:
    static Database& getInstance();
    
    // 禁止拷贝
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // 初始化数据库
    bool initialize(const std::string& db_path);
    
    // 关闭数据库
    void close();
    
    // ====== 用户操作 ======
    bool createUser(const models::User& user);
    bool updateUser(const models::User& user);
    bool deleteUser(const std::string& id);
    std::unique_ptr<models::User> getUserById(const std::string& id);
    std::unique_ptr<models::User> getUserByUsername(const std::string& username);
    std::vector<models::User> getAllUsers(int page = 1, int pageSize = 20, const std::string& role = "");
    int getUserCount(const std::string& role = "");
    
    // ====== 预约操作 ======
    bool createAppointment(const models::Appointment& appointment);
    bool updateAppointment(const models::Appointment& appointment);
    bool updateAppointmentStatus(const std::string& id, const std::string& status, 
                                  const std::string& approver_id = "", 
                                  const std::string& reject_reason = "");
    bool deleteAppointment(const std::string& id);
    std::unique_ptr<models::Appointment> getAppointmentById(const std::string& id);
    std::vector<models::Appointment> getAppointments(int page = 1, int pageSize = 20, 
                                                      const std::string& status = "",
                                                      const std::string& visitor_id = "",
                                                      const std::string& host_id = "");
    int getAppointmentCount(const std::string& status = "", 
                            const std::string& visitor_id = "",
                            const std::string& host_id = "");
    
    // ====== 签到记录操作 ======
    bool createCheckRecord(const models::CheckRecord& record);
    bool updateCheckRecord(const models::CheckRecord& record);
    std::unique_ptr<models::CheckRecord> getCheckRecordByAppointmentId(const std::string& appointment_id);
    std::vector<models::CheckRecord> getCheckRecords(int page = 1, int pageSize = 20);
    
    // ====== 操作日志 ======
    bool createLog(const models::OperationLog& log);
    std::vector<models::OperationLog> getLogs(int page = 1, int pageSize = 50);
    
    // ====== 统计查询 ======
    json getOverviewStats();
    json getVisitorStats(const std::string& start_date, const std::string& end_date);
    json getAppointmentStats(const std::string& start_date, const std::string& end_date);

private:
    Database() = default;
    ~Database();
    
    sqlite3* db_ = nullptr;
    
    // 创建表
    bool createTables();
    
    // 初始化默认数据
    bool initDefaultData();
    
    // 执行SQL
    bool execute(const std::string& sql);
    
    // 获取当前时间字符串
    std::string getCurrentTime();
};

} // namespace db
