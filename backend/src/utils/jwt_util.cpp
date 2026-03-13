#include "jwt_util.h"
#include "config.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

namespace utils {

// 简单的Base64编码表
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char base64url_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string JwtUtil::base64UrlEncode(const std::string& input) {
    std::string result;
    int val = 0;
    int valb = -6;
    
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64url_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        result.push_back(base64url_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    return result;
}

std::string JwtUtil::base64UrlDecode(const std::string& input) {
    std::string result;
    std::vector<int> T(256, -1);
    
    for (int i = 0; i < 64; i++) {
        T[base64url_chars[i]] = i;
    }
    
    int val = 0;
    int valb = -8;
    
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return result;
}

// 简化的HMAC-SHA256实现 (用于演示，生产环境应使用OpenSSL)
std::string JwtUtil::hmacSha256(const std::string& key, const std::string& data) {
    // 简化实现：使用key和data的组合进行简单哈希
    // 生产环境应使用 OpenSSL HMAC
    std::string combined = key + data;
    
    // 简单哈希函数
    uint64_t hash1 = 14695981039346656037ULL;
    uint64_t hash2 = 1099511628211ULL;
    
    for (char c : combined) {
        hash1 ^= static_cast<uint64_t>(c);
        hash1 *= hash2;
    }
    
    // 第二轮哈希
    for (char c : combined) {
        hash2 ^= static_cast<uint64_t>(c);
        hash2 *= 16777619;
    }
    
    // 转换为字符串
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash1 << std::setw(16) << hash2;
    return ss.str();
}

std::string JwtUtil::generateToken(const std::string& user_id, 
                                   const std::string& username,
                                   const std::string& role) {
    // Header
    json header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    std::string header_encoded = base64UrlEncode(header.dump());
    
    // Payload
    json payload;
    payload["user_id"] = user_id;
    payload["username"] = username;
    payload["role"] = role;
    payload["iat"] = std::time(nullptr);
    payload["exp"] = std::time(nullptr) + (config::JWT_EXPIRE_HOURS * 3600);
    std::string payload_encoded = base64UrlEncode(payload.dump());
    
    // Signature
    std::string signature_input = header_encoded + "." + payload_encoded;
    std::string signature = base64UrlEncode(hmacSha256(config::JWT_SECRET, signature_input));
    
    return header_encoded + "." + payload_encoded + "." + signature;
}

std::optional<TokenPayload> JwtUtil::verifyToken(const std::string& token) {
    // 分割token
    std::vector<std::string> parts;
    std::stringstream ss(token);
    std::string part;
    while (std::getline(ss, part, '.')) {
        parts.push_back(part);
    }
    
    if (parts.size() != 3) {
        return std::nullopt;
    }
    
    // 验证签名
    std::string signature_input = parts[0] + "." + parts[1];
    std::string expected_signature = base64UrlEncode(hmacSha256(config::JWT_SECRET, signature_input));
    
    if (parts[2] != expected_signature) {
        return std::nullopt;
    }
    
    // 解析payload
    try {
        std::string payload_json = base64UrlDecode(parts[1]);
        json payload = json::parse(payload_json);
        
        TokenPayload result;
        result.user_id = payload["user_id"].get<std::string>();
        result.username = payload["username"].get<std::string>();
        result.role = payload["role"].get<std::string>();
        result.exp = payload["exp"].get<int64_t>();
        
        // 检查过期
        if (result.exp < std::time(nullptr)) {
            return std::nullopt;
        }
        
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

std::string JwtUtil::extractToken(const std::string& auth_header) {
    if (auth_header.length() > 7 && auth_header.substr(0, 7) == "Bearer ") {
        return auth_header.substr(7);
    }
    return auth_header;
}

} // namespace utils
