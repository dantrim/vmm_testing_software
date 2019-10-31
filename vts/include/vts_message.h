#ifndef VTS_MESSAGE_H
#define VTS_MESSAGE_H

// std/stl
#include <string>
#include <sstream>
#include <iostream>

// json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace vts
{
    enum VTSMessageType
    {
        SERVER=0
        ,TEST
        ,REPLY
        ,MessageTypeInvalid
        ,NMessageType
    };

    std::string MessageTypeToStr(const VTSMessageType& type)
    {
        std::string out = "MessageTypeInvalid";
        switch(type)
        {
            case VTSMessageType::SERVER : { out = "SERVER"; break; }
            case VTSMessageType::TEST : { out = "TEST"; break; }
            case VTSMessageType::REPLY : { out = "REPLY"; break; }
            case VTSMessageType::MessageTypeInvalid : { out = "MessageTypeInvalid"; break; }
            case VTSMessageType::NMessageType : { out = "NMessageType"; break; }
        } // swtich
        return out;
    }

    VTSMessageType StrToMessageType(const std::string& type)
    {
        VTSMessageType out = VTSMessageType::MessageTypeInvalid;
        if(type == "SERVER") { out = VTSMessageType::SERVER; }
        else if(type == "TEST") { out = VTSMessageType::TEST; }
        else if(type == "REPLY") { out = VTSMessageType::REPLY; }
        else { out = VTSMessageType::MessageTypeInvalid; }
        return out;
    }

    class VTSMessage
    {
        public :
            VTSMessage(int cmd_id, vts::VTSMessageType type,
                        bool expects_reply, json message_data) :
                m_cmd_id(cmd_id),
                m_type(type),
                m_expects_reply(expects_reply),
                m_message_data(message_data)
            {
            }
            VTSMessage(const json& message)
            {
                try
                {
                    m_cmd_id = int(message.at("ID"));
                    m_type = StrToMessageType(message.at("TYPE"));
                    m_expects_reply = bool(message.at("EXPECTS_REPLY"));
                    m_message_data = message.at("DATA");
                }
                catch(std::exception& e)
                {
                    std::stringstream err;
                    err << "VTSMessage failed to load: " << e.what();
                    throw std::runtime_error(err.str());
                }
            }

            int id() { return m_cmd_id; }
            VTSMessageType type() { return m_type; }
            bool expects_reply() { return m_expects_reply; }
            json message_data() { return m_message_data; }
            json message()
            {
                json out = {
                    {"ID", m_cmd_id},
                    {"TYPE", MessageTypeToStr(m_type)},
                    {"EXPECTS_REPLY", m_expects_reply},
                    {"DATA", message_data()}
                };
                return out;
            }
            std::string str()
            {
                std::stringstream msg;
                msg << "ID: " << id() << ", TYPE: " << type()
                    << ", EXPECTS_REPLY: " << expects_reply()
                    << ", DATA: " << message_data();
                return msg.str();
            }

        private :
            int m_cmd_id;
            vts::VTSMessageType m_type;
            bool m_expects_reply;
            json m_message_data;
    }; // class VTSMessage

    class VTSReply
    {
        public :
            VTSReply(int cmd_id, json reply_data) :
                m_cmd_id(cmd_id),
                m_type(VTSMessageType::REPLY),
                m_reply_data(reply_data)
            {
            }
            VTSReply(const json& message)
            {
                try
                {
                    m_cmd_id = int(message.at("ID"));
                    m_type = VTSMessageType::REPLY;
                    m_reply_data = message.at("DATA");
                }
                catch(std::exception& e)
                {
                    std::stringstream err;
                    err << "VTSReply failed to load: " << e.what();
                    throw std::runtime_error(err.str());
                }
            }
            int id() { return m_cmd_id; }
            VTSMessageType type() { return m_type; }
            json message_data() { return m_reply_data; }
            json message()
            {
                json out = {
                    {"ID", m_cmd_id},
                    {"TYPE", MessageTypeToStr(m_type)},
                    {"DATA", message_data()}
                };
                return out;
            }
            std::string str()
            {
                std::stringstream msg;
                msg << "ID: " << id() << ", TYPE: " << type()
                    << ", DATA: " << message_data();
                return msg.str();
            }

        private :
            int m_cmd_id;
            vts::VTSMessageType m_type;
            json m_reply_data;
    }; // class VTSReply

} // namespace vts

#endif
