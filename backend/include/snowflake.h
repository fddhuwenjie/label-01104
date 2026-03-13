#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <chrono>

namespace utils {

/**
 * 雪花算法ID生成器
 * 
 * ID结构 (64位):
 * - 1位符号位 (始终为0)
 * - 41位时间戳 (毫秒级，可用约69年)
 * - 5位数据中心ID (0-31)
 * - 5位机器ID (0-31)
 * - 12位序列号 (0-4095)
 * 
 * 生成的ID格式: {前缀}-{时间戳部分}-{序列部分}
 */
class Snowflake {
public:
    Snowflake(int64_t worker_id, int64_t datacenter_id);
    
    // 生成纯数字ID
    int64_t nextId();
    
    // 生成带前缀的字符串ID
    std::string nextIdWithPrefix(const std::string& prefix);
    
    // 解析ID获取时间戳
    static int64_t getTimestamp(int64_t id);

private:
    // 起始时间戳 (2024-01-01 00:00:00 UTC)
    static constexpr int64_t EPOCH = 1704067200000LL;
    
    // 各部分位数
    static constexpr int64_t WORKER_ID_BITS = 5;
    static constexpr int64_t DATACENTER_ID_BITS = 5;
    static constexpr int64_t SEQUENCE_BITS = 12;
    
    // 最大值
    static constexpr int64_t MAX_WORKER_ID = (1LL << WORKER_ID_BITS) - 1;
    static constexpr int64_t MAX_DATACENTER_ID = (1LL << DATACENTER_ID_BITS) - 1;
    static constexpr int64_t MAX_SEQUENCE = (1LL << SEQUENCE_BITS) - 1;
    
    // 位移量
    static constexpr int64_t WORKER_ID_SHIFT = SEQUENCE_BITS;
    static constexpr int64_t DATACENTER_ID_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS;
    static constexpr int64_t TIMESTAMP_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS + DATACENTER_ID_BITS;
    
    int64_t worker_id_;
    int64_t datacenter_id_;
    int64_t sequence_ = 0;
    int64_t last_timestamp_ = -1;
    
    std::mutex mutex_;
    
    int64_t currentTimeMillis();
    int64_t waitNextMillis(int64_t last_timestamp);
};

// 全局ID生成器实例
Snowflake& getIdGenerator();

} // namespace utils
