#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>


#include "zerotrust.h"

#include "config.h"
#include "client_auth_list.h"
#include "logger.h"
#include "sys_info.h"
#include "db.h"
#include <mutex>

// jsoncpp的头文件 在 /usr/include/jsoncpp中
#include <json/value.h>

#ifdef __APPLE__
    #include <json/json.h>
#else
    #include <jsoncpp/json/json.h>
#endif


// role 是server还是client ["server" | "client"]
// net_name 网络名称
bool configure_tinc(string role, string net_name , string local_name, string host_name, string vpn_ip, string host_local_ip, string host_vpn_ip, string port){
	// 初始化tinc 创建目录
    char cmd[MAX_STRING_LEN] = {0};
    char result[MAX_STRING_LEN] = {0};
    bool ret= false;
	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;
	string tun_id=gConfig->tun_id;
	string tun_name="tinc"+tun_id;
	gConfig->tun_id=to_string(std::stoi(tun_id)+1);
	gConfig->write_config();
//	int int_ret;
//    snprintf(cmd, MAX_STRING_LEN, "/etc/tinc/%s", net_name.c_str());
//    int_ret=mkdir(cmd,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
//    cout<<"mkdir"<<int_ret<<strerror(errno)<<endl;
//    snprintf(cmd, MAX_STRING_LEN, "/etc/tinc/%s/hosts", net_name.c_str());
//    mkdir(cmd,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
//    cout<<"mkdir"<<int_ret<<strerror(errno)<<endl;
//	snprintf(cmd, MAX_STRING_LEN, "touch /etc/tinc/%s/hosts/test",  \
//			net_name.c_str());
//	ret = getShellResult(cmd, result);
//
//
//    return ;

//    snprintf(cmd, MAX_STRING_LEN, "ls /etc/tinc/");
//    ret = getShellResult(cmd, result);
//    if(NULL == strstr(result, net_name.c_str())){
//    	cout<< "未找到"<<net_name<<endl;
//    	return;
//    }
	// tinc.conf
    if(0== role.compare("server")){
		snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_server_conf.sh %s %s %s %s", shell_path.c_str(), \
				net_name.c_str(), host_name.c_str(), port.c_str(), tun_name.c_str());
		cout<<cmd<<endl;
		gLogger->info(cmd);
		ret = getShellResult(cmd, result);
	    if(!ret){
	    	cout <<"tinc_server_conf失败"<<endl;
	    	return false;
	    }
    }else{
		snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_client_conf.sh %s %s %s", shell_path.c_str(), \
				net_name.c_str(), local_name.c_str(), host_name.c_str());
		cout<<cmd<<endl;
		ret = getShellResult(cmd, result);
	    if(!ret){
	    	cout <<"tinc_client_conf失败"<<endl;
	    	return false;
	    }
    }

	// hosts/<主机名>
    if(0== role.compare("server")){
		snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_server_hosts.sh %s %s %s %s", shell_path.c_str(), \
				net_name.c_str(), host_name.c_str(), host_vpn_ip.c_str(), local_ip.c_str());
		gLogger->info(cmd);
		cout<<cmd<<endl;
		ret = getShellResult(cmd, result);
	    if(!ret){
	    	cout <<"tinc_server_hosts失败"<<endl;
	    	return false;
	    }
    }else{

		snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_client_hosts.sh %s %s %s %s %s %s %s", shell_path.c_str(), \
				net_name.c_str(), local_name.c_str(), local_ip.c_str(), vpn_ip.c_str(),  \
				host_name.c_str(), host_local_ip.c_str(), host_vpn_ip.c_str());
		cout<<cmd<<endl;
		gLogger->info(cmd);
		ret = getShellResult(cmd, result);
	    if(!ret){
	    	cout <<"tinc_client_conf失败"<<endl;
	    	return false;
	    }
    }

	// tinc-up <vpn_ip>
    if(0== role.compare("server")){
    	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_tinc_up.sh %s %s", shell_path.c_str(), \
    			net_name.c_str(), host_vpn_ip.c_str());
    	gLogger->info(cmd);
    	cout<<cmd<<endl;
    	ret = getShellResult(cmd, result);
        if(!ret){
        	cout <<"tinc_tinc_up 失败"<<endl;
        	return false;
        }
    }else{
    	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_tinc_up.sh %s %s", shell_path.c_str(), \
    			net_name.c_str(), vpn_ip.c_str());
    	gLogger->info(cmd);
    	cout<<cmd<<endl;
    	ret = getShellResult(cmd, result);
        if(!ret){
        	cout <<"tinc_tinc_up 失败"<<endl;
        	return false;
        }
    }


	// 生成公钥
	// 	tincd -n <网络名称> -K
	snprintf(cmd, MAX_STRING_LEN, "tincd -n %s -K", net_name.c_str());
	gLogger->info(cmd);
	cout<<cmd<<endl;
	ret = getShellResult(cmd, result);
    if(!ret){
    	cout <<"tinc生成密钥 失败"<<endl;
    	return false;
    }

	// 启动tinc
    // 	tincd -n <网络名称>
	snprintf(cmd, MAX_STRING_LEN, "tincd -n %s", net_name.c_str());
	gLogger->info(cmd);
	cout<<cmd<<endl;
	ret = getShellResult(cmd, result);
    if(!ret){
    	cout <<"tinc启动 失败"<<endl;
    	return false;
    }

    char tinc_process[MAX_STRING_LEN] = {0};
	snprintf(cmd, MAX_STRING_LEN, "ifconfig %s", \
			tun_name.c_str());
	sleep(2);
	ret = getShellResult(cmd, result);
	gLogger->info("check net config {}", result);
//	snprintf(tinc_process, MAX_STRING_LEN, "%s", host_vpn_ip.c_str());
	snprintf(tinc_process, MAX_STRING_LEN, "Device not found");

	if(strlen(result)==0){
		return false;
	}else{
		return true;

	}
}

