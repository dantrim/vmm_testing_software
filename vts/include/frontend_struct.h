#ifndef VTS_FRONTEND_STRUCT_H
#define VTS_FRONTEND_STRUCT_H

//std/stl
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace vts
{
namespace vmm
{
    struct Channel
    {
        Channel() :
            id(0),
            sc(false),
            sl(false),
            sth(false),
            st(false),
            sm(false),
            sd(0x0),
            smx(false)
        {}
        Channel(uint8_t id, bool sc, bool sl, bool sth,
                bool st, bool sm, uint8_t sd, bool smx) :
            id(id),
            sc(sc),
            sl(sl),
            sth(sth),
            st(st),
            sm(sm),
            sd(sd),
            smx(smx)
        {}
        void load(const json& jch)
        {
            id = (uint8_t)jch.at("id").get<int>();
            sc = (bool)jch.at("sc").get<int>();
            sl = (bool)jch.at("sl").get<int>();
            sth = (bool)jch.at("sth").get<int>();
            st = (bool)jch.at("st").get<int>();
            sm = (bool)jch.at("sm").get<int>();
            smx = (bool)jch.at("smx").get<int>();
            sd = (uint8_t)jch.at("sd").get<int>();
        }

        uint32_t id;
        bool sc;
        bool sl;
        bool sth;
        bool st;
        bool sm;
        uint8_t sd; // trim
        bool smx;

        // in order to sort a vector of vmm channel objects
        bool operator<(const Channel& a) const
        {
            return id < a.id;
        }
    }; // struct Channel

    // from a set of channels, construc the JSON for the configuration (needed
    // as input to building the SPI, for example)
    json channel_vec_to_json_config(const std::vector<Channel>& channels);

} // namespace vmm

} // namespace vts

#endif
