#ifndef VTS_MESSAGE_H
#define VTS_MESSAGE_H

/////////////////////////////////////////////////////
//
// vts_message.h
//
// enum to hold the types of commands that the VTS
// server will handle
//
// daniel joseph antrim
// october 2019
// daniel.joseph.antrim@cern.ch
//
/////////////////////////////////////////////////////

// std/stl
#include <string>
#include <sstream>
#include <iostream>

// json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

// Qt
#include <QByteArray>

namespace vts
{
    enum VTSMessageType
    {
        SERVER=0
        ,VMMTEST
        ,REPLY
        ,FRONTEND
        ,MessageTypeInvalid
        ,NMessageType
    };

    std::string MessageTypeToStr(const VTSMessageType& type)
    {
        std::string out = "MessageTypeInvalid";
        switch(type)
        {
            case VTSMessageType::SERVER : { out = "SERVER"; break; }
            case VTSMessageType::VMMTEST : { out = "VMMTEST"; break; }
            case VTSMessageType::REPLY : { out = "REPLY"; break; }
            case VTSMessageType::FRONTEND : { out = "FRONTEND"; break; }
            case VTSMessageType::MessageTypeInvalid : { out = "MessageTypeInvalid"; break; }
            case VTSMessageType::NMessageType : { out = "NMessageType"; break; }
        } // swtich
        return out;
    }

    VTSMessageType StrToMessageType(const std::string& type)
    {
        VTSMessageType out = VTSMessageType::MessageTypeInvalid;
        if(type == "SERVER") { out = VTSMessageType::SERVER; }
        else if(type == "VMMTEST") { out = VTSMessageType::VMMTEST; }
        else if(type == "REPLY") { out = VTSMessageType::REPLY; }
        else if(type == "FRONTEND") { out = VTSMessageType::FRONTEND; }
        else { out = VTSMessageType::MessageTypeInvalid; }
        return out;
    }

    class VTSMessage
    {
        public :
            VTSMessage() :
                m_cmd_id(-1),
                m_type(VTSMessageType::MessageTypeInvalid),
                m_expects_reply(false),
                m_message_data({})
            {
            }
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

            QByteArray byte_array()
            {
                QByteArray block;
                block.append(message().dump().c_str());
                return block;
            }

            int id() const { return m_cmd_id; }
            void id(int id) { m_cmd_id = id; }

            VTSMessageType type() const { return m_type; }
            void type(VTSMessageType type) { m_type = type; }

            bool expects_reply() const { return m_expects_reply; }
            void expects_reply(bool does_it) { m_expects_reply = does_it; }

            json data() const { return m_message_data; }
            void data(json data) { m_message_data = data; }

            json message() const
            {
                json out = {
                    {"ID", m_cmd_id},
                    {"TYPE", MessageTypeToStr(m_type)},
                    {"EXPECTS_REPLY", m_expects_reply},
                    {"DATA", data()}
                };
                return out;
            }
            std::string str() const
            {
                std::stringstream msg;
                msg << "ID: " << id() << ", TYPE: " << MessageTypeToStr(type())
                    << ", EXPECTS_REPLY: " << expects_reply()
                    << ", DATA: " << data();
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
            VTSReply() :
                m_cmd_id(-1),
                m_type(VTSMessageType::REPLY),
                m_reply_data({})
            {
            }

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

            QByteArray byte_array()
            {
                QByteArray block;
                block.append(message().dump().c_str());
                return block;
            }

            int id() const { return m_cmd_id; }
            void id(int id) { m_cmd_id = id; }

            VTSMessageType type() const { return m_type; }
            void type(VTSMessageType type) { m_type = type; }

            json data() const { return m_reply_data; }
            void data(json data) { m_reply_data = data; }

            json message() const
            {
                json out = {
                    {"ID", m_cmd_id},
                    {"TYPE", MessageTypeToStr(m_type)},
                    {"DATA", data()}
                };
                return out;
            }
            std::string str() const
            {
                std::stringstream msg;
                msg << "ID: " << id() << ", TYPE: " << MessageTypeToStr(type())
                    << ", DATA: " << data();
                return msg.str();
            }

        private :
            int m_cmd_id;
            vts::VTSMessageType m_type;
            json m_reply_data;
    }; // class VTSReply

} // namespace vts

#endif
