//vts
#include "vts_test_handler.h"
#include "vts_test_types.h"
#include "filemanager.h"
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

void VTSTestHandler::load_output_config(const json& output_cfg)
{
    m_output_cfg = output_cfg;
}

void VTSTestHandler::load_frontend_config(const json& frontend_cfg, const json& daq_cfg)
{
    m_frontend_cfg = frontend_cfg;
    m_daq_cfg = daq_cfg;
}

void VTSTestHandler::load_test_config(const json& test_cfg)
{
    log = spdlog::get("vts_logger");

    m_vmm_serial_id = test_cfg.at("VMM_SERIAL_ID").get<std::string>();
    log->debug("{0} - {1} {2}",__VTFUNC__,"Initializing VTS Test Handler for VMM",m_vmm_serial_id);


    vector<string> test_config_files = test_cfg.at("TEST_CONFIG");
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
    log->info("{0} - {1} {2}",__VTFUNC__,"Starting tests for VMM",m_vmm_serial_id);

    // testing
    vts::FileManager* fmg = new vts::FileManager(m_vmm_serial_id, m_output_cfg);
    if(!fmg->create_output()) return;

    stringstream msg;
    for(const auto & tf : m_test_config_map)
    {
        msg.str("");
        string test_name = tf.first;
        string test_config_file = tf.second;

        // add the test directory to the output
        if(!fmg->add_test_dir(test_name))
        {
            log->warn("{0} - Test data already seems to be present in output file",__VTFUNC__);
        }

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
        bool initialize_ok = m_test->initialize(jtest, m_frontend_cfg, m_daq_cfg);
        initialize_ok &= (m_test->current_state() == vts::VTSTestState::INITIAL);
        if(!initialize_ok)
        {
            // at this point, nothing in the test has started, so we can just exit
            return;
        }
        // once initialized we can safely provide the FileManager to the Test
        m_test->load_file_manager(fmg);

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
                if(!status) break;

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
    }

    delete fmg;
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
    log->trace("{0} - {1}",__VTFUNC__,msg.str());
}


} // namespace vts;
