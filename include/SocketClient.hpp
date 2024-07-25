#ifndef SOCKETCLIENT_HPP
#define SOCKETCLIENT_HPP

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <functional>


/**
 * @brief SocketClient 类, 基础的 socket 客户端类
 * 
 */
class SocketClient {
public:
    /**
     * @brief SocketClient 构造函数
     * 
     * @param address 服务器地址
     * @param port 服务器端口
     * @note 构造函数中不会自动连接服务器，需要调用 connect 函数
     */
    SocketClient(const char *address, int port);
    /**
     * @brief 连接服务器 
     * 
     */
    void connect();
    /**
     * @brief 主动断开连接
     * 
     */
    void disconnect();
    /**
     * @brief 发送消息
     * 
     * @param message 消息内容
     * @param lenth 消息长度
     * @return int < 0 表示发送失败
     */
    int send(const char *message, int lenth);
    /**
     * @brief 设置接收到消息时的回调函数
     * 
     * @param on_message 回调函数
     */
    void set_on_message(std::function<void(const char *)> on_message);
    /**
     * @brief 设置连接成功时的回调函数
     * 
     * @param on_connect 回调函数
     */
    void set_on_connect(std::function<void()> on_connect);
    /**
     * @brief 设置断开连接时的回调函数
     * 
     * @param on_disconnect 回调函数
     */
    void set_on_disconnect(std::function<void()> on_disconnect);
    /**
     * @brief 阻塞, 死循环接收消息
     * 
     */
    void join();

private:
    const char* address;    ///< 服务器地址
    int port;            ///< 服务器端口
    int client_fd;      ///< socket 描述符
    std::function<void(const char*)> on_message;    ///< 接收到消息时的回调函数
    std::function<void()> on_connect;   ///< 连接成功时的回调函数
    std::function<void()> on_disconnect;    ///< 断开连接时的回调函数

    void receive();   ///< 接收消息线程处理函数
};
#endif