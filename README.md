# Socket DEMO

这是 HDU-PHOENIX 战队的暑假培训的一次考核任务，要求用 Socket 通信实现一个类自瞄框架。

## 任务要求
1. 从服务器获取视频
2. 从服务器获取相机内参
3. 从服务器获取当前坐标转换关系
4. 向服务器发送观测坐标和预测坐标

目的是熟悉通信的基本操作，为后续 ROS 通信打下基础。

## 仓库内容

本仓库仅为一个简单的 DEMO，仅包含消息收发，不包含任何算法。

### 类说明

- `class Message{}` 消息类，用于封装消息
- `class SocketServer{}` 基础的 Socket 服务器
- `class SocketClient{}` 基础的 Socket 客户端
- `class Application{}` 应用层协议处理器
- `class Info{}` 用于输出, 防止多线程输出冲突

### 程序说明

- `server.cpp` 视频发送服务器端程序
- `camerainfo_server.cpp` 相机内参发送服务器端程序
- `tf.cpp` 坐标转换关系发送服务器端程序
- `client.cpp` 客户端程序

---
注: `tf.cpp` 使用了 `RMCV-PHOENIX2024`, 安装方式如下：

1. 下载 [源码](https://github.com/HDU-PHOENIX/RMCV2024-PHOENIX/releases/download/v0.7.3/RMCV-PHOENIX-0.7.3-Linux.tar.xz)
2. 进入根目录, 执行
```shell
cd scripts
bash install.sh
# or
bash scripts/install.sh
```
3. 全部勾选安装即可