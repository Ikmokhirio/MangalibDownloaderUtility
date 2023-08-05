#include "MangalibSession.h"

std::string GetCookie(std::string header, std::string name)
{
  std::string marker = std::format("{0}=", name);
  int start = header.find(marker);
  int end = header.find(";");

  return header.substr(start + marker.size(), end - start - marker.size());
}

std::string GetToken(std::string body)
{
  const std::string MARKER_START = "window._PushData";
  const std::string MARKER_END = "</script>";
  const std::string CSRF_TOKEN = "csrfToken";

  int start = body.find(MARKER_START);
  if(start == body.npos) {
    throw std::runtime_error("Ошибка логина");
  }
  start += 3;

  int end = body.find(MARKER_END);
  if(end == body.npos) {
    throw std::runtime_error("Ошибка логина");
  }

  std::string jsonData = body.substr(start + MARKER_START.size(), end - (start + MARKER_END.size()));
  while(!jsonData.ends_with(';')) {
    jsonData.pop_back();
  }
  jsonData.pop_back();

  auto parsedJson = nlohmann::json::parse(jsonData);
  auto token = parsedJson[CSRF_TOKEN];

  return token;
}

bool MangalibSession::Login(std::string username, std::string password)
{
  auto res = libSocialCli.Get("/login");

  if(!res) {
    throw MangalibError("Нет интернет соединения");
  }

  if(res->status != 200) {
    throw MangalibError(std::format("Сервер вернул {0} на запрос аутентификации", res->status));
  }

  auto cookieHeader = res->get_header_value(SET_COOKIE);
  std::string xsrfToken = GetCookie(cookieHeader, "XSRF-TOKEN");

  cookieHeader = res->get_header_value(SET_COOKIE, 1);
  std::string mangalibSession = GetCookie(cookieHeader, "mangalib_session");

  libSocialCli.set_default_headers({{"Cookie", std::format("mangalib_session={0}; XSRF-TOKEN={1}", mangalibSession, xsrfToken)}});

  std::string token = GetToken(res->body);

  httplib::Params params{
      {"_token", token},
      {"from", "https://mangalib.me/?section=home-updates"},
      {"email", username},
      {"password", password},
      {"remember", "on"}};
  res = libSocialCli.Post("/login", params);

  authCookie = GetCookie(res->get_header_value(SET_COOKIE, 1), "mangalib_session");
  mangalibCli.set_default_headers({{"Cookie", std::format("mangalib_session={0}; XSRF-TOKEN={1}", mangalibSession, xsrfToken)}});

  return res->body.find(INCORRECT_LOGIN_FLAG) == res->body.npos;
}

MangaCollection MangalibSession::Search(std::string query)
{
  auto res = mangalibCli.Get(std::format("/search?type=manga&q={0}", query));

  if(!res) {
    throw MangalibError("Нет интернет соединения");
  }

  if(res.error() != httplib::Error::Success) {
    throw MangalibError(std::format("Ошибка при запросе : {0}", httplib::to_string(res.error())));
  }

  return MangaCollection(res->body);
}

void MangalibSession::GetTranslationList(Manga& manga)
{
  auto res = mangalibCli.Get(manga.link);
  if(res.error() != httplib::Error::Success) {
    throw MangalibError(std::format("Ошибка при запросе : {0}", httplib::to_string(res.error())));
  }

  const std::string MARKER_START = "window.__DATA__ = ";
  const std::string MARKER_END = "window._SITE_COLOR_";// TODO : Script tag looks better

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

  if(jsonData.empty()) {
    throw MangalibError("Нет json данных");
  }

  manga.branches = std::vector<Team>();

  auto translationRes = nlohmann::json::parse(jsonData);

  const std::string MANGA_BLOCK = "manga";
  const std::string CHAPTERS = "chapters";
  const std::string BRANCHES = "branches";
  const std::string TEAMS = "teams";
  const std::string NAME = "name";

  if(!translationRes.contains(MANGA_BLOCK)) {
    throw MangalibError("Не удалось найти блок \"manga\"");
  }

  if(!translationRes.contains(CHAPTERS)) {
    throw MangalibError("Не удалось найти блок \"chapters\"");
  }

  if(!translationRes[CHAPTERS].contains(BRANCHES)) {
    throw MangalibError("Не удалось найти блок \"branches\"");
  }

  auto branchesList = translationRes[CHAPTERS][BRANCHES];

  for(auto& branch: branchesList) {
    std::stringstream ss;
    for(auto& team: branch[TEAMS]) {
      ss << team[NAME] << " ";
    }
    manga.branches.push_back(Team(ss.str(), branch["id"]));
  }
}

Downloader MangalibSession::StartDownloadSession(Manga& manga)
{
  return Downloader(mangalibCli, manga, authCookie);
}
