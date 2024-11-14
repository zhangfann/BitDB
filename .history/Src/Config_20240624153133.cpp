#include "Config.h"
// #include "KvJson.h"
// #include "SysInfo.h"
#include <string>

// json
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

Config* Config::m_pSingle = NULL;
Config::Config(){

}
Config::Config(string file){
	m_config_file= file;
}

void Config::read_config(std::string file ){
	std::ifstream f(file);
	json json_root= json::parse(f);

    // Json::Reader reader;
    // Json::Value json_root;
    // std::ifstream json_file_stream(file, std::ifstream::binary);
    // json_file_stream >> json_root;
    // std::cout<< json_root;
    // std::cout<< json_root["username"];
    // std::cout<< json_root["password"];
    m_pSingle->ip= json_root["ip"];
    m_pSingle->client_addr= json_root["client_addr"];
    m_pSingle->platform_address= json_root["platform_address"];
    m_pSingle->shell_path= json_root["shell_path"];
    m_pSingle->local_ip= json_root["local_ip"];
    m_pSingle->host_port= json_root["host_port"];
    m_pSingle->tun_id= json_root["tun_id"];
    m_pSingle->if_signup_platform= json_root["if_signup_platform"];
    // m_pSingle->uuid= sys_msg_md5_to_platform();
    m_pSingle->db_path= json_root["db_path"];
    m_pSingle->log_path= json_root["log_path"];
    m_pSingle->python3_path= json_root["python3_path"];
    std::cout<<m_pSingle->ip<< m_pSingle->client_addr<< m_pSingle->platform_address<<m_pSingle->shell_path \
    		<<m_pSingle->local_ip<<m_pSingle->host_port<<m_pSingle->tun_id<<m_pSingle->uuid<<m_pSingle->if_signup_platform \
			<<m_pSingle->db_path <<m_pSingle->log_path<<m_pSingle->python3_path<<std::endl;
};

void Config::write_config( ){
	const std::lock_guard<std::mutex> lock(m_mutex);

	json json_root;
	json_root["ip"]= m_pSingle->ip;
	json_root["client_addr"]= m_pSingle->client_addr;
	json_root["platform_address"]= m_pSingle->platform_address;
	json_root["shell_path"]= m_pSingle->shell_path;
	json_root["local_ip"]= m_pSingle->local_ip;
	json_root["host_port"]= m_pSingle->host_port;
	json_root["tun_id"]= m_pSingle->tun_id;
	json_root["uuid"]= m_pSingle->uuid;
	json_root["if_signup_platform"]= m_pSingle->if_signup_platform;
	json_root["db_path"]= m_pSingle->db_path;
	json_root["log_path"]= m_pSingle->log_path;
	json_root["python3_path"]= m_pSingle->python3_path;

	// 写入文件
	std::ofstream o(m_config_file);
	o<< std::setw(4) << json_root << std::endl;
	o.close();

	// 缩进
// 	Json::StyledWriter sw;
// 	sw.write(json_root);

// 	// 写入文件
// 	ofstream os;
// //	os.open(m_config_file, std::ios::out| std::ios::app);
// 	os.open(m_config_file, std::ios::out);
// 	if(!os.is_open()){
// 		gLogger->error("write_config file not open");
// 		return ;
// 	}
// 	os<< sw.write(json_root);
// 	os.close();

};

Config* Config::instance(std::string file){
    if(m_pSingle == NULL){
        m_pSingle = new Config(file);
        gConfig->read_config(file);
    }
    return m_pSingle;
}
