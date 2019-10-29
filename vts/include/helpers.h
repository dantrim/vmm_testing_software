#ifndef VTS_HELPERS_H
#define VTS_HELPERS_H

//std/stl
#include <string>

std::string computeMethodName(const std::string& function, const std::string& prettyFunction);
#define __VTFUNC__ computeMethodName(__FUNCTION__,__PRETTY_FUNCTION__).c_str()

#endif
