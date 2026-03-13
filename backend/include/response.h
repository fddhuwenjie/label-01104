#pragma once

#include "json.hpp"
#include <string>

namespace utils {

using json = nlohmann::json;

// 统一响应格式
inline json successResponse(const json& data = nullptr, const std::string& message = "success") {
    json response;
    response["code"] = 0;
    response["message"] = message;
    response["data"] = data;
    return response;
}

inline json errorResponse(int code, const std::string& message) {
    json response;
    response["code"] = code;
    response["message"] = message;
    response["data"] = nullptr;
    return response;
}

// 分页响应
inline json paginatedResponse(const json& list, int total, int page, int pageSize) {
    json data;
    data["list"] = list;
    data["total"] = total;
    data["page"] = page;
    data["pageSize"] = pageSize;
    data["totalPages"] = (total + pageSize - 1) / pageSize;
    return successResponse(data);
}

// 错误码定义
namespace ErrorCode {
    constexpr int SUCCESS = 0;
    constexpr int INVALID_PARAMS = 1001;
    constexpr int UNAUTHORIZED = 1002;
    constexpr int FORBIDDEN = 1003;
    constexpr int NOT_FOUND = 1004;
    constexpr int ALREADY_EXISTS = 1005;
    constexpr int INTERNAL_ERROR = 5000;
}

} // namespace utils