void configure_tinc_callback(const httplib::Request& req, httplib::Response& resp){
	std::string role = req.get_param_value("role");
	std::string net_name = req.get_param_value("net_name");
	std::string local_name = "";
	std::string host_local_ip = "";
	std::string vpn_ip = "";
	if(0== role.compare("client")){
		local_name = req.get_param_value("client_name");
		host_local_ip = req.get_param_value("server_real_ip");
		vpn_ip = req.get_param_value("client_vpn_ip");

	}
	std::string host_name = req.get_param_value("server_name");
	std::string host_vpn_ip = req.get_param_value("server_vpn_ip");
	std::string port = req.get_param_value("port");


//	configure_tinc(role, net_name , local_name, host_name, vpn_ip);
	gLogger->info("configure_tinc_callback, role:{}, net_name:{}, local_name:{}, host_name:{}, vpn_ip:{}\
			host_local_ip:{}, host_vpn_ip:{}, port:{}", role, net_name , local_name, host_name, vpn_ip, \
			host_local_ip, host_vpn_ip,port);
	configure_tinc(role, net_name , local_name, host_name, vpn_ip, \
			host_local_ip, host_vpn_ip, port);

	gLogger->info("configure_tinc_callback"+local_name+net_name);

	// 请求公钥
	  if(0==role.compare("server")){
		  gLogger->info("sign_up_to_platform : server");
	  }else if(0==role.compare("client")){
		  gLogger->info("sign_up_to_platform : client");
		  // 请求主网关交换公钥
			string shell_path=gConfig->shell_path;
			string local_ip= gConfig->local_ip;
			char cmd[MAX_STRING_LEN] = {0};
			char result[MAX_STRING_LEN] = {0};
			bool ret= false;
		  string client_pub_key="";
			// 返回公钥
			snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_get_host_pub_key.sh %s %s %s", shell_path.c_str(), \
					net_name.c_str(), local_name.c_str(), vpn_ip.c_str());
			ret = getShellResult(cmd, result);
			gLogger->info(cmd);
			cout<< cmd <<endl;
			if(!ret){
				cout <<"tinc_get_host_pub_key.sh 失败"<<endl;
				return ;
			}
			client_pub_key= result;
			gLogger->info("gate_pub_key: {}", client_pub_key);
			cout << "client_pub_key"<< client_pub_key<<endl;
		  get_host_gate_pubkey(host_local_ip, net_name, local_name, host_name, \
		  		vpn_ip, client_pub_key);
	  }else{
		  gLogger->error("sign_up_to_platform : unkown role");
		  return ;
	  }

}

