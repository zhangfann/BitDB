#ifndef KV_SERVICE
#define KV_SERVICE
#include "httplib.h"

#include <stdio.h>
// jsoncpp的头文件 在 /usr/include/jsoncpp中
#include <json/value.h>

#ifdef __APPLE__
    #include <json/json.h>
#else
    #include <jsoncpp/json/json.h>
#endif
#include <fstream>
#include <string>


#include "KvJson.h"
using namespace std;
// 子目录
class MarkdownSubItem{
public:
  int id;
  string title;
};
// 父目录
class MarkdownItem{
public:
  int id;
  string title;
  string content;
  vector<MarkdownSubItem> nested_list;

  MarkdownItem(){
    this->id=0;
    this->title="";
    this->content="";
  };
};


// callback ***************************************
void get_list_callback(const httplib::Request& req, httplib::Response& resp);
void search_callback(const httplib::Request& req, httplib::Response& resp);

void index_callback(const httplib::Request& req, httplib::Response& resp);

void add_callback(const httplib::Request& req, httplib::Response& resp);

void login_callback(const httplib::Request& req, httplib::Response& resp);
#endif
