#pragma once

#include <string>

namespace utils {

class Password {
public:
    // 哈希密码 (使用简单的SHA256 + salt)
    static std::string hash(const std::string& password);
    
    // 验证密码
    static bool verify(const std::string& password, const std::string& hash);
    
private:
    // 生成随机盐值
    static std::string generateSalt(size_t length = 16);
    
    // SHA256哈希
    static std::string sha256(const std::string& input);
};

} // namespace utils
