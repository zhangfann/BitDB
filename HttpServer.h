#ifndef SERVER_H
#define SERVER_H

//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>

#include <stdio.h>
// jsoncpp的头文件 在 /usr/include/jsoncpp中
#include <json/value.h>
#include <fstream>

//#include "http_service.h"
#include "KvJson.h"
#include "Config.h"
#include "zerotrust.h"
#include "ClientAuthList.h"
#include "Logger.h"
#include "db.h"

#include <vector>
#include <string>
#include <thread>

class HTTPServer {
public:
	int start(int port);
	int stop();

private:
	static int http_server(int port);
	std::thread http_thread;
};



#endif
