#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <thread>

#include <opencv2/opencv.hpp>

#include "Message.hpp"

#include <PHOENIX/Utils/Info/Info.hpp>

/**
 * @brief Application 类, 用于解析应用层消息, 并提供发送消息的接口
 * 
 * @tparam T socket 类型, 可以是 SocketServer 或者 SocketClient
 */
template <typename T> class Application {
public:
    /**
     * @brief Application 构造函数
     * 
     * @param socket SocketServer 或者 SocketClient 的智能指针
     */
    Application(std::shared_ptr<T> &socket);
    /**
     * @brief Application 析构函数
     * 
     */
    ~Application() = default;
    /**
     * @brief 添加命令
     * 
     * @param command 命令名称
     * @param callback 回调函数
     */
    void add_command(std::string command,
                     std::function<void(std::string)> callback);
    /**
     * @brief 编码并发送消息，编码按照 Message 类的格式
     * 
     * @param type 消息类型
     * @param dataID 消息 ID
     * @param data 数据区
     * @param total_lenth 数据总长度, 如果数据长度超过 10218 字节，将会分包发送 
     * @param sendto 发送目标, -1 为广播
     * @note SocketClient 调用时，sendto 参数无效
     * @return int < 0 表示发送失败
     */
    int encode_and_send(unsigned short type, unsigned int dataID,
                        unsigned char *data, unsigned int total_lenth,
                        int sendto);
    /**
     * @brief 编码并发送图片，编码按照 Message 类的格式
     * 
     * @param dataID 消息 ID
     * @param img cv::Mat 图片
     * @param sendto 发送目标, -1 为广播
     * @note 图片将会被编码为 jpg 格式后发送
     * @note SocketClient 调用时，sendto 参数无效
     */
    void encode_and_send(unsigned int dataID, cv::Mat img, int sendto);
    /**
     * @brief 主动断开某一客户端连接
     * 
     * @param client 客户端 ID
     */
    void disconnect(int client);
    /**
     * @brief 接收并解码消息
     * 
     * @param message 接收到的消息
     * @return unsigned char* nullptr 表示消息包错误或者未接收完整，否则返回数据区指针
     */
    unsigned char *receive_and_decode(Message &message);
    /**
     * @brief 阻塞主线程，等待命令输入
     * 
     */
    void join();
    /**
     * @brief 获取当前连接的客户端列表
     * 
     * @return std::string 客户端列表
     */
    std::string get_clients();

private:
    std::map<std::string, std::function<void(std::string)>>
        commands; ///< 命令列表
    std::map<unsigned int, unsigned char *> data_temp; ///< 数据缓存
    std::map<unsigned int, unsigned int> received_lenth; ///< 已接收数据长度
    std::shared_ptr<T> socket; ///< SocketServer 或者 SocketClient 的智能指针
};

template <typename T> Application<T>::Application(std::shared_ptr<T> &socket)
{
    this->socket = socket;
}

template <typename T>
void Application<T>::add_command(std::string command,
                                 std::function<void(std::string)> callback)
{
    this->commands[command] = callback;
}

template <typename T>
int Application<T>::encode_and_send(unsigned short type, unsigned int dataID,
                                    unsigned char *data,
                                    unsigned int total_lenth, int sendto)
{
    return 0;
}

template <typename T> void Application<T>::disconnect(int client)
{
}

template <typename T> std::string Application<T>::get_clients()
{
    return "";
}

#ifdef SOCKETSERVER_HPP
template <>
int Application<SocketServer>::encode_and_send(unsigned short type,
                                               unsigned int dataID,
                                               unsigned char *data,
                                               unsigned int total_lenth,
                                               int sendto)
{
    Message message(type);
    message.set_dataID(dataID);
    message.set_dataTotalLenth(total_lenth);

    int ret = 0;

    if (total_lenth < 10218) {
        message.set_data((char *)data, total_lenth);
        if (sendto == -1)
            ret = socket->broadcast((char *)(message.get_buffer()), 10240);
        else
            ret = socket->send(sendto, (char *)(message.get_buffer()), 10240);
    } else {
        unsigned int offset = 0;
        while (offset < total_lenth) {
            message.set_offset(offset);
            unsigned int lenth =
                total_lenth - offset > 10218 ? 10218 : total_lenth - offset;
            message.set_data((char *)(data + offset), lenth);
            if (sendto == -1)
                ret = socket->broadcast((char *)(message.get_buffer()), 10240);
            else
                ret =
                    socket->send(sendto, (char *)(message.get_buffer()), 10240);
            offset += lenth;
            if (ret < 0)
                return ret;
        }
    }

    return ret;
}

