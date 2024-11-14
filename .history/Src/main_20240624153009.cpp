// g++ main.cpp -lpthread -o main
/*
*/

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>

#include <stdio.h>
// jsoncpp的头文件 在 /usr/include/jsoncpp中
// #include <json/value.h>
#include <fstream>

// #include "http_service.h"
// #include "KvJson.h"
#include "Config.h"
#include "zerotrust.h"
#include "ClientAuthList.h"
#include "Logger.h"
// #include "db.h"
#include "HttpServer.h"
#include "semaphore.h"
//#include "AuditTaskServer.h"
//#include "AuditTaskList.h"

#include <vector>
#include <string>
#include <thread>
using namespace std;

semaphore gRunSem(0);
static bool gExitFlag = false;
static bool gReloadFlag = false;

void signal_handler(int sig)
{
	//cout << "sig:" << sig << endl;

	if ((SIGQUIT == sig) || (SIGINT == sig) || (SIGTERM == sig)) {
		gExitFlag = true;
		//cout << "gExitFlag==true" << endl;
		gRunSem.signal();
	}
	if (SIGUSR1 == sig) {
		gReloadFlag = true;
		//cout << "gReloadFlag==true" << endl;
		gRunSem.signal();
	}
}

void set_signal_handler()
{
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL); // 为了实现动态加载配置文件
}

// 主线程runLoop，阻塞住主线程
void run_loop()
{
	while (1) {
		gRunSem.wait();
		//cout << "run_loop wait finish:" << gExitFlag<<endl;

		if (gExitFlag) {
			gExitFlag = false;
			return;
		}

		if (gReloadFlag) {
			gReloadFlag = false;
			// todo 重新加载配置文件
		}

	}
}


// main ***************************************
int main() {

	// 初始化
	// 配置文件
	string config_file = "./config.json";
	Config::instance(config_file);

	// 日志
//	string log_file="./logs/daily.txt";
	string log_file = gConfig->log_path;
	Logger::instance(log_file);
	gLogger->info("Welcome to spdlog!");



	//  client(gConfig->client_addr);
	//  ClientAuthList::instance();

	  // 加载数据库中client_auth_str
	//  read_client_auth_str();

	//  std::thread t;
	//    // 上报数据
	//    if(gConfig->if_signup_platform.compare("true")==0){
	//  	  t= std::thread(t_sys_info_to_platform);
	//    }

	//  // 注册到平台
	//  if(gConfig->if_signup_platform.compare("true")==0){
	//	    while(!sign_up_to_platform()){
	//	  	  gLogger->error("sign_up_to_platform failed sleep 5s");
	//	  	  sleep(5);
	//	    }
	//  }
	set_signal_handler();

	  // 启动http server
	  //http_server(stoi(gConfig->host_port));
	//std::thread http_thread;
	//http_thread = std::thread(http_server, stoi(gConfig->host_port));

	//http服务
	HTTPServer http_server;
	http_server.start(stoi(gConfig->host_port));
	
//	// 审计任务
//	gAuditTaskList->init();
//
//	AuditTaskServer audit_task_server;
//	audit_task_server.start();

	run_loop();
	
	http_server.stop();
	//http_thread.join();
	//  t.join();
	return 0;
}

