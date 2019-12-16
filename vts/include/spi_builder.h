#ifndef VTS_SPI_BUILDER_H
#define VTS_SPI_BUILDER_H

/////////////////////////////////////////////////////
//
// spi_builder.h
//
// functions to build the VMM SPI string
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//std/stl
#include <string>
#include <vector>
#include "stdint.h"

//Qt
#include "QString"


namespace vts
{
    namespace spi
    {
        std::vector<std::string> spi_string_vec(json vmm_spi);
        std::vector<std::string> spi_global_register_vec(json global_reg, bool reset = false);
        std::vector<std::string> spi_channel_register_vec(const json& channel_reg);
        int ena_dis(std::string val, bool inverted = false);
        int pos_neg(std::string val, bool inverted = false);
        QString reverseString(QString string);
    } // namespace spi
} // namespace vts

#endif
