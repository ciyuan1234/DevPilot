cmake_minimum_required(VERSION 3.20)

project(devpilot-backend
    VERSION 0.1.0
    DESCRIPTION "DevPilot backend"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MYSQL_INCLUDE_DIRS "/usr/include/mysql")
set(MYSQL_LIBRARIES "/usr/lib/x86_64-linux-gnu/libmysqlclient.so")
find_package(Drogon CONFIG REQUIRED)

add_executable(devpilot-backend
    src/main.cpp
)

target_link_libraries(
    devpilot-backend
    PRIVATE Drogon::Drogon
)