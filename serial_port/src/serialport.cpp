#include "serialport/serialport.h"

SerialPort::SerialPort() = default;
SerialPort::SerialPort(const std::string& port, uint32_t baudrate) : port_(port), baudrate_(baudrate) {}

SerialPort::~SerialPort()
{
  close();
}

std::vector<serial::PortInfo> SerialPort::listPorts()
{
  return serial::list_ports();
}

SerialPort& SerialPort::setPort(const std::string& port)
{
  port_ = port;
  return *this;
}

SerialPort& SerialPort::setBaudRate(uint32_t baudrate)
{
  baudrate_ = baudrate;
  return *this;
}

SerialPort& SerialPort::setTimeout(uint32_t timeout_ms)
{
  timeout_ms_ = timeout_ms;
  return *this;
}

SerialPort& SerialPort::setReconnectLimit(size_t limit)
{
  reconnect_max_ = limit;
  return *this;
}

SerialPort& SerialPort::setDataCallback(DataCallback cb)
{
  data_cb_ = std::move(cb);
  return *this;
}

SerialPort& SerialPort::setLogCallback(LogCallback cb)
{
  log_cb_ = std::move(cb);
  return *this;
}

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

void SerialPort::close()
{
  stop();

  std::lock_guard<std::mutex> lock(mtx_);
  if (serial_.isOpen())
  {
    serial_.close();
    logMsg(LogLevel::Info, "SerialPort closed");
  }
}

bool SerialPort::isOpen() const
{
  return serial_.isOpen();
}

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

void SerialPort::stop()
{
  running_ = false;
  if (reader_thread_.joinable())
  {
    reader_thread_.join();
  }
}

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

// ---------- 内部日志函数 ----------
void SerialPort::logMsg(LogLevel level, const std::string& msg)
{
  if (log_cb_)
  {
    std::ostringstream oss;
    oss << "[" << port_ << "@" << baudrate_ << "] " << msg;
    log_cb_(level, oss.str());
  }
}