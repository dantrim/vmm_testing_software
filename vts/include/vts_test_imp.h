#ifndef VTS_TEST_IMP_H
#define VTS_TEST_IMP_H

//vts
#include "vts_result.h"
namespace vts {
    namespace daq {
        class DataFragment;
    }
}

//Qt
#include <QObject>

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//logging
namespace spdlog {
    class logger;
}

//std/stl
#include <atomic>

namespace vts
{

class VTSTestImp : public QObject
{
    Q_OBJECT

    public :

        void load_test_config(const json& test_config, const json& frontend_config)
        {
            m_test_config = test_config;
            m_frontend_config = frontend_config;
        }

        virtual bool initialize(const json& config) = 0;
        virtual bool load() = 0;
        virtual bool configure() = 0;
        virtual bool run() = 0;
        virtual bool process_event(vts::daq::DataFragment* fragment) = 0;
        virtual bool analyze() = 0;
        virtual bool analyze_test() = 0;
        virtual bool finalize() = 0;

        virtual json get_results() = 0;

        int get_current_state()
        {
            return m_current_state;
        }
        int get_n_states()
        {
            return m_n_states;
        }

        void processing(bool is_or_is_not)
        {
            m_processing_flag.store(is_or_is_not);
        }

        bool processing_events()
        {
            return m_processing_flag.load(std::memory_order_acquire);
        }

    protected :
        std::shared_ptr<spdlog::logger> log;

        json m_test_config;
        json m_frontend_config;


        int m_current_state;
        int m_n_states;
        long int m_events_processed;
        long int m_events_per_step;

        std::atomic<bool> m_processing_flag;
        

        void set_current_state(int s)
        {
            m_current_state = s;
        }
        void set_n_states(int s)
        {
            m_n_states = s;
        }

        void set_events_per_step(long int e)
        {
            m_events_per_step = e;
        }
        long int& events_per_step()
        {
            return m_events_per_step;
        }

    signals :
        void finished();
        void signal_current_state(int, int);

}; // class VTSTestImp

} // namespace vts

#endif
