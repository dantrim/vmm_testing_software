//vts
#include "vts_test_imp.h"
#include "communicator_frontend.h"
#include "filemanager.h"

//std/stl
#include <string>
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
    return m_file_manager->store(obj);
}

} // namespace vts
