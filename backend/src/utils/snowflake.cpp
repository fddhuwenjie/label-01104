#include "snowflake.h"
#include "config.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace utils {

Snowflake::Snowflake(int64_t worker_id, int64_t datacenter_id) 
    : worker_id_(worker_id), datacenter_id_(datacenter_id) {
    if (worker_id_ > MAX_WORKER_ID || worker_id_ < 0) {
        throw std::invalid_argument("Worker ID out of range");
    }
    if (datacenter_id_ > MAX_DATACENTER_ID || datacenter_id_ < 0) {
        throw std::invalid_argument("Datacenter ID out of range");
    }
}

int64_t Snowflake::currentTimeMillis() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int64_t Snowflake::waitNextMillis(int64_t last_timestamp) {
    int64_t timestamp = currentTimeMillis();
    while (timestamp <= last_timestamp) {
        timestamp = currentTimeMillis();
    }
    return timestamp;
}

int64_t Snowflake::nextId() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int64_t timestamp = currentTimeMillis();
    
    // 时钟回退检查
    if (timestamp < last_timestamp_) {
        throw std::runtime_error("Clock moved backwards");
    }
    
    // 同一毫秒内
    if (timestamp == last_timestamp_) {
        sequence_ = (sequence_ + 1) & MAX_SEQUENCE;
        if (sequence_ == 0) {
            timestamp = waitNextMillis(last_timestamp_);
        }
    } else {
        sequence_ = 0;
    }
    
    last_timestamp_ = timestamp;
    
    // 组装ID
    int64_t id = ((timestamp - EPOCH) << TIMESTAMP_SHIFT)
               | (datacenter_id_ << DATACENTER_ID_SHIFT)
               | (worker_id_ << WORKER_ID_SHIFT)
               | sequence_;
    
    return id;
}

std::string Snowflake::nextIdWithPrefix(const std::string& prefix) {
    int64_t id = nextId();
    std::stringstream ss;
    ss << prefix << "-" << std::setfill('0') << std::setw(18) << id;
    return ss.str();
}

int64_t Snowflake::getTimestamp(int64_t id) {
    return (id >> TIMESTAMP_SHIFT) + EPOCH;
}

// 全局ID生成器
Snowflake& getIdGenerator() {
    static Snowflake generator(config::SNOWFLAKE_WORKER_ID, config::SNOWFLAKE_DATACENTER_ID);
    return generator;
}

} // namespace utils
