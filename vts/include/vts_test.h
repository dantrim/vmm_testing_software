#ifndef VTS_TEST_H
#define VTS_TEST_H

// std/stl
#include <memory>
#include <map>

// vts
namespace vts {
    class VTSTestImp;
}
#include "vts_test_states.h"

// Qt
#include <QObject>

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

        bool initialize(const json& test_config);
        bool load();
        bool configure();
        bool run();
        bool analyze();
        bool analyze_test();
        bool finalize();
        //void start();
        void stop();

        int current_step();
        int n_steps();
        vts::VTSTestState current_state();


    private :
        std::shared_ptr<spdlog::logger> log;
        std::shared_ptr<vts::VTSTestImp> m_imp;
        void initialize_fsm();
        void update_fsm(vts::VTSTestState s);
        vts::VTSTestState m_fsm_state;
        void broadcast_state();
        std::string current_state_name();
        std::string transition_string(vts::VTSTestState current, vts::VTSTestState next);

    signals :
        void broadcast_state_signal(QString);
        void test_completed();
        void test_ready();
        void begin_test();
        void revert();
        void test_done();

    public slots :
        void state_updated_slot();
        void test_finished_slot();
        

}; // class VTSTest

} // namespace vts

#endif
