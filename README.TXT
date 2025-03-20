Super Turbo MEGA Pac-Man v2.12
Pac-Man для Sega Mega Drive / Sega Genesis

Игра для двух игроков, включая возможность игры по сети на двух приставках
через SEGA Link Cable (Zero Tolerance Link Cable), подключаемый во второй порт.

Посмотреть, что такое SEGA Link Cable на примере Zero Tolerance и Super Turbo MEGA Pac-Man, можно тут:
https://youtu.be/aLk8tDrjZ50

Я использовал SGDK версию 2.12 - свободный и открытый development kit для Sega Mega Drive / Sega Genesis
https://github.com/Stephane-D/SGDK.git

И SGDK_wine - Wine wrapper скрипт, генерирующий /opt/SGDK/makefile_wine.gen
для SGDK, чтобы использовать его в Linux
https://github.com/Franticware/SGDK_wine.git



Реализация SEGA Link Cable Protocol (LCP):
Для игры был реализован SEGA Link Cable Protocol (LCP) — протокол для обмена данными через SEGA Link Cable 
между двумя приставками Sega Mega Drive / Sega Genesis. Это библиотека на языке программирования C для SGDK.

Мой протокол был добавлен в виде библиотеки в проект SGDK версии 2.12. Поэтому исходный код link_cable.h и 
link_cable.c были удалены из английской версии (spacman-eng) данного проекта.

Важно для сборки английской версии: Чтобы собрать английскую версию, необходимо в проекте SGDK в файле 
inc/config.h установить константу MODULE_LINK_CABLE в значение 1 (по умолчанию LCP отключен). 
После этого нужно пересобрать проект SGDK, запустив build_lib.bat.

Для русской версии: В версии spacman-rus я оставил исходный код link_cable.h и link_cable.c, 
поэтому константа MODULE_LINK_CABLE в проекте SGDK 2.12 в inc/config.h должна быть равна 0. 
Пересобирать библиотеки SGDK для русской версии не нужно.

В проект SGDK 2.12 в качестве примера работы с протоколом LCP я добавил исходный код данной игры на английском языке (с небольшими изменениями). 
Он находится в папке SGDK/sample/linkcable/pacman. Отличие в том, что на заставке вместо "SEGA" будет отображаться "SGDK".



Описание игры:
1-й игрок управляет Pac-Man, за которым гоняется красный призрак.
Если призрак догонит Pac-Man — игра заканчивается.
Если Pac-Man или Pac-Girl съедят зеленую точку, призрак станет фиолетовым и начнет убегать, так как теперь его можно съесть.
2-й игрок управляет Pac-Girl. Она помогает собирать точки на уровне. При этом ни Pac-Man, ни призрак не могут съесть 
Pac-Girl (она бессмертна), и она не может съесть их.
Игра завершается победой, если игроки соберут всю еду (белые и зеленые точки) на уровне.
В доме призраков через некоторое время появляется вишня — за ее поедание просто начисляются очки; чтобы выиграть, есть ее не обязательно!
В меню выбора количества игроков можно выбрать, кто каким персонажем будет управлять. Для этого в меню '2 PLAYERS' нажмите вправо, 
чтобы первый игрок управлял Pac-Girl, а второй — Pac-Man, или влево, чтобы вернуть настройки по умолчанию.


Изменения:
- моя библиотека Link Cable Protocol (LCP) включена в проект SGDK 2.12
- исправлено что после смены персонажей если включить одиночную игру, управляем сразу и Pac-Man и Pac-Girl 
- добавлена пауза в игре при нажатии на Start
- возможность смены персонажей (нажать вправо в меню '2 PLAYERS')
- игра по сети на 2-х приставках через SEGA Link Cable
- вывод последней ошибки при передаче через SEGA Link Cable (A+C)
- вывод количества ошибок при передаче через SEGA Link Cable (B+C)
- вывод количества отрисованных фреймов с начала соединения (A+X)
- сброс информации для отладки (A+B)
- при движении вниз призрак не смотрит больше в сторону
- призрак не 'ест' черешню
- отправка нажатий на контроллерах через SEGA Link Cable только при реальном нажатии
- оставлены доступными только необходимые внешние функции в библиотеке link_cable.h
- отправка через SEGA Link Cable не всего состояния игры сразу, а только при наступлении какого-либо события

