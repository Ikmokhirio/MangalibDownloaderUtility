#include "Combiners/HtmlCombiner.h"
#include "Combiners/RawImageCombiner.h"
#include "MangalibSession.h"
#include <iostream>

int main()
{
  SetConsoleOutputCP(CP_UTF8);

  MangalibSession session;// Создаем сессию

  // Заходим в аккаунт
  if(!session.Login("login", "password")) {
    std::cerr << "Неправильный логин или пароль" << std::endl;
    return 0;
  }

  //Получаем и выводим список манги по поисковому запросу
  auto collection = session.Search("12");
  for(auto& manga: (std::vector<Manga>) collection) {
    std::cout << manga << std::endl;
  }

  // Допустим пользователь выбрал мангу
  auto selectedManga = collection.GetMangaById(24249);

  //Получаем и выводим список переводов
  session.GetTranslationList(selectedManga);
  for(auto& team: selectedManga.branches) {
    std::cout << "Перевод от : " << team << std::endl;
  }

  // Допустим человек выбирает конкретный перевод
  int selectedBranch = selectedManga.branches.size() > 0 ? selectedManga.branches[0].branch : 0;

  // Создаем сессию для загрузки и настраиваем как нам надо
  auto downloaderSession = session.StartDownloadSession(selectedManga);
  auto chapters = downloaderSession->Branch(selectedBranch)
                      .SkipPagesEnd(0)
                      .SkipPagesStart(0)
                      .ErrorSleep(1)
                      .MaxAttempts(5)
                      .RequestDelay(0)
                      .AddCombiner(new HtmlCombiner(60))
                      .AddCombiner(new RawCombiner())
                      .DownloadFolder("тестовая прогонка")
                      .ExtractChapters();// ExtractChapters должен быть последним в списке. У остальных порядок не важен

  for(int i = 0; i < 3; i++) {
    chapters[i].selected = true;
  }
  downloaderSession->DownloaderChapters(chapters);

  downloaderSession->DownloadChapter(4);// Пятая глава в списке

  return 0;
}
