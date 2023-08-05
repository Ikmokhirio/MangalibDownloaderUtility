#include "Combiners/HtmlCombiner.h"
#include "Combiners/RawImageCombiner.h"
#include "MangalibSession.h"
#include <iostream>

int main()
{
  SetConsoleOutputCP(CP_UTF8);

  MangalibSession session;// Создаем сессию

  // Заходим в аккаунт
  session.Login("login", "password");

  //Получаем и выводим список манги по поисковому запросу
  auto collection = session.Search("То, что я не знаю о тебе");
  for(auto& manga: (std::vector<Manga>) collection) {
    std::cout << manga << std::endl;
  }

  // Допустим пользователь выбрал мангу
  auto selectedManga = collection.GetMangaById(78314);

  //Получаем и выводим список переводов
  session.GetTranslationList(selectedManga);
  for(auto& team: selectedManga.branches) {
    std::cout << "Перевод от : " << team << std::endl;
  }

  // Допустим человек выбирает конкретный перевод
  auto selectedBranch = selectedManga.branches[1].branch;

  // Создаем сессию для загрузки и настраиваем как нам надо
  auto downloaderSession = session.StartDownloadSession(selectedManga);
  auto chapters = downloaderSession.Branch(selectedBranch)
                      .SkipPagesEnd(0)
                      .SkipPagesStart(0)
                      .ErrorSleep(1)
                      .MaxAttempts(5)
                      .RequestDelay(0)
                      .AddCombiner(new HtmlCombiner(60))
                      .AddCombiner(new RawCombiner())
                      .DownloadFolder("тестовая прогонка")
                      .ExtractChapters();// ExtractChapters должен быть последним в списке. У остальных порядок не важен

  downloaderSession.DownloadChapter(0);// Первая глава в списке
  downloaderSession.DownloadChapter(1);// Вторая глава в списке
  downloaderSession.DownloadChapter(2);// ...

  return 0;
}
