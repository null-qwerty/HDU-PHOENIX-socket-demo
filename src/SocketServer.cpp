#include "SocketServer.hpp"

#include <PHOENIX/Utils/Info/Info.hpp>

SocketServer::SocketServer(int port)
{
    this->port = port;
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_fd == -1) {
        ERROR("Failed to create socket");
        exit(1);
    }
    // 设置端口复用
    int opt = 1;
    if (setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        ERROR("Failed to set socket options");
        exit(1);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(this->server_fd, (struct sockaddr *)&address, sizeof(address)) ==
        -1) {
        ERROR("Failed to bind socket");
        exit(1);
    }

    if (listen(this->server_fd, 30) == -1) {
        ERROR("Failed to listen on socket");
        exit(1);
    }
}

void SocketServer::start()
{
    std::thread accept_thread(&SocketServer::accept, this);
    accept_thread.detach();
}

void SocketServer::stop()
{
    close(this->server_fd);
}

void SocketServer::join()
{
}

int SocketServer::send(int client, const char *message, int lenth)
{
    if (clients.find(client) == clients.end()) {
        WARNING("Client " + std::to_string(client) + " not found.");
        return -2;
    }
    int ret = 0;
    ret = write(clients[client], message, lenth);
    if (ret < 0) {
        ERROR("Failed to send message to Client" + std::to_string(client));
        disconnect(client);
    }
    return ret;
}

int SocketServer::broadcast(const char *message, int lenth)
{
    int ret = 0;
    std::vector<std::thread> tt;

    for (auto const &client : clients) {
        tt.push_back(std::thread([this, client, message, lenth]() {
            int ret = write(client.second, message, lenth);
            if (ret < 0) {
                ERROR("Failed to send message to Client" +
                      std::to_string(client.first));
                disconnect(client.first);
            }
        }));
    }
    for (auto &t : tt) {
        t.join();
    }
    ret = clients.size() == 0 ? -1 : 0;
    return ret;
}

void SocketServer::set_on_message(
    std::function<void(int, const char *)> on_message)
{
    this->on_message = on_message;
}

void SocketServer::set_on_connect(std::function<void(int)> on_connect)
{
    this->on_connect = on_connect;
}

void SocketServer::set_on_disconnect(std::function<void(int)> on_disconnect)
{
    this->on_disconnect = on_disconnect;
}

void SocketServer::accept()
{
    while (true) {
        int client_fd = ::accept(this->server_fd, NULL, NULL);
        if (client_fd == -1) {
            ERROR("Failed to accept client");
            continue;
        }
        // 查找可用的 client_id
        int client_id = 0;
        while (clients.find(client_id) != clients.end()) {
            client_id++;
        }
        clients[client_id] = client_fd;

        if (this->on_connect != NULL) { // 调用连接处理函数
            this->on_connect(client_id);
        }
        // 创建并分离消息接收线程
        std::thread receive_thread(&SocketServer::receive, this, client_id);
        receive_threads[client_id] = receive_thread.native_handle();
        receive_thread.detach();
        client_id++; //? 与上面代码重复, 冗余代码
    }
}

void SocketServer::receive(int client)
{
    int cli = clients[client];

    while (true) {
        char buffer[10240] = { 0 };
        int lenth = read(cli, buffer, 10240);
        if (lenth <= 0 || std::string(buffer) == "/disconnect" ||
            clients.find(client) == clients.end()) {
            disconnect(client);
            break;
        }
        while (lenth < 10240) { // 读取完整应用层消息
            int valread = read(cli, buffer + lenth, 10240 - lenth);
            if (valread <= 0) {
                break;
            }
            lenth += valread;
        }

        if (this->on_message != NULL) { // 调用消息处理函数
            this->on_message(client, buffer);
        }
    }
}

void SocketServer::disconnect(int client)
{
    close(clients[client]); // 关闭客户端连接
    clients.erase(client); // 删除客户端

    if (this->on_disconnect != NULL) { // 调用断开连接处理函数
        this->on_disconnect(client);
    }
}

std::string SocketServer::get_clients()
{
    std::string clients_str = "";
    for (auto const &client : clients) {
        clients_str += std::to_string(client.first) + " ";
    }
    return clients_str;
}