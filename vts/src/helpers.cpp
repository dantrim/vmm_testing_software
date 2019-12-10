#include "helpers.h"

//std/stl
#include <string>
#include <sstream>

std::string computeMethodName(const std::string& function, const std::string& prettyFunction) {
    size_t locFunName = prettyFunction.find(function); //If the input is a constructor, it gets the beginning of the class name, not of the method. That's why later on we have to search for the first parenthesys
    size_t begin = prettyFunction.rfind(" ",locFunName) + 1;
    size_t end = prettyFunction.find("(",locFunName + function.length()); //Adding function.length() make this faster and also allows to handle operator parenthesys!
    if (prettyFunction[end + 1] == ')')
        return (prettyFunction.substr(begin,end - begin));// + "()");
    else
        return (prettyFunction.substr(begin,end - begin));// + "(...)");
}

std::string int_ip_to_string(uint32_t ip)
{
    uint32_t ip_arr[4];
    ip_arr[0] = ip & 0xff;
    ip_arr[1] = (ip>>8) & 0xff;
    ip_arr[2] = (ip>>16) & 0xff;
    ip_arr[3] = (ip>>24) & 0xff;
    std::stringstream ip_str;
    ip_str << ip_arr[3] << "." << ip_arr[2] << "." << ip_arr[1] << "." << ip_arr[0];
    return ip_str.str();
}

uint32_t string_ip_to_int(std::string ip)
{
    std::stringstream s(ip);
    uint32_t a, b, c, d;
    char ch;
    s >> a >> ch >> b >> ch >> c >> ch >> d;
    uint32_t ip_out = 0x0;
    ip_out |= (a << 24);
    ip_out |= (b << 16);
    ip_out |= (c << 8);
    ip_out |= d;
    return ip_out;
}
