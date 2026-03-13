cmake_minimum_required(VERSION 3.16)
project(visitor_system VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 编译选项
add_compile_options(-Wall -Wextra -O2)

# 查找依赖
find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/crow
    ${CMAKE_SOURCE_DIR}/third_party/sqlite
    ${CMAKE_SOURCE_DIR}/third_party/jwt-cpp
    ${CMAKE_SOURCE_DIR}/third_party/json
)

# 源文件
set(SOURCES
    src/main.cpp
    src/database/database.cpp
    src/utils/snowflake.cpp
    src/utils/jwt_util.cpp
    src/utils/password.cpp
    src/middleware/auth_middleware.cpp
    src/controllers/auth_controller.cpp
    src/controllers/user_controller.cpp
    src/controllers/appointment_controller.cpp
    src/controllers/check_controller.cpp
    src/controllers/stats_controller.cpp
    third_party/sqlite/sqlite3.c
)

# 可执行文件
add_executable(visitor_system ${SOURCES})

# 链接库
target_link_libraries(visitor_system
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# 安装
install(TARGETS visitor_system DESTINATION bin)
