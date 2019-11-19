//vts
#include "vts_decode.h"

//std/stl
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

namespace vts
{
namespace decode
{

///////////////////////////////////////////////////////////////////////
// VMM
///////////////////////////////////////////////////////////////////////
namespace vmm
{

vector<vts::decode::vmm::Sample> decode(std::vector<uint32_t>& datagram)
{
    vector<Sample> out;
    std::vector<uint32_t> data;
    for(const auto& d : datagram)
    {
        data.push_back(vts::decode::endian_swap32()(d));
    }
    //cout << "vts::decode::vmm::decode    Data size: " << data.size() << endl;

    if(data.size()<4)
    {
        stringstream err;
        err << "VMM Sample decoding error: Input data has invalid size (<4)";
        throw std::runtime_error(err.str());
    }

    // first three 32-bit words are ART+header information
    uint32_t art = data.at(0);
    uint32_t header0 = data.at(1);
    uint32_t header1 = data.at(2);

    vts::decode::vmm::Header header;
    cout << "ART data: " << std::hex << (unsigned)art << endl;

    // art info
    header.set_art_trigger( (0xfc00 & art) >> 10 );
    header.set_art_valid_flag( (0x100 & art) >> 8 );
    header.set_art_address( (0x3f & art) );

    // vmm info
    header.set_frame( header0 );
    header.set_trigger_counter( header0 );
    header.set_vmm_id( (header1 & 0xff000000) >> 24 );
    header.set_overflow_bit( (header1 & 0x8000) >> 15 );
    header.set_parity_bit( (header1 & 0x4000) >> 14 );
    header.set_orbit_counter( (header1 & 0x3000) >> 12 );
    header.set_bcid( (header1 & 0xfff) );
    header.set_gray_decoded_bcid( 0x0 ); // implement gray decoding

    // go over the hits
    vts::decode::vmm::Sample sample;
    for(size_t i = 3; i < (data.size()); i+=1)
    {
        sample.clear();
        if(data.at(i) == 0xffffffff) break; // TRANSPORT TRAILER
        sample.set_header(header);

        sample.set_frame( data.at(i) );
        sample.set_parity_bit( (data.at(i) & 0x40000000) >> 30 );
        sample.set_truncation_bit( (data.at(i) & 0x10000000) >> 28 );
        sample.set_channel( (data.at(i) & 0xfc00000) >> 22 );
        sample.set_pdo( (data.at(i) & 0x3ff000) >> 12 );
        sample.set_tdo( (data.at(i) & 0xff0) >> 4 );
        sample.set_is_neighbor( (data.at(i) & 0x8) >> 3 );
        sample.set_rel_bcid( (data.at(i) & 0x7) );
        out.push_back(sample);
    } // i

    return out;
}

} // namespace vmm


} // namespace decode
} // namespace vts
