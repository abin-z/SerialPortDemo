## 一个跨平台的C++串口库

### 简介

**本库是一个跨平台简单的C++串口工具库, 其目的是简化串口开发**

- 简单易用, 只需注册回调即可实现异步接收数据.
- 跨平台, 支持*Windows / Linux / macOS*等主流操作系统.

库文件在[serial_port](serial_port)文件夹下

### 使用教程

1. 拷贝[`serial_port`](serial_port)到项目文件夹中.

2. 在CMakeLists.txt中添加以下内容:

   ```cmake
   add_subdirectory(serial_port)    # 添加子文件夹
   
   add_executable(serialportDemo main.cpp)                   # 根据实际项目更改
   target_link_libraries(serialportDemo PRIVATE serialport)  # 链接serialport
   ```

3. 在源文件中包含头文件`#include "serialport/serialport.h"`即可使用.

### 使用案例

该案例在[main2.cpp](main2.cpp)中

```cpp
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
```


运行结果:
```sh
PS ***\serialport\build_output\bin\Debug> ."/serialport/build_output/bin/Debug/serialportTest.exe"
Available serial ports:
Port: COM1 | Description: 通信端口 (COM1) | Hardware ID: ACPI\VEN_PNP&DEV_0501
Port: COM2 | Description: ELTIMA Virtual Serial Port (COM2->COM3) | Hardware ID: EVSERIAL
Port: COM3 | Description: ELTIMA Virtual Serial Port (COM3->COM2) | Hardware ID: EVSERIAL
Port: COM4 | Description: ELTIMA Virtual Serial Port (COM4->COM5) | Hardware ID: EVSERIAL
Port: COM5 | Description: ELTIMA Virtual Serial Port (COM5->COM4) | Hardware ID: EVSERIAL
[INFO] [COM5@115200] SerialPort opened
[DATA] Welcome to UartAssist
[INFO] [COM5@115200] SerialPort closed
PS ***\serialport\build_output\bin\Debug> 
```

### 说明

本库**使用了这个三方库: [serial](https://github.com/wjwwood/serial.git)**

- 该库跨平台,足够轻量且稳定, 已在大量知名项目中使用(如ROS)

- 只包含改库的`include`和`src`文件夹
- 简化了原来自带的CMakeLists.txt
- 也可以直接使用serial库, 案例在[main.cpp](main.cpp)中