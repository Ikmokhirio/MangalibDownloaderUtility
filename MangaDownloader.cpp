#include "MangaDownloader.h"
#include "UriParser.h"

std::string ExtractCookie(std::string header, std::string name)
{
  if(header.empty() || name.empty()) {
    return {};
  }
  std::string marker = std::format("{0}=", name);
  int start = header.find(marker);
  int end = header.find(";");

  return header.substr(start + marker.size(), end - start - marker.size());
}

std::vector<Chapter>& Downloader::ExtractChapters()
{
  auto res = cli.Get(currentManga.link);
  if(res.error() != httplib::Error::Success) {
    throw MangalibError(std::format("Ошибка при запросе : {0}", httplib::to_string(res.error())));
  }

  const std::string MARKER_START = "window.__DATA__ = ";
  const std::string MARKER_END = "window._SITE_COLOR_";// TODO : Script tag works better

  std::string body = res->body;

  int start = body.find(MARKER_START);
  if(start == body.npos) {
    throw MangalibError("Некорректный формат ответа");
  }

  int end = body.find(MARKER_END);
  if(end == body.npos) {
    throw MangalibError("Некорректный формат ответа");
  }

  jsonData = body.substr(start + MARKER_START.size(), end - (start + MARKER_END.size()));
  while(!jsonData.ends_with(';')) {
    jsonData.pop_back();
  }
  jsonData.pop_back();
  //

  auto jsonRes = nlohmann::json::parse(jsonData);

  const std::string MANGA_BLOCK = "manga";
  const std::string CHAPTERS = "chapters";
  const std::string BRANCHES = "branches";
  const std::string LIST = "list";

  if(!jsonRes.contains(MANGA_BLOCK)) {
    throw MangalibError("Не удалось найти блок \"manga\"");
  }

  if(!jsonRes.contains(CHAPTERS)) {
    throw MangalibError("Не удалось найти блок \"chapters\"");
  }

  if(!jsonRes[CHAPTERS].contains(LIST)) {
    throw MangalibError("Не удалось найти блок \"list\"");
  }

  auto chaptersList = jsonRes[CHAPTERS][LIST];

  for(auto it = chaptersList.rbegin(); it != chaptersList.rend(); ++it) {
    currentChapter = it.value();
    ProcessCurrentChapter();
  }

  return chapters;
}

void Downloader::ProcessCurrentChapter()
{
  std::string volumeNumber, chapterNumber, chapterId;
  try {
    volumeNumber = std::to_string(TryGetValue<int>(currentChapter, VOLUME));
    chapterNumber = TryGetValue<std::string>(currentChapter, CHAPTER_NUMBER);// Can be XX.YY
    chapterId = std::to_string(TryGetValue<int>(currentChapter, CHAPTER_ID));
  } catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
    return;
  }
  if(branchId != 0) {
    try {
      int currentBranch = TryGetValue<int>(currentChapter, BRANCH_ID);
      if(currentBranch != branchId) {
        return;
      }
    } catch(MangalibError& e) {
      return;
    } catch(std::exception& e) {
      return;
    }
  }
  chapters.emplace_back(Chapter(chapterId, volumeNumber, chapterNumber, branchId));
}

std::wstring ExecutablePath()
{
  WCHAR buffer[MAX_PATH] = {0};
  GetModuleFileNameW(NULL, buffer, MAX_PATH);
  std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
  return std::wstring(buffer).substr(0, pos);
}

