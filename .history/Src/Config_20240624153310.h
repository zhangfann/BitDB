#ifndef TINC_CONFIG_H
#define TINC_CONFIG_H

#include <string>
#include <mutex>
#include <iostream>
#include "Logger.h"

using namespace std;

class Config{
public:
	std::mutex m_mutex;
	std::string m_config_file;

	std::string ip;
    std::string  client_addr;
    std::string platform_address;
    std::string shell_path;
    std::string local_ip;
    std::string host_port; // tinc_server port
    std::string tun_id;
    // std::string uuid;
    std::string if_signup_platform;
    std::string db_path;
    std::string log_path;
    std::string python3_path;

    static Config* instance(std::string file="");
    Config();
    Config(string file);
    void read_config(std::string file );
    void write_config();

    static Config* m_pSingle;
};
#define gConfig Config::instance()

#endif