template <> void Application<SocketServer>::disconnect(int client)
{
    socket->disconnect(client);
    return;
}

template <> std::string Application<SocketServer>::get_clients()
{
    return socket->get_clients();
}

#endif

#ifdef SOCKETCLIENT_HPP
template <>
int Application<SocketClient>::encode_and_send(unsigned short type,
                                               unsigned int dataID,
                                               unsigned char *data,
                                               unsigned int total_lenth,
                                               int sendto)
{
    Message message(type);
    message.set_dataID(dataID);
    message.set_dataTotalLenth(total_lenth);

    int ret = 0;

    if (total_lenth < 10218) {
        message.set_data((char *)data, total_lenth);
        ret = socket->send((char *)(message.get_buffer()), 10240);
    } else {
        unsigned int offset = 0;
        while (offset < total_lenth) {
            message.set_offset(offset);
            unsigned int lenth =
                total_lenth - offset > 10218 ? 10218 : total_lenth - offset;
            message.set_data((char *)(data + offset), lenth);

            ret = socket->send((char *)(message.get_buffer()), 10240);
            offset += lenth;
            if (ret < 0)
                return ret;
        }
    }

    return ret;
}
#endif

template <typename T>
void Application<T>::encode_and_send(unsigned int dataID, cv::Mat img,
                                     int sendto)
{
    std::vector<unsigned char> data;
    cv::imencode(".jpg", img, data);

    encode_and_send(Message::MessageType::IMAGE_MSG, dataID, data.data(),
                    data.size(), sendto);
}

template <typename T>
unsigned char *Application<T>::receive_and_decode(Message &message)
{
    if (((Message::MessageBuffer *)(message.get_buffer()))->Start != 0x0D00 ||
        ((Message::MessageBuffer *)(message.get_buffer()))->End != 0x0721) {
        std::string hex;
        std::stringstream ss;

        ss << std::hex << "0x"
           << ((Message::MessageBuffer *)(message.get_buffer()))->Start
           << std::endl
           << "0x"
           << ((Message::MessageBuffer *)(message.get_buffer()))->MessageType
           << std::endl
           << "0x" << ((Message::MessageBuffer *)(message.get_buffer()))->End
           << std::endl
           << std::dec;

        WARNING("Warning: Invalid message. Meta data bellow:");
        ss >> hex;
        WARNING("Start: " + hex);
        ss >> hex;
        WARNING("MessageType: " + hex);
        WARNING(
            "DataID: " +
            std::to_string(
                ((Message::MessageBuffer *)(message.get_buffer()))->DataID));
        WARNING(
            "DataTotalLenth: " +
            std::to_string(((Message::MessageBuffer *)(message.get_buffer()))
                               ->DataTotalLenth));
        WARNING(
            "Offset: " +
            std::to_string(
                ((Message::MessageBuffer *)(message.get_buffer()))->Offset));
        WARNING(
            "DataLenth: " +
            std::to_string(
                ((Message::MessageBuffer *)(message.get_buffer()))->DataLenth));
        ss >> hex;
        WARNING("End: " + hex + "\n");
        return nullptr;
    }
    unsigned int offset = message.get_offset();
    unsigned int lenth = message.get_dataLenth();
    unsigned int total_lenth = message.get_dataTotalLenth();
    unsigned int dataID = message.get_dataID();

    if (total_lenth == 0) {
        ERROR("Error: Received message's total_lenth is 0, drop it.");
        return nullptr;
    }

    if (data_temp.find(dataID) == data_temp.end()) {
        data_temp[dataID] = new unsigned char[total_lenth];
        received_lenth[dataID] = 0;
    }
    memcpy(data_temp[dataID] + offset, message.get_data(), lenth);
    received_lenth[dataID] += lenth;
    if (received_lenth[dataID] >= total_lenth) {
        unsigned char *data = data_temp[dataID];
        data_temp.erase(dataID);
        received_lenth.erase(dataID);
        return data;
    } else {
        return nullptr;
    }
}

template <typename T> void Application<T>::join()
{
    while (true) {
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd == "exit")
            break;
        std::istringstream iss(cmd);
        std::string command;

        iss >> command;

        if (commands.find(command) != commands.end()) {
            std::string args;
            std::getline(iss, args);
            std::thread(commands[command], args).detach();
        } else {
            // std::cout << "Command not found.(press any key to continue)" << std::endl;
            ERROR("Command not found.");
        }
    }
}
