// Crow - A C++ micro web framework (Simplified)
#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>

namespace crow {

// HTTP方法结构
struct HTTPMethod {
    std::string name;
};

// URL参数解析
class query_string {
public:
    query_string() = default;
    query_string(const std::string& url) {
        size_t pos = url.find('?');
        if (pos != std::string::npos) {
            parse(url.substr(pos + 1));
        }
    }
    
    void parse(const std::string& query) {
        std::stringstream ss(query);
        std::string item;
        while (std::getline(ss, item, '&')) {
            size_t eq = item.find('=');
            if (eq != std::string::npos) {
                params_[item.substr(0, eq)] = item.substr(eq + 1);
            }
        }
    }
    
    const char* get(const std::string& key) const {
        auto it = params_.find(key);
        if (it != params_.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }
    
private:
    std::map<std::string, std::string> params_;
};

// HTTP请求
struct request {
    std::string method;
    std::string url;
    std::string body;
    std::map<std::string, std::string> headers;
    query_string url_params;
    
    std::string get_header_value(const std::string& key) const {
        auto it = headers.find(key);
        if (it != headers.end()) {
            return it->second;
        }
        return "";
    }
};

// HTTP响应
struct response {
    int code = 200;
    std::string body;
    std::map<std::string, std::string> headers;
    bool completed = false;
    
    response() = default;
    response(int c) : code(c) {}
    response(int c, const std::string& b) : code(c), body(b) {}
    response(const std::string& b) : body(b) {}
    
    void set_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }
    
    void end() {
        completed = true;
    }
    
    void write(const std::string& data) {
        body += data;
    }
};

// 路由处理器
class Router {
public:
    using Handler = std::function<response(const request&)>;
    using HandlerWithParam = std::function<response(const request&, const std::string&)>;
    
    void add_route(const std::string& method, const std::string& path, Handler handler) {
        routes_[method + ":" + path] = handler;
    }
    
    void add_route_with_param(const std::string& method, const std::string& path, HandlerWithParam handler) {
        param_routes_[method + ":" + path] = handler;
    }
    
    response handle(const request& req) {
        try {
            std::string key = req.method + ":" + req.url;
            auto it = routes_.find(key);
            if (it != routes_.end()) {
                return it->second(req);
            }
            
            for (const auto& route : param_routes_) {
                std::string pattern = route.first;
                size_t colonPos = pattern.find(':');
                if (colonPos == std::string::npos) continue;
                
                std::string routeMethod = pattern.substr(0, colonPos);
                std::string routePath = pattern.substr(colonPos + 1);
                
                if (req.method != routeMethod) continue;
                
                size_t paramPos = routePath.find("<string>");
                if (paramPos != std::string::npos) {
                    std::string prefix = routePath.substr(0, paramPos);
                    std::string suffix = routePath.substr(paramPos + 8);
                    
                    if (req.url.length() >= prefix.length() && 
                        req.url.substr(0, prefix.length()) == prefix) {
                        std::string remaining = req.url.substr(prefix.length());
                        std::string param;
                        
                        if (suffix.empty()) {
                            param = remaining;
                        } else {
                            size_t suffixPos = remaining.find(suffix);
                            if (suffixPos != std::string::npos) {
                                param = remaining.substr(0, suffixPos);
                            }
                        }
                        
                        if (!param.empty() && param.find('/') == std::string::npos) {
                            return route.second(req, param);
                        }
                    }
                }
            }
            
            return response(404, R"({"code":1004,"message":"Not Found","data":null})");
        } catch (const std::exception& e) {
            std::cerr << "Router error: " << e.what() << std::endl;
            return response(500, R"({"code":1000,"message":"Internal Server Error","data":null})");
        } catch (...) {
            std::cerr << "Router unknown error" << std::endl;
            return response(500, R"({"code":1000,"message":"Internal Server Error","data":null})");
        }
    }
    
private:
    std::map<std::string, Handler> routes_;
    std::map<std::string, HandlerWithParam> param_routes_;
};

// CORS处理器
class CORSHandler {
public:
    CORSHandler& global() { return *this; }
    
    template<typename... Args>
    CORSHandler& headers(Args... args) {
        std::vector<std::string> h = {args...};
        for (size_t i = 0; i < h.size(); ++i) {
            if (i > 0) allowed_headers_ += ", ";
            allowed_headers_ += h[i];
        }
        return *this;
    }
    
    template<typename... Args>
    CORSHandler& methods(Args... args) {
        std::vector<HTTPMethod> m = {args...};
        allowed_methods_ = "";
        for (size_t i = 0; i < m.size(); ++i) {
            if (i > 0) allowed_methods_ += ", ";
            allowed_methods_ += m[i].name;
        }
        return *this;
    }
    
    std::string get_allowed_headers() const { return allowed_headers_; }
    std::string get_allowed_methods() const { return allowed_methods_; }
    
private:
    std::string allowed_headers_;
    std::string allowed_methods_ = "GET, POST, PUT, DELETE, OPTIONS";
};

// 简单HTTP服务器
class SimpleApp {
public:
    SimpleApp() : cors_() {}
    
    template<typename F>
    void route(const std::string& path, const std::string& method, F handler) {
        router_.add_route(method, path, handler);
    }
    
    template<typename F>
    void route_with_param(const std::string& path, const std::string& method, F handler) {
        router_.add_route_with_param(method, path, handler);
    }
    
    CORSHandler& get_middleware() {
        return cors_;
    }
    
    template<typename T>
    T& get_middleware() {
        return cors_;
    }
    
