#include "Config.h"
#include "KvJson.h"
#include "SysInfo.h"
#include <string>

Config* Config::m_pSingle = NULL;
Config::Config(){

}
Config::Config(string file){
	m_config_file= file;
}

void Config::read_config(std::string file ){
    Json::Reader reader;
    Json::Value json_root;
    std::ifstream json_file_stream(file, std::ifstream::binary);
    json_file_stream >> json_root;
    // std::cout<< json_root;
    // std::cout<< json_root["username"];
    // std::cout<< json_root["password"];
    m_pSingle->ip= json_root["ip"].asString();
    m_pSingle->client_addr= json_root["client_addr"].asString();
    m_pSingle->platform_address= json_root["platform_address"].asString();
    m_pSingle->shell_path= json_root["shell_path"].asString();
    m_pSingle->local_ip= json_root["local_ip"].asString();
    m_pSingle->host_port= json_root["host_port"].asString();
    m_pSingle->tun_id= json_root["tun_id"].asString();
    m_pSingle->if_signup_platform= json_root["if_signup_platform"].asString();
    m_pSingle->uuid= sys_msg_md5_to_platform();
    m_pSingle->db_path= json_root["db_path"].asString();
    m_pSingle->log_path= json_root["log_path"].asString();
    m_pSingle->python3_path= json_root["python3_path"].asString();
    std::cout<<m_pSingle->ip<< m_pSingle->client_addr<< m_pSingle->platform_address<<m_pSingle->shell_path \
    		<<m_pSingle->local_ip<<m_pSingle->host_port<<m_pSingle->tun_id<<m_pSingle->uuid<<m_pSingle->if_signup_platform \
			<<m_pSingle->db_path <<m_pSingle->log_path<<m_pSingle->python3_path<<std::endl;
};

void Config::write_config( ){
	const std::lock_guard<std::mutex> lock(m_mutex);

	Json::Value json_root;
	json_root["ip"]= Json::Value(m_pSingle->ip);
	json_root["client_addr"]= Json::Value(m_pSingle->client_addr);
	json_root["platform_address"]= Json::Value(m_pSingle->platform_address);
	json_root["shell_path"]= Json::Value(m_pSingle->shell_path);
	json_root["local_ip"]= Json::Value(m_pSingle->local_ip);
	json_root["host_port"]= Json::Value(m_pSingle->host_port);
	json_root["tun_id"]= Json::Value(m_pSingle->tun_id);
	json_root["uuid"]= Json::Value(m_pSingle->uuid);
	json_root["if_signup_platform"]= Json::Value(m_pSingle->if_signup_platform);
	json_root["db_path"]= Json::Value(m_pSingle->db_path);
	json_root["log_path"]= Json::Value(m_pSingle->log_path);
	json_root["python3_path"]= Json::Value(m_pSingle->python3_path);

	// 缩进
	Json::StyledWriter sw;
	sw.write(json_root);

	// 写入文件
	ofstream os;
//	os.open(m_config_file, std::ios::out| std::ios::app);
	os.open(m_config_file, std::ios::out);
	if(!os.is_open()){
		gLogger->error("write_config file not open");
		return ;
	}
	os<< sw.write(json_root);
	os.close();

};

Config* Config::instance(std::string file){
    if(m_pSingle == NULL){
        m_pSingle = new Config(file);
        gConfig->read_config(file);
    }
    return m_pSingle;
}
