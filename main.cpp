#include <serial/serial.h>

#include <iostream>

int main()
{
  std::vector<serial::PortInfo> ports = serial::list_ports();

  if (ports.empty())
  {
    std::cout << "No serial ports found." << std::endl;
    return 0;
  }

  std::cout << "Available serial ports:" << std::endl;
  for (const auto &port : ports)
  {
    std::cout << "Port: " << port.port << " | Description: " << port.description
              << " | Hardware ID: " << port.hardware_id << std::endl;
  }

  try
  {
    std::string port = "COM3";
    unsigned long baud = 115200;

    serial::Serial ser(port, baud, serial::Timeout::simpleTimeout(1000));

    if (ser.isOpen())
    {
      std::cout << "Port opened: " << port << " @ " << baud << "bps" << std::endl;
    }
    else
    {
      std::cerr << "Failed to open port: " << port << std::endl;
      return 1;
    }

    std::string msg = "Hello Serial!\n";
    ser.write(msg);
    std::cout << "Sent: " << msg;

    std::string result = ser.read(100);
    std::cout << "Received: " << result << std::endl;

    ser.close();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
