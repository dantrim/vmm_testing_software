//vts
#include "vts_test_imp.h"
#include "communicator_frontend.h"

namespace vts
{

std::shared_ptr<CommunicatorFrontEnd> VTSTestImp::comm()
{
    std::shared_ptr<vts::CommunicatorFrontEnd> comm;
    comm = std::make_shared<vts::CommunicatorFrontEnd>();
    comm->load_config(m_frontend_config);
    return comm;
}



} // namespace vts
