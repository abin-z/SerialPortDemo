/// 说明：串口通信封装类实现
/// 作者：abin
/// 日期：2025-09-20
/// 备注：基于 serial 库(https://github.com/wjwwood/serial)，跨平台支持 Windows / Linux / macOS

#include "serialport/serialport.h"

SerialPort::SerialPort(const std::string& port, uint32_t baudrate) : port_(port), baudrate_(baudrate) {}

SerialPort::~SerialPort()
{
  close();
}

/// @brief 列出系统当前可用串口
std::vector<serial::PortInfo> SerialPort::listPorts()
{
  return serial::list_ports();
}

/// @brief 设置串口端口名
SerialPort& SerialPort::setPort(const std::string& port)
{
  port_ = port;
  return *this;
}

/// @brief 设置波特率
SerialPort& SerialPort::setBaudRate(uint32_t baudrate)
{
  baudrate_ = baudrate;
  return *this;
}

/// @brief 设置串口读取超时时间, 单位毫秒
SerialPort& SerialPort::setTimeout(uint32_t timeout_ms)
{
  timeout_ms_ = timeout_ms;
  return *this;
}

/// @brief 设置自动重连最大次数
SerialPort& SerialPort::setReconnectLimit(size_t limit)
{
  reconnect_max_ = limit;
  return *this;
}

/// @brief 设置数据接收回调
SerialPort& SerialPort::setDataCallback(DataCallback cb)
{
  data_cb_ = std::move(cb);
  return *this;
}
/// @brief 设置日志输出回调
SerialPort& SerialPort::setLogCallback(LogCallback cb)
{
  log_cb_ = std::move(cb);
  return *this;
}

/// @brief 打开串口
bool SerialPort::open()
{
  std::lock_guard<std::mutex> lock(mtx_);

  if (port_.empty())
  {
    logMsg(LogLevel::Error, "open failed: port not set");
    return false;
  }
  if (baudrate_ == 0)
  {
    logMsg(LogLevel::Error, "open failed: baudrate not set");
    return false;
  }

  try
  {
    auto timeout = serial::Timeout::simpleTimeout(timeout_ms_);
    serial_.setPort(port_);
    serial_.setBaudrate(baudrate_);
    serial_.setTimeout(timeout);
    serial_.open();

    if (serial_.isOpen())
    {
      running_ = true;
      if (!reader_thread_.joinable()) reader_thread_ = std::thread(&SerialPort::readLoop, this);
      logMsg(LogLevel::Info, "SerialPort opened");
      return true;
    }
  }
  catch (const std::exception& e)
  {
    logMsg(LogLevel::Warning, std::string("open exception: ") + e.what());
  }

  // 重连尝试
  for (size_t attempt = 1; attempt <= reconnect_max_; ++attempt)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500 * attempt));
    try
    {
      serial_.open();
      if (serial_.isOpen())
      {
        running_ = true;
        if (!reader_thread_.joinable()) reader_thread_ = std::thread(&SerialPort::readLoop, this);
        logMsg(LogLevel::Info, "SerialPort reconnected");
        return true;
      }
    }
    catch (const std::exception& e)
    {
      logMsg(LogLevel::Warning, "Reconnect attempt " + std::to_string(attempt) + " failed: " + e.what());
    }
  }

  logMsg(LogLevel::Error, "open failed after retries");
  return false;
}

/// @brief 关闭串口
void SerialPort::close()
{
  stop();  // 停止读线程

  std::lock_guard<std::mutex> lock(mtx_);
  if (serial_.isOpen())
  {
    serial_.close();
    logMsg(LogLevel::Info, "SerialPort closed");
  }
}

/// @brief 检查串口是否已打开
bool SerialPort::isOpen() const
{
  return serial_.isOpen();
}

/// @brief 向串口发送数据
size_t SerialPort::write(const std::string& data)
{
  std::lock_guard<std::mutex> lock(mtx_);
  if (!serial_.isOpen())
  {
    logMsg(LogLevel::Error, "write failed: not open");
    return 0;
  }

  try
  {
    return serial_.write(data);
  }
  catch (const std::exception& e)
  {
    logMsg(LogLevel::Error, std::string("write exception: ") + e.what());
    return 0;
  }
}

/// @brief 内部停止读线程
void SerialPort::stop()
{
  running_ = false;
  if (reader_thread_.joinable())
  {
    reader_thread_.join();
  }
}

/// @brief 内部读线程主循环
void SerialPort::readLoop()
{
  std::vector<uint8_t> buffer(1024 * 64);  // 64KB 缓冲区
  while (running_)
  {
    try
    {
      if (!serial_.isOpen())
      {
        logMsg(LogLevel::Warning, "disconnected, try reconnect");
        reconnect();
        continue;
      }

      // 读取最多 buffer.size() 字节，复用 buffer
      size_t n = serial_.read(buffer.data(), buffer.size());
      if (n > 0 && data_cb_)
      {
        data_cb_(std::string(reinterpret_cast<const char*>(buffer.data()), n));
      }
      else if (n == 0)
      {
        // 没有数据，短暂 sleep 避免 CPU 占满
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    }
    catch (const std::exception& e)
    {
      logMsg(LogLevel::Warning, std::string("read exception: ") + e.what());
      reconnect();
    }
  }
}

/// @brief 内部尝试重连串口
void SerialPort::reconnect()
{
  stop();

  std::lock_guard<std::mutex> lock(mtx_);
  if (serial_.isOpen()) serial_.close();

  for (size_t attempt = 1; attempt <= reconnect_max_ && running_; ++attempt)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500 * attempt));
    try
    {
      serial_.open();
      if (serial_.isOpen())
      {
        running_ = true;
        if (!reader_thread_.joinable()) reader_thread_ = std::thread(&SerialPort::readLoop, this);
        logMsg(LogLevel::Info, "SerialPort reconnected");
        return;
      }
    }
    catch (...)
    {
      logMsg(LogLevel::Warning, "Reconnect attempt " + std::to_string(attempt) + " failed");
    }
  }

  logMsg(LogLevel::Error, "reconnect failed after retries");
}

/// @brief 内部日志输出
void SerialPort::logMsg(LogLevel level, const std::string& msg)
{
  if (log_cb_)
  {
    std::ostringstream oss;
    oss << "[" << port_ << "@" << baudrate_ << "] " << msg;
    log_cb_(level, oss.str());
  }
}