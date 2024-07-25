#include "Message.hpp"

#include <cstring>
#include <cstdlib>

Message::Message(){
    message.Start = 0x0D00;
    message.MessageType = MessageType::STRING_MSG;
    message.DataID = 0;
    message.DataTotalLenth = 0;
    message.Offset = 0;
    message.DataLenth = 0;
    message.End = 0x0721;
    memset(message.Data, 0, sizeof(message.Data));
}

Message::Message(const unsigned short type){
    message.Start = 0x0D00;
    message.MessageType = type;
    message.DataID = 0;
    message.DataTotalLenth = 0;
    message.Offset = 0;
    message.DataLenth = 0;
    message.End = 0x0721;
    memset(message.Data, 0, sizeof(message.Data));
}

Message::Message(const Message &message){
    this->message = message.message;
}

Message::Message(const unsigned char* buffer)
{
    memcpy(&message, buffer, sizeof(message));
}
Message::Message(const char* buffer)
{
    memcpy(&message, buffer, sizeof(message));
}

void Message::set_data(const char *data, const unsigned int length){
    message.DataLenth = length;
    memcpy(message.Data, data, length);
}

void Message::set_messageType(const unsigned short type){
    message.MessageType = type;
}

void Message::set_dataID(const unsigned int id){
    message.DataID = id;
}

void Message::set_dataTotalLenth(const unsigned int length){
    message.DataTotalLenth = length;
}

void Message::set_offset(const unsigned int offset){
    message.Offset = offset;
}

unsigned short Message::get_messageType() const{
    return message.MessageType;
}

unsigned int Message::get_dataID() const{
    return message.DataID;
}

unsigned int Message::get_dataTotalLenth() const{
    return message.DataTotalLenth;
}

unsigned int Message::get_offset() const{
    return message.Offset;
}

const unsigned char *Message::get_data() const{
    return message.Data;
}

unsigned int Message::get_dataLenth() const{
    return message.DataLenth;
}

const unsigned char* Message::get_buffer() const{
    return (unsigned char*)&message;
}