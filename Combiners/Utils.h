#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include <codecvt>
#include <format>
#include <functional>
#include <locale>
#include <nlohmann/json.hpp>
#include <string>

#include "../Error.h"

class Converter
{
private:
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

public:
  static std::wstring ToWString(const std::string& src);

  static std::string ToString(const std::wstring& src);
};

std::string GetLastErrorAsString();

void CreateDirectoryWithChecking(const wchar_t* i_LPCWSTR_FolderPath);

template<typename T>
T TryGetValue(nlohmann::json data, const std::string& field)
{
  if(!data.contains(field)) {
    throw MangalibError(std::format("Cannot find {0}", field));
  }

  return data[field].get<T>();
}

#endif// UTILS_H