void Downloader::DownloadChapter(int chapterNumber)
{
  std::wstring path = ExecutablePath();
  path.append(L"\\");
  path.append(downloadFolder);
  CreateDirectoryWithChecking(path.c_str());

  ML_INFO("Цель : Том {0} глава {1}", chapters[chapterNumber].volumeNumber, chapters[chapterNumber].chapterNumber);
  auto res = cli.Get(std::format("/download/{0}", chapters[chapterNumber].chapterId));
  xsrfToken = ExtractCookie(res->get_header_value(SET_COOKIE, 0), "XSRF-TOKEN");
  // ML_INFO("XSRF-TOKEN : {0}", xsrfToken);

  httplib::Headers headers = {
      {"Accept-Encoding", "gzip, deflate"},
      {"Cookie", std::format("mangalib_session={0};XSRF-TOKEN={1}", cookie, xsrfToken)},
      {"Origin", "https://mangalib.me"},
      {"Referer", "https://mangalib.me/"},
      {"Sec-Fetch-Dest", "empty"},
      {"Sec-Fetch-Mode", "cors"},
      {"Sec-Fetch-Site", "cross-site"}};

  auto downloadData = res->body;
  ML_INFO("Получение ссылок на картинки");
  auto downloadDataJson = nlohmann::json::parse(downloadData);

  auto uri = Uri::Parse(currentManga.link);

  std::string server = TryGetValue<std::string>(downloadDataJson, DOWNLOAD_SERVER);
  httplib::Client pictureDownloader(server);
  pictureDownloader.set_connection_timeout(std::chrono::seconds(5));
  pictureDownloader.set_read_timeout(std::chrono::seconds(5));
  pictureDownloader.set_write_timeout(std::chrono::seconds(5));
  //ML_DEBUG("Сервер для скачки : {0}", server);

  auto images = TryGetValue<nlohmann::json>(downloadDataJson, IMAGES);
  auto chapterStruct = TryGetValue<nlohmann::json>(downloadDataJson, CHAPTER);
  std::string chapterId = TryGetValue<std::string>(chapterStruct, SLUG);
  int attemptCount = 0;

  int listNumber = 1;
  for(auto& img: images) {

    // Skip last pages
    if(images.size() - (listNumber - 1) <= skipCountEnd) {
      listNumber++;
      continue;
    }
    //Skip first pages
    if(listNumber - 1 < skipCountStart) {
      listNumber++;
      continue;
    }

    attemptCount = 0;

    for(auto combiner: combiners) {
      combiner->Start();
    }

    while(attemptCount < maxAttempt) {
      attemptCount++;

      httplib::Result file = pictureDownloader.Get(std::format("/manga{0}/chapters/{1}/{2}", uri.Path, chapterId, img.get<std::string>()), headers);

      if(!file) {
        ML_ERROR("Сервер ничего не вернул");
        ML_ERROR("Повторная попытка после паузы в 250мс");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        continue;
      }
      if(file.error() != httplib::Error::Success) {
        ML_ERROR("Ошибка при загрузке : {0}", httplib::to_string(file.error()));
        ML_ERROR("Повторная попытка после паузы в {0}с", errorSleepTime);
        std::this_thread::sleep_for(std::chrono::seconds(errorSleepTime));
        continue;
      }
      if(file->status != 200) {
        ML_ERROR("Ошибка {0} | Тело ответа : {1}", file->status, file->body);
        ML_ERROR("Повторная попытка после паузы в {0}с", errorSleepTime);
        std::this_thread::sleep_for(std::chrono::seconds(errorSleepTime));
        continue;
      }
      antiDDOSToken = ExtractCookie(file->get_header_value(SET_COOKIE, 0), "__ddg1_");
      if(!antiDDOSToken.empty()) {// New token
        headers = {
            {"Accept-Encoding", "gzip, deflate"},
            {"Cookie", std::format("mangalib_session={0};XSRF-TOKEN={1};__ddg1_={2}", cookie, xsrfToken, antiDDOSToken)},
            {"Origin", "https://mangalib.me"},
            {"Referer", "https://mangalib.me/"},
            {"Sec-Fetch-Dest", "empty"},
            {"Sec-Fetch-Mode", "cors"},
            {"Sec-Fetch-Site", "cross-site"}};
      }

      std::wostringstream ss;
      //ss << "./" << mangaName << "/" << Converter::ToWString(img.get<std::string>());
      std::filesystem::path downloadFile = img.get<std::string>();
      ss << "./" << downloadFolder << "/vol" << Converter::ToWString(chapters[chapterNumber].volumeNumber) << "_ch" << Converter::ToWString(chapters[chapterNumber].chapterNumber) << "_p" << listNumber << Converter::ToWString(downloadFile.extension().string());
      //ML_INFO("{0}", Converter::ToString(ss.str()));
      for(auto combiner: combiners) {
        combiner->AddFile(file->body, ss.str());
      }
      break;
    }
    if(attemptCount >= maxAttempt) {
      chapters[chapterNumber].errorOnLastOperation = true;
      throw MangalibError(std::format("Не получилось загрузить после {0} попыток", maxAttempt));
    }
    listNumber++;
  }

  std::wostringstream ss;
  std::wstring outputPath;
  ss << "./" << downloadFolder << "/vol_" << Converter::ToWString(chapters[chapterNumber].volumeNumber) << "_ch_" << Converter::ToWString(chapters[chapterNumber].chapterNumber);
  //std::string outputPath = std::format("./{0}/vol_{1}_ch_{2}", mangaName, chapter.volumeNumber, chapter.chapterNumber);

  std::string previousChapter;
  if(chapterNumber > 0) {
    previousChapter = std::format("vol_{0}_ch_{1}", chapters[chapterNumber - 1].volumeNumber, chapters[chapterNumber - 1].chapterNumber);
  }
  std::string nextChapter;
  if(chapterNumber < chapters.size()) {
    nextChapter = std::format("vol_{0}_ch_{1}", chapters[chapterNumber + 1].volumeNumber, chapters[chapterNumber + 1].chapterNumber);
  }

  for(auto combiner: combiners) {
    combiner->SaveTo(ss.str(), previousChapter, nextChapter);
    combiner->End();
  }
  chapters[chapterNumber].finished = true;
  ML_INFO("Успешно том {0} глава {1}", chapters[chapterNumber].volumeNumber, chapters[chapterNumber].chapterNumber);
}

void Downloader::DownloaderChapters(std::vector<Chapter>& chapters)
{
  for(int i = 0; i < chapters.size(); i++) {
    if(chapters[i].selected) {
      DownloadChapter(i);
    }
  }
}
