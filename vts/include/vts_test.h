#ifndef VTS_TEST_H
#define VTS_TEST_H

// std/stl
#include <memory>
#include <map>
#include <atomic>

// vts
namespace vts {
    class VTSTestImp;
    class FileManager;
    namespace daq {
        class DataFragment;
    }
}
#include "vts_test_states.h"
#include "daq_handler.h"

// Qt
#include <QObject>
#include <QUdpSocket>

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//logging
namespace spdlog {
    class logger;
}

namespace vts
{

class VTSTest : public QObject
{
    Q_OBJECT

    public :
        explicit VTSTest(QObject* parent =  0);
        ~VTSTest();
        void load_file_manager(vts::FileManager* mgr);

        bool initialize(const json& test_config, const json& frontend_cfg, const json& daq_cfg, const json& config_dirs);
        bool load();
        bool configure();
        bool run();
        bool process_event(vts::daq::DataFragment* fragment);
        bool analyze();
        bool analyze_test();
        bool finalize();
        void stop();
        json get_results();

        int current_step();
        int n_steps();
        vts::VTSTestState current_state();

        bool start_processing_events();
        bool stop_processing_events();
        bool continue_processing();

    private :
        std::shared_ptr<spdlog::logger> log;
        std::shared_ptr<vts::VTSTestImp> m_imp;
        void initialize_fsm();
        void update_fsm(vts::VTSTestState s);
        vts::VTSTestState m_fsm_state;
        void broadcast_state();
        std::string current_state_name();
        std::string transition_string(vts::VTSTestState current, vts::VTSTestState next);
        std::shared_ptr<vts::daq::DaqHandler> m_daq_handler;
        QUdpSocket m_status_socket;


    signals :
        void broadcast_state_signal(QString);
        void test_completed();
        void test_ready();
        void begin_test();
        void revert();
        void test_done();
        void signal_test_status_update(float);
        void signal_stop_current_test();

    public slots :
        void state_updated_slot();
        void test_finished_slot();
        void test_status_update_slot(float);
        void stop_current_test();
        

}; // class VTSTest

} // namespace vts

#endif
