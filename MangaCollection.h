#ifndef MANGACOLLECTION_H
#define MANGACOLLECTION_H

#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Combiners/Utils.h"

#include "Error.h"

struct Chapter {
  std::string chapterId;
  std::string volumeNumber;
  std::string chapterNumber;
  int branchNumber;

  bool selected;
  bool finished;
  bool loading;
  bool errorOnLastOperation;

  Chapter(const std::string& id, const std::string& v, const std::string& n, const int branch = 0)
  {
    chapterId = id;
    volumeNumber = v;
    chapterNumber = n;
    branchNumber = branch;

    selected = false;
    errorOnLastOperation = false;
    finished = false;
    loading = false;
  }
};

struct Team {
  std::string name;
  int branch;

  friend std::ostream& operator<<(std::ostream& os, const Team& m);

  Team(const std::string& teamName, const int& branchNumber)
  {
    branch = branchNumber;
    name = teamName;
  }
};

struct Manga {
  uint64_t id;
  std::string mangaName;
  std::string originalName;
  std::string russianName;
  std::string englishName;
  std::string summary;
  std::string cover;
  std::string thumbnail;
  std::string link;
  std::string thumbnailPath;
  std::vector<Team> branches;

  bool empty;

  friend std::ostream& operator<<(std::ostream& os, const Manga& m);

  Manga()
  {
    empty = true;
  }
};

class MangaCollection
{
public:
  MangaCollection(std::string body);

  inline operator std::vector<Manga>&()
  {
    return manga;
  }

  inline Manga& GetMangaById(uint64_t id)
  {
    for(auto& m: manga) {
      if(m.id == id) {
        return m;
      }
    }
    return emptyManga;// Empty Manga instance
  }

private:
  int current;
  std::vector<Manga> manga;
  Manga emptyManga;
};

#endif// MANGACOLLECTION_H
