#ifndef INFO_HPP
#define INFO_HPP

#include <mutex>
#include <map>
#include <functional>
#include <string>

/**
 * @brief Info 类, 用于输出信息。进程内单例, 用于防止多线程输出混乱。
 * 
 */
class Info
{
public:
    /**
     * @brief Info 构造函数
     * 
     */
    Info();
    /**
     * @brief Info 析构函数
     * 
     */
    ~Info();
    std::function<void(std::string, std::string)> print;    ///< 输出函数
private:
    std::mutex mtx; ///< 互斥锁
    std::map<std::string, std::string> colors;  ///< 输出颜色
};

extern Info info;

// 定义输出宏
#define INFO(msg) info.print(std::string(msg), "white")
#define WARNING(msg) info.print(std::string(msg), "yellow")
#define ERROR(msg) info.print(std::string(msg), "red")
#define SUCCESS(msg) info.print(std::string(msg), "green")
#define DEBUG(msg) info.print(std::string(msg), "blue")

#endif