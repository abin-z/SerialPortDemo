#include <chrono>
#include <fstream>
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

  //   // 根据操作系统选择串口名称
  //   std::string port_name;
  // #if defined(_WIN32) || defined(_WIN64)
  //   port_name = "COM5";  // Windows 示例
  // #else
  //   port_name = "/dev/ttyS000";  // Linux 示例
  // #endif

  // 让用户输入串口号
  std::string port_name;
  std::cout << "Enter the serial port name to use (e.g., COM5 or /dev/ttyUSB0): ";
  std::getline(std::cin, port_name);

  // 打开文件用于记录数据
  std::ofstream log_file("serial_log.txt", std::ios::out | std::ios::app);
  if (!log_file.is_open())
  {
    std::cerr << "Failed to open log file!" << std::endl;
    return 1;
  }

  // 创建串口对象
  SerialPort sp;

  // 配置串口参数并注册回调 (回调函数在独立线程中调用)
  sp.setPort(port_name)
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
    .setDataCallback([&log_file](const std::string& data) {
      for (unsigned char c : data)
      {
        log_file << std::hex << std::uppercase << (int)c << " ";
      }
      log_file.flush();                   // 保证立即写入文件
      log_file << std::dec << std::endl;  // 每条数据换行，并恢复十进制格式
    });

  // 打开串口
  if (!sp.open())
  {
    std::cerr << "Failed to open serial port" << std::endl;
    return 1;
  }

  std::cout << "Serial port opened. Type messages to send. Type 'exit' to quit.\n";

  // 循环读取用户输入并发送
  std::string line;
  while (true)
  {
    std::cout << "> ";
    std::getline(std::cin, line);

    if (line == "exit") break;

    sp.write(line + "\r\n");  // 发送回车换行，可按需要调整
  }

  // 关闭串口
  sp.close();

  return 0;
}
