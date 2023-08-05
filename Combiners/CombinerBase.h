#ifndef COMBINER_H
#define COMBINER_H

#include <Base64.hpp>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "Utils.h"

#include "../Error.h"

class Combiner
{
private:
public:
  Combiner() = default;

  // Вызывается до начала скачивания первой картинки
  virtual void Start() {}
  
  // Вызывается для каждого файла (картинки), скаченного с мангалиба
  virtual void AddFile(const std::string& file, const std::wstring& name = L"") = 0;

  // Вызывается после последнего AddFile (при завершении скачивания главы)
  virtual void SaveTo(const std::wstring& path, const std::string& prev, const std::string& next) = 0;
 
  // Вызывается после SaveTo
  virtual void End() {}
};

#endif// COMBINER_H
