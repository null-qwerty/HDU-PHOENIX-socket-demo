#include "Info.hpp"
#include <cstring>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "SocketClient.hpp"
#include "Message.hpp"
#include "Application.hpp"
#include <sstream>
#include <vector>

// 客户端处理类
// 由于原先的 Application 类不太适合多 fd 处理, 因此新建一个类
// 此处仅作为示例, 并未按照最佳实践进行设计
struct ClientApp {
    std::vector<cv::Mat> frames;    ///< 存储接收到的图像
    cv::Mat cameraMatrix, distCoeffs;   ///< 相机内参和畸变系数

    std::shared_ptr<Application<SocketClient>> video_app;   ///< 视频接收应用
    std::shared_ptr<Application<SocketClient>> camerainfo_app;  ///< 相机信息接收应用
    std::shared_ptr<Application<SocketClient>> transformer_app; ///< 变换请求应用

    /**
     * @brief 构造函数
     * 
     * @param video_receiver 
     * @param camerainfo_receiver 
     * @param transformer 
     */
    ClientApp(std::shared_ptr<Application<SocketClient>> &video_receiver,
              std::shared_ptr<Application<SocketClient>> &camerainfo_receiver,
              std::shared_ptr<Application<SocketClient>> &transformer)
    {
        video_app = video_receiver;
        camerainfo_app = camerainfo_receiver;
        transformer_app = transformer;
    }
    /**
     * @brief 图像处理函数
     * 
     * @param frame 接收到的图像
     */
    void getFrame(cv::Mat &frame)
    {
        // 显示图像
        cv::imshow("received", frame);
        cv::waitKey(1);

        frames.push_back(frame);
        // 请求获取当前坐标变换
        Message::TransformRequestData data;
        memcpy(data.From, "Camera", sizeof("Camera"));
        memcpy(data.To, "Odom", sizeof("Odom"));

        transformer_app->encode_and_send(
            Message::MessageType::TRANSFORM_REQUEST, frames.size(),
            (unsigned char *)&data, sizeof(data), 0);
    }
};
std::shared_ptr<SocketClient> video_receiver =
    std::make_shared<SocketClient>("127.0.0.1", 8000);
std::shared_ptr<SocketClient> camerainfo_receiver =
    std::make_shared<SocketClient>("127.0.0.1", 5140);
std::shared_ptr<SocketClient> transformer =
    std::make_shared<SocketClient>("127.0.0.1", 4399);

std::shared_ptr<Application<SocketClient>> video_app =
    std::make_shared<Application<SocketClient>>(video_receiver);
std::shared_ptr<Application<SocketClient>> camerainfo_app =
    std::make_shared<Application<SocketClient>>(camerainfo_receiver);
std::shared_ptr<Application<SocketClient>> transformer_app =
    std::make_shared<Application<SocketClient>>(transformer);
ClientApp clientApp(video_app, camerainfo_app, transformer_app);

/**
 * @brief 初始化客户端
 * 
 * @param socket 基础 socket
 * @param app 应用层消息处理器
 */
void initClient(std::shared_ptr<SocketClient> &socket,
                std::shared_ptr<Application<SocketClient>> &app)
{
    // 接收消息回调函数
    std::function recv_callback = [&app](const char *message) {
        Message msg(message);
        unsigned char *m = app->receive_and_decode(msg);
        if(m == nullptr)    // 解码失败或消息不完整, 不处理
            return;
        if (msg.get_messageType() == Message::MessageType::IMAGE_MSG) { // 图像消息
            DEBUG("Image received.");
            std::vector<unsigned char> data(m, m + msg.get_dataTotalLenth());
            cv::Mat image = cv::imdecode(data, cv::IMREAD_COLOR);
            clientApp.getFrame(image);  // 处理图像
        }
        if(msg.get_messageType() == Message::MessageType::STRING_MSG)   // 字符串消息
            INFO("Server: " + std::string((char *)m));  // 输出消息
        if (msg.get_messageType() == Message::MessageType::CAMERA_INFO) {   // 相机信息
            DEBUG("Camera info received.");
            std::vector<unsigned char> data(m, m + msg.get_dataTotalLenth());
            // 存下相机内参和畸变系数
            clientApp.cameraMatrix = cv::Mat(3, 3, CV_64F, ((Message::CameraInfoData *)data.data())->CameraMatrix);
            clientApp.distCoeffs = cv::Mat(1, 5, CV_64F, ((Message::CameraInfoData *)data.data())->DistortionCoefficients);
        }
        if (msg.get_messageType() == Message::MessageType::TRANSFORM) { // 变换信息
            DEBUG("Transform received.");
            std::vector<unsigned char> data(m, m + msg.get_dataTotalLenth());
            // 输出变换信息
            std::stringstream ss;
            // clang-format off
            ss << "Translation:" << std::endl 
                << " x = " << ((Message::TransformData *)data.data())->Translation[0]
                << " y = " << ((Message::TransformData *)data.data())->Translation[1]
                << " z = " << ((Message::TransformData *)data.data())->Translation[2] << std::endl
               << "Rotation:" << std::endl
                << " x = " << ((Message::TransformData *)data.data())->Rotation[0]
                << " y = " << ((Message::TransformData *)data.data())->Rotation[1]
                << " z = " << ((Message::TransformData *)data.data())->Rotation[2]
                << " w = " << ((Message::TransformData *)data.data())->Rotation[3] << std::endl;
            // clang-format on
            INFO(ss.str());
        }

        delete[] m;
    };

    socket->set_on_message(recv_callback);  // 设置消息处理函数
    socket->set_on_connect([&app]() {   // 设置连接成功处理函数
        INFO("Connected to server.");
        std::string msg("Hello server\n");
        app->encode_and_send(Message::MessageType::STRING_MSG, 0,
                            (unsigned char *)(msg.c_str()), msg.size(),0);
    });
    socket->set_on_disconnect([]() { INFO("Disconnected from server."); });   // 设置断开连接处理函数

    socket->connect();  // 连接服务器
}

int main(int argc, char *argv[])
{
    cv::namedWindow("received", cv::WINDOW_NORMAL);

    initClient(video_receiver, video_app);
    initClient(camerainfo_receiver, camerainfo_app);
    initClient(transformer, transformer_app);

    while(1);   // 保持程序运行

    return 0;
}