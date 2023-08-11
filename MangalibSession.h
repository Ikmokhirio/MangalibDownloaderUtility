#ifndef MANGALIBUTILITY_SESSION_H
#define MANGALIBUTILITY_SESSION_H

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <format>
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "MangaCollection.h"

#include "MangaDownloader.h"

#include "Error.h"

class MangalibSession {
private:
  const std::string LIBSOCIAL = "https://lib.social";
  const std::string MANGALIB = "https://mangalib.me";
  const std::string CSRF_TOKEN = "csrfToken";
  const std::string SET_COOKIE = "set-cookie";
  const std::string INCORRECT_LOGIN_FLAG = "Redirecting to http://lib.social/login";

  httplib::Client mangalibCli;
  httplib::Client libSocialCli;

  std::string authCookie;
  std::string jsonData;

public:
  inline MangalibSession()
      : mangalibCli(MANGALIB)
      , libSocialCli(LIBSOCIAL)
  {}

  /*
  * Осуществляет вход в аккаунте true - успешно, false - непраильный логин/пароль
  * В остальных случаях выбрасывается MangalibError
  */
  bool Login(std::string username, std::string password);

  /*
  * Осуществляет поиск по сайту и возвращает список манги в виде MangaCollection
  * В остальных случаях выбрасывается MangalibError
  */
  MangaCollection Search(std::string query);

  /*
  * Получает все варианты перевода и добавляет их в объект манги
  * В остальных случаях выбрасывается MangalibError
  */
  void GetTranslationList(Manga& manga);

  /*
  * Получает превью манги в виде последовательности байт
  */
  void GetThumbnail(Manga& manga);

  /*
  * Возвращает объекта класса Downloader с установленной сессией
  */
  Downloader* StartDownloadSession(Manga& manga);
};

#endif// MANGALIBUTILITY_SESSION_H