    SimpleApp& bindaddr(const std::string& addr) {
        bind_addr_ = addr;
        return *this;
    }
    
    SimpleApp& port(int p) {
        port_ = p;
        return *this;
    }
    
    SimpleApp& multithreaded() {
        multithreaded_ = true;
        return *this;
    }
    
    void run() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }
        
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(bind_addr_.c_str());
        address.sin_port = htons(port_);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Failed to bind to port " << port_ << std::endl;
            close(server_fd);
            return;
        }
        
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Failed to listen" << std::endl;
            close(server_fd);
            return;
        }
        
        std::cout << "Server is running..." << std::endl;
        
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (client_fd < 0) continue;
            
            if (multithreaded_) {
                std::thread([this, client_fd]() {
                    handle_client(client_fd);
                }).detach();
            } else {
                handle_client(client_fd);
            }
        }
        
        close(server_fd);
    }
    
    Router router_;
    
private:
    void handle_client(int client_fd) {
        try {
            char buffer[65536] = {0};
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            
            if (bytes_read <= 0) {
                close(client_fd);
                return;
            }
            
            request req = parse_request(buffer);
            
            response res;
            if (req.method == "OPTIONS") {
                res.code = 204;
            } else {
                res = router_.handle(req);
            }
            
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", cors_.get_allowed_methods());
            res.set_header("Access-Control-Allow-Headers", cors_.get_allowed_headers());
            
            if (res.headers.find("Content-Type") == res.headers.end()) {
                res.set_header("Content-Type", "application/json");
            }
            
            std::stringstream ss;
            ss << "HTTP/1.1 " << res.code << " OK\r\n";
            for (const auto& h : res.headers) {
                ss << h.first << ": " << h.second << "\r\n";
            }
            ss << "Content-Length: " << res.body.length() << "\r\n";
            ss << "\r\n";
            ss << res.body;
            
            std::string response_str = ss.str();
            ::write(client_fd, response_str.c_str(), response_str.length());
        } catch (const std::exception& e) {
            std::cerr << "Client handler error: " << e.what() << std::endl;
            std::string error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nContent-Length: 51\r\n\r\n{\"code\":1000,\"message\":\"Internal Server Error\"}";
            ::write(client_fd, error_response.c_str(), error_response.length());
        } catch (...) {
            std::cerr << "Client handler unknown error" << std::endl;
            std::string error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nContent-Length: 51\r\n\r\n{\"code\":1000,\"message\":\"Internal Server Error\"}";
            ::write(client_fd, error_response.c_str(), error_response.length());
        }
        close(client_fd);
    }
    
    request parse_request(const char* buffer) {
        request req;
        std::istringstream stream(buffer);
        std::string line;
        
        if (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::istringstream line_stream(line);
            line_stream >> req.method >> req.url;
            
            size_t query_pos = req.url.find('?');
            if (query_pos != std::string::npos) {
                req.url_params = query_string(req.url);
                req.url = req.url.substr(0, query_pos);
            }
        }
        
        while (std::getline(stream, line) && line != "\r" && !line.empty()) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) break;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string value = (colon + 1 < line.length()) ? line.substr(colon + 1) : "";
                while (!value.empty() && value[0] == ' ') {
                    value = value.substr(1);
                }
                req.headers[key] = value;
            }
        }
        
        std::stringstream body_stream;
        body_stream << stream.rdbuf();
        req.body = body_stream.str();
        
        while (!req.body.empty() && (req.body[0] == '\r' || req.body[0] == '\n')) {
            req.body = req.body.substr(1);
        }
        
        return req;
    }
    
    std::string bind_addr_ = "0.0.0.0";
    int port_ = 8080;
    bool multithreaded_ = false;
    CORSHandler cors_;
};

// 模板特征类用于检测函数签名
template<typename T>
struct function_traits;

// Lambda/函数指针特征
template<typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {
    static constexpr size_t arity = sizeof...(Args);
};

template<typename Func>
struct handler_arity {
    static constexpr size_t value = function_traits<decltype(&Func::operator())>::arity;
};

// 路由辅助类
class CrowRouteHelper {
public:
    CrowRouteHelper(SimpleApp& app, const std::string& path) 
        : app_(app), path_(path) {}
    
    template<typename... Args>
    CrowRouteHelper& methods(Args... args) {
        std::vector<HTTPMethod> m = {args...};
        for (const auto& method : m) {
            methods_.push_back(method.name);
        }
        return *this;
    }
    
    // 处理只有一个参数的处理器 (const request&) - 无路径参数
    template<typename Func>
    typename std::enable_if<handler_arity<Func>::value == 1>::type
    operator()(Func handler) {
        if (methods_.empty()) {
            methods_.push_back("GET");
        }
        
        for (const auto& method : methods_) {
            app_.route(path_, method, handler);
        }
    }
    
    // 处理两个参数的处理器 (const request&, const std::string&) - 有路径参数
    template<typename Func>
    typename std::enable_if<handler_arity<Func>::value == 2>::type
    operator()(Func handler) {
        if (methods_.empty()) {
            methods_.push_back("GET");
        }
        
        for (const auto& method : methods_) {
            app_.route_with_param(path_, method, handler);
        }
    }
    
private:
    SimpleApp& app_;
    std::string path_;
    std::vector<std::string> methods_;
};

} // namespace crow

// HTTP方法字面量 - 必须在全局命名空间
inline crow::HTTPMethod operator""_method(const char* str, std::size_t len) {
    return crow::HTTPMethod{std::string(str, len)};
}

// 路由宏 - 使用完全限定名称
#define CROW_ROUTE(app, path) crow::CrowRouteHelper(app, path)