Папки:
spacman-eng - проект с интерфейсом на английском
spacman-rus - проект с интерфейсом на русском

Бинарники:
spacman-eng/spacman[ENG].bin - английская версия игры с комментариями к коду на английском языке
spacman-rus/spacman[RUS].bin - русская версия игры с комментариями к коду на русском языке

Библиотека с реализацией SEGA Link Cable Protocol (LCP) - протокол для обмена данными через SEGA Link Cable:
spacman-rus/src/link_cable.h - заголовочный файл библиотеки Link Cable Protocol (LCP)
                   протокол для обмена данными через SEGA Link Cable двух приставок
                   Sega Mega Drive / Sega Genesis,
                   который подключается во второй порт приставок (из английской версии удален!)
spacman-rus/src/link_cable.c - реализация протокола для обмена данными через SEGA Link Cable
                   двух приставок Sega Mega Drive / Sega Genesis (из английской версии удален!)


Остальные файлы относительно каталогов spacman-eng или spacman-rus

src - исходные коды
res - ресурсные файлы
out - результаты сборки

Файлы для сборки из консоли:
compile.sh - сборка под Linux! Запускает сборку через wine, использует SGDK
             у меня лежит тут /opt/SGDK
             нужно сгенерировать /opt/SGDK/makefile_wine.gen, чтобы собралось
             также нужна Java (у меня работало с OpenJDK 17)

Сборка под Linux:
> ./compile.sh

Собранный ROM игры будет тут:
[папка с проектом]/spacman/out/rom.bin

Как настроить среду разработки Eclipse и собрать проект из исходного кода
в Linux можно посмотреть тут:
https://youtu.be/-aXpth57_bk

в MacOS тут:
https://youtu.be/Y1cP78FogVw

в Windows тут:
https://youtu.be/pKn18b16gmI

Ресурсные файлы:
res/bg/sega.png - задний фон с логотипом SEGA (заставка)
res/bg/menu.png - задний фон с меню (выбор количества игроков и распределение персонажей)
res/bg/map.png  - задний фон с картой (стены, еда, усиления)

res/sound/cherry.wav - звук поедания вишни
res/sound/comicszone.vgm - музыка во время игры из Comix Zone
res/sound/connect.wav - звук при соединении приставок через Link Cable
res/sound/contrah.vgm - музыка во время выбора количества игроков из Contra - Hard Corps
res/sound/disconnect.wav - звук при отключении приставки от другой
res/sound/eat.wav - звук поедания точки
res/sound/eatred.wav - звук поедания призрака
res/sound/fatality.vgm - музыка при GAME OVER из Mortal Kombat 3
res/sound/powerup.wav - звук при поедании усиления
res/sound/sega.wav - звук SEGA на заставке
res/sound/shadow.wav - звук, когда призрак съедобен
res/sound/sonic.vgm - музыка на заставке из Sonic
res/sound/victory.vgm - музыка при победе из Contra - Hard Corps

res/sprite/cherry.png - спрайт черешни
res/sprite/door.png - спрайт двери
res/sprite/pacgirl.png - спрайты Pac-Girl
res/sprite/pacman.png - спрайты Pac-Man
res/sprite/pause.png - спрайт PAUSE
res/sprite/red.png - спрайт призрака RED
res/sprite/sonic.png - спрайты Соника для заставки

res/font.png - шрифты для русского языка

res/resources.res - описание импортируемых ресурсов

Исходный код:
src/main.c - код игры
src/main.h - заголовочный файл игры с глобальными переменными,
             константами, объявлением функций приложения
                
res/resources.h - заголовочный файл, описывающий импортируемые ресурсы (генерируется при сборке из res/resources.res)

BlodTor 12.11.2025
