// std/stl
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <fstream>
using namespace std;

// Qt
#include <QCoreApplication>

// logger
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

// VTS
#include "vts_server.h"
#include "helpers.h"

// json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

void help()
{
    cout << "-------------------------------------------------" << endl;
    cout << " VTS Server" << endl;
    cout << "-------------------------------------------------" << endl;
}

void setup_logging(json& logging_config)
{
    std::string log_pattern = "[%D %H:%M:%S] [%t] [%^%L%$] %v";
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern(log_pattern);
    spdlog::flush_every(std::chrono::seconds(1)); // experimental

    stringstream logfile;
    try
    {
        std::string logfiledir = logging_config.at("log_location");
        logfile << logfiledir;
        logfile << "/vts_server_run.log";
    }
    catch(std::exception& e)
    {
        cout << "WARNING Failed to setup log file, using default: ./vts_server_run.log" << endl;
        logfile.str("");
        logfile << "vts_server_run.log";
    }

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile.str(), true);
    file_sink->set_pattern(log_pattern);
    file_sink->set_level(spdlog::level::trace);

    vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console_sink);
    sinks.push_back(file_sink);
    auto logger = std::make_shared<spdlog::logger>("vts_logger", begin(sinks), end(sinks));
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);

    std::string log_level = "INFO";
    try
    {
        log_level = logging_config.at("log_level");
    }
    catch(std::exception& e)
    {
        log_level = "INFO";
        cout << "WARNING Failed to get logging level from configuration, using \"INFO\"" << endl;
    }

    if(log_level=="TRACE")
    {
        console_sink->set_level(spdlog::level::trace);
    }
    else if(log_level=="DEBUG")
    {
        console_sink->set_level(spdlog::level::debug);
    }
    else if(log_level=="INFO")
    {
        console_sink->set_level(spdlog::level::info);
    }
    else if(log_level=="WARN")
    {
        console_sink->set_level(spdlog::level::warn);
    }
    else if(log_level=="ERROR")
    {
        console_sink->set_level(spdlog::level::err);
    }
    else if(log_level=="CRITICAL")
    {
        console_sink->set_level(spdlog::level::critical);
    }
}

bool check_config(const json& config)
{
    // at some point a scheme will be enforced using python's json schema
    // validation, but for now just do some kludge

    // check top-level
    try
    {
        auto top_level = config.at("vts_config");
    }
    catch(std::exception& e)
    {
        cout << "ERROR JSON configuration does not have expected top-level (=\"vts_config\")" << endl;
        return false;
    }

    // check subsequent levels
    auto top_level = config.at("vts_config");
    vector<std::string> nodes = { "vts_server", "logging", "frontend", "daq" };
    bool all_ok = true;
    for(auto & exp : nodes)
    {
        bool found = false;
        for(json::iterator it = top_level.begin(); it != top_level.end(); ++it)
        {
            if(it.key() == exp)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            all_ok = false;
            cout << "ERROR Failed to find node \"" << exp << "\" in vts_config top-level configuration" << endl;
        }
    }
    if(!all_ok)
    {
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
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
            cout << "ERROR Unable to start VTS server: unknown command line argument provided (=" << in << ")" << endl;
            return 1;
        }
        optin++;
    }

    std::ifstream config_file(vts_config_file);
    if(!config_file.good())
    {
        cout << "ERROR Unable to locate provided VTS configuration file (=" << vts_config_file << ")" << endl;
        return 1;
    }
    std::ifstream json_config_file(vts_config_file);
    json config_data;
    try
    {
        json_config_file >> config_data;
    }
    catch(std::exception& e)
    {
        cout << "ERROR Unable to parse JSON configuration" << endl;
        return 1;
    }
    if(!check_config(config_data))
    {
        cout << "ERROR Provided JSON configuration does not match expected schema" << endl;
        return 1;
    }
    config_data = config_data.at("vts_config");
    auto logging_config = config_data.at("logging");
    setup_logging(logging_config);
    auto logger = spdlog::get("vts_logger");

    QCoreApplication a(argc, argv);
    vts::VTSServer* server = new vts::VTSServer();
    logger->debug("{0} - {1}", __VTFUNC__, "Starting VTS");
    server->load_config(config_data);
    if(!server->start())
    {
        logger->critical("failed to start VTS server!");
        delete server;
        exit(1);
    }
    return a.exec();
}
