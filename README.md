# MangalibDownloaderUtility

Утилита для скачивания манги с мангалиба с открытым исходым кодом
Текущая версия : 0.0.1

## Установка

1) Склонировать
```
git clone --recurse-submodules
```

2) Билдим при помощи cmake
```
cmake -DCMAKE_BUILD_TYPE=Release -S . -B Release
или
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B Debug

потом 
cmake --build Debug
или
cmake --build Release
```

3) Взять сбилженые бинари для openssl и закинуть в папку билда Debug или Release к экзешнику


