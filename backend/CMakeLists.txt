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
find_package(CURL REQUIRED)

add_executable(devpilot-backend
    src/main.cpp
    src/util/file_validate.cpp
    src/util/password_hash.cpp
    src/auth/user_repository.cpp
    src/auth/mysql_user_repository.cpp
    src/auth/jwt.cpp
    src/auth/auth_handlers.cpp
    src/auth/auth_guard.cpp
    src/gateway/gateway.cpp
    src/gateway/gateway_handlers.cpp
)

target_link_libraries(
    devpilot-backend
    PRIVATE Drogon::Drogon argon2 jwt-cpp ${MYSQL_LIBRARIES} CURL::libcurl
)
target_include_directories(
    devpilot-backend
    PRIVATE src
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    jwt-cpp
    URL https://github.com/Thalhammer/jwt-cpp/archive/refs/tags/v0.7.0.zip
)
FetchContent_MakeAvailable(jwt-cpp)

enable_testing()

add_executable(test_util
    tests/test_file_validate.cpp
    tests/test_password_hash.cpp
    tests/test_user_repository.cpp
    tests/test_jwt.cpp
    tests/test_gateway.cpp
    tests/test_auth_guard.cpp
    src/util/file_validate.cpp
    src/util/password_hash.cpp
    src/auth/user_repository.cpp
    src/auth/jwt.cpp
    src/auth/auth_guard.cpp
    src/gateway/gateway.cpp
)
target_link_libraries(
    test_util
    PRIVATE gtest_main argon2 jwt-cpp Drogon::Drogon
)
target_include_directories(
    test_util
    PRIVATE src
)

include(GoogleTest)
gtest_discover_tests(test_util)