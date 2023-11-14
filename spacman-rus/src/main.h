/**
 * Super Turbo MEGA Pac-Man 1.0 для Sega Mega Drive / Sega Genesis
 *
 * сборка из под Linux
 * > ./compile.sh
 *
 * собирать с помащью Docker (в папке /opt/SGDK должен лежать development kit для Sega Mega Drive / Sega Genesis)
 * > cd /opt/SGDK
 * > docker build -t sgdk .
 * > cd [папка где лежит проект]/spacman
 * > docker run --rm -v "$PWD":/src sgdk
 *
 * собранный ROM игры будет тут:
 * [папка где лежит проект]/spacman/out/rom.bin
 *
 * SGDK - свободный и открытый development kit для Sega Mega Drive / Sega Genesis (Я использовал версию 1.90)
 *	https://github.com/Stephane-D/SGDK.git
 *
 * SGDK_wine - Wine wrapper скрипт генерирующий /opt/SGDK/makefile_wine.gen для SGDK чтоб использовать его в Linux
 * https://github.com/Franticware/SGDK_wine.git
 *
 * Gens - это Sega Mega Drive / Sega Genesis / Sega CD / Sega 32X эмулятор для Windows и Linux.
 * http://www.gens.me/
 *
 * Разрешение у Sega Genesis / Sega Megadrive - 320 x 240
 *
 */

/**
 * Константы 
 */


// стартовый экран - выбор количества игроков
const unsigned char STATE_SELECT = 0;

// игра - второй экран после выбора количества игроков
const unsigned char STATE_GAME  = 1;

// третий экран, идет после игры - результат игры
const unsigned char STATE_RESULT = 2;

// заставка
const unsigned char STATE_SCREENSAVER = 3;

// Еда
const unsigned char FOOD = '.';

// Поверап позваляющий есть призраков
const unsigned char POWER_FOOD = '*';

// Дверь в комнату призраков
const unsigned char DOOR = '-';

// Ни кем не занятая клетка
const unsigned char EMPTY = ' ';

// Pac-Man
const unsigned char PACMAN = 'O';

// Pac Girl
const unsigned char PACGIRL ='Q';

// Красный призрак (BLINKY)
const unsigned char RED = '^';

// Красный призрак когда его можно съесть (SHADOW или BLINKY)
const unsigned char SHADOW = '@';

// Вишня
const unsigned char CHERRY = '%';

// первый символ русских содержащий Русские буквы в виде Tile в VRAM
const u16 FIRST_INDEX_RUSSIAN_TILE = 1300;

// скорость перехода pacman с одной клетки на другую в милисекундах
#define PACMAN_SPEED (9);

// скорость перехода pacGirl с одной клетки на другую в милисекундах
#define PACGIRL_SPEED (9);

// скорость перехода Красного призрака с одной клетки на другую в милисекундах
#define RED_SPEED (9);

// Время через которое появляеться вишня
#define CHERRY_TIME (255);

// количество очков за поедание вишни
#define SCORE_CHERY_BONUS (200);

// количество очков за поедание POWER_FOOD
#define SCORE_POWER_BONUS (25);

// количество очков за поедание RED
#define SCORE_RED_EAT (50);

// время через которое RED перестает быть съедобным
#define RED_TIME (255);

// размер карты по x
#define MAP_SIZE_Y (23)

// размер карты по y
#define MAP_SIZE_X (32)

// размер карты по y для 3х сегментов
#define MAP_SIZE_Y8 (8)
#define MAP_SIZE_Y16 (16)

// размер массива с смещениями для 'КОНЕЦ ИГРЫ' и 'ТЫ ВЫИГРАЛ'
#define TEXT_GAME_OVER_SIZE (10)

// размер массива с смещениями для 'ОЧКИ'
#define TEXT_SCORE_SIZE (4)


/**
 * Глобальные переменные
 */

// карта 1 часть 
// разбил на 3 части, т.к. массив не может 
// быть больше 256 байт для 8 битной консоли
unsigned char map[MAP_SIZE_Y][MAP_SIZE_X] = {
 "7888888888888895788888888888889",
 "4.............654.............6",
 "4*i220.i22220.l8d.i22220.i220*6",
 "4..............Q..............6",
 "4.i220.fxj.i22mxn220.fxj.i220.6",
 "4......654....654....654......6",
 "1xxxxj.65s220.l8d.222e54.fxxxx3",
 "555554.654...........654.655555",
 "555554.654.fxxj-fxxj.654.655555",
 "88888d.l8d.678d l894.l8d.l88888",
 "...........64  %  64..^........",
 "xxxxxj.fxj.61xxxxx34.fxj.fxxxxx",
 "555554.654.l8888888d.654.655555",
 "555554.654...........654.655555",
 "78888d.l8d.i22mxn220.l8d.l88889",
 "4.............654.............6",
 "4.i2mj.i22220.l8d.i22220.fn20.6",
 "4*..64.........O.........64..*6",
 "s20.ld.fxj.i22mxn220.fxj.ld.i2e",
 "4......654....654....654......6",
 "4.i2222y8z220.l8d.i22y8z22220.6",
 "4.............................6",
 "1xxxxxxxxxxxxxxxxxxxxxxxxxxxxx3"
 };

// для обработки нажатия кнопок первым игроком на контроллере
s16 pad1;

// для обработки нажатия кнопок вторым игроком на контроллере
s16 pad2;

// переменная для отрисовки текстовой информации
// очки, бонусы, GAME OVER, YOU WINNER
char text[4];

// для получения адреса по координатам x, y на tile заднего фона
s16 address;

// состояние иры (на каком экране находимся)
// 0 - STATE_SELECT - стартовый экран (выбор количества игроков)
// 1 - STATE_GAME - игра - второй экран после выбора количества игроков
// 2 - STATE_GAME_RESULT - третий экран, идет после игры - результат игры
// 3 - STATE_SCREENSAVER - заставка
unsigned char gameState = 3;


// текущие координаты PACMAN
s16 pacmanX = 15;
s16 pacmanY = 17;

// текущие координаты PACGIRL
s16 pacGirlX = 15;
s16 pacGirlY = 3;

// старые координаты PACMAN
s16 oldX = 15;
s16 oldY = 17;

// направление движение PACMAN
s16 dx = 0;
s16 dy = 0;

// направление движение PACGIRL
s16 dxPacGirl = 0;
s16 dyPacGirl = 0;

// старые координаты PACGIRL
s16 oldPacGirlX = 15;
s16 oldPacGirlY = 3;

// направление движения RED (SHADOW или BLINKY)
s16 dxRed = 1;
s16 dyRed = 0;

// координаты RED (SHADOW или BLINKY)
s16 redX = 22;
s16 redY = 10;

// старые координаты RED (SHADOW или BLINKY)
s16 oldXRed = 22;
s16 oldYRed = 10;

// 1 - RED в режиме охоты
// 0 - PACMAN съел POWER_FOOD и RED сейчас съедобен
unsigned char redFlag = 1;

// время когда RED стал съедобным последний раз
unsigned char redTime = 0;

// 1 - Вишня есть
// 0 - Вишни нет
unsigned char cherryFlag = 0;

// надо перерисовать черешню
unsigned char refreshCherry = 0;

// x координата черешни
s16 cherryX = 15;
// y координата черешни
s16 cherryY = 10;

// x координата двери
s16 doorX = 15;
// y координата двери
s16 doorY = 8;

// надо перерисовать дверь
unsigned char refreshDoor = 1;

// что лежит на клетке с RED (BLINKY)
unsigned char oldRedVal = '.';

// что лежит на клетке с PACGIRL
unsigned char oldPacGirlVal = '.';

// бонус за съедание RED (BLINKY)
unsigned char redBonus = 0;

// бонус за съедание POWER_FOOD
unsigned char powerBonus = 0;

// бонус за съеденные вишни
unsigned char cherryBonus = 0;

// счетчики циклов начинаются с разных значений
// чтоб рендеринг каждого персонажа был в разном глобальном цикле
// время последнего обновления положения pacman
unsigned char pacmanLastUpdateTime = PACMAN_SPEED;

// время поседнего обновления RED
unsigned char redLastUpdateTime = 4; //RED_SPEED;

// время последнего обновления положения pacGirl
unsigned char pacGirlLastUpdateTime = 6; //PACGIRL_SPEED;

// время через которое появится вишня
unsigned char cherryTime = CHERRY_TIME;

// играет ли музыка призрака
unsigned char shadowLastSoundTime = 0;

// переменные для работы с массивами map1, map2, map3 и для циклов
// // через функции setValToMap getValFromMap
s16 i = 0;
s16 j = 0;

// переменная через которую записываем и получаем значения из map1, map2, map3
// через функции setValToMap getValFromMap
unsigned char val;

// временная переменная, в нее часто сохраняем значение val
unsigned char val2;

// координаты y, y привязанные к верхнему левому углу (в пикселях)
// используются для определения где рисовать объекты
s16 y = 0;
s16 x = 0;

// количество выбранных игроков для игры
unsigned char players = 1;

// счетчик циклов для защиты от 2го нажатия на стартовом экране 
// кнопки Select
unsigned char playersTime = 0;


// для ывода сколько точек белых съели их больше 256 поэтому
// храню в 3х счетчиках, ну и для вывода проще так
// единицы для съеденной еды
unsigned char food001 = 1;

