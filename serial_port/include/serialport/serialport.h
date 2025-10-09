#pragma once
#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <serial/serial.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

/**
 * @brief 串口通信封装类（基于 serial 库）
 *
 * 该类封装了串口的打开、关闭、读写、自动重连与回调通知机制，
 * 提供线程安全的接口，适用于跨平台串口通信（Windows / Linux / macOS）。
 */
class SerialPort
{
 public:
  /**
   * @brief 日志等级枚举
   */
  enum class LogLevel
  {
    Info,     ///< 一般信息
    Warning,  ///< 警告信息
    Error     ///< 错误信息
  };

  /// 数据接收回调函数类型（参数为接收到的字符串数据）
  using DataCallback = std::function<void(const std::string&)>;

  /// 日志回调函数类型（参数为日志级别与消息内容）
  using LogCallback = std::function<void(SerialPort::LogLevel, const std::string&)>;

 public:
  /**
   * @brief 默认构造函数
   */
  SerialPort();

  /**
   * @brief 指定端口与波特率的构造函数
   * @param port 串口名称（如 "COM3" 或 "/dev/ttyUSB0"）
   * @param baudrate 波特率（如 9600、115200 等）
   */
  SerialPort(const std::string& port, uint32_t baudrate);

  /**
   * @brief 析构函数，会自动关闭串口与停止读取线程
   */
  ~SerialPort();

  /**
   * @brief 列出系统中所有可用的串口
   * @return 返回包含所有串口信息的向量
   */
  static std::vector<serial::PortInfo> listPorts();

  /**
   * @brief 设置串口端口名
   * @param port 串口名称（如 "COM3" 或 "/dev/ttyUSB0"）
   * @return 返回自身引用以支持链式调用
   */
  SerialPort& setPort(const std::string& port);

  /**
   * @brief 设置波特率
   * @param baudrate 波特率值
   * @return 返回自身引用以支持链式调用
   */
  SerialPort& setBaudRate(uint32_t baudrate);

  /**
   * @brief 设置串口读取超时时间
   * @param timeout_ms 超时时间（毫秒）
   * @return 返回自身引用以支持链式调用
   */
  SerialPort& setTimeout(uint32_t timeout_ms);

  /**
   * @brief 设置自动重连最大次数
   * @param limit 重连次数上限（0 表示不重连）
   * @return 返回自身引用以支持链式调用
   */
  SerialPort& setReconnectLimit(size_t limit);

  /**
   * @brief 设置数据接收回调函数
   * @param cb 回调函数，参数为接收到的数据字符串
   * @return 返回自身引用以支持链式调用
   */
  SerialPort& setDataCallback(DataCallback cb);

  /**
   * @brief 设置日志输出回调函数
   * @param cb 回调函数，参数为日志级别与内容
   * @return 返回自身引用以支持链式调用
   */
  SerialPort& setLogCallback(LogCallback cb);

  /**
   * @brief 打开串口并启动读线程
   * @return 打开成功返回 true，否则返回 false
   */
  bool open();

  /**
   * @brief 关闭串口连接并停止读线程
   */
  void close();

  /**
   * @brief 检查串口是否处于打开状态
   * @return true 表示已打开，false 表示未打开
   */
  bool isOpen() const;

  /**
   * @brief 向串口发送数据
   * @param data 要发送的字符串数据
   * @return 实际写入的字节数
   */
  size_t write(const std::string& data);

 private:
  /**
   * @brief 停止数据读取线程（内部使用）
   */
  void stop();

  /**
   * @brief 数据读取循环函数（后台线程）
   *
   * 负责持续读取串口数据并在收到数据时触发 data_cb_。
   */
  void readLoop();

  /**
   * @brief 串口断开时尝试重连
   */
  void reconnect();

  /**
   * @brief 输出日志消息（触发 log_cb_）
   * @param level 日志级别
   * @param msg 日志内容
   */
  void logMsg(LogLevel level, const std::string& msg);

 private:
  serial::Serial serial_;            ///< serial 库的串口对象
  std::string port_;                 ///< 串口名称
  uint32_t baudrate_{0};             ///< 波特率
  size_t reconnect_max_{0};          ///< 最大重连次数（0 表示不重连）
  uint32_t timeout_ms_{10};          ///< 读超时时间（默认 10ms）
  std::atomic_bool running_{false};  ///< 读线程运行标志
  std::thread reader_thread_;        ///< 后台读取线程
  std::mutex mtx_;                   ///< 串口访问互斥锁
  DataCallback data_cb_;             ///< 数据接收回调
  LogCallback log_cb_;               ///< 日志回调
};

#endif  // SERIAL_PORT_H