void configure_tinc_json_callback(const httplib::Request& req, httplib::Response& resp, const httplib::ContentReader &content_reader){
//	std::string params_str= req.get_param_value("param");
	std::string params_str= "";


    content_reader([&](const char *data, size_t data_length) {
    	params_str.append(data, data_length);
      return true;
    });

    gLogger->info("configure_tinc_json_callback params_str {}", params_str);

    Json::Reader reader;
    Json::Value json_root;

    if(!reader.parse(params_str, json_root)){
    	gLogger->error("configure_tinc_json_callback read json error");
    	return ;
    }

    std::string role ="server";

	std::string net_name = json_root["tinc"]["name"].asString();
	std::string local_name = "";
	std::string host_local_ip = json_root["tinc"]["server"]["address"].asString();
	std::string vpn_ip = "";
	if(0== role.compare("client")){
		local_name = req.get_param_value("client_name");
//		host_local_ip = req.get_param_value("server_real_ip");
		vpn_ip = req.get_param_value("client_vpn_ip");

	}
	std::string host_name = json_root["tinc"]["server"]["name"].asString();
	std::string host_vpn_ip = json_root["tinc"]["server"]["subnet"].asString();
	std::string port = json_root["tinc"]["server"]["port"].asString();

//	std::string role = req.get_param_value("role");
//	std::string net_name = req.get_param_value("net_name");
//	std::string local_name = "";
//	std::string host_local_ip = "";
//	std::string vpn_ip = "";
//	if(0== role.compare("client")){
//		local_name = req.get_param_value("client_name");
//		host_local_ip = req.get_param_value("server_real_ip");
//		vpn_ip = req.get_param_value("client_vpn_ip");
//
//	}
//	std::string host_name = req.get_param_value("server_name");
//	std::string host_vpn_ip = req.get_param_value("server_vpn_ip");
//	std::string port = req.get_param_value("port");


//	configure_tinc(role, net_name , local_name, host_name, vpn_ip);
	gLogger->info("configure_tinc_callback, role:{}, net_name:{}, local_name:{}, host_name:{}, vpn_ip:{}\
			host_local_ip:{}, host_vpn_ip:{}, port:{}", role, net_name , local_name, host_name, vpn_ip, \
			host_local_ip, host_vpn_ip,port);
	bool ret= configure_tinc(role, net_name , local_name, host_name, vpn_ip, \
			host_local_ip, host_vpn_ip, port);

//	gLogger->info("configure_tinc_callback"+local_name+net_name);

	// 请求公钥
	  if(0==role.compare("server")){
		  gLogger->info("sign_up_to_platform : server");
	  }else if(0==role.compare("client")){
		  gLogger->info("sign_up_to_platform : client");
		  // 请求主网关交换公钥
			string shell_path=gConfig->shell_path;
			string local_ip= gConfig->local_ip;
			char cmd[MAX_STRING_LEN] = {0};
			char result[MAX_STRING_LEN] = {0};
			bool ret= false;
		  string client_pub_key="";
			// 返回公钥
			snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_get_host_pub_key.sh %s %s %s", shell_path.c_str(), \
					net_name.c_str(), local_name.c_str(), vpn_ip.c_str());
			ret = getShellResult(cmd, result);
			gLogger->info(cmd);
			cout<< cmd <<endl;
			if(!ret){
				cout <<"tinc_get_host_pub_key.sh 失败"<<endl;
				return ;
			}
			client_pub_key= result;
			gLogger->info("gate_pub_key: {}", client_pub_key);
			cout << "client_pub_key"<< client_pub_key<<endl;
		  get_host_gate_pubkey(host_local_ip, net_name, local_name, host_name, \
		  		vpn_ip, client_pub_key);
	  }else{
		  gLogger->error("sign_up_to_platform : unkown role");
		  return ;
	  }

//		string shell_path=gConfig->shell_path;
//		string local_ip= gConfig->local_ip;
//		char cmd[MAX_STRING_LEN] = {0};
//		char result[MAX_STRING_LEN] = {0};
//		char tinc_process[MAX_STRING_LEN] = {0};
//		bool ret= false;
//		// 返回公钥
//		snprintf(cmd, MAX_STRING_LEN, "ps -elf| grep tincd | grep %s", \
//				net_name.c_str());
//		ret = getShellResult(cmd, result);
//		gLogger->info("check net config {}", result);
//		snprintf(tinc_process, MAX_STRING_LEN, "tincd -n %s", \
//						net_name.c_str());
		if(ret){
			resp.set_content("success", "text/plain");
			gLogger->info("net config success");
		}else{
			resp.set_content("error", "text/plain");
			gLogger->error("net config error");
		}


}

