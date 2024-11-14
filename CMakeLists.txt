cmake_minimum_required (VERSION 3.10) # cmake版本
project (fandb) # 项目名称

# source dir是定义project的地方
MESSAGE(STATUS "This is PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})
# binary dir是build目录
MESSAGE(STATUS "This is PROJECT_BINARY_DIR " ${PROJECT_BINARY_DIR})

# cmake配置项
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
OPTION(WITH_UNIT_TESTS "Build unit tests" ON)

# 指定 C 编译器为 gcc
#set(CMAKE_C_COMPILER gcc-13)
set(CMAKE_C_COMPILER /usr/local/bin/gcc)


# 指定 C++ 编译器为 g++
#set(CMAKE_CXX_COMPILER g++-13)
set(CMAKE_CXX_COMPILER /usr/local/bin/g++)

# g++配置项, 头文件, 库文件
# add_definitions(-g -std=c++11 `mysql_config --cflags --libs`) 
add_definitions(-g -std=c++23) 
include_directories(. 
    /usr/include/jsoncpp 
    ./Include 
    /opt/homebrew/opt/openssl/include
    /usr/local/include
    /opt/homebrew/Cellar/tbb/2021.13.0/include
    /opt/homebrew/Cellar/google-benchmark/1.9.0/include)
# include_directories(. 
#     /usr/include/jsoncpp 
#     /opt/homebrew/Cellar/jsoncpp/1.9.4_1/include 
#     ./Include 
#     /opt/homebrew/Cellar/openssl@1.1/1.1.1o/include 
#     /opt/homebrew/opt/openssl/include
#     /usr/local/include())
link_directories(
    /opt/homebrew/opt/openssl/lib 
    /usr/local/lib
    /opt/homebrew/Cellar/tbb/2021.13.0/lib
    /opt/homebrew/Cellar/google-benchmark/1.9.0/lib
    )
# add_definitions(-lpthread -ljsoncpp -lssl -lcrypto -lsqlite3)


# aux_source_directory(. DIR_SRCS)

# 依赖
find_package(benchmark CONFIG REQUIRED)
find_package(tabulate CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Crc32c CONFIG REQUIRED)
#find_package(httplib CONFIG REQUIRED)
#find_package(PkgConfig)
#pkg_check_modules(libunwind REQUIRED IMPORTED_TARGET GLOBAL libunwind)

# 编译源码
add_subdirectory(Src)

# 单元测试
IF(WITH_UNIT_TESTS)
    enable_testing()
    add_subdirectory(Test)
ENDIF(WITH_UNIT_TESTS)

# 性能测试
add_subdirectory(benchmark)
