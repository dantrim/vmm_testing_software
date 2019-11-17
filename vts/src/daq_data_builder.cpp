//vts
#include "helpers.h"
#include "daq_data_builder.h"

//logging
#include "spdlog/spdlog.h"

//std/stl
#include <iostream>
#include <chrono>
using namespace std;

namespace vts
{
namespace daq
{

DataBuilder::DataBuilder(DataQueue* input_queue,
            vts::VTSTest* test,
            std::atomic<bool> & build_flag) :
    m_active(false)
{
    log = spdlog::get("vts_logger");
    m_in_queue = input_queue;
    m_build_flag = & build_flag;
    m_test = test;
}

bool DataBuilder::active()
{
    return m_active;
}

void DataBuilder::start()
{
    m_thread = std::thread(
        [this] () { build(); }
    );
    m_active = true;
}

bool DataBuilder::continue_building()
{
    return m_build_flag->load(std::memory_order_acquire);
}

void DataBuilder::build()
{
    bool continue_processing = true;
    m_test->start_processing_events();
    while(continue_processing && continue_building())
    {
        DataFragment* fragment = nullptr;
        if(!m_in_queue->try_dequeue(fragment))
        {
            continue;
        }

        continue_processing = m_test->process_event(fragment);
        delete fragment;
    } // while
    log->info("{0} - Build is setting processing flag to FALSE",__VTFUNC__);
    m_test->stop_processing_events();
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void DataBuilder::stop()
{
    m_build_flag->store(false);
    if(!m_active) return;
    if(m_thread.joinable())
    {
        m_thread.join();
    }
    m_active = false;
}

} // namespace daq
} // namespace vts
