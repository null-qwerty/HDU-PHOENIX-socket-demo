#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <signal.h>

#include "SocketServer.hpp"
#include "Message.hpp"
#include "Application.hpp"

#include <PHOENIX/Utils/Info/Info.hpp>

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

    server.set_on_message([&app](int client, const char *message) {});
    // 设置连接成功处理函数, 直接发送相机内参
    server.set_on_connect([&app](int client) {
        INFO("Client " + std::to_string(client) + " connected.");
        if (client > 15)
            app.disconnect(client);

        double camera_info[9 + 5] = { 2142.4253006101626,
                                      0.0,
                                      654.8800557555103,
                                      0.0,
                                      2139.740720699495,
                                      247.26009197675802,
                                      0.0,
                                      0.0,
                                      1.0,
                                      -0.04313325802537415,
                                      0.3598873080850437,
                                      -0.011789027160352577,
                                      -0.0068734187976891474,
                                      0.0 };
        app.encode_and_send(Message::MessageType::CAMERA_INFO, 0,
                            (unsigned char *)(camera_info), sizeof(camera_info),
                            client);
    });

    server.set_on_disconnect([&server](int client) {
        INFO("Client " + std::to_string(client) + " disconnected.");
    });

    server.start();
    app.join();

    return 0;
}