#ifndef ZEROTRUST_H
#define ZEROTRUST_H
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <string>
#include <vector>

using namespace std;

#define MAX_STRING_LEN 1024
class ClientInfo{
public:
  string uuid;
  string auth_str;
};
bool getShellResult(char *cmd, char *result);


// 回调
// 网关组网
void configure_tinc_callback(const httplib::Request& req, httplib::Response& resp);
void configure_tinc_json_callback(const httplib::Request& req, httplib::Response& resp, const httplib::ContentReader &content_reader);
// 添加从网关公钥
void auth_client_gate_callback(const httplib::Request& req, httplib::Response& resp);
// 客户端添加hosts
void client_configure_tinc_callback(const httplib::Request& req, httplib::Response& resp);
void client_configure_tinc_json_callback(const httplib::Request& req, httplib::Response& resp, const httplib::ContentReader &content_reader);
// 获取认证字符串
void get_client_auth_str_from_platform_callback(const httplib::Request& req, httplib::Response& resp);
void client_auth_str_print_all_callback(const httplib::Request& req, httplib::Response& resp);
// 认证客户端
void server_gate_auth_client_callback(const httplib::Request& req, httplib::Response& resp);
// 删除客户端
void del_client_callback(const httplib::Request& req, httplib::Response& resp);
//删除网络
void del_net(string net_name);

// 实际工作函数
// 配置tinc
//void configure_tinc(string role, string net_name , string local_name, string host_name, string vpn_ip);
bool configure_tinc(string role, string net_name , string local_name, string host_name, string vpn_ip, string host_local_ip, string host_vpn_ip, string port);
// 添加从网关公钥
bool auth_client_gate(string net_name, string host_name, string local_name, string vpn_ip, string client_pub_key, string& gate_pub_key);
// 添加客户端公钥
bool auth_client(string net_name, string host_name, string local_name, string client_uuid, string client_auth_str, string client_pub_key, string& gate_pub_key);
// 删除客户端
void del_client(string client_name, string net_name);
// 删除网络
void del_net_callback(const httplib::Request& req, httplib::Response& resp);

// 注册到平台
bool sign_up_to_platform();
void client(string client_addr);
// 上报系统信息
bool t_sys_info_to_platform(); //线程封装
bool sys_info_to_platform();
// 获取主网关公钥
void get_host_gate_pubkey(string host_ip, string net_name, string local_name, string host_name, \
		string vpn_ip, string client_pub_key);

// 测试
int simple_add(int a, int b);



#endif
