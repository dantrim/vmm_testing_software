#ifndef VTS_HELPERS_H
#define VTS_HELPERS_H

//std/stl
#include <string>
#include <cstdint>

std::string computeMethodName(const std::string& function, const std::string& prettyFunction);
#define __VTFUNC__ computeMethodName(__FUNCTION__,__PRETTY_FUNCTION__).c_str()
std::string int_ip_to_string(uint32_t ip);
uint32_t string_ip_to_int(std::string ip);

#endif
