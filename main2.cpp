#include <chrono>
#include <iostream>
#include <thread>

#include "serialport/serialport.h"

int main()
{
  // 创建串口对象
  SerialPort sp;

  // 配置串口参数并注册回调
  sp.setPort("COM5")  // Windows 示例, Linux: "/dev/ttyUSB0"
    .setBaudRate(115200)
    .setTimeout(10)
    .setReconnectLimit(3)
    .setLogCallback([](SerialPort::LogLevel level, const std::string& msg) {
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
    .setDataCallback([](const std::string& data) { std::cout << "[DATA] " << data << std::endl; });

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
