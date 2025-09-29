#include <serial/serial.h>
#include <iostream>

int main()
{
    // List all available serial ports
    std::vector<serial::PortInfo> ports = serial::list_ports();

    if (ports.empty())
    {
        std::cout << "No serial ports found." << std::endl;
        return 0;
    }

    std::cout << "Available serial ports:" << std::endl;
    for (const auto &port : ports)
    {
        // Print port name, description, and hardware ID
        std::cout << "Port: " << port.port 
                  << " | Description: " << port.description
                  << " | Hardware ID: " << port.hardware_id
                  << std::endl;
    }

    try
    {
        // Specify the port to open
        std::string port = "COM3";       // Windows example, Linux: "/dev/ttyUSB0"
        unsigned long baud = 115200;     // Baud rate

        // Open the serial port with a 1-second timeout
        serial::Serial ser(port, baud, serial::Timeout::simpleTimeout(1000));

        // Check if the port is successfully opened
        if (ser.isOpen())
        {
            std::cout << "Port opened: " << port << " @ " << baud << "bps" << std::endl;
        }
        else
        {
            std::cerr << "Failed to open port: " << port << std::endl;
            return 1;
        }

        // Send a simple message
        std::string msg = "Hello Serial!\n";
        ser.write(msg);
        std::cout << "Sent: " << msg;

        // Read up to 100 bytes from the serial port
        std::string result = ser.read(100);
        std::cout << "Received: " << result << std::endl;

        // Close the port
        ser.close();
    }
    catch (std::exception &e)
    {
        // Catch any exception and print the error message
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
