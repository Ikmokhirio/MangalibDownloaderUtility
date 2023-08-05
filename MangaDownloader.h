#ifndef MANGADOWNLOADER_H
#define MANGADOWNLOADER_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <format>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <thread>

#include "Error.h"

#include "Combiners/CombinerBase.h"
#include "MangaCollection.h"

class Downloader
{
public:
  inline Downloader(httplib::Client& cli, Manga& manga, std::string c)
      : cookie(c)
      , cli(cli)
      , currentManga(manga)
  {
    cli.set_default_headers({{"Cookie", std::format("mangalib_session={0}", cookie)}, {"Origin", "https://mangalib.me"}, {"Referer", "https://mangalib.me/"}});

    cli.set_connection_timeout(std::chrono::seconds(5));
    cli.set_read_timeout(std::chrono::seconds(5));
    cli.set_write_timeout(std::chrono::seconds(5));

    // По-умолчанию папка = имя манги
    downloadFolder = Converter::ToWString(currentManga.mangaName);
  }

  inline Downloader& Branch(int id)
  {
    branchId = id;
    return *this;
  }

  inline Downloader& ErrorSleep(int errorSleep)
  {
    errorSleepTime = errorSleep;
    return *this;
  }

  inline Downloader& RequestDelay(int requestDelay)
  {
    requestDelayMs = requestDelay;
    return *this;
  }

  inline Downloader& SkipPagesStart(int skip)
  {
    skipCountStart = skip;
    return *this;
  }

  inline Downloader& SkipPagesEnd(int skip)
  {
    skipCountEnd = skip;
    return *this;
  }

  inline Downloader& AddCombiner(Combiner* comb)
  {
    combiners.push_back(comb);
    return *this;
  }

  inline Downloader& MaxAttempts(int a)
  {
    maxAttempt = a;
    return *this;
  }

  inline Downloader& DownloadFolder(const std::string &s)
  {
    downloadFolder = Converter::ToWString(s);
    return *this;
  }

  std::vector<Chapter>& ExtractChapters();

  // Скачивает главу, используя установленные параметры
  void DownloadChapter(int chapterId);

private:
  const std::string ENG_NAME = "engName";
  const std::string RUS_NAME = "rusName";
  const std::string ORIG_NAME = "name";

  const std::string VOLUME = "chapter_volume";
  const std::string CHAPTER_NUMBER = "chapter_number";
  const std::string CHAPTER = "chapter";
  const std::string CHAPTER_ID = "chapter_id";
  const std::string SLUG = "slug";
  const std::string IMAGES = "images";
  const std::string DOWNLOAD_SERVER = "downloadServer";
  const std::string BRANCH_ID = "branch_id";
  const std::string SET_COOKIE = "set-cookie";

  std::string cookie;

  httplib::Client& cli;
  Manga& currentManga;

  std::vector<Combiner*> combiners;
  std::vector<Chapter> chapters;

  std::string jsonData;
  std::string xsrfToken;
  std::string antiDDOSToken;

  nlohmann::json currentChapter;

  int branchId;

  int maxAttempt = 1;
  int errorSleepTime = 0;
  int requestDelayMs = 0;
  int skipCountStart = 0;
  int skipCountEnd = 0;

  std::wstring downloadFolder;

  void ProcessCurrentChapter();
};

#endif// MANGADOWNLOADER_H
