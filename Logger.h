#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <string>
using namespace std;

class Logger{
public:

	static std::shared_ptr<spdlog::logger> instance(std::string file="");
    Logger();


    static std::shared_ptr<spdlog::logger> _logger;
};
#define gLogger Logger::instance()



#endif
