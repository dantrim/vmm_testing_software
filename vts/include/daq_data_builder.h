#ifndef VTS_DAQ_BUILDER_H
#define VTS_DAQ_BUILDER_H

//vts
#include "daq_defs.h"
#include "vts_test.h"

//std/stl
#include <atomic>
#include <thread>

namespace spdlog {
    class logger;
}

namespace vts
{
namespace daq
{

    class DataBuilder
    {
        public :
            DataBuilder(DataQueue* input_queue,
                vts::VTSTest* test,
                std::atomic<bool>& build_flag);
            virtual ~DataBuilder(){};

            void start();
            bool active();
            void build();
            void stop();
            bool continue_building();

        private :
            std::shared_ptr<spdlog::logger> log;
            bool m_active;
            std::atomic<bool>* m_build_flag;
            vts::VTSTest* m_test;

            DataQueue* m_in_queue;
            std::thread m_thread;

    }; // class DataBuilder

} // namespace daq
} // namespace vts

#endif
