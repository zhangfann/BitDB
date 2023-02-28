#pragma once


#include "httplib.h"

#include <stdio.h>
// jsoncpp的头文件 在 /usr/include/jsoncpp中
//#include <json/value.h>
//
//#ifdef __APPLE__
//    #include <json/json.h>
//#else
//    #include <jsoncpp/json/json.h>
//#endif

#include <fstream>
#include <string>

// Bitcask
#include "Bitcask.h"


//#include "KvJson.h"
using namespace std;



// callback ***************************************
void bitcask_get_callback(const httplib::Request& req, httplib::Response& resp);
void bitcask_set_callback(const httplib::Request& req, httplib::Response& resp);
//void get_list_callback(const httplib::Request& req, httplib::Response& resp);
//void search_callback(const httplib::Request& req, httplib::Response& resp);
//
//void index_callback(const httplib::Request& req, httplib::Response& resp);
//
//void add_callback(const httplib::Request& req, httplib::Response& resp);
//
//void login_callback(const httplib::Request& req, httplib::Response& resp);

