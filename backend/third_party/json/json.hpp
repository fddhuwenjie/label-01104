// nlohmann/json - JSON for Modern C++ (Simplified)
#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <initializer_list>
#include <iomanip>
#include <utility>

namespace nlohmann {

class json {
public:
    using object_t = std::map<std::string, json>;
    using array_t = std::vector<json>;
    using string_t = std::string;
    using number_int_t = int64_t;
    using number_float_t = double;
    using boolean_t = bool;
    
    enum class value_t {
        null, object, array, string, number_integer, number_float, boolean
    };
    
    json() : type_(value_t::null), object_value_(), array_value_(), string_value_(), int_value_(0), float_value_(0.0), bool_value_(false) {}
    json(std::nullptr_t) : type_(value_t::null), object_value_(), array_value_(), string_value_(), int_value_(0), float_value_(0.0), bool_value_(false) {}
    json(bool v) : type_(value_t::boolean), object_value_(), array_value_(), string_value_(), int_value_(0), float_value_(0.0), bool_value_(v) {}
    json(int v) : type_(value_t::number_integer), object_value_(), array_value_(), string_value_(), int_value_(v), float_value_(0.0), bool_value_(false) {}
    json(int64_t v) : type_(value_t::number_integer), object_value_(), array_value_(), string_value_(), int_value_(v), float_value_(0.0), bool_value_(false) {}
    json(double v) : type_(value_t::number_float), object_value_(), array_value_(), string_value_(), int_value_(0), float_value_(v), bool_value_(false) {}
    json(const char* v) : type_(value_t::string), object_value_(), array_value_(), string_value_(v ? v : ""), int_value_(0), float_value_(0.0), bool_value_(false) {}
    json(const std::string& v) : type_(value_t::string), object_value_(), array_value_(), string_value_(v), int_value_(0), float_value_(0.0), bool_value_(false) {}
    json(std::initializer_list<std::pair<const std::string, json>> init) : type_(value_t::object), object_value_(), array_value_(), string_value_(), int_value_(0), float_value_(0.0), bool_value_(false) {
        for (const auto& item : init) {
            object_value_[item.first] = item.second;
        }
    }
    
    // 拷贝构造函数
    json(const json& other) : type_(other.type_), object_value_(other.object_value_), array_value_(other.array_value_), 
                              string_value_(other.string_value_), int_value_(other.int_value_), float_value_(other.float_value_), bool_value_(other.bool_value_) {}
    
    // 移动构造函数
    json(json&& other) noexcept : type_(other.type_), object_value_(std::move(other.object_value_)), array_value_(std::move(other.array_value_)),
                                   string_value_(std::move(other.string_value_)), int_value_(other.int_value_), float_value_(other.float_value_), bool_value_(other.bool_value_) {
        other.type_ = value_t::null;
    }
    
    // 拷贝赋值运算符
    json& operator=(const json& other) {
        if (this != &other) {
            type_ = other.type_;
            object_value_ = other.object_value_;
            array_value_ = other.array_value_;
            string_value_ = other.string_value_;
            int_value_ = other.int_value_;
            float_value_ = other.float_value_;
            bool_value_ = other.bool_value_;
        }
        return *this;
    }
    
    // 移动赋值运算符
    json& operator=(json&& other) noexcept {
        if (this != &other) {
            type_ = other.type_;
            object_value_ = std::move(other.object_value_);
            array_value_ = std::move(other.array_value_);
            string_value_ = std::move(other.string_value_);
            int_value_ = other.int_value_;
            float_value_ = other.float_value_;
            bool_value_ = other.bool_value_;
            other.type_ = value_t::null;
        }
        return *this;
    }
    
    static json array() {
        json j;
        j.type_ = value_t::array;
        return j;
    }
    
    static json object() {
        json j;
        j.type_ = value_t::object;
        return j;
    }
    
    static json parse(const std::string& str) {
        size_t pos = 0;
        return parse_value(str, pos);
    }
    
    std::string dump(int indent = -1) const {
        return to_string(indent, 0);
    }
    
    json& operator[](const std::string& key) {
        if (type_ == value_t::null) {
            type_ = value_t::object;
        }
        return object_value_[key];
    }
    
    const json& operator[](const std::string& key) const {
        static json null_json;
        if (type_ != value_t::object) return null_json;
        auto it = object_value_.find(key);
        return (it != object_value_.end()) ? it->second : null_json;
    }
    
    json& operator[](size_t index) {
        return array_value_[index];
    }
    
    void push_back(const json& val) {
        if (type_ == value_t::null) type_ = value_t::array;
        array_value_.push_back(val);
    }
    
