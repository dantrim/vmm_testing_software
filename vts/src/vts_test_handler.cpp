//vts
#include "vts_test_handler.h"
#include "vts_test_types.h"
#include "helpers.h"

//std/stl
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

//logging
#include "spdlog/spdlog.h"

namespace vts
{

VTSTestHandler::VTSTestHandler(QObject* parent) :
    QObject(parent)
{
}

VTSTestHandler::~VTSTestHandler()
{
}

bool VTSTestHandler::tests_are_ok(vector<string> test_config_files)
{
    // static function : call a new instance of a logger that
    // will only live within this function's scope

    auto logger = spdlog::get("vts_logger");

    // only continue if all tests are known and ok
    bool all_ok = true;
    for(auto config_file : test_config_files)
    {
        std::ifstream input_config_file(config_file);
        json jf; 
        input_config_file >> jf;
        string test_name = jf.at("test_type").get<std::string>();
        if(!is_valid_test(test_name))
        {
            stringstream err;
            err << "Unknown test type \"" << test_name << "\"";
            logger->error("{0} - {1}",__VTFUNC__,err.str());
            all_ok = false;
        }
    }
    return all_ok;
}

bool VTSTestHandler::is_valid_test(string test_type)
{
    return(!(StrToVTSTestType(test_type) == VTSTestType::VTSTESTTYPEINVALID));
}

void VTSTestHandler::load_test_configs(vector<string> test_config_files)
{
    log = spdlog::get("vts_logger");
    log->debug("{0} - {1}",__VTFUNC__,"Initializing VTS Test Handler");

    m_test_configs.clear();
    m_test_config_map.clear();
    for(auto config_file : test_config_files)
    {
        std::ifstream input_file(config_file);
        json jf;
        input_file >> jf;
        string test_name = jf.at("test_type").get<std::string>();
        m_test_configs.push_back(config_file);
        m_test_config_map[test_name] = config_file;
    }
    stringstream msg;
    msg << "Loaded " << m_test_configs.size() << " test(s): {";
    for(const auto & tf : m_test_config_map)
        msg << " " << tf.first << " ";
    msg << "}";
    log->info("{0} - {1}",__VTFUNC__,msg.str());
}

void VTSTestHandler::start()
{
    log->info("{0} - {1}",__VTFUNC__,"Starting tests");

    stringstream msg;
    for(const auto & tf : m_test_config_map)
    {
        msg.str("");
        string test_name = tf.first;
        string test_config_file = tf.second;

        std::ifstream input_file(test_config_file);
        json jtest;
        input_file >> jtest;

        msg << "MOVING TO TEST: " << test_name;
        log->info("{0} - {1}",__VTFUNC__,msg.str());

        if(m_test.get() != nullptr)
        {
            // just reset it for now -- should take care to close it first
            m_test.reset();
        }
        m_test = std::make_shared<vts::VTSTest>();
        connect(m_test.get(), SIGNAL(broadcast_state_signal(QString)),
                            this, SLOT(update_state(QString)));
        bool status = m_test->initialize(jtest);
        if(status)
        {
            //stringstream msg;
            //msg << "STARTING FROM MAIN THREAD: 0x" << std::hex << std::this_thread::get_id() << std::dec;
            //log->critical("{0} - {1}",__VTFUNC__,msg.str());
            // start the process
            m_test_thread = std::thread(&vts::VTSTest::start, m_test.get()); // [this] () {
        
//        m_thread = std::thread( [m_test.get()] () { } 
//            m_test->start();
        }
    }
}

void VTSTestHandler::stop()
{
    log->info("{0} - {1}",__VTFUNC__,"Stopping all tests");
    m_test->stop();
}

void VTSTestHandler::update_state(QString state_qstring)
{
    string current_state = state_qstring.toStdString();
    stringstream msg;
    msg << "Current test state: " << current_state;
    log->info("{0} - {1}",__VTFUNC__,msg.str());
}


} // namespace vts;
