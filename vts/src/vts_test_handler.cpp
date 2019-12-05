//vts
#include "vts_test_handler.h"
#include "vts_test_types.h"
#include "vts_result.h"
#include "filemanager.h"
#include "helpers.h"

//std/stl
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

//logging
#include "spdlog/spdlog.h"

//Qt
#include <QUdpSocket>

namespace vts
{

VTSTestHandler::VTSTestHandler(QObject* parent) :
    QObject(parent),
    m_is_running(false)
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
    try
    {
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
    }
    catch(std::exception& e)
    {
        stringstream err;
        err << "Provided test configuration files are invalid, exception caught: " << e.what();
        logger->error("{0} - {1}",__VTFUNC__,err.str());
        all_ok = false;
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
    m_test_names.clear();
    m_test_configs.clear();
    for(size_t test_idx = 0; test_idx < test_config_files.size(); test_idx++)
    {
        string config_file = test_config_files.at(test_idx);
        std::ifstream input_file(config_file);
        json jf;
        input_file >> jf;
        string test_name = jf.at("test_type").get<std::string>();

        m_test_names.push_back(test_name);
        m_test_configs.push_back(config_file);
    }
    stringstream msg;
    msg << "Loaded " << m_test_configs.size() << " test(s):";
    stringstream test_msg;
    for(size_t i = 0; i < m_test_names.size(); i++)
    {
        test_msg.str("");
        if(i>0)
        {
            size_t len = msg.str().length();
            msg.str("");
            msg << std::setw(len) << "";
        }
        test_msg << msg.str() << " " << i << ") " << m_test_names.at(i);
        log->info("{0} - {1}",__VTFUNC__, test_msg.str());
    }
}

void VTSTestHandler::run()
{
    log->info("{0} - {1} {2}",__VTFUNC__,"Starting tests for VMM",m_vmm_serial_id);
    m_stop_all_tests.store(false);

    // testing
    vts::FileManager* fmg = new vts::FileManager(m_vmm_serial_id, m_output_cfg);
    if(!fmg->create_output())
    {
        delete fmg;
        return;
    }
    m_is_running = true;

    stringstream msg;
    int test_idx = -1;
    size_t n_total_tests = m_test_names.size();
    string current_test = "";
    std::vector<json> test_results;

    if(!fmg->setup_output(m_test_names))
    {
        log->critical("{0} - Failed to setup output directory structure in ROOT file",__VTFUNC__);
        delete fmg;
        return;
    }

    for(size_t itest = 0; itest < m_test_names.size(); itest++)
    {
        if(stop_all_tests()) break;

        test_idx++;
        msg.str("");
        string test_name = m_test_names.at(itest);
        string test_config_file = m_test_configs.at(itest);
        current_test = test_name;

        // add the test directory to the output
        if(!fmg->add_test_dir(test_name))
        {
            log->warn("{0} - Test data already seems to be present in output file",__VTFUNC__);
        }

        // test status
        {
            QUdpSocket socket;
            stringstream s;
            s << test_idx;
            json msg_data = {
                {"TEST_NAME", test_name},
                {"TEST_IDX", s.str()},
                {"TEST_OUTPUT_DIR", fmg->output_directory()},
                {"TEST_OUTPUT_EXT", fmg->file_extension()}
            };
            json status_msg = {
                {"TYPE","START_OF_TEST"},
                {"DATA",msg_data}
            };
            QByteArray data;
            data.append(QString::fromStdString(status_msg.dump()));
            socket.writeDatagram(data, QHostAddress::LocalHost,1236);
        }

        std::ifstream input_file(test_config_file);
        json jtest;
        input_file >> jtest;

        msg << "Moving to next test: " << test_name;
        log->debug("{0} - {1}",__VTFUNC__,msg.str());

        if(m_test.get() != nullptr)
        {
            // just reset it for now -- should take care to close it first
            m_test.reset();
        }
        m_test = std::make_shared<vts::VTSTest>();

        connect(m_test.get(), SIGNAL(broadcast_state_signal(QString)),
                            this, SLOT(update_state(QString)), Qt::DirectConnection);
        connect(m_test.get(), SIGNAL(signal_test_status_update(float)),
                            this, SLOT(test_status_update_slot(float)), Qt::DirectConnection);
        connect(this, SIGNAL(signal_stop_current_test()),
                        m_test.get(), SLOT(stop_current_test()), Qt::DirectConnection);

        /////////////////////////////////////////////////////////////////
        // INITIALIZE
        /////////////////////////////////////////////////////////////////
        bool initialize_ok = m_test->initialize(jtest, m_frontend_cfg, m_daq_cfg);
        initialize_ok &= (m_test->current_state() == vts::VTSTestState::INITIAL);
        if(!initialize_ok)
        {
            // at this point, nothing in the test has started, so we can just exit
            continue;
        }
        // once initialized we can safely provide the FileManager to the Test
        m_test->load_file_manager(fmg);

        bool status = true;
        stringstream msg;
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        msg << "STARTING TEST [" << test_name << "]";
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
                if(stop_all_tests())
                {
                    log->info("{0} - Stopping all tests",__VTFUNC__);
                    break;
                }

                ////////////////////////////////////////////////////////
                // ANALYZE THIS STEPS DATA
                ////////////////////////////////////////////////////////
                status = m_test->analyze();

                ////////////////////////////////////////////////////////
                // IF WE ARE AT THE END FINISH THE TEST LOOP
                ////////////////////////////////////////////////////////
                if((m_test->current_step() == m_test->n_steps()) || stop_all_tests())
                {
                    status = m_test->finalize();
                    if(!stop_all_tests())
                        emit signal_test_status_update(1.0);
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
        if(!stop_all_tests())
            m_test->analyze_test();

        msg.str();
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        if(stop_all_tests())
        {
            msg << "TEST STOPPED [" << test_name << "]";
        }
        else
        {
            msg << "TEST COMPLETE [" << test_name << "]";
        }
        log->info("{0} - {1}",__VTFUNC__,msg.str());
        msg.str("");
        msg << "====================================";
        log->info("{0} - {1}",__VTFUNC__,msg.str());

        VTSTestResult result = VTSTestResult::TESTRESULTINVALID;
        try
        {
            if(!stop_all_tests())
            {
                json jtest_result = m_test->get_results();
                result = StrToVTSTestResult(jtest_result.at("RESULT").get<std::string>());
                test_results.push_back(jtest_result);
            }
            else
            {
                json tmp = {"RESULT",VTSTestResultToStr(VTSTestResult::INCOMPLETE)};
                result = StrToVTSTestResult(tmp.at("RESULT").get<std::string>());
                test_results.push_back(tmp);
            }
        }
        catch(std::exception& e)
        {
            result = VTSTestResult::INCOMPLETE;
            stringstream err;
            err << "Failed to obtain test result for test \"" << test_name << "\", marking it as " << VTSTestResultToStr(result);
            log->warn("{0} - {1}",__VTFUNC__,err.str());
            json tmp = {
                {"RESULT",VTSTestResultToStr(VTSTestResult::INCOMPLETE)}
            };
            test_results.push_back(tmp);
        }
    } // loop over loaded tests


    m_is_running = false;

    // test status
    {
        QUdpSocket socket;
        stringstream s;
        if(stop_all_tests())
        {
            s << "INCOMPLETE";
        }
        else
        {
            s << "COMPLETE";
        }
        stringstream test_stat;
        test_stat << (test_idx+1);
        string n_tests_touched = test_stat.str(); test_stat.str("");
        test_stat << n_total_tests;
        string n_tests_loaded = test_stat.str(); test_stat.str("");
        json msg_data = {
            {"VMM_SERIAL_ID", m_vmm_serial_id},
            {"TEST_OUTPUT_DIR", fmg->output_directory()},
            {"TEST_OUTPUT_EXT", fmg->file_extension()},
            {"TEST_COMPLETION", s.str()},
            {"N_TESTS_TOUCHED", n_tests_touched},
            {"N_TESTS_LOADED", n_tests_loaded},
            {"LAST_TEST_TOUCHED", current_test}
        };
        
        json jr;
        for(size_t i = 0; i < m_test_names.size(); i++)
        {
            if(i>=(test_results.size()))
            {
                log->warn("{0} - More test names than test results encountered",__VTFUNC__);
                json tmp = {"RESULT",VTSTestResultToStr(VTSTestResult::INCOMPLETE)};
                jr[m_test_names.at(i)] = tmp;
            }
            else
            {
                jr[m_test_names.at(i)] = test_results.at(i);
            }
        }
        s.str("");
        json status_msg = {
            {"TYPE","END_OF_TEST"},
            {"DATA",msg_data},
            {"TEST_RESULTS", jr}
        };
        QByteArray data;
        data.append(QString::fromStdString(status_msg.dump()));
        socket.writeDatagram(data, QHostAddress::LocalHost,1236);
    }
    emit tests_finished();
    if(fmg) delete fmg;
}

void VTSTestHandler::stop()
{
    m_stop_all_tests.store(true);
    //emit signal_stop_current_test();
    //m_test->stop();
    //m_test->stop_processing_events();
}

void VTSTestHandler::update_state(QString state_qstring)
{
    string current_state = state_qstring.toStdString();
    stringstream msg;
    msg << "Current test state: " << current_state;
    log->trace("{0} - {1}",__VTFUNC__,msg.str());
}

void VTSTestHandler::test_status_update_slot(float frac)
{
    emit signal_test_status_update(frac);
}


} // namespace vts;
