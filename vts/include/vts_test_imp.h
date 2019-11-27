#ifndef VTS_TEST_IMP_H
#define VTS_TEST_IMP_H

//vts
#include "vts_result.h"
namespace vts {
    class CommunicatorFrontEnd;
    class FileManager;
    namespace daq {
        class DataFragment;
    }
}

//ROOT
#include "TROOT.h"
class TTree;

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
#include <memory>

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
        void load_file_manager(vts::FileManager* mgr) { m_file_manager = mgr; }
        bool store(TObject* obj);

        std::shared_ptr<vts::CommunicatorFrontEnd> comm();

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

        vts::FileManager* m_file_manager;

        json m_test_config;
        json m_frontend_config;

        json m_test_data;
        json m_base_fpga_config;
        json m_base_vmm_config;

        virtual void check_status();
        virtual void reset_vmm();

        int m_current_state;
        int m_n_states;
        std::atomic<long int> m_events_processed;
        std::atomic<long int> m_events_per_step;

        std::atomic<long int> m_total_events_processed;
        std::atomic<long int> m_n_total_events;

        std::atomic<bool> m_processing_flag;

        void reset_event_count()
        {
            m_events_processed.store(0);
        }

        void event_processed()
        {
            m_events_processed++;
            m_total_events_processed++;
        }

        unsigned int n_events_processed()
        {
            return m_events_processed.load(std::memory_order_acquire);
        }

        unsigned int n_total_events_processed()
        {
            return m_total_events_processed.load(std::memory_order_acquire);
        }

        void set_current_state(int s)
        {
            m_current_state = s;
        }
        void set_n_states(int s)
        {
            m_n_states = s;
        }

        void set_n_events_per_step(long int e)
        {
            m_events_per_step.store(e);
        }

        void set_n_events_for_test(unsigned int e)
        {
            m_n_total_events.store(e);
        }
        unsigned int n_events_for_test()
        {
            return m_n_total_events.load(std::memory_order_acquire);
        }

        unsigned int n_events_per_step()
        {
            return m_events_per_step.load(std::memory_order_acquire);
        }

        float event_fraction_processed()
        {
            float num = n_total_events_processed() * 1.0;
            float den = n_events_for_test() * 1.0;
            return (num / den);
        }

    signals :
        void finished();
        void signal_current_state(int, int);
        void signal_status_update(float);

    public slots :
        void stop_current_test();

}; // class VTSTestImp
} // namespace vts

#endif
