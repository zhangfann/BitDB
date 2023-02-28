#ifndef KV_JSON
#define KV_JSON
#include <iostream>

#include <stdio.h>
// jsoncpp的头文件 在 /usr/include/jsoncpp中
#include <json/value.h>

#ifdef __APPLE__
    #include <json/json.h>
#else
    #include <jsoncpp/json/json.h>
#endif

#include <fstream>
#include <sstream>



// json **********************************************

void string2num(std::string str, double &num);

#endif