    bool contains(const std::string& key) const {
        if (type_ != value_t::object) return false;
        return object_value_.find(key) != object_value_.end();
    }
    
    bool is_null() const { return type_ == value_t::null; }
    bool is_object() const { return type_ == value_t::object; }
    bool is_array() const { return type_ == value_t::array; }
    bool is_string() const { return type_ == value_t::string; }
    bool is_number() const { return type_ == value_t::number_integer || type_ == value_t::number_float; }
    bool is_boolean() const { return type_ == value_t::boolean; }
    
    bool empty() const {
        if (type_ == value_t::null) return true;
        if (type_ == value_t::array) return array_value_.empty();
        if (type_ == value_t::object) return object_value_.empty();
        if (type_ == value_t::string) return string_value_.empty();
        return false;
    }
    
    size_t size() const {
        if (type_ == value_t::array) return array_value_.size();
        if (type_ == value_t::object) return object_value_.size();
        return 0;
    }
    
    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, std::string>) {
            if (type_ == value_t::string) return string_value_;
            if (type_ == value_t::null) return "";
            throw std::runtime_error("Not a string");
        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int64_t>) {
            if (type_ == value_t::number_integer) return static_cast<T>(int_value_);
            if (type_ == value_t::number_float) return static_cast<T>(float_value_);
            throw std::runtime_error("Not a number");
        } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
            if (type_ == value_t::number_float) return static_cast<T>(float_value_);
            if (type_ == value_t::number_integer) return static_cast<T>(int_value_);
            throw std::runtime_error("Not a number");
        } else if constexpr (std::is_same_v<T, bool>) {
            if (type_ == value_t::boolean) return bool_value_;
            throw std::runtime_error("Not a boolean");
        }
        throw std::runtime_error("Unsupported type");
    }
    
    // 字符串特化的 value 方法
    std::string value(const std::string& key, const std::string& default_value) const {
        if (type_ != value_t::object) return default_value;
        auto it = object_value_.find(key);
        if (it == object_value_.end()) return default_value;
        if (it->second.type_ == value_t::string) return it->second.string_value_;
        return default_value;
    }
    
    // int 特化的 value 方法
    int value(const std::string& key, int default_value) const {
        if (type_ != value_t::object) return default_value;
        auto it = object_value_.find(key);
        if (it == object_value_.end()) return default_value;
        if (it->second.type_ == value_t::number_integer) return static_cast<int>(it->second.int_value_);
        return default_value;
    }
    
    // 获取字符串值
    std::string get_string() const {
        if (type_ == value_t::string) return string_value_;
        return "";
    }
    
    // 获取整数值
    int get_int() const {
        if (type_ == value_t::number_integer) return static_cast<int>(int_value_);
        return 0;
    }
    
    array_t::iterator begin() { return array_value_.begin(); }
    array_t::iterator end() { return array_value_.end(); }
    array_t::const_iterator begin() const { return array_value_.begin(); }
    array_t::const_iterator end() const { return array_value_.end(); }
    
