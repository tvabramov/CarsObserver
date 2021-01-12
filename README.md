# CarsObserver

## Описание проекта
CarsObserver является прикладным программным продуктом, предназначенным для распознавания в видеопотоке различных предметов, транспортных средств, животных, людей, их трекинга и подсчета проходимости через указанную границу.

## Технологии, библиотеки и зависимости
Проект написан на C++14. Для выполнения задач распознавания, трекинга, аттрибутирования и кластеризации используются библиотеки OpenCV, работающие с предобученными нейронными сетями (находятся в папке models, для скачивания потребуется [git lfs](https://git-lfs.github.com/)).
Также CarsObserver использует библиотеки Boost и nlohmann_json.

## Сборка под Linux
1. Установка библиотек, необходимых для OpenCV:
```bash
user@user:~$ sudo apt install libavutil-dev libavcodec-dev libavfilter-dev \
  libavformat-dev libavdevice-dev pkg-config libgtk2.0-dev \
  libgstreamer1.0-0 gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc \
  gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl \
  gstreamer1.0-gtk3 gstreamer1.0-pulseaudio libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev build-essential ffmpeg
```

2. Сборка OpenCV из исходных кодов:
```bash
# Скачиваем opencv и opencv_contrib из репозиториев:
user@user:~$ git clone https://github.com/opencv/opencv
user@user:~$ git clone https://github.com/opencv/opencv_contrib
# Переходим на нужную версию, от 4.1.0:
user@user:~$ cd opencv_contrib
user@user:~/opencv_contrib$ git checkout 4.2.0
user@user:~/opencv_contrib$ cd ../opencv
user@user:~/opencv$ git checkout 4.2.0
# Собираем
user@user:~/opencv$ mkdir build && cd build
user@user:~/opencv/build$ cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D WITH_TBB=ON -D WITH_V4L=ON -D WITH_OPENGL=ON \
  -D WITH_GSTREAMER=ON -D WITH_FFMPEG=ON \
  -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
  ..
user@user:~/opencv/build$ make
user@user:~/opencv/build$ sudo make install
```

3. Boost:
```bash
user@user:~$ sudo apt install libboost-dev
user@user:~$ sudo apt install libboost-all-dev
```

4. nlohmann_json:
```bash
# Скачиваем nlohmann_json из репозиториев
user@user:~$ git clone https://github.com/nlohmann/json
user@user:~$ cd json
# Собираем
user@user:~/json$ mkdir build && cd build
user@user:~/json/build$ cmake ..
user@user:~/json/build$ make
user@user:~/json/build$ sudo make install
```

5. CarsObserver:
```bash
# Скачиваем CarsObserver из репозитория
user@user:~$ git clone https://github.com/tvabramov/CarsObserver.git
user@user:~$ cd CarsObserver
# Собираем
user@user:~/CarsObserver$ mkdir build && cd build
user@user:~/CarsObserver/build$ cmake -D CMAKE_INSTALL_PREFIX=./ .. 
user@user:~/CarsObserver/build$ make install
```

## Сборка под Windows
Происходит аналогично сборке под Linux. Рекомендуется использовать тулчейн [mingw-w64](http://mingw-w64.org/doku.php), выбирать сборку с posix потоками, например "x86_64-8.1.0-posix-seh-rt_v6-rev0".

1. Сборка OpenCV из исходных кодов:
```cmd
C:\Users\User>git clone https://github.com/opencv/opencv
C:\Users\User>git clone https://github.com/opencv/opencv_contrib
C:\Users\User>cd opencv_contrib
C:\Users\User\opencv_contrib>git checkout 4.2.0
C:\Users\User\opencv_contrib>cd ../opencv
C:\Users\User\opencv>git checkout 4.2.0
C:\Users\User\opencv>mkdir build && cd build
C:\Users\User\opencv\build>cmake -G "MinGW Makefiles" \
  -D CMAKE_BUILD_TYPE=RELEASE \
  -D WITH_TBB=ON -D WITH_V4L=ON -D WITH_OPENGL=ON \
  -D WITH_GSTREAMER=ON -D WITH_FFMPEG=ON \
  -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
  ..
C:\Users\User\opencv\build>mingw32-make install
```

2. Собираем Boost:
Скачиваем исходники (версии не ниже 1.65) и собираем с библиотеками "program_options" и "log":
```cmd
C:\Users\User\boost_1_72_0>bootstrap.bat gcc
C:\Users\User\boost_1_72_0>b2 toolset=gcc link=shared ^
  --prefix=C:/boost --with-program_options ^
  --with-log install
```
После сборки может потребоваться переименовать dll библиотеки следующим образом:
libboost_program_options-mgw81-mt-x64-1_70.dll -> libboost_program_options.dll.

3. nlohmann_json:
```cmd
C:\Users\User>git clone https://github.com/nlohmann/json
C:\Users\User>cd json
C:\Users\User\json>
C:\Users\User\json>mkdir build && cd build
C:\Users\User\json\build>cmake -G "MinGW Makefiles" ^
  -DCMAKE_INSTALL_PREFIX="C:/nlohmann_json" ..
C:\Users\User\json\build>mingw32-make install
```

4. CarsObserver:
```cmd
C:\Users\User>git clone https://github.com/tvabramov/CarsObserver.git
C:\Users\User>cd CarsObserver
C:\Users\User\CarsObserver>mkdir build && cd build
C:\Users\User\CarsObserver\build>cmake -G "MinGW Makefiles" ^
  -D "OpenCV_DIR"="C:/OpenCV/install" ^
  -D "nlohmann_json_DIR"="C:\nlohmann_json\lib\cmake\nlohmann_json" ^
  -DCMAKE_INSTALL_PREFIX=./ ^
  ..
C:\Users\User\CarsObserver\build>mingw32-make install
```

## Использование
CarsObserver является CLI (интерфейс командной строки) утилитой с возможностью вывода видопотока с распознанными в нем объектами, треками и другой информацией в отдельное графическое окно. Для вызова справки:
```bash
user@user:~$ ./CarsObserver -h
Command line options:
  -h [ --help ]         Show help
  -c [ --config ] arg   JSON config file name
```
Запуск с ключом `-h` или `--help` означает вывод справки по допустимым аргументам командной строки. Как видно, кроме `-h` есть только один ключ `-c` (`--config`). С помощью него указывается путь к JSON файлу конфигурации, содержащий **все** необходимые настройки для работы CarsObserver. Таким образом, запуск в большинстве случаев осуществляется командой вида:
```bash
user@user:~$ ./CarsObserver -c /path/to/custom/config/myconf.json
```

## Автор
* **Тимофей Абрамов** - *[timohamail@inbox.ru](mailto://timohamail@inbox.ru)*.
