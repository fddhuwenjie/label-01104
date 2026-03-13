#include "database.h"
#include "snowflake.h"
#include "password.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>

namespace db {

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

Database::~Database() {
    close();
}

std::string Database::getCurrentTime() {
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool Database::initialize(const std::string& db_path) {
    // 创建目录
    std::string dir = db_path.substr(0, db_path.find_last_of('/'));
    mkdir(dir.c_str(), 0755);
    
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    // 启用外键约束
    execute("PRAGMA foreign_keys = ON;");
    
    if (!createTables()) {
        std::cerr << "Failed to create tables" << std::endl;
        return false;
    }
    
    if (!initDefaultData()) {
        std::cerr << "Failed to init default data" << std::endl;
        return false;
    }
    
    std::cout << "Database initialized successfully" << std::endl;
    return true;
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id VARCHAR(32) PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(256) NOT NULL,
            role VARCHAR(20) NOT NULL,
            real_name VARCHAR(50),
            phone VARCHAR(20),
            email VARCHAR(100),
            department VARCHAR(50),
            status INT DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS appointments (
            id VARCHAR(32) PRIMARY KEY,
            visitor_id VARCHAR(32) NOT NULL,
            host_id VARCHAR(32) NOT NULL,
            purpose VARCHAR(200),
            visit_date DATE NOT NULL,
            visit_time_start TIME,
            visit_time_end TIME,
            status VARCHAR(20) DEFAULT 'pending',
            approver_id VARCHAR(32),
            approved_at DATETIME,
            reject_reason VARCHAR(200),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (visitor_id) REFERENCES users(id),
            FOREIGN KEY (host_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS check_records (
            id VARCHAR(32) PRIMARY KEY,
            appointment_id VARCHAR(32) NOT NULL,
            check_in_time DATETIME,
            check_out_time DATETIME,
            security_id VARCHAR(32),
            gate_location VARCHAR(50),
            remarks VARCHAR(200),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (appointment_id) REFERENCES appointments(id)
        );

        CREATE TABLE IF NOT EXISTS operation_logs (
            id VARCHAR(32) PRIMARY KEY,
            user_id VARCHAR(32),
            action VARCHAR(50),
            target_type VARCHAR(50),
            target_id VARCHAR(32),
            details TEXT,
            ip_address VARCHAR(50),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_appointments_visitor ON appointments(visitor_id);
        CREATE INDEX IF NOT EXISTS idx_appointments_host ON appointments(host_id);
        CREATE INDEX IF NOT EXISTS idx_appointments_status ON appointments(status);
        CREATE INDEX IF NOT EXISTS idx_appointments_date ON appointments(visit_date);
        CREATE INDEX IF NOT EXISTS idx_check_records_appointment ON check_records(appointment_id);
    )";
    
    return execute(sql);
}

bool Database::initDefaultData() {
    // 不再创建默认账户
    // 首次运行时通过 /api/auth/setup 创建管理员账户
    return true;
}

// ====== 用户操作实现 ======

bool Database::createUser(const models::User& user) {
    const char* sql = R"(
        INSERT INTO users (id, username, password_hash, role, real_name, phone, email, department, status, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    
    sqlite3_bind_text(stmt, 1, user.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.password_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.real_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, user.department.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 9, user.status);
    sqlite3_bind_text(stmt, 10, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, now.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::updateUser(const models::User& user) {
    const char* sql = R"(
        UPDATE users SET 
            real_name = ?, phone = ?, email = ?, department = ?, 
            role = ?, status = ?, updated_at = ?
        WHERE id = ?
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    
    sqlite3_bind_text(stmt, 1, user.real_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.department.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, user.status);
    sqlite3_bind_text(stmt, 7, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, user.id.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::deleteUser(const std::string& id) {
    const char* sql = "DELETE FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::unique_ptr<models::User> Database::getUserById(const std::string& id) {
    const char* sql = "SELECT * FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<models::User>();
        user->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->real_name = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        user->phone = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        user->email = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        user->department = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        user->status = sqlite3_column_int(stmt, 8);
        user->created_at = sqlite3_column_text(stmt, 9) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        user->updated_at = sqlite3_column_text(stmt, 10) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return nullptr;
}

std::unique_ptr<models::User> Database::getUserByUsername(const std::string& username) {
    const char* sql = "SELECT * FROM users WHERE username = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<models::User>();
        user->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->real_name = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        user->phone = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        user->email = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        user->department = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        user->status = sqlite3_column_int(stmt, 8);
        user->created_at = sqlite3_column_text(stmt, 9) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        user->updated_at = sqlite3_column_text(stmt, 10) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return nullptr;
}

std::vector<models::User> Database::getAllUsers(int page, int pageSize, const std::string& role) {
    std::vector<models::User> users;
    std::string sql = "SELECT * FROM users";
    
    if (!role.empty()) {
        sql += " WHERE role = ?";
    }
    sql += " ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    int paramIdx = 1;
    if (!role.empty()) {
        sqlite3_bind_text(stmt, paramIdx++, role.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_int(stmt, paramIdx++, pageSize);
    sqlite3_bind_int(stmt, paramIdx, (page - 1) * pageSize);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        models::User user;
        user.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.password_hash = "";  // 不返回密码
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.real_name = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        user.phone = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        user.email = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        user.department = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        user.status = sqlite3_column_int(stmt, 8);
        user.created_at = sqlite3_column_text(stmt, 9) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        user.updated_at = sqlite3_column_text(stmt, 10) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        users.push_back(user);
    }
    
    sqlite3_finalize(stmt);
    return users;
}

int Database::getUserCount(const std::string& role) {
    std::string sql = "SELECT COUNT(*) FROM users";
    if (!role.empty()) {
        sql += " WHERE role = ?";
    }
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (!role.empty()) {
        sqlite3_bind_text(stmt, 1, role.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    
    return count;
}

// ====== 预约操作实现 ======

bool Database::createAppointment(const models::Appointment& apt) {
    const char* sql = R"(
        INSERT INTO appointments (id, visitor_id, host_id, purpose, visit_date, 
            visit_time_start, visit_time_end, status, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    
    sqlite3_bind_text(stmt, 1, apt.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, apt.visitor_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, apt.host_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, apt.purpose.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, apt.visit_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, apt.visit_time_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, apt.visit_time_end.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, apt.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, now.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::updateAppointment(const models::Appointment& apt) {
    const char* sql = R"(
        UPDATE appointments SET 
            purpose = ?, visit_date = ?, visit_time_start = ?, 
            visit_time_end = ?, updated_at = ?
        WHERE id = ?
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    
    sqlite3_bind_text(stmt, 1, apt.purpose.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, apt.visit_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, apt.visit_time_start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, apt.visit_time_end.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, apt.id.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::updateAppointmentStatus(const std::string& id, const std::string& status,
                                        const std::string& approver_id,
                                        const std::string& reject_reason) {
    std::string sql = "UPDATE appointments SET status = ?, updated_at = ?";
    
    if (!approver_id.empty()) {
        sql += ", approver_id = ?, approved_at = ?";
    }
    if (!reject_reason.empty()) {
        sql += ", reject_reason = ?";
    }
    sql += " WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    int idx = 1;
    
    sqlite3_bind_text(stmt, idx++, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, now.c_str(), -1, SQLITE_TRANSIENT);
    
    if (!approver_id.empty()) {
        sqlite3_bind_text(stmt, idx++, approver_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, now.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!reject_reason.empty()) {
        sqlite3_bind_text(stmt, idx++, reject_reason.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_text(stmt, idx, id.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::deleteAppointment(const std::string& id) {
    const char* sql = "DELETE FROM appointments WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::unique_ptr<models::Appointment> Database::getAppointmentById(const std::string& id) {
    const char* sql = "SELECT * FROM appointments WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto apt = std::make_unique<models::Appointment>();
        apt->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        apt->visitor_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        apt->host_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        apt->purpose = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        apt->visit_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        apt->visit_time_start = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        apt->visit_time_end = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        apt->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        apt->approver_id = sqlite3_column_text(stmt, 8) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) : "";
        apt->approved_at = sqlite3_column_text(stmt, 9) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        apt->reject_reason = sqlite3_column_text(stmt, 10) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        apt->created_at = sqlite3_column_text(stmt, 11) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11)) : "";
        apt->updated_at = sqlite3_column_text(stmt, 12) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12)) : "";
        
        sqlite3_finalize(stmt);
        return apt;
    }
    
    sqlite3_finalize(stmt);
    return nullptr;
}

std::vector<models::Appointment> Database::getAppointments(int page, int pageSize,
                                                            const std::string& status,
                                                            const std::string& visitor_id,
                                                            const std::string& host_id) {
    std::vector<models::Appointment> appointments;
    std::string sql = "SELECT * FROM appointments WHERE 1=1";
    
    if (!status.empty()) {
        sql += " AND status = ?";
    }
    if (!visitor_id.empty()) {
        sql += " AND visitor_id = ?";
    }
    if (!host_id.empty()) {
        sql += " AND host_id = ?";
    }
    sql += " ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    int idx = 1;
    if (!status.empty()) {
        sqlite3_bind_text(stmt, idx++, status.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!visitor_id.empty()) {
        sqlite3_bind_text(stmt, idx++, visitor_id.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!host_id.empty()) {
        sqlite3_bind_text(stmt, idx++, host_id.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_int(stmt, idx++, pageSize);
    sqlite3_bind_int(stmt, idx, (page - 1) * pageSize);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        models::Appointment apt;
        apt.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        apt.visitor_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        apt.host_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        apt.purpose = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        apt.visit_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        apt.visit_time_start = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        apt.visit_time_end = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        apt.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        apt.approver_id = sqlite3_column_text(stmt, 8) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) : "";
        apt.approved_at = sqlite3_column_text(stmt, 9) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        apt.reject_reason = sqlite3_column_text(stmt, 10) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        apt.created_at = sqlite3_column_text(stmt, 11) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11)) : "";
        apt.updated_at = sqlite3_column_text(stmt, 12) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12)) : "";
        appointments.push_back(apt);
    }
    
    sqlite3_finalize(stmt);
    return appointments;
}

int Database::getAppointmentCount(const std::string& status,
                                   const std::string& visitor_id,
                                   const std::string& host_id) {
    std::string sql = "SELECT COUNT(*) FROM appointments WHERE 1=1";
    
    if (!status.empty()) {
        sql += " AND status = ?";
    }
    if (!visitor_id.empty()) {
        sql += " AND visitor_id = ?";
    }
    if (!host_id.empty()) {
        sql += " AND host_id = ?";
    }
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    int idx = 1;
    if (!status.empty()) {
        sqlite3_bind_text(stmt, idx++, status.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!visitor_id.empty()) {
        sqlite3_bind_text(stmt, idx++, visitor_id.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!host_id.empty()) {
        sqlite3_bind_text(stmt, idx, host_id.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    
    return count;
}

// ====== 签到记录操作实现 ======

bool Database::createCheckRecord(const models::CheckRecord& record) {
    const char* sql = R"(
        INSERT INTO check_records (id, appointment_id, check_in_time, security_id, gate_location, remarks, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    
    sqlite3_bind_text(stmt, 1, record.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.appointment_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, record.check_in_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, record.security_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, record.gate_location.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, record.remarks.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, now.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::updateCheckRecord(const models::CheckRecord& record) {
    const char* sql = "UPDATE check_records SET check_out_time = ? WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    sqlite3_bind_text(stmt, 1, record.check_out_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.id.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::unique_ptr<models::CheckRecord> Database::getCheckRecordByAppointmentId(const std::string& appointment_id) {
    const char* sql = "SELECT * FROM check_records WHERE appointment_id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, appointment_id.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto record = std::make_unique<models::CheckRecord>();
        record->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        record->appointment_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record->check_in_time = sqlite3_column_text(stmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";
        record->check_out_time = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        record->security_id = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        record->gate_location = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        record->remarks = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        record->created_at = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        
        sqlite3_finalize(stmt);
        return record;
    }
    
    sqlite3_finalize(stmt);
    return nullptr;
}

std::vector<models::CheckRecord> Database::getCheckRecords(int page, int pageSize) {
    std::vector<models::CheckRecord> records;
    const char* sql = "SELECT * FROM check_records ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, pageSize);
    sqlite3_bind_int(stmt, 2, (page - 1) * pageSize);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        models::CheckRecord record;
        record.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        record.appointment_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.check_in_time = sqlite3_column_text(stmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";
        record.check_out_time = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        record.security_id = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        record.gate_location = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        record.remarks = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        record.created_at = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        records.push_back(record);
    }
    
    sqlite3_finalize(stmt);
    return records;
}

// ====== 操作日志实现 ======

bool Database::createLog(const models::OperationLog& log) {
    const char* sql = R"(
        INSERT INTO operation_logs (id, user_id, action, target_type, target_id, details, ip_address, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    std::string now = getCurrentTime();
    
    sqlite3_bind_text(stmt, 1, log.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, log.user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, log.action.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, log.target_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, log.target_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, log.details.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, log.ip_address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, now.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<models::OperationLog> Database::getLogs(int page, int pageSize) {
    std::vector<models::OperationLog> logs;
    const char* sql = "SELECT * FROM operation_logs ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, pageSize);
    sqlite3_bind_int(stmt, 2, (page - 1) * pageSize);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        models::OperationLog log;
        log.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        log.user_id = sqlite3_column_text(stmt, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
        log.action = sqlite3_column_text(stmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";
        log.target_type = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        log.target_id = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        log.details = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        log.ip_address = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        log.created_at = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        logs.push_back(log);
    }
    
    sqlite3_finalize(stmt);
    return logs;
}

// ====== 统计查询实现 ======

json Database::getOverviewStats() {
    json stats;
    
    // 总用户数
    stats["total_users"] = getUserCount();
    
    // 各角色用户数
    stats["superadmin_count"] = getUserCount("superadmin");
    stats["admin_count"] = getUserCount("admin");
    stats["employee_count"] = getUserCount("employee");
    stats["security_count"] = getUserCount("security");
    stats["visitor_count"] = getUserCount("visitor");
    
    // 预约统计
    stats["total_appointments"] = getAppointmentCount();
    stats["pending_appointments"] = getAppointmentCount("pending");
    stats["approved_appointments"] = getAppointmentCount("approved");
    stats["checked_in_count"] = getAppointmentCount("checked_in");
    stats["checked_out_count"] = getAppointmentCount("checked_out");
    
    // 今日统计
    const char* today_sql = "SELECT COUNT(*) FROM appointments WHERE date(visit_date) = date('now')";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, today_sql, -1, &stmt, nullptr);
    sqlite3_step(stmt);
    stats["today_appointments"] = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    
    return stats;
}

json Database::getVisitorStats(const std::string& start_date, const std::string& end_date) {
    json stats;
    json daily_list = json::array();
    
    const char* sql = R"(
        SELECT date(visit_date) as day, COUNT(*) as count 
        FROM appointments 
        WHERE date(visit_date) BETWEEN ? AND ?
        GROUP BY date(visit_date)
        ORDER BY day
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json item;
        item["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item["count"] = sqlite3_column_int(stmt, 1);
        daily_list.push_back(item);
    }
    sqlite3_finalize(stmt);
    
    stats["daily"] = daily_list;
    return stats;
}

json Database::getAppointmentStats(const std::string& start_date, const std::string& end_date) {
    json stats;
    
    // 按状态统计
    const char* sql = R"(
        SELECT status, COUNT(*) as count 
        FROM appointments 
        WHERE date(visit_date) BETWEEN ? AND ?
        GROUP BY status
    )";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
    
    json by_status;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int count = sqlite3_column_int(stmt, 1);
        by_status[status] = count;
    }
    sqlite3_finalize(stmt);
    
    stats["by_status"] = by_status;
    return stats;
}

} // namespace db