private:
    value_t type_;
    object_t object_value_;
    array_t array_value_;
    string_t string_value_;
    number_int_t int_value_ = 0;
    number_float_t float_value_ = 0.0;
    boolean_t bool_value_ = false;
    
    std::string to_string(int indent, int current_indent) const {
        std::stringstream ss;
        // 确保 current_indent 非负
        int safe_indent = (current_indent > 0) ? current_indent : 0;
        int safe_next_indent = (indent > 0) ? (safe_indent + indent) : 0;
        std::string indent_str(static_cast<size_t>(safe_indent), ' ');
        std::string next_indent_str(static_cast<size_t>(safe_next_indent), ' ');
        
        switch (type_) {
            case value_t::null: ss << "null"; break;
            case value_t::boolean: ss << (bool_value_ ? "true" : "false"); break;
            case value_t::number_integer: ss << int_value_; break;
            case value_t::number_float: ss << float_value_; break;
            case value_t::string: ss << "\"" << escape_string(string_value_) << "\""; break;
            case value_t::array:
                ss << "[";
                if (indent > 0 && !array_value_.empty()) ss << "\n";
                for (size_t i = 0; i < array_value_.size(); ++i) {
                    if (indent > 0) ss << next_indent_str;
                    ss << array_value_[i].to_string(indent, safe_next_indent);
                    if (i < array_value_.size() - 1) ss << ",";
                    if (indent > 0) ss << "\n";
                }
                if (indent > 0 && !array_value_.empty()) ss << indent_str;
                ss << "]";
                break;
            case value_t::object:
                ss << "{";
                if (indent > 0 && !object_value_.empty()) ss << "\n";
                {
                    size_t i = 0;
                    for (const auto& kv : object_value_) {
                        if (indent > 0) ss << next_indent_str;
                        ss << "\"" << escape_string(kv.first) << "\":";
                        if (indent > 0) ss << " ";
                        ss << kv.second.to_string(indent, safe_next_indent);
                        if (i < object_value_.size() - 1) ss << ",";
                        if (indent > 0) ss << "\n";
                        ++i;
                    }
                }
                if (indent > 0 && !object_value_.empty()) ss << indent_str;
                ss << "}";
                break;
        }
        return ss.str();
    }
    
    static std::string escape_string(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }
    
    static void skip_whitespace(const std::string& str, size_t& pos) {
        while (pos < str.length() && (str[pos] == ' ' || str[pos] == '\n' || str[pos] == '\r' || str[pos] == '\t')) {
            ++pos;
        }
    }
    
    static json parse_value(const std::string& str, size_t& pos) {
        skip_whitespace(str, pos);
        if (pos >= str.length()) return json();
        
        char c = str[pos];
        if (c == '{') return parse_object(str, pos);
        if (c == '[') return parse_array(str, pos);
        if (c == '"') return parse_string(str, pos);
        if (c == 't' || c == 'f') return parse_boolean(str, pos);
        if (c == 'n') return parse_null(str, pos);
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number(str, pos);
        return json();
    }
    
    static json parse_object(const std::string& str, size_t& pos) {
        json obj; obj.type_ = value_t::object;
        ++pos;
        skip_whitespace(str, pos);
        if (pos < str.length() && str[pos] == '}') { ++pos; return obj; }
        
        while (pos < str.length()) {
            skip_whitespace(str, pos);
            if (str[pos] != '"') break;
            std::string key = parse_string(str, pos).get<std::string>();
            skip_whitespace(str, pos);
            if (str[pos] != ':') break;
            ++pos;
            obj.object_value_[key] = parse_value(str, pos);
            skip_whitespace(str, pos);
            if (str[pos] == '}') { ++pos; break; }
            if (str[pos] == ',') ++pos;
        }
        return obj;
    }
    
    static json parse_array(const std::string& str, size_t& pos) {
        json arr; arr.type_ = value_t::array;
        ++pos;
        skip_whitespace(str, pos);
        if (pos < str.length() && str[pos] == ']') { ++pos; return arr; }
        
        while (pos < str.length()) {
            arr.array_value_.push_back(parse_value(str, pos));
            skip_whitespace(str, pos);
            if (str[pos] == ']') { ++pos; break; }
            if (str[pos] == ',') ++pos;
        }
        return arr;
    }
    
    static json parse_string(const std::string& str, size_t& pos) {
        ++pos;
        std::string result;
        while (pos < str.length() && str[pos] != '"') {
            if (str[pos] == '\\' && pos + 1 < str.length()) {
                ++pos;
                switch (str[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += str[pos];
                }
            } else {
                result += str[pos];
            }
            ++pos;
        }
        if (pos < str.length()) ++pos;
        json j; j.type_ = value_t::string; j.string_value_ = result;
        return j;
    }
    
    static json parse_number(const std::string& str, size_t& pos) {
        size_t start = pos;
        bool is_float = false;
        if (str[pos] == '-') ++pos;
        while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9') ++pos;
        if (pos < str.length() && str[pos] == '.') {
            is_float = true; ++pos;
            while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9') ++pos;
        }
        if (pos < str.length() && (str[pos] == 'e' || str[pos] == 'E')) {
            is_float = true; ++pos;
            if (pos < str.length() && (str[pos] == '+' || str[pos] == '-')) ++pos;
            while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9') ++pos;
        }
        std::string num_str = str.substr(start, pos - start);
        json j;
        if (is_float) { j.type_ = value_t::number_float; j.float_value_ = std::stod(num_str); }
        else { j.type_ = value_t::number_integer; j.int_value_ = std::stoll(num_str); }
        return j;
    }
    
    static json parse_boolean(const std::string& str, size_t& pos) {
        json j; j.type_ = value_t::boolean;
        if (str.substr(pos, 4) == "true") { j.bool_value_ = true; pos += 4; }
        else if (str.substr(pos, 5) == "false") { j.bool_value_ = false; pos += 5; }
        return j;
    }
    
    static json parse_null(const std::string& str, size_t& pos) {
        if (str.substr(pos, 4) == "null") pos += 4;
        return json();
    }
};

} // namespace nlohmann
