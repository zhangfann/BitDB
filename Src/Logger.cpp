#include "Logger.h"
// #include "KvJson.h"
#include <string>

std::shared_ptr<spdlog::logger> Logger::_logger = NULL;
Logger::Logger(){

}

std::shared_ptr<spdlog::logger> Logger::instance(std::string file){
    if(_logger == NULL){
//    	_logger = spdlog::daily_logger_mt("daily_logger", "/etc/tinc/logs/daily.txt", 2, 30);
    	_logger = spdlog::daily_logger_mt("daily_logger", file, 2, 30);

    	_logger->flush_on(spdlog::level::debug);  // 遇到 err 级别缓冲区倾倒写入文本
        spdlog::flush_every(std::chrono::seconds(5));
    }
    return _logger;
}

void daily_example()
{
    // Create a daily logger - a new file is created every day on 2:30am
    auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);

    logger->flush_on(spdlog::level::debug);  // 遇到 err 级别缓冲区倾倒写入文本
    //spdlog::flush_every(std::chrono::seconds(5));

    logger->info("Welcome to spdlog!");
    logger->error("Some error message with arg: {}", 1);
    logger->warn("Easy padding in numbers like {:08d}", 12);
    logger->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    logger->info("Support for floats {:03.2f}", 1.23456);
    logger->info("Positional args are {1} {0}..", "too", "supported");
    logger->info("{:<30}", "left aligned");
    logger->set_level(spdlog::level::debug); // Set global log level to debug
    logger->debug("This message should be displayed..");
}