// 添加从网关公钥
bool auth_client_gate(string net_name, string host_name, string local_name, string vpn_ip, string client_pub_key, string& gate_pub_key){

	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;

	char cmd[MAX_STRING_LEN] = {0};
	char result[MAX_STRING_LEN] = {0};
	bool ret= false;

	// 比对认证字符串
	// 添加客户端公钥
	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_add_client_gate.sh %s %s %s '%s'", shell_path.c_str(), \
			net_name.c_str(), local_name.c_str(), vpn_ip.c_str(), client_pub_key.c_str());
	ret = getShellResult(cmd, result);
	gLogger->info(cmd);
	if(!ret){
		cout <<"tinc_add_client失败"<<endl;
		return false;
	}

	// 返回公钥
	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_get_host_pub_key.sh %s %s %s", shell_path.c_str(), \
			net_name.c_str(), host_name.c_str(), vpn_ip.c_str());
	ret = getShellResult(cmd, result);
	gLogger->info(cmd);
	cout<< cmd <<endl;
	if(!ret){
		cout <<"tinc_get_host_pub_key.sh 失败"<<endl;
		return false;
	}
	gate_pub_key= result;
	gLogger->info("gate_pub_key: {}", gate_pub_key);
	cout << "gate_pub_key"<< gate_pub_key<<endl;
	return true;

}

// 添加从网关公钥
void auth_client_gate_callback(const httplib::Request& req, httplib::Response& resp){
//	std::string client_uuid = req.get_param_value("client_uuid");
//	std::string client_auth_str = req.get_param_value("client_auth_str");



	string net_name = req.get_param_value("net_name");
	string local_name= req.get_param_value("local_name");
	string host_name= req.get_param_value("host_name");
	string vpn_ip=req.get_param_value("vpn_ip");
	string client_pub_key=req.get_param_value("client_pub_key");
	string gate_pub_key="";

	cout<<"auth_client_gate_callback"<<endl;

	// 添加至本地客户端字符串列表
//	ClientAuthInfo info;
//	info.client_uuid="123";
//	info.client_auth_str="4444";
//	gClientAuthList->add(info);

	bool ret=auth_client_gate(net_name, host_name, local_name, vpn_ip, client_pub_key, gate_pub_key);

	resp.set_content(gate_pub_key, "text/plain");
}


