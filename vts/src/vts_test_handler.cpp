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

void VTSTestHandler::load_frontend(const json& cfg)
{
    m_frontend_cfg = cfg;
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
                            this, SLOT(update_state(QString)), Qt::DirectConnection);

        /////////////////////////////////////////////////////////////////
        // INITIALIZE
        /////////////////////////////////////////////////////////////////
        bool initialize_ok = m_test->initialize(jtest, m_frontend_cfg);
        initialize_ok &= (m_test->current_state() == vts::VTSTestState::INITIAL);
        if(!initialize_ok)
        {
            // at this point, nothing in the test has started, so we can just exit
            return;
        }

        bool status = true;
        stringstream msg;
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        msg << "STARTING TEST LOOP";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        while((m_test->current_state() != vts::VTSTestState::FINISHED) && status)
        {
            try
            {
                ////////////////////////////////////////////////////////
                // LOAD
                ////////////////////////////////////////////////////////
                status = m_test->load();

                ////////////////////////////////////////////////////////
                // CONFIGURE
                ////////////////////////////////////////////////////////
                status = m_test->configure();

                ////////////////////////////////////////////////////////
                // RUN/COLLECT DATA
                ////////////////////////////////////////////////////////
                status = m_test->run();

                ////////////////////////////////////////////////////////
                // ANALYZE THIS STEPS DATA
                ////////////////////////////////////////////////////////
                status = m_test->analyze();

                ////////////////////////////////////////////////////////
                // IF WE ARE AT THE END FINISH THE TEST LOOP
                ////////////////////////////////////////////////////////
                if(m_test->current_step() == m_test->n_steps())
                {
                    status = m_test->finalize();
                }

            } // try
            catch(std::exception& e)
            {
                status = false;
                msg.str("");
                msg << "Error during test: " << e.what();
                log->error("{0} - {1}",__VTFUNC__,msg.str());
            } // catch
        } // while

        /////////////////////////////////////////////////////////////////
        // ANALYZE COMPLETE TEST RESULTS AND DATA
        /////////////////////////////////////////////////////////////////
        bool analyze_status = m_test->analyze_test();

       // /////////////////////////////////////////////////////////////////
       // // FINALIZE
       // /////////////////////////////////////////////////////////////////
       // bool finalize_status = m_test->finalize();

        msg.str();
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        msg << "TEST LOOP COMPLETE";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        

//        if(status)
//        {
//            //stringstream msg;
//            //msg << "STARTING FROM MAIN THREAD: 0x" << std::hex << std::this_thread::get_id() << std::dec;
//            //log->critical("{0} - {1}",__VTFUNC__,msg.str());
//            // start the process
////            m_test_thread = std::thread(&vts::VTSTest::start, m_test.get()); // [this] () {
//        
////        m_thread = std::thread( [m_test.get()] () { } 
////            m_test->start();
//        }
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
