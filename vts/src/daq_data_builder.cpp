//vts
#include "helpers.h"
#include "daq_data_builder.h"

//logging
#include "spdlog/spdlog.h"

//std/stl
#include <iostream>
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
    //while(continue_building())
    while(m_test->continue_processing())
    {
        DataFragment* fragment = nullptr;
        if(!m_in_queue->try_dequeue(fragment))
        {
            continue;
        }

        m_test->process_event(fragment);
    } // while
}

void DataBuilder::stop()
{
    if(!m_active) return;
    if(m_thread.joinable())
    {
        m_thread.join();
    }
    m_active = false;
}

} // namespace daq
} // namespace vts
