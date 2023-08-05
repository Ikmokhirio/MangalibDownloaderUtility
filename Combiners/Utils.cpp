#include "Utils.h"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> Converter::converter;

std::wstring Converter::ToWString(const std::string& src)
{
  return converter.from_bytes(src);
}

std::string Converter::ToString(const std::wstring& src)
{
  return converter.to_bytes(src);
}

std::string GetLastErrorAsString()
{
  //Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if(errorMessageID == 0) {
    return std::string();//No error message has been recorded
  }

  LPWSTR messageBuffer = nullptr;

  //Ask Win32 to give us the string version of that message ID.
  //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
  size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

  //Copy the error message into a std::string.
  std::wstring message(messageBuffer, size);

  //Free the Win32's string's buffer.
  LocalFree(messageBuffer);

  return Converter::ToString(message);
}

void CreateDirectoryWithChecking(const wchar_t* i_LPCWSTR_FolderPath)
{
  WIN32_FIND_DATAW data;
  HANDLE handle = ::FindFirstFileW(i_LPCWSTR_FolderPath, &data);
  if(handle == INVALID_HANDLE_VALUE) {
    auto result = ::CreateDirectoryW(i_LPCWSTR_FolderPath, NULL);
    if(result == 0) {
      throw std::runtime_error(std::format("Ошибка при создании папки код {0} : {1}", GetLastError(), GetLastErrorAsString()));
      //DS_ERROR("Ошибка при создании папки код {0} : {1}", GetLastError(), GetLastErrorAsString());
    }
  } else {
    FindClose(handle);
  }
}

