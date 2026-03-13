#pragma once

#include <string>
#include <optional>
#include "json.hpp"

namespace utils {

using json = nlohmann::json;

struct TokenPayload {
    std::string user_id;
    std::string username;
    std::string role;
    int64_t exp;
};

class JwtUtil {
public:
    // 生成JWT Token
    static std::string generateToken(const std::string& user_id, 
                                     const std::string& username,
                                     const std::string& role);
    
    // 验证JWT Token
    static std::optional<TokenPayload> verifyToken(const std::string& token);
    
    // 从Authorization头提取Token
    static std::string extractToken(const std::string& auth_header);
    
private:
    // Base64 URL编码
    static std::string base64UrlEncode(const std::string& input);
    
    // Base64 URL解码
    static std::string base64UrlDecode(const std::string& input);
    
    // HMAC-SHA256签名
    static std::string hmacSha256(const std::string& key, const std::string& data);
};

} // namespace utils
