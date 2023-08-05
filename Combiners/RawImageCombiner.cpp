#include "RawImageCombiner.h"

RawCombiner::RawCombiner() {}

void RawCombiner::AddFile(const std::string& file, const std::wstring& name)
{
  std::ofstream out;
  out.open(name, std::ios::binary);
  if(!out) {
    throw MangalibError(std::format("{0} Ошибка : {1}", Converter::ToString(name), GetLastErrorAsString()));
  }
  out << file;
  out.close();
}

void RawCombiner::SaveTo(const std::wstring& path, const std::string& prev, const std::string& next)
{
}

