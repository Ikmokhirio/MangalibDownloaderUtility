#ifndef LOKI_MANGALIBERROR_H
#define LOKI_MANGALIBERROR_H

#include <iostream>
#include <stdexcept>

enum class LogLevel
{
  info,
  error,
};

inline std::string GetTime()
{
  std::time_t t = std::time(nullptr);
  std::tm* now = std::localtime(&t);

  char buffer[128];
  strftime(buffer, sizeof(buffer), "%X", now);
  return buffer;
}

template<typename... Args>
void Log(LogLevel level, const char* fmt, Args&&... args)
{
  std::string m = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
  std::string result = std::format("[{0}] APP : {1}", GetTime(), m);

  switch(level) {
  case LogLevel::info:
    std::cout << result << std::endl;
    break;
  case LogLevel::error:
    std::cerr << result << std::endl;
    break;
  default:
    std::cout << result << std::endl;
  }
}

#define ML_INFO(...) Log(LogLevel::info, __VA_ARGS__)
#define ML_ERROR(...) Log(LogLevel::error, __VA_ARGS__)

class MangalibError : public std::runtime_error
{
public:
  inline MangalibError(std::string errorMessage)
      : std::runtime_error(errorMessage)
  {
    ML_ERROR("{0}", errorMessage);
  }
};

#endif// LOKI_MANGALIBERROR_H

