#include <QCoreApplication>
#include <iostream>
#include <vector>
#include <memory>
#include <QObject>
#include <chrono>
using namespace std;

// logger
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "vts_server.h"

int main(int argc, char *argv[])
{
    // initialize the logger
    std::string log_pattern = "[%D %H:%M:%S] [%t] [%^%L%$] %v";
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern(log_pattern);
    spdlog::flush_every(std::chrono::seconds(1)); // experimental

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vts_server_run.log", true);
    file_sink->set_pattern(log_pattern);
    file_sink->set_level(spdlog::level::trace);

    vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console_sink);
    sinks.push_back(file_sink);
    auto logger = std::make_shared<spdlog::logger>("vts", begin(sinks), end(sinks));
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
    logger->debug("this is debug");
    logger->info("this is info");
    logger->warn("this is warn");
    logger->error("this is error");
    logger->critical("this is critical");


    QCoreApplication a(argc, argv);
    
    cout << "Running application" << endl;
    cout << "Starting VTS..." << endl;
    vts::VTSServer* server = new vts::VTSServer();
    if(!server->start())
    {
        logger->critical("failed to start VTS server!");
        delete server;
        exit(1);
    }
    return a.exec();
}
