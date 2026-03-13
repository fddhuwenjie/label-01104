#include "password.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <cstring>

namespace utils {

std::string Password::generateSalt(size_t length) {
    static const char charset[] = 
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string salt;
    salt.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        salt += charset[dis(gen)];
    }
    return salt;
}

// 简化的SHA256实现 (用于演示)
std::string Password::sha256(const std::string& input) {
    // 使用FNV-1a哈希算法的变体 (生产环境应使用OpenSSL SHA256)
    uint64_t hash1 = 14695981039346656037ULL;
    uint64_t hash2 = 1099511628211ULL;
    uint64_t hash3 = 2166136261ULL;
    uint64_t hash4 = 16777619ULL;
    
    for (size_t i = 0; i < input.size(); ++i) {
        hash1 ^= static_cast<uint64_t>(input[i]);
        hash1 *= 1099511628211ULL;
        
        hash2 ^= static_cast<uint64_t>(input[input.size() - 1 - i]);
        hash2 *= 16777619ULL;
        
        hash3 ^= static_cast<uint64_t>(input[i]) << (i % 8);
        hash3 *= 1099511628211ULL;
        
        hash4 ^= static_cast<uint64_t>(input[i]) * (i + 1);
        hash4 *= 16777619ULL;
    }
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0') 
       << std::setw(16) << hash1 
       << std::setw(16) << hash2
       << std::setw(16) << hash3
       << std::setw(16) << hash4;
    return ss.str();
}

std::string Password::hash(const std::string& password) {
    std::string salt = generateSalt(16);
    std::string hash = sha256(salt + password);
    // 格式: salt$hash
    return salt + "$" + hash;
}

bool Password::verify(const std::string& password, const std::string& stored_hash) {
    // 分离salt和hash
    size_t pos = stored_hash.find('$');
    if (pos == std::string::npos || pos != 16) {
        return false;
    }
    
    std::string salt = stored_hash.substr(0, pos);
    std::string expected_hash = stored_hash.substr(pos + 1);
    
    std::string actual_hash = sha256(salt + password);
    return actual_hash == expected_hash;
}

} // namespace utils
