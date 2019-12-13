//vts
#include "frontend_struct.h"

//std/stl
#include <iostream>
#include <sstream>
#include <map>
using namespace std;

namespace vts
{

namespace vmm
{

json channel_vec_to_json_config(const vector<Channel>& channels)
{
    uint64_t sc_str = 0x0;
    uint64_t sl_str = 0x0;
    uint64_t sth_str = 0x0;
    uint64_t st_str = 0x0;
    uint64_t sm_str = 0x0;
    uint64_t smx_str = 0x0;

    // sort in ascending order (ch 0 --> ch 63)
    std::map<int, Channel> ch_map;
    //std::sort(channels.begin(), channels.end());
    for(const auto ch : channels)
    {
        ch_map[ch.id] = ch;
        
    } // ch

    json chtrims;

    stringstream key;
    for(uint64_t i = 0; i < 64; i++)
    {
        if(ch_map.find(i) == ch_map.end())
        {
            Channel x;
            ch_map[i] = x;
        }
        int sc_val = ch_map.at(i).sc ? 1 : 0;
        int sl_val = ch_map.at(i).sl ? 1 : 0;
        int sth_val = ch_map.at(i).sth ? 1 : 0;
        int st_val = ch_map.at(i).st ? 1 : 0;
        int sm_val = ch_map.at(i).sm ? 1 : 0;
        int smx_val = ch_map.at(i).smx ? 1 : 0;

        sc_str |= ((uint64_t)sc_val << i);
        sl_str |= ((uint64_t)sl_val << i);
        sth_str |= ((uint64_t)sth_val << i);
        st_str |= ((uint64_t)st_val << i);
        sm_str |= ((uint64_t)sm_val << i);
        smx_str |= ((uint64_t)smx_val << i);

        key.str("");
        key << "channel_";
        if(i<10) key<<"0";
        key << i;
        chtrims[key.str()] = ch_map.at(i).sd;
    }

    stringstream sc;
    sc << "0x" << std::hex << sc_str;
    stringstream sl;
    sl << "0x" << std::hex << sl_str;
    stringstream sth;
    sth << "0x" << std::hex << sth_str;
    stringstream st;
    st << "0x" << std::hex << st_str;
    stringstream sm;
    sm << "0x" << std::hex << sm_str;
    stringstream smx;
    smx << "0x" << std::hex << smx_str;

    json jout;
    json jchreg;
    jchreg["sc"] = sc.str();
    jchreg["sl"] = sl.str();
    jchreg["sth"] = sth.str();
    jchreg["st"] = st.str();
    jchreg["sm"] = sm.str();
    jchreg["smx"] = smx.str();
    jchreg["channel_trims"] = chtrims;

    //cout << "SC  STR = " << sc.str() << endl;
    //cout << "SL  STR = " << sl.str() << endl;
    //cout << "STH STR = " << sth.str() << endl;
    //cout << "ST  STR = " << st.str() << endl;
    //cout << "SM  STR = " << sm.str() << endl;
    //cout << "SMX STR = " << smx.str() << endl;

    jout["channel_registers"] = jchreg;

    return jout;
}

} // namespace vmm

} // namespace vts
