#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <signal.h>
#include <sstream>

#include "SocketServer.hpp"
#include "Message.hpp"
#include "Application.hpp"
#include "Info.hpp"

int main(int argc, char *argv[])
{
    // 忽略 SIGPIPE 信号, 防止因为发送消息时客户端断开连接导致服务器崩溃
    // SIGPIPE 信号会在向一个已经关闭的 socket 发送消息时产生, 默认行为是终止进程
    // 后续需要对发送消息的返回值进行判断, 防止因为发送消息失败导致服务器崩溃
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    SocketServer server(std::stoi(argv[1]));
    std::shared_ptr<SocketServer> server_ptr(&server);
    Application<SocketServer> app(server_ptr);

    // 设置消息处理函数
    server.set_on_message([&app](int client, const char *message) {
        Message msg(message);
        unsigned char *m = app.receive_and_decode(msg); // 解码消息
        if (m != nullptr &&
            msg.get_messageType() == Message::MessageType::STRING_MSG)  // 字符串消息
            INFO("Client " + std::to_string(client) + ": " +        // 输出消息
                 std::string((char *)m));
        if (m != nullptr &&
            msg.get_messageType() == Message::MessageType::IMAGE_MSG) { // 图像消息
            DEBUG("Image received.");                           
            std::vector<unsigned char> data(m, m + msg.get_dataTotalLenth());
            cv::Mat image = cv::imdecode(data, cv::IMREAD_COLOR);
            std::thread([=]() {
                cv::namedWindow("Client " + std::to_string(client), cv::WINDOW_NORMAL);
                cv::imshow("Image", image); // 显示图像
                cv::waitKey(0);
                cv::destroyAllWindows();
            }).detach();    // 分离显示图像线程
        }
        if (m != nullptr)
            delete[] m; // 释放内存
    });
    // 设置连接成功处理函数
    server.set_on_connect([&app](int client) {
        if (client > 15)    // 最多支持 16 个客户端
            app.disconnect(client);
        INFO("Client " + std::to_string(client) +
             " connected."); // 输出连接信息
        // 发送 Hello client 消息
        std::string msg("Hello client " + std::to_string(client) + "\n");
        app.encode_and_send(Message::MessageType::STRING_MSG, 0,
                            (unsigned char *)(msg.c_str()), msg.size(), client);
    });
    // 设置断开连接处理函数
    server.set_on_disconnect([&server](int client) {
        INFO("Client " + std::to_string(client) + " disconnected.");    // 输出断开连接信息
    });
    // 添加主动发送消息命令
    app.add_command("sendto", [&app](std::string args) {
        int sp = std::count(args.begin(), args.end(), ' ');
        if (sp < 2) {
            ERROR("sendto: not enough arguments.");
            WARNING("Usage: sendto <client> <message>");
            WARNING("Broadcast: sendto -1 <message>");
            return;
        }

        std::istringstream iss(args);

        int client;
        std::string message;

        iss >> client;
        std::getline(iss, message);

        app.encode_and_send(Message::MessageType::STRING_MSG, 0,
                            (unsigned char *)(message.c_str()), message.size(),
                            client);
    });
    // 添加发送图像命令
    app.add_command("sendimage", [&app](std::string args) {
        int sp = std::count(args.begin(), args.end(), ' ');
        if (sp < 2) {
            ERROR("sendimage: not enough arguments.");
            WARNING("Usage: sendimage <client> <image_path>");
            WARNING("Broadcast: sendimage -1 <image_path>");
            return;
        }

        std::istringstream iss(args);

        int client;
        std::string image_path;

        iss >> client >> image_path;

        cv::Mat image = cv::imread(image_path);
        std::vector<unsigned char> data;
        cv::imencode(".jpg", image, data);

        app.encode_and_send(Message::MessageType::IMAGE_MSG, 0, data.data(),
                            data.size(), client);
    });
    // 添加发送视频命令
    app.add_command("sendvideo", [&app](std::string args) {
        int sp = std::count(args.begin(), args.end(), ' ');
        if (sp < 2) {
            ERROR("sendvideo: not enough arguments.");
            WARNING("Usage: sendvideo <client> <video_path>");
            WARNING("Broadcast: sendvideo -1 <video_path>");
            return;
        }

        std::istringstream iss(args);

        int client;
        std::string image_path;

        iss >> client >> image_path;

        cv::VideoCapture cap(image_path);
        cv::Mat frame;
        std::vector<std::vector<unsigned char>> datas;
        int i = 0;

        DEBUG("Processing video...");
        while (cap.read(frame)) {
            datas.push_back(std::vector<unsigned char>());
            cv::imencode(".jpg", frame, datas[i]);
            i++;
        }
        DEBUG("Sending video...");
        i = 0;
        for (auto &data : datas) {
            if (app.encode_and_send(Message::MessageType::IMAGE_MSG, i,
                                    data.data(), data.size(), client) < 0) {
                ERROR("Failed to send frame " + std::to_string(i));
                break;
            }
            i++;
        }
        datas.clear();
        SUCCESS("done.");
    });
    // 添加断开连接命令
    app.add_command("disconnect", [&app](std::string args) {
        int sp = std::count(args.begin(), args.end(), ' ');
        if (sp < 1) {
            ERROR("disconnect: not enough arguments.");
            WARNING("Usage: disconnect <client>");
            return;
        }
        std::istringstream iss(args);
        int client;

        iss >> client;
        app.disconnect(client);
    });
    // 添加获取客户端列表命令
    app.add_command("client_list", [&app](std::string args) {
        int sp = std::count(args.begin(), args.end(), ' ');
        if (sp > 0)
            WARNING("client_list: no arguments needed.");
        DEBUG("Connected clients: " + app.get_clients());
    });
    // 添加清屏命令
    app.add_command("clear", [&app](std::string args) { 
        int sp = std::count(args.begin(), args.end(), ' ');
        if (sp > 0)
            WARNING("clear: no arguments needed.");
        system("clear"); });

    server.start(); // 启动服务器
    app.join();    // 阻塞并等待命令输入

    return 0;
}