Super Turbo MEGA Pac-Man v1.0 - Pac-Man для игры в 2м под Sega Mega Drive / Sega Genesis

папки:
src - тут исходники
res - тут ресурсные файлы
out - тут результаты сборки

файлы для сборки из консоли: 
compile.sh - сборка из под Linux! запускает сборку через wine использует SGDK
			 у меня лежит тут /opt/SGDK
			 нужно сгенерировать /opt/SGDK/makefile_wine.gen чтоб собралось
			 также нужна java (у меня работало с OpenJDK 17)

сборка из под Linux			 
> ./compile.sh			 
					 
собирать с помощью Docker (в папке /opt/SGDK должен лежать development kit для Sega Mega Drive / Sega Genesis)
> cd /opt/SGDK
> docker build -t sgdk .
> cd [папка где лежит проект]/spacman
> docker run --rm -v "$PWD":/src sgdk

собранный ROM игры будет тут
[папка где лежит проект]/spacman/out/rom.bin

ресурсные файлы:
res/bg/sega.png - задний фон с логотипом SEGA (заставка)
res/bg/menu.png - задний фон с меню (выбор количества игроков)
res/bg/map.png - задний фон с картой (стены, еда, поверапы)

res/sound/cherry.wav - звук поедания вишни
res/sound/comicszone.vgm - музыка во время игры из Comix Zone
res/sound/contrah.vgm - музыка во время выбора количества игроков из Contra - Hard Corps 
res/sound/eat.wav - звук поедания точки
res/sound/eatred.wav - звук поедания призрака
res/sound/fatality.vgm - музыка при GAME OVER из Mortal Kombat 3
res/sound/powerup.wav - звук при поедании поверапа
res/sound/sega.wav - звук SEGA на заставке
res/sound/shadow.wav - звук когда призрак съедобен
res/sound/sonic.vgm - музыка на заставке из Sonic
res/sound/victory.vgm - музыка при победе из Contra - Hard Corps 

res/sprite/cherry.png - спрайт черешни
res/sprite/door.png - спрайт двери
res/sprite/pacgirl.png - спрайты Pac-Girl
res/sprite/pacman.png - спрайты Pac-Man
res/sprite/red.png - спрайт призрака RED
res/sprite/sonic.png - спрайты Соника для заставки

res/resources.res - как импортировать ресурсы

исходный код:
src/main.c - код игры
src/main.h - заголовочный файл игры с глобальными переменными, константами, объявлением функций приложения
res/resources.h - заголовочный файл описывающий импортируемые ресурсы

Описание игры:
1 игрок управляет Pac-Man за которым гоняется 1 призрак красного цвета, 
в случае если призрак догонит Pac-Man - игра закончится.
в случае если Pac-Man или Pac-Girl съедят большую зеленую точку, призрак станет 
фиолетового цвета и начнет убегать от Pac-Man т.к. теперь его самого можно съесть
2 игрок управляет Pac-Girl. Она помогает собирать точки на уровне при этом ни Pac-Man ни 
призрак не могут съесть Pac-Girl а она их (бессмертная) 
Игра закончится победой если игроки смогут собрать всю еду (маленькие и большие точки) на уровне
В доме призраков через некоторое время появляется вишня - за нее просто дают очки 
чтоб выиграть есть не обязательно ее!

SGDK - свободный и открытый development kit для Sega Mega Drive / Sega Genesis (Я использовал версию 1.90) 
https://github.com/Stephane-D/SGDK.git

SGDK_wine - Wine wrapper скрипт генерирующий /opt/SGDK/makefile_wine.gen для SGDK чтоб использовать его в Linux
https://github.com/Franticware/SGDK_wine.git

Gens - это Sega Mega Drive / Sega Genesis / Sega CD / Sega 32X эмулятор для Windows и Linux.
http://www.gens.me/

spacman.bin - ROM игры Super Turbo MEGA Pac-Man для Sega Mega Drive / Sega Genesis

BlodTor 30.10.2023