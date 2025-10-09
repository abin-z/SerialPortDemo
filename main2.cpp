#include <chrono>
#include <iostream>
#include <thread>

#include "serialport/serialport.h"

int main()
{
  // 列出所有可用的串口
  std::vector<serial::PortInfo> ports = SerialPort::listPorts();
  if (ports.empty())
  {
    std::cout << "No serial ports found." << std::endl;
    return 0;
  }
  std::cout << "Available serial ports:" << std::endl;
  for (const auto& port : ports)
  {
    std::cout << "Port: " << port.port << " | Description: " << port.description
              << " | Hardware ID: " << port.hardware_id << std::endl;
  }

  // 创建串口对象
  SerialPort sp;

  // 配置串口参数并注册回调 (回调函数在独立线程中调用)
  sp.setPort("COM5")  // Windows 示例, Linux: "/dev/ttyUSB0"
    .setBaudRate(115200)
    .setTimeout(10)                                                           // 读超时 10ms
    .setReconnectLimit(3)                                                     // 最大重连 3 次
    .setLogCallback([](SerialPort::LogLevel level, const std::string& msg) {  // 日志回调
      std::string levelStr;
      switch (level)
      {
      case SerialPort::LogLevel::Info:
        levelStr = "[INFO] ";
        break;
      case SerialPort::LogLevel::Warning:
        levelStr = "[WARN] ";
        break;
      case SerialPort::LogLevel::Error:
        levelStr = "[ERROR] ";
        break;
      }
      std::cout << levelStr << msg << std::endl;
    })
    .setDataCallback([](const std::string& data) { std::cout << "[DATA] " << data << std::endl; });  // 数据回调

  // 打开串口
  if (!sp.open())
  {
    std::cerr << "Failed to open serial port" << std::endl;
    return 1;
  }

  for (int i = 0; i < 5; ++i)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sp.write("Hello Serial!\r\n");
  }

  // 等待 5 秒，接收数据
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // 关闭串口
  sp.close();

  return 0;
}
