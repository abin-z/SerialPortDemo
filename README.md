## 一个串口库的基本使用demo

**使用了这个三方库: [serial](https://github.com/wjwwood/serial.git)**

- 只包含改库的`include`和`src`文件夹
- 简化了原来自带的CMakeLists.txt


运行结果:
```sh
PS ***\serialport\build_output\bin\Debug> ."/serialport/build_output/bin/Debug/serialport.exe"
Available serial ports:
Port: COM1 | Description: 通信端口 (COM1) | Hardware ID: ACPI\VEN_PNP&DEV_0501
Port: COM2 | Description: ELTIMA Virtual Serial Port (COM2->COM3) | Hardware ID: EVSERIAL
Port: COM3 | Description: ELTIMA Virtual Serial Port (COM3->COM2) | Hardware ID: EVSERIAL
Port opened: COM3 @ 115200bps
Sent: Hello Serial!
Received: 
PS ***\serialport\build_output\bin\Debug> 

```