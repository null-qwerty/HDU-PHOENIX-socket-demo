#ifndef SOCKETSERVER_HPP
#define SOCKETSERVER_HPP

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <thread>
#include <map>
#include <functional>

/**
 * @brief SocketServer 类, 基础的 socket 服务器类
 * 
 */
class SocketServer {
public:
    /**
     * @brief SocketServer 构造函数
     * 
     * @param port 服务器端口
     */
    SocketServer(int port);
    /**
     * @brief 服务器启动函数, bind, listen, accept
     * 
     */
    void start();
    /**
     * @brief 关闭服务器
     * 
     */
    void stop();
    /**
     * @brief 阻塞等待服务器关闭
     * 
     */
    void join();
    /**
     * @brief 发送消息
     * 
     * @param client 客户端 socket
     * @param message 消息内容
     * @param lenth 消息长度
     * @return int < 0 表示发送失败
     */
    int send(int client, const char *message, int lenth);
    /**
     * @brief 广播消息
     * 
     * @param message 消息内容
     * @param lenth 消息长度
     * @return int < 0 表示所有客户端发送失败
     * @note 单一客户端发送失败不会影响其他客户端
     */
    int broadcast(const char *message, int lenth);
    /**
     * @brief 设置接收到消息时的回调函数
     * 
     * @param on_message 回调函数
     */
    void set_on_message(std::function<void(int, const char *)> on_message);
    /**
     * @brief 设置连接成功时的回调函数
     * 
     * @param on_connect 回调函数
     */
    void set_on_connect(std::function<void(int)> on_connect);
    /**
     * @brief 设置断开连接时的回调函数
     * 
     * @param on_disconnect 回调函数
     */
    void set_on_disconnect(std::function<void(int)> on_disconnect);
    /**
     * @brief 主动断开某一客户端连接
     * 
     * @param client 客户端 ID
     */
    void disconnect(int client);
    /**
     * @brief 获取当前连接的客户端列表
     * 
     * @return std::string 客户端列表
     */
    std::string get_clients();

private:
    int port;   ///< 服务器端口
    int server_fd;  ///< 服务器 socket 描述符
    std::map<int, int> clients; ///< 客户端列表
    std::map<int, std::thread::native_handle_type> receive_threads;   ///< 接收线程列表

    std::function<void(int, const char*)> on_message;   ///< 接收到消息时的回调函数
    std::function<void(int)> on_connect;    ///< 连接成功时的回调函数
    std::function<void(int)> on_disconnect; ///< 断开连接时的回调函数

    void accept();  ///< accept 线程处理函数
    void receive(int client);   ///< receive 线程处理函数
};
#endif