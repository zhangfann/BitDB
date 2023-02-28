#include "KvJson.h"
#include "config.h"


// json **********************************************


void string2num(std::string str, double &num)
{
	std::stringstream ss;
	ss << str;
	ss >> num;
}
