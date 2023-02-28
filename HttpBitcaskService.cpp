#include "HttpBitcaskService.h"

void bitcask_get_callback(const httplib::Request& req, httplib::Response& resp){
	std::string key = req.get_param_value("key");

	std::string value="";
	gBitcaskService->get(key, value);

	resp.set_content(value, "text/plain");
}


void bitcask_set_callback(const httplib::Request& req, httplib::Response& resp){
	std::string key = req.get_param_value("key");
	std::string value = req.get_param_value("value");

	gBitcaskService->set(key, value);

	resp.set_content("success", "text/plain");

}
