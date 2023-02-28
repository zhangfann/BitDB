cmake_minimum_required (VERSION 2.8) # cmake版本
project (fandb) # 项目名称

add_definitions(-g -std=c++11 `mysql_config --cflags --libs`) 

include_directories(. /usr/include/jsoncpp /opt/homebrew/Cellar/jsoncpp/1.9.4_1/include ./Include /opt/homebrew/Cellar/openssl@1.1/1.1.1o/include /opt/homebrew/opt/openssl/include)

add_definitions(-lpthread -ljsoncpp -lssl -lcrypto -lsqlite3)

link_directories(/opt/homebrew/Cellar/jsoncpp/1.9.4_1/lib/ /opt/homebrew/Cellar/openssl@1.1/1.1.1o/lib /opt/homebrew/opt/openssl/lib )

aux_source_directory(. DIR_SRCS)

add_executable(main ${DIR_SRCS})
