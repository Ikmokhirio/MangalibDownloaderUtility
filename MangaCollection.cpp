#include "MangaCollection.h"

std::ostream& operator<<(std::ostream& os, const Manga& manga)
{
  os << "ID : " << manga.id << "\n"
     << "ИМЯ : " << manga.originalName << "\n"
     << "РУССКОЕ ИМЯ : " << manga.russianName << "\n"
     << "ССЫЛКА : " << manga.link << "\n"
     << "ОБЛОЖКА : " << manga.thumbnail << "\n"
     << "\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Team& m)
{
  os << "TEAM : " << m.name << "\n"
     << "ID : " << m.branch << "\n"
     << "\n";
  return os;
}

MangaCollection::MangaCollection(std::string body)
{
  auto jsonData = nlohmann::json::parse(body);

  manga = std::vector<Manga>();

  for(auto& data: jsonData) {
    manga.emplace_back();

    manga.back().id = TryGetValue<uint64_t>(data, "id");
    manga.back().mangaName = TryGetValue<std::string>(data, "eng_name");

    manga.back().originalName = TryGetValue<std::string>(data, "name");
    manga.back().englishName = TryGetValue<std::string>(data, "eng_name");
    manga.back().russianName = TryGetValue<std::string>(data, "rus_name");

    manga.back().summary = TryGetValue<std::string>(data, "summary");

    manga.back().cover = TryGetValue<std::string>(data, "coverImage");
    manga.back().thumbnail = TryGetValue<std::string>(data, "coverImageThumbnail");

    manga.back().link = TryGetValue<std::string>(data, "href");
    // DS_INFO("{0}", Uri::Parse(manga.back().thumbnail).Path);

    manga.back().empty = false;
  }
}
