//vts
#include "vts_test_imp.h"
#include "communicator_frontend.h"
#include "filemanager.h"

//std/stl
#include <string>
#include <iostream>
using namespace std;

namespace vts
{

std::shared_ptr<CommunicatorFrontEnd> VTSTestImp::comm()
{
    std::shared_ptr<vts::CommunicatorFrontEnd> comm;
    comm = std::make_shared<vts::CommunicatorFrontEnd>();
    comm->load_config(m_frontend_config);
    return comm;
}

bool VTSTestImp::store(TObject* obj)
{
    if(m_file_manager)
        return m_file_manager->store(obj);
    return false;
}

void VTSTestImp::stop_current_test()
{
    m_processing_flag.store(false);
    cout << "VTSTestImp::stop_current_test  BLAH" << endl;
    emit signal_status_update(event_fraction_processed());
    m_events_processed.store(2.0 * n_events_for_test());
}

} // namespace vts
