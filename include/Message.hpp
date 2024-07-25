#pragma once
/**
 * @brief Message 类, 应用层消息格式
 * 
 */
class Message {
public:
    enum MessageType {
        STRING_MSG = 0x0000,
        IMAGE_MSG = 0x1145,
        CAMERA_INFO = 0x1419,
        TRANSFORM = 0x1981,
        TRANSFORM_REQUEST = 0x1982
    };

#pragma pack(1) // 关闭内存对齐
    typedef struct MessageBuffer_s {
        // 数据头部
        unsigned short Start = 0x0D00;  // 起始位       0 ~ 1
        unsigned short MessageType;     // 消息类型     2 ~ 3
        unsigned int DataID;            // 数据 ID     4 ~ 7
        unsigned int DataTotalLenth;    // 数据总长度   8 ~ 11
        unsigned int Offset;            // 数据偏移     12 ~ 15
        unsigned int DataLenth;         // 数据长度     16 ~ 19
        // 数据
        unsigned char Data[10218];      // 数据         20 ~ 10237
        // 数据尾部
        unsigned short End = 0x0721;    // 结束位       10238 ~ 10239
    } MessageBuffer;                // 10240 Bytes

    // MessageType 为 IMAGE_MSG 时的 Data 部分数据结构
    // 此时 Offset 为 ImageData 的偏移
    //* 已弃用
    typedef struct{
        unsigned int Length;
        unsigned int Width;
        unsigned int Type;
        unsigned char ImageData[10218 - 12];
    } ImageData;

    // MessageType 为 CAMERA_INFO 时的 Data 部分数据结构
    typedef struct{
        double CameraMatrix[9];
        double DistortionCoefficients[5];
    } CameraInfoData;

    // MessageType 为 TRANSFORM 时的 Data 部分数据结构
    typedef struct{
        double Translation[3];
        double Rotation[4];
    } TransformData;

    // MessageType 为 TRANSFORM_REQUEST 时的 Data 部分数据结构
    typedef struct{
        char From[10218 / 2];
        char To[10218 / 2];
    } TransformRequestData;
#pragma pack()  // 开启内存对齐

    Message();
    Message(const unsigned short type);
    Message(const Message &message);
    Message(const unsigned char *buffer);
    Message(const char *buffer);
    ~Message() = default;

    void set_messageType(const unsigned short type);
    void set_dataID(const unsigned int id);
    void set_dataTotalLenth(const unsigned int lenth);
    void set_offset(const unsigned int offset);
    void set_data(const char *data, const unsigned int lenth);

    unsigned short get_messageType() const;
    unsigned int get_dataID() const;
    unsigned int get_dataTotalLenth() const;
    unsigned int get_offset() const;
    unsigned int get_dataLenth() const;
    const unsigned char *get_data() const;
    const unsigned char *get_buffer() const;

private:
    MessageBuffer message;
};