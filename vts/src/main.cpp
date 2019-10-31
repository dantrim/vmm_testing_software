#include <QCoreApplication>
#include <iostream>
#include <vector>
#include <memory>
#include <QObject>
#include <chrono>
#include <fstream>
using namespace std;

// logger
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

// VTS
#include "vts_server.h"

// json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

void help()
{
    cout << "-------------------------------------------------" << endl;
    cout << " VTS Server" << endl;
    cout << "-------------------------------------------------" << endl;
}

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

    // gather the input options
    std::string vts_config_file = "";
    std::string verbosity = "DEBUG";
    int optin(1);
    while(optin < argc)
    {
        string in = argv[optin];
        if(in == "-c" || in == "--config") { vts_config_file = argv[++optin]; }
        else
        {
            logger->critical("Unable to start VTS Server: unknown command line argument: " + in);
            return 1;
        }
        optin++;
    }

    std::ifstream config_file(vts_config_file);
    if(!config_file.good())
    {
        string err = "Unable to locate provided config file: " + vts_config_file;
        logger->critical(err);
        return 1;
    }
    std::ifstream json_config_file(vts_config_file);
    json config_data;
    json_config_file >> config_data;
//    cout << "FUCK: " << config_data.dump() << endl;
//    return 0;
    config_data = config_data.at("vts_config");
    auto logging_config = config_data.at("logging");
    string lvl = logging_config.at("log_level");
    if(lvl=="TRACE")
    {
        console_sink->set_level(spdlog::level::trace);
    }
    else if(lvl=="DEBUG")
    {
        console_sink->set_level(spdlog::level::debug);
    }
    else if(lvl=="INFO")
    {
        console_sink->set_level(spdlog::level::info);
    }
    else if(lvl=="WARN")
    {
        console_sink->set_level(spdlog::level::warn);
    }
    else if(lvl=="ERROR")
    {
        console_sink->set_level(spdlog::level::err);
    }
    else if(lvl=="CRITICAL")
    {
        console_sink->set_level(spdlog::level::critical);
    }

    QCoreApplication a(argc, argv);
    
    cout << "Running application" << endl;
    cout << "Starting VTS..." << endl;
    vts::VTSServer* server = new vts::VTSServer();
    server->load_config(config_data);
    if(!server->start())
    {
        logger->critical("failed to start VTS server!");
        delete server;
        exit(1);
    }
    return a.exec();
}
