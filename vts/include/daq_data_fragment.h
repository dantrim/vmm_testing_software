#ifndef VTS_DATA_FRAGMENT_H
#define VTS_DATA_FRAGMENT_H

/////////////////////////////////////////////////////
//
// daq_data_fragment.h
//
// structure to hold incoming data packets from
// the frontend
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////

//std/stl
#include <vector>
#include <cstdint>

namespace vts
{

namespace daq
{

    struct VMMHeader
    {
        unsigned int level1id : 16;
        unsigned int bcid : 12;
        unsigned int orbit : 2;
        unsigned int reserved : 1;
        unsigned int timeset : 1;
    };

    class DataFragment
    {
        public :
            DataFragment()
            {
                clear();
            };

            void clear()
            {
                m_level1id = 0x0;
                m_ip_recv = 0x0;
                m_port_recv = 0x0;
                packet.clear();
            }

            std::vector<uint32_t> packet;
            void set_level1id(uint32_t t) { m_level1id = t; }
            void set_ip_recv(uint32_t i) { m_ip_recv = i; }
            void set_port_recv(uint32_t p) { m_port_recv = p; }

        private :
            uint32_t m_level1id;
            uint32_t m_ip_recv;
            uint32_t m_port_recv;

    }; // class DataFragment

} // namespace daq

} // namespace vts


#endif
