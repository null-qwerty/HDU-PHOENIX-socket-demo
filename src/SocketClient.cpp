#include <thread>

#include "SocketClient.hpp"
#include "Info.hpp"

SocketClient::SocketClient(const char *address, int port)
{
    this->address = address;
    this->port = port;
}

void SocketClient::connect()
{
    this->client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->client_fd == -1) {
        ERROR("Failed to create socket.");
        exit(1);
    }
    // 设置端口复用
    int opt = 1;
    if (setsockopt(this->client_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        ERROR("Failed to set socket options.");
        exit(1);
    }
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);
    inet_pton(AF_INET, this->address, &server_address.sin_addr);

    if (::connect(this->client_fd, (struct sockaddr *)&server_address,
                  sizeof(server_address)) == -1) {
        ERROR("Failed to connect to server.");
        exit(1);
    }
    // 创建并分离接收线程
    std::thread receive_thread(&SocketClient::receive, this);
    receive_thread.detach();

    if (this->on_connect != nullptr) {  // 连接成功处理函数
        this->on_connect();
    }
}

void SocketClient::disconnect()
{
    // 发送断开连接消息，用于服务器端清理资源
    //* 已弃用
    send("/disconnect", sizeof("/disconnect"));
    close(this->client_fd); // 关闭连接

    if (this->on_disconnect != nullptr) {   // 断开连接处理函数
        this->on_disconnect();
    }
}

int SocketClient::send(const char *message, int lenth)
{
    return write(this->client_fd, message, lenth);
}

void SocketClient::set_on_message(std::function<void(const char *)> on_message)
{
    this->on_message = on_message;
}

void SocketClient::set_on_connect(std::function<void()> on_connect)
{
    this->on_connect = on_connect;
}

void SocketClient::set_on_disconnect(std::function<void()> on_disconnect)
{
    this->on_disconnect = on_disconnect;
}

void SocketClient::receive()
{
    char buffer[10240] = { 0 };
    while (true) {
        int lenth = read(this->client_fd, buffer, 10240);
        
        if (lenth <= 0) {
            WARNING("Server closed.");
            this->disconnect();
            break;
        }

        while(lenth < 10240){   // 读取完整应用层消息
            int valread = read(this->client_fd, buffer + lenth, 10240 - lenth);
            if(valread <= 0){
                break;
            }
            lenth += valread;
        }

        if (this->on_message != nullptr) {  // 消息处理函数
            this->on_message(buffer);
        }
    }
}

void SocketClient::join()
{
    while (true) {
    }
}