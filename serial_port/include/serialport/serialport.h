#pragma once
#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <serial/serial.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

class SerialPort
{
 public:
  enum class LogLevel
  {
    Info,
    Warning,
    Error
  };

  using DataCallback = std::function<void(const std::string&)>;
  using LogCallback = std::function<void(SerialPort::LogLevel, const std::string&)>;

  SerialPort();
  SerialPort(const std::string& port, uint32_t baudrate);
  ~SerialPort();

  SerialPort& setPort(const std::string& port);  // 设置串口名称
  SerialPort& setBaudRate(uint32_t baudrate);    // 设置波特率
  SerialPort& setTimeout(uint32_t timeout_ms);   // 设置超时时间（毫秒）
  SerialPort& setReconnectLimit(size_t limit);   // 设置重连次数
  SerialPort& setDataCallback(DataCallback cb);  // 设置数据接收回调
  SerialPort& setLogCallback(LogCallback cb);    // 设置日志回调

  bool open();                            // 打开串口
  void close();                           // 关闭串口
  bool isOpen() const;                    // 检查串口是否打开
  size_t write(const std::string& data);  // 发送数据

 private:
  void stop();
  void readLoop();
  void reconnect();

 private:
  serial::Serial serial_;
  std::string port_;
  uint32_t baudrate_{0};
  size_t reconnect_max_{0};
  uint32_t timeout_ms_{10};
  std::atomic_bool running_{false};
  std::thread reader_thread_;
  std::mutex mtx_;
  DataCallback data_cb_;
  LogCallback log_cb_;
};

#endif  // SERIAL_PORT_H