// 客户端添加hosts
void client_configure_tinc(string net_name , string local_name, string vpn_ip){
    char cmd[MAX_STRING_LEN] = {0};
    char result[MAX_STRING_LEN] = {0};
    bool ret= false;
	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;

	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_add_client_hosts.sh %s %s %s", shell_path.c_str(), \
			net_name.c_str(), local_name.c_str(), vpn_ip.c_str());
	cout<<cmd<<endl;
	ret = getShellResult(cmd, result);
	gLogger->info("client_configure_tinc : {}", cmd);
    if(!ret){
    	gLogger->error("client_configure_tinc 失败");
    	return ;
    }
}

void client_configure_tinc_callback(const httplib::Request& req, httplib::Response& resp){
	std::string net_name = req.get_param_value("net_name");
	std::string local_name = req.get_param_value("client_name");
	std::string vpn_ip = req.get_param_value("client_vpn_ip");


//	configure_tinc(role, net_name , local_name, host_name, vpn_ip);
	client_configure_tinc(net_name , local_name, vpn_ip);

	gLogger->info("client_configure_tinc_callback"+local_name+net_name);

	std::string client_uuid = req.get_param_value("client_uuid");
	std::string client_auth_str = req.get_param_value("client_auth_str");

	cout<<"get_client_auth_str_from_platform_callback"<< client_uuid << client_auth_str<< endl;

	gLogger->info("get_client_auth_str_from_platform_callback", client_uuid, client_auth_str);
	// 添加至本地客户端字符串列表
	ClientAuthInfo info;
	info.client_uuid=client_uuid;
	info.client_auth_str=client_auth_str;
	gClientAuthList->add(info);

}

void client_configure_tinc_json_callback(const httplib::Request& req, httplib::Response& resp, const httplib::ContentReader &content_reader){
	std::string params_str= "";
//	std::string params_str= req.get_param_value("param");

	content_reader([&](const char *data, size_t data_length) {
		params_str.append(data, data_length);
	  return true;
	});

	gLogger->info("client_configure_tinc_json_callback params_str {}", params_str);

	Json::Reader reader;
	Json::Value json_root;

	if(!reader.parse(params_str, json_root)){
		gLogger->error("configure_tinc_json_callback read json error");
	}

	std::string role ="server";


	std::string net_name = json_root["tinc"]["name"].asString();
	std::string local_name = json_root["tinc"]["client"]["name"].asString();
	std::string vpn_ip = json_root["tinc"]["client"]["subnet"].asString();


//	configure_tinc(role, net_name , local_name, host_name, vpn_ip);
	client_configure_tinc(net_name , local_name, vpn_ip);

	gLogger->info("client_configure_tinc_callback"+local_name+net_name);

	std::string client_uuid = json_root["tinc"]["client"]["clientUuid"].asString();
	std::string client_auth_str = json_root["tinc"]["client"]["authKey"].asString();

	cout<<"get_client_auth_str_from_platform_callback"<< client_uuid << client_auth_str<< endl;

	gLogger->info("get_client_auth_str_from_platform_callback", client_uuid, client_auth_str);
	// 添加至本地客户端字符串列表
	ClientAuthInfo info;
	info.client_uuid=client_uuid;
	info.client_auth_str=client_auth_str;
	add_client_auth_str(client_uuid, client_auth_str);
	gClientAuthList->add(info);

}


// 接收客户端认证字符串
void get_client_auth_str_from_platform_callback(const httplib::Request& req, httplib::Response& resp){
	std::string client_uuid = req.get_param_value("client_uuid");
	std::string client_auth_str = req.get_param_value("client_auth_str");

	cout<<"get_client_auth_str_from_platform_callback"<< client_uuid << client_auth_str<< endl;

	gLogger->info("get_client_auth_str_from_platform_callback", client_uuid, client_auth_str);
	// 添加至本地客户端字符串列表
	ClientAuthInfo info;
	info.client_uuid=client_uuid;
	info.client_auth_str=client_auth_str;
	add_client_auth_str(client_uuid, client_auth_str);
	gClientAuthList->add(info);
}