// десятки для съеденной еды
unsigned char food010 = 0;

// сотни для съеденной еды
unsigned char food100 = 0;


// для вывода результата игры значение от 000 до 999
// единицы для итоговых очков
unsigned char score001 = 0;

// десятки для итоговых очков
unsigned char score010 = 0;

// сотни для итоговых очков
unsigned char score100 = 0;


// спрайт Соника
Sprite* sonicSprite;

// спрайт Pac-Man
Sprite* pacmanSprite;

// спрайт RED
Sprite* redSprite;

// спрайт Pac-Girl
Sprite* pacGirlSprite;

// спрайт вишни
Sprite* cherrySprite;

// спрайт двери
Sprite* doorSprite;

// координаты где рисуем спрайт Соника
s16 sonicX = -95;
s16 sonicY = 83;

// скорость перемещения Соника
s16 dxSonic = 1;

// массив смещений относительно FIRST_INDEX_RUSSIAN_TILE, чтоб написать 'КОНЕЦ ИГРЫ'
u16 textGameOver[TEXT_GAME_OVER_SIZE] = {
						44, // К
					    48, // О
					    47, // Н
					    39, // Е
					    56, // Ц
					    1,  // пробел
					    42, // И
					    37, // Г
					    50, // Р
					    61  // Ы

};

u16 textScore[TEXT_SCORE_SIZE] = {
						48, // О
						57, // Ч
						44, // К
						42  // И

};

u16 textYouWinner[TEXT_GAME_OVER_SIZE] = {
						52, // Т
					    61, // Ы
					    1,  // пробел
					    36, // В
					    61, // Ы
					    42, // И
					    37, // Г
					    50, // Р
					    34, // А
					    45  // Л

};


/**
 * Функции
 */

void moveBound(s16 *x, s16 *y);

/**
 * Клетка по заданным координатам не стена (WALL)
 * i - строка в массиве карты
 * j - столбец в массиве карты
 * return val = 1 - не стена, 0 - стена
 */
int isNotWell(s16 y, s16 x);

/**
 * Клетка по заданным координатам не стена и не дверь (WALL, DOOR)
 * y - координата Y на карте (map[][])
 * x - координата X на карте (map[][])
 * return val = 1 - не стена и не дверь, 0 - стена или дверь
 */
int isNotWellOrDoor(s16 y, s16 x);

/**
 * Открыть двери к вишне и дому призраков
 */
void openDoors();

/**
 * Закрыть двери к дому призраков
 */
void closeDoors();

/**
 * Съедена еда
 * пересчитать значения счетчиков 
 * food001 food010 food100
 */
void incFood();

/**
 * Подсчет отчков с учетом всех бонусов
 */
void calcScore();

/**
 * Сбрасываем все на начальные настройки по карте:
 * начальные значения счетчиков циклов
 * начальное положение персонажей
 * где будет еда и поверапы
 */
void init();

/**
 * Проиграл ли PACMAN или он мог съесть призрака
 * и что съел на месте призрака
 */
int pacmanLooser();

/**
 * Алгоритм обработки движения PACMAN на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacManState();

/**
 * Алгоритм призрака гоняющегося за PACMAN
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int redState();

/**
 * Алгоритм обработки движения PACGIRL на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacGirlState();

/**
 *  Обработка нажатых кнопок игроком
 *  передвижение персонажей во время игры
 */
void actions();

/**
 * Нарисовать только 1 объект с карты
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void draw(s16 i, s16 j);

/**
 * Нарисовать задний фон
 */
void drawBackground();

/**
 * Нарисовать спрайты
 */
void drawSprites();

/**
 *  Нарисовать бонусы, очки
 *  результат игры (GAME OVER или YOU WINNER)
 */
void drawText();

/**
 *  Перерисовать на заднем фоне tile когда съели точку
 *  рисуем черный квадрат 8x8
 */
void drawBlackBox(s16 y, s16 x);

/**
 * Обновить карту / персонажей, двери, черешню
 * тут только отрисовка
 */
void refreshGame();

/**
 * сбрасываем значения переменных для отображения заставкии
 */
void initScreensaver();

/**
 * отображаем заставку
 */
void screensaver();

/**
 * Рисуем Русский текст
 *
 * plane - слой, на котором будет расположен тайл
 * vramOffsets - массив смещений относительно FIRST_INDEX_RUSSIAN_TILE
 * length - количество символов которые нужно отобразить на экране (размер массива vramOffsets)
 * xTile - x координата где нарисовать текст
 * yTile - y координата где нарисовать текст
 */
void drawRussianText(VDPPlane plane, const u16 *vramOffsets, u16 length, u16 xTile, u16 yTile);