void client_auth_str_print_all_callback(const httplib::Request& req, httplib::Response& resp){
	gClientAuthList->print_all();
}



// 客户端认证
bool auth_client(string net_name, string host_name, string local_name, string client_uuid, string client_auth_str, string client_pub_key, string& gate_pub_key){

	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;

	char cmd[MAX_STRING_LEN] = {0};
	char result[MAX_STRING_LEN] = {0};
	bool ret= false;


	if(gClientAuthList->client_auth(client_uuid, client_auth_str)){
		// 比对认证字符串
		// 添加客户端公钥
		snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_add_client.sh %s %s %s", shell_path.c_str(), \
				net_name.c_str(), local_name.c_str(), client_pub_key.c_str());
		ret = getShellResult(cmd, result);
	    if(!ret){
	    	cout <<"tinc_add_client失败"<<endl;
	    	return false;
	    }

		//	TODO 返回公钥
		snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_get_host_pub_key.sh %s %s", shell_path.c_str(), \
				net_name.c_str(), host_name.c_str());
		ret = getShellResult(cmd, result);
		gLogger->info(cmd);
		cout<< cmd <<endl;
	    if(!ret){
	    	cout <<"tinc_get_host_pub_key.sh 失败"<<endl;
	    	return false;
	    }
	    gate_pub_key= result;
	    gLogger->info("gate_pub_key: {}", gate_pub_key);
	    cout << "gate_pub_key"<< gate_pub_key<<endl;
		return true;
	}else{
		// 比对认证字符串不同
		return false;
	}
}



//server网关客户端认证
void server_gate_auth_client_callback(const httplib::Request& req, httplib::Response& resp){
//	std::string client_uuid = req.get_param_value("client_uuid");
//	std::string client_auth_str = req.get_param_value("client_auth_str");

	string server_uuid=gConfig->uuid;

	string net_name = req.get_param_value("net_name");
	string local_name= req.get_param_value("local_name");
	string host_name= req.get_param_value("host_name");
	string client_uuid=req.get_param_value("client_uuid");
	string client_auth_str=req.get_param_value("client_auth_str");
	string client_pub_key=req.get_param_value("client_pub_key");
	string gate_pub_key="";

	gLogger->info("server_gate_auth_client_callback {} {} {} {} {} {}", net_name, local_name,\
			host_name, client_uuid, client_auth_str, client_pub_key);
	cout<<"server_gate_auth_client_callback"<< client_uuid << client_auth_str<< endl;

	// 添加至本地客户端字符串列表
//	ClientAuthInfo info;
//	info.client_uuid="123";
//	info.client_auth_str="4444";
//	gClientAuthList->add(info);

	bool ret=auth_client(net_name, host_name, local_name, client_uuid, client_auth_str, client_pub_key, gate_pub_key);

	string ret_str="";
	if(ret){

		Json::Value json_root;
		json_root["code"]= Json::Value(0);
		json_root["server_uuid"]= Json::Value(server_uuid);
		json_root["pub_key"]= Json::Value(gate_pub_key);
//		ret_str= json_root.toStyledString();
		Json::FastWriter writer;
		ret_str = writer.write(json_root);

		gLogger->info("auth client success");

//		ret_str="{\"code\":0,\"server_uuid\":"+server_uuid+",\"pub_key\":\""+gate_pub_key+"\"}";
	}else{
		Json::Value json_root;
		json_root["code"]= Json::Value(500);
		json_root["msg"]= Json::Value("client auth failed!");
//		json_root["pub_key"]= Json::Value(gate_pub_key);
//		ret_str= json_root.toStyledString();
		Json::FastWriter writer;
		ret_str = writer.write(json_root);
		gLogger->info("auth client failed");

//		ret_str="{\"code\":500,\"msg\":\"client auth failed!\"}";
	}

	resp.set_content(ret_str, "application/json");
//	resp.set_content(gate_pub_key, "text/plain");
}

void del_client(string client_name, string net_name){

    char cmd[MAX_STRING_LEN] = {0};
    char result[MAX_STRING_LEN] = {0};
    bool ret= false;
	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;

	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_del_client.sh %s %s", shell_path.c_str(), \
			net_name.c_str(), client_name.c_str());
	gLogger->info(cmd);
	ret = getShellResult(cmd, result);
    if(!ret){
    	gLogger->error("del_client 失败");
    	cout <<"del_client 失败"<<endl;
    	return ;
    }
}



void del_client_callback(const httplib::Request& req, httplib::Response& resp){
	std::string client_name = req.get_param_value("client_name");
	std::string net_name = req.get_param_value("net_name");


	gLogger->info("del_client_callback"+client_name+net_name);
	del_client(client_name, net_name);

}

void del_net(string net_name){

    char cmd[MAX_STRING_LEN] = {0};
    char result[MAX_STRING_LEN] = {0};
    bool ret= false;
	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;

	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_del_net.sh %s", shell_path.c_str(), \
			net_name.c_str());
	gLogger->info(cmd);
	ret = getShellResult(cmd, result);
    if(!ret){
    	gLogger->error("del_net 失败");
    	cout <<"del_net 失败"<<endl;
    	return ;
    }
}


void del_net_callback(const httplib::Request& req, httplib::Response& resp){
	std::string net_name = req.get_param_value("net_name");


	gLogger->info("del_net_callback"+net_name);
	del_net( net_name);

}
bool getShellResult(char* cmd, char* result)
{
	char buf[MAX_STRING_LEN] = { 0 };
	char tmp[MAX_STRING_LEN] = { 0 };
	FILE* fp = NULL;

	snprintf(buf, MAX_STRING_LEN, cmd);

	if ((fp = popen(buf, "r")) == NULL)
	{
		cout << "failed to open" << endl;
		return false;
	}

	string res_str = "";
	while ((fgets(tmp, sizeof(tmp), fp)) != NULL)
	{
		res_str.append(tmp);
	}
	int str_len = strlen(res_str.c_str());
	strncpy(result, res_str.c_str(), str_len);

	//    if (result[strlen(result) - 1] == '\n' || result[strlen(result) - 1] == '\r')
	//        result[strlen(result) - 1] = '\0';
	pclose(fp);

	return true;
}



void client(string client_addr){
  httplib::Client cli(client_addr);
  auto res = cli.Get("/");
  cout << res->status << res->body << endl;
}

// 注册到平台
bool sign_up_to_platform(){
	gLogger->info("sign_up_to_platform");

	string platform_address= gConfig->platform_address;
	string ip= gConfig->local_ip;
	string uuid=gConfig->uuid;


	string sign_up_url="/equipment/api/server/uploadUuid";
  httplib::Client cli(platform_address);
  cli.enable_server_certificate_verification(false);
  httplib::Params params;
  params.emplace("ip", ip);
  params.emplace("uuid", uuid);
//  params.emplace("note", "coder");
  try{
	  if(auto res = cli.Post(sign_up_url.c_str(), params)){
		  gLogger->info("sign_up_to_platform {}",res );
		  cout<<"res"<<res<<endl;
		  gLogger->info("sign_up_to_platform {} {}",res->status, res->body );
		  cout << "res->status"<<res->status <<"res->body"<< res->body << endl;
		  if(res->status==200){
			  return true;
			}else{
			  return false;
			}
	  }else{
		  cout << "error code: " << res.error() << std::endl;
		  gLogger->error("sign_up_to_platform error code: {}", to_string(res.error()));
		  return false;
	  }


  }
  catch(exception &e){
	  gLogger->error("sign_up_to_platform exception {}", e.what());
	  return false;
  }


//  if(res)


//  return true;

}

bool sys_info_to_platform(){
	gLogger->info("sys_info_to_platform");

	string platform_address= gConfig->platform_address;
	string sign_up_url="/equipment/api/uploadData";

	string cpu="";
	string mem="";
	string disk="";
	string upload="";
	string download="";

	double cpu_data, mem_data;
	float net_in, net_out;
	sys_msg_to_platform(cpu_data, mem_data, net_in,  net_out );

	cpu= to_string(cpu_data);
	mem= to_string(mem_data);
	upload= to_string(net_out);
	download=to_string(net_in);

	string uuid=gConfig->uuid;
  httplib::Client cli(platform_address);
  cli.enable_server_certificate_verification(false);
  httplib::Params params;
  params.emplace("uuid", uuid);
  params.emplace("type", "server");
  params.emplace("cpu", cpu);
  params.emplace("mem", mem);
  params.emplace("disk", disk);
  params.emplace("upload", upload);
  params.emplace("download", download);
  params.emplace("onlineStatus", "1");
  params.emplace("onlineTime", "1");
//  params.emplace("uuid", uuid);
//  params.emplace("note", "coder");
  try{
	  if(auto res = cli.Post(sign_up_url.c_str(), params)){
		  gLogger->info("sys_info_to_platform {}",res );
		  cout<<"res"<<res<<endl;
		  gLogger->info("sys_info_to_platform {} {}",res->status, res->body );
		  cout << "res->status"<<res->status <<"res->body"<< res->body << endl;
		  if(res->status==200){
			  return true;
			}else{
			  return false;
			}
	  }else{
		  cout << "error code: " << res.error() << std::endl;
		  gLogger->error("sys_info_to_platform error code: {}", to_string(res.error()));
		  return false;
	  }


  }
  catch(exception &e){
	  gLogger->error("sys_info_to_platform exception {}", e.what());
	  return false;
  }



//  if(res)


//  return true;

}

bool t_sys_info_to_platform(){
    while(1){
    	if(!sys_info_to_platform()){
    		gLogger->error("t_sys_info_to_platform failed sleep 5s");
    	}else{
    		gLogger->info("t_sys_info_to_platform ok");
    	}

		  sleep(5);
    }
}
// 获取主网关公钥
void get_host_gate_pubkey(string host_ip, string net_name, string local_name, string host_name, \
		string vpn_ip, string client_pub_key){
	gLogger->info("get_host_gate_pubkey");

	string host_port= gConfig->host_port;
	string host_address="http://"+host_ip+":"+host_port;
	string get_host_gate_pubkey_url="/auth_client_gate";
  httplib::Client cli(host_address);
  httplib::Params params;
  params.emplace("net_name", net_name);
  params.emplace("local_name", local_name);
  params.emplace("host_name", host_name);
  params.emplace("vpn_ip", vpn_ip);
  params.emplace("client_pub_key", client_pub_key);
//  params.emplace("note", "coder");
  auto res = cli.Post(get_host_gate_pubkey_url.c_str(), params);

//  if(res)
  // 主网关公钥
  cout << res->status << res->body << endl;
  string server_gate_pubkey=res->body;

	// 添加客户端公钥
	string shell_path=gConfig->shell_path;
	string local_ip= gConfig->local_ip;

	char cmd[MAX_STRING_LEN] = {0};
	char result[MAX_STRING_LEN] = {0};
	bool ret= false;
	snprintf(cmd, MAX_STRING_LEN, "sh %s/tinc_add_server_gate.sh %s %s %s '%s'", shell_path.c_str(), \
			net_name.c_str(), host_name.c_str(), vpn_ip.c_str(), server_gate_pubkey.c_str());
	ret = getShellResult(cmd, result);
//	cout<<cmd<<strlen(cmd)<<endl;
	gLogger->info(cmd);
	if(!ret){
		cout <<"tinc_add_client失败"<<endl;
		return ;
	}
}


// 上报系统信息
void post_sys_msg_to_platform(){
//	sys_msg_to_platform();
}


int simple_add(int a, int b){
    return a+b;
}
