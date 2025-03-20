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
 * BlodTor 2025 г.
 */

/**
 * Константы 
 */


// выбор количества игроков - экран после заставки, т.е. после экрана STATE_SCREENSAVER
#define STATE_SELECT  	  			0

// игра - экран с картой уровня, после выбора количества игроков, т.е. после экрана STATE_SELECT
#define STATE_GAME    	  			1

// результат игры - экран после завершения игры, т.е. после экрана STATE_GAME
#define STATE_RESULT  	  			2

// заставка - экран самый первый с анимацией поедания SEGA
#define STATE_SCREENSAVER 			3

// пауза - экран карта уровня с пробегающим соником и словом PAUSE летающей по всему экрану, при нажатии на старт на STATE_GAME
#define STATE_PAUSE 	  			4

// Еда
#define FOOD 					    '.'

// Поверап позваляющий есть призраков
#define POWER_FOOD 					'*'

// Дверь в комнату призраков
#define DOOR						'-'

// Ни кем не занятая клетка
#define EMPTY 						' '

// Pac-Man
#define PACMAN 						'O'

// Pac Girl
#define PACGIRL 					'Q'

// Красный призрак (BLINKY)
#define RED 						'^'

// Красный призрак когда его можно съесть (SHADOW или BLINKY)
#define SHADOW 						'@'

// Вишня
#define CHERRY 						'%'

// скорость перехода pacman с одной клетки на другую в циклах отрисовки
#define PACMAN_SPEED 				9

// скорость перехода pacGirl с одной клетки на другую в циклах отрисвки
#define PACGIRL_SPEED 				9

// скорость перехода Красного призрака с одной клетки на другую в циклах отрисовки
#define RED_SPEED 					9

// Время через которое появляеться вишня в циклах отрисовки
#define CHERRY_TIME 				255

// количество очков за поедание вишни
#define SCORE_CHERY_BONUS 			200

// количество очков за поедание POWER_FOOD
#define SCORE_POWER_BONUS 			25

// количество очков за поедание RED
#define SCORE_RED_EAT 				50

// время через которое RED перестает быть съедобным в циклах отрисовки
#define RED_TIME 					255

// размер карты по x
#define MAP_SIZE_Y 					23

// размер карты по y
#define MAP_SIZE_X 					32

// не известно есть ли соединение между приставками через SEGA Link Cable и воткнут ли в 2 порт контроллер
#define MODE_PORT2_UNKNOWN			0

// наша приставка ведущая (сначала передает данные затем читает данные через SEGA Link cable)
#define MODE_PORT2_MASTER			1

// наша приставка ведомая (сначала читает данные затем передает данные через SEGA Link cable)
#define MODE_PORT2_SLAVE			2

// в втором порту 3 или 6 кнопочний контроллер, нет соединения через SEGA Link cable
#define MODE_MULTY_PLAYER			3

// нет соединения между приставками и в втором порту ничего не подключено
#define MODE_SINGLE_PLAYER          4

// голос во время заставки произносящий слово SEGA
#define SFX_SOUND_SEGA				64

// звук поедания белой точки - еды
#define SFX_SOUND_EAT				66

// звук поедания черешни
#define SFX_SOUND_CHERRY			67

// звук поедания зеленой точки - powerup
#define SFX_SOUND_POWERUP			68

// звук когда призрак съедобен
#define SFX_SOUND_SHADOW			69

// звук когда съели призрака
#define SFX_SOUND_EAT_SHADOW		70

// звук создания соединения через Link cable
#define SFX_SOUND_CONNECT_LINK_CABLE	72

// звук отключения соединения через Link cable
#define SFX_SOUND_DISCONNECT_LINK_CABLE	74

// Ни чего не показываем из отладочной информации
#define SHOW_LINK_CABLE_NO			0

// выводим на экран ошибки при передаче через Link Cadle
#define SHOW_LINK_CABLE_LAST_ERROR	1

// выводим на экран количество ошибок при передаче через Link Cadle
#define SHOW_LINK_CABLE_ERROS_COUNT	2

// количество отресованных фреймов с начала создания соединения через Link Cadle
#define SHOW_LINK_CABLE_FRAME_COUNT 3

// При игре на одной приставке первый игрок - Pac-Man, второй игрок - Pac-Girl.
// При игре на двух приставках через Link cable, master - Pac-Man, slave - Pac-Girl.
#define P1_PACMAN__P2_PACGIRL 		0

// При игре на одной приставке первый игрок - Pac-Girl, второй игрок - Pac-Man.
// При игре на двух приставках через Link cable, master - Pac-Girl, slave - Pac-Man.
#define P1_PACGIRL__P2_PACMAN		1

// длинна текста выводимого на экран о типе игры, текст может быть таким:
// '1P NO LINK!' игра на одной приставке, во втором порту ни чего не обнаружено, controllerPort2Mode = MODE_SINGLE_PLAYER
// '2P NO Link!' игра на одной приставке, на двух контроллерах, controllerPort2Mode = MODE_MULTY_PLAYER
// 'TRY MASTER!' пытаемся стать ведущей приставкой, предполагаем что во втором порту Linc cable
// 'TRY  SLAVE!' пытаемся стать ведомой приставкой, предполагаем что во втором порту Linc cable
// 'LINK MASTER' игра на двух приставках через Link cable мы ведущая приставка, controllerPort2Mode = MODE_PORT2_MASTER
// 'LINK  SLAVE' игра на двух приставках через Link cable мы ведомая приставка, controllerPort2Mode = MODE_PORT2_SLAVE
// '           ' не известно есть ли соединение между приставками и что воткнуто в 2 порт, controllerPort2Mode = MODE_PORT2_UNKNOWN
#define GAME_MODE_TEXT_SIZE	 	11

// Ниже объявлены константы для функций LCP_objectToPacketForSend() и LCP_getNextObjectFromRecivePacket()
// чтоб протокол понял какой объект был передан в массиве transferObject или получен от другой приставки и какого он размера
// в протоколе нет информации об объектах каторые он передает, ему всеравно что передавать!

// тип данных объекта: master - ключевое слово которое пришлет ведомой приставке ведущая при инициализации соединения
#define OBJECT_TYPE_MASTER 			1

// тип данных объекта: slave - ключевое слово которое пришлет ведущей приставке ведомая при инициализации соединения
#define OBJECT_TYPE_SLAVE  			2

// тип данных объекта: joy - нажатые кнопки на контроллере в первом порту SEGA
#define OBJECT_TYPE_JOY    			3

// тип данных объекта: switch plaers кто каким персонажем играет
#define OBJECT_TYPE_SWITCH_PLAYERS 	4

// тип данных объекта: кординаты и направление движение PAC-MAN
#define OBJECT_TYPE_PAC_MAN_STATE	5

// тип данных объекта: кординаты и направление движение PAC-GIRL
#define OBJECT_TYPE_PAC_GIRL_STATE	6

// тип данных объекта: кординаты и направление движение RED
#define OBJECT_TYPE_RED_STATE		7

// тип данных объекта: конец игры
#define OBJECT_TYPE_END_GAME		8

// тип данных объекта: пауза
#define OBJECT_TYPE_PAUSE 			9

// тип данных объекта: продолжение игры
#define OBJECT_TYPE_RESUME_GAME		10

// тип данных объекта: съели точку
#define OBJECT_TYPE_EAT_POINT		11

// тип данных объекта: съели поверап
#define OBJECT_TYPE_EAT_POWERUP		12

// тип данных объекта: съели призрака
#define OBJECT_TYPE_EAT_SHADOW		13

// тип данных объекта: съели черешню
#define OBJECT_TYPE_EAT_CHERRY		14

// тип данных объекта: открыть дверь
#define OBJECT_TYPE_OPEN_DOOR		15


// длинна типа данных 'master' - 8 байт, текст: 'Pac-Girl'
#define MASTER_OBJECT_LENGTH 		8

// длинна типа данных 'slave' - 8 байт, текст: 'Pac-Man!'
#define SLAVE_OBJECT_LENGHT  		8

// длинна типа данных 'joy' - 2 байта (состояние всех нажатых кнопок на контроллере)
#define JOY_OBJECT_LENGHT 	 		2

// длинна типа данных 'switch plaers' - 2 байта
#define SWITCH_PLAYERS_LENGHT 		2

// длинна типа данных 'кординаты и направление движение PAC-MAN' - 4 байта
#define PAC_MAN_STATE_LENGHT 		4

// длинна типа данных 'кординаты и направление движение PAC-GIRL' - 4 байта
#define PAC_GIRL_STATE_LENGHT 		4

// длинна типа данных 'кординаты и направление движение RED' - 2 байта
#define RED_STATE_LENGHT 			2

// длинна типа данных 'конец игры'  - 2 байта
#define END_GAME_LENGTH				2

// длинна типа данных 'пауза' - 2 байта
#define PAUSE_LENGTH				2

// длинна типа данных 'продолжение игры' - 2 байта
#define RESUME_GAME_LENGTH			2

// длинна типа данных 'съели точку' - 2 байта
#define EAT_POINT_LENGTH			2

// длинна типа данных 'съели поверап' - 2 байта
#define EAT_POWERUP_LENGTH			2

// длинна типа данных 'съели призрака' - 2 байта
#define EAT_SHADOW_LENGTH			2

// длинна типа данных 'съели черешню' - 2 байта
#define EAT_CHERRY_LENGTH			2

// длинна типа данных 'открыть дверь' - 2 байта
#define OPEN_DOOR_LENGTH			2


/**
 * Глобальные переменные
 */

/**
 * 0 - не объект
 * 1 OBJECT_TYPE_MASTER 	    - MASTER_OBJECT_LENGTH  = 8  байт
 * 2 OBJECT_TYPE_SLAVE  	    - SLAVE_OBJECT_LENGHT   = 8  байт
 * 3 OBJECT_TYPE_JOY    	    - JOY_OBJECT_LENGHT     = 2  байта
 * 4 OBJECT_TYPE_SWITCH_PLAYERS - SWITCH_PLAYERS_LENGHT = 2  байта
 * 5 OBJECT_TYPE_PAC_MAN_STATE  - PAC_MAN_STATE_LENGHT  = 4  байта
 * 6 OBJECT_TYPE_PAC_GIRL_STATE - PAC_GIRL_STATE_LENGHT = 4  байта
 * 7 OBJECT_TYPE_RED_STATE      - RED_STATE_LENGHT 	    = 2  байта
 * 8 OBJECT_TYPE_END_GAME       - END_GAME_LENGTH		= 2  байта
 * 9 OBJECT_TYPE_PAUSE          - PAUSE_LENGTH			= 2  байта
 *10 OBJECT_TYPE_RESUME_GAME    - RESUME_GAME_LENGTH	= 2  байта
 *11 OBJECT_TYPE_EAT_POINT      - EAT_POINT_LENGTH		= 2  байта
 *12 OBJECT_TYPE_EAT_POWERUP    - EAT_POWERUP_LENGTH    = 2  байта
 *13 OBJECT_TYPE_EAT_SHADOW     - EAT_SHADOW_LENGTH	    = 2  байта
 *14 OBJECT_TYPE_EAT_CHERRY     - EAT_CHERRY_LENGTH     = 2  байта
 *15 OBJECT_TYPE_OPEN_DOOR      - OPEN_DOOR_LENGTH		= 2  байта
 *
 * Массив содержащий размеры передаваемых через SEGA_LINK_CABLE объектов
 * т.к. протакол за раз передает 2 байта (16 бит) нет смысла иметь объекты размером не кратные двум байтам
 * всеравно будет передано минимум 2 байта, но опребелять такие тоже можно!
 * используется как последний параметр функций LCP_objectToPacketForSend() и LCP_getNextObjectFromRecivePacket()
 * протокол не знает размеры ваших объектов, вот ему и надо их передавать!
 */
u16 LINK_TYPES_LENGHT[] = {
							0, MASTER_OBJECT_LENGTH, SLAVE_OBJECT_LENGHT, JOY_OBJECT_LENGHT, SWITCH_PLAYERS_LENGHT, PAC_MAN_STATE_LENGHT,
							   PAC_GIRL_STATE_LENGHT, RED_STATE_LENGHT, END_GAME_LENGTH, PAUSE_LENGTH, RESUME_GAME_LENGTH,
							   EAT_POINT_LENGTH, EAT_POWERUP_LENGTH, EAT_SHADOW_LENGTH, EAT_CHERRY_LENGTH, OPEN_DOOR_LENGTH
						  };

// карта игры
u8 map[MAP_SIZE_Y][MAP_SIZE_X] = {
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

// режим работы приставки с вторым портом для контроллероллера
// MODE_PORT2_UNKNOWN 		0 - не известно есть ли соединение между приставками через SEGA Link Cable и воткнут ли в 2 порт контроллер
// MODE_PORT2_MASTER  		1 - master наша приставка ведущая (сначала передает данные затем читает данные через SEGA Link cable)
// MODE_PORT2_SLAVE   		2 - slave наша приставка ведомая (сначала читает данные затем передает данные через SEGA Link cable)
// MODE_MULTY_PLAYER	 	3 - multiplayer в втором порту 3 или 6 кнопочний контроллер, нет соединения через SEGA Link cable
// MODE_SINGLE_PLAYER		4 - singlepayer нет соединения между приставками и в втором порту ничего не подключено
u8 controllerPort2Mode = MODE_PORT2_UNKNOWN;

// для обработки нажатия кнопок первым игроком на контроллере
u16 pad1 = 0;

// для обработки нажатия кнопок вторым игроком на контроллере
u16 pad2 = 0;

// SHOW_LINK_CABLE_NO - ничего не показывать
// SHOW_LINK_CABLE_LAST_ERROR - показывать ошибки происходящие при передаче данных через Link Cable при нажатии A + C
// SHOW_LINK_CABLE_ERROS_COUNT - показывать количество ошибок происходящие при передаче данных через Link Cable при нажатии B + C
// SHOW_LINK_CABLE_FRAME_COUNT - показывать количество отресованных фреймов с начала создания соединения, при нажании A + X
u8 showLinkCableErrors = SHOW_LINK_CABLE_NO;

// ошибка которая произошла при передаче данных через Link cable, выводим на экран при нажатии A + C
u16 linkCableErrors = 0;

// количество ошибок при передаче через Link cable, выводим на экран при нажании B + C
u16 linkCableErrorsCount = 0;

// количество отресованных фреймов с начала создания соединения, выводим на экран при нажании A + X
u16 linkCableFrameCount = 0;


// какой игрок каким персонажем играет
// если switchPlayers == P1_PACMAN__P2_PACGIRL при игре через Link cable, master - Pac-Man, slave - Pac-Girl. При игре на одной
// приставке первый игрок - Pac-Man, второй игрок - Pac-Girl.
// если switchPlayers == 1 при игре через Link cable, master - Pac-Girl, slave - Pac-Man. При игре на одной
// приставке первый игрок - Pac-Girl, второй игрок - Pac-Man.
// Чтоб изменить кто кем будет играть, нужно выбрать в меню выбора игроков '2 PLAYRS' и нажать
// на любом из контроллеров ВПРАВО switchPlayers = 1 и ВЛЕВО чтоб switchPlayers = 0
u8 switchPlayers = P1_PACMAN__P2_PACGIRL;

// содержит объект в виде байтового массива который собираемся передать через SEGA Link Cable
// другой приставке и должен быть размером >= наибольшего размера из типов данных объектов
u8 transferObject[MASTER_OBJECT_LENGTH + 1];

// текст 'ВЕДУЩАЯ ПРИСТАВКА'     ('LINK MASTER') - игра на двух приставках через Link cable мы ведущая приставка (controllerPort2Mode = MODE_PORT2_MASTER)
const u8 TEXT_LINK_MASTER[GAME_MODE_TEXT_SIZE] = "LINK MASTER";

// текст 'ВЕДОМАЯ ПРИСТАВКА'     ('LINK  SLAVE') - игра на двух приставках через Link cable мы ведомая приставка (controllerPort2Mode = MODE_PORT2_SLAVE)
const u8 TEXT_LINK_SLAVE[GAME_MODE_TEXT_SIZE] =  "LINK  SLAVE";

// текст '1 ИГРОК НЕ СЕТЕВАЯ!'   ('1P NO LINK!') - игра на одной приставке, во втором порту ни чего не обнаружено (controllerPort2Mode = MODE_SINGLE_PLAYER)
const u8 TEXT_1P_NO_LINK[GAME_MODE_TEXT_SIZE] =  "1P NO LINK!";

// текст '2 ИГРОКА НЕ СЕТЕВАЯ!'  ('2P NO Link!') - игра на одной приставке на двух контроллерах (controllerPort2Mode = MODE_MULTY_PLAYER)
const u8 TEXT_2P_NO_LINK[GAME_MODE_TEXT_SIZE] =  "2P NO Link!";

// текст 'ПЫТАЕМСЯ БЫТЬ ВЕДУЩЕЙ' ('TRY MASTER!') - пытаемся стать ведущей приставкой, предполагаем что во втором порту Link cable
const u8 TEXT_TRY_MASTER[GAME_MODE_TEXT_SIZE] =  "TRY MASTER!";

// текст 'ПЫТАЕМСЯ БЫТЬ ВЕДОМОЙ' ('TRY  SLAVE!') - пытаемся стать ведомой приставкой, предполагаем что во втором порту Link cable
const u8 TEXT_TRY_SLAVE[GAME_MODE_TEXT_SIZE] = 	 "TRY  SLAVE!";

// для вывода на экран в виде текста режима работы приставки с учетом того что воткнуто в 2 порт приставки
// '1P NO LINK!' (TEXT_1P_NO_LINK)  игра на одной приставке, во втором порту ни чего не обнаружено (controllerPort2Mode = MODE_SINGLE_PLAYER)
// '2P NO Link!' (TEXT_2P_NO_LINK)  игра на одной приставке на двух контроллерах (controllerPort2Mode = MODE_MULTY_PLAYER)
// 'TRY MASTER!' (TEXT_TRY_MASTER)  пытаемся стать ведущей приставкой, предполагаем что во втором порту Link cable
// 'TRY  SLAVE!' (TEXT_TRY_SLAVE)   пытаемся стать ведомой приставкой, предполагаем что во втором порту Link cable
// 'LINK MASTER' (TEXT_LINK_MASTER) игра на двух приставках через Link cable мы ведущая приставка (controllerPort2Mode = MODE_PORT2_MASTER)
// 'LINK  SLAVE' (TEXT_LINK_SLAVE)  игра на двух приставках через Link cable мы ведомая приставка (controllerPort2Mode = MODE_PORT2_SLAVE)
// '           ' не известно есть ли соединение между приставками и что воткнуто в 2 порт (controllerPort2Mode = MODE_PORT2_UNKNOWN)
char gameModeText[GAME_MODE_TEXT_SIZE + 1];

// тип объекта передаваемого через Link Cable Protocol
u16 objectType = 0;

// переменная для отрисовки текстовой информации
// очки, бонусы, GAME OVER, YOU WINNER
char text[4];

// состояние иры (на каком экране находимся)
// 0 - STATE_SELECT - выбор количества игроков
// 1 - STATE_GAME - игра, второй экран после выбора количества игроков
// 2 - STATE_GAME_RESULT - результат игры, после окончания игры
// 3 - STATE_SCREENSAVER - заставка, самый первый экран с поеданием SEGA
// 4 - STATE_PAUSE - пауза вовремя игры
u8 gameState = STATE_SCREENSAVER;


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
s8 dx = 0;
s8 dy = 0;

// направление движение PACGIRL
s8 dxPacGirl = 0;
s8 dyPacGirl = 0;

// старые координаты PACGIRL
s16 oldPacGirlX = 15;
s16 oldPacGirlY = 3;

// направление движения RED (SHADOW или BLINKY)
s8 dxRed = 1;
s8 dyRed = 0;

// координаты RED (SHADOW или BLINKY)
s16 redX = 22;
s16 redY = 10;

// старые координаты RED (SHADOW или BLINKY)
s16 oldXRed = 22;
s16 oldYRed = 10;

// 1 - RED в режиме охоты
// 0 - PACMAN съел POWER_FOOD и RED сейчас съедобен
u8 redFlag = 1;

// время когда RED стал съедобным последний раз
u8 redTime = 0;

// 1 - Вишня есть
// 0 - Вишни нет
u8 cherryFlag = 0;

// надо перерисовать черешню
u8 refreshCherry = 0;

// x координата черешни
s16 cherryX = 15;
// y координата черешни
s16 cherryY = 10;

// x координата двери
s16 doorX = 15;
// y координата двери
s16 doorY = 8;

// координаты спрайта с словом PAUSE
s16 pauseX = 120;
s16 pauseY = 120;

//  направление движения спрайта с словом PAUSE
s16 pauseDX = 2;
s16 pauseDY = 1;

// надо перерисовать дверь
u8 refreshDoor = 1;

// что лежит на клетке с RED (BLINKY)
u8 oldRedVal = '.';

// что лежит на клетке с PACGIRL
u8 oldPacGirlVal = '.';

// бонус за съедание RED (BLINKY)
u8 redBonus = 0;

// бонус за съедание POWER_FOOD
u8 powerBonus = 0;

// бонус за съеденные вишни
u8 cherryBonus = 0;

// счетчики циклов начинаются с разных значений
// чтоб рендеринг каждого персонажа был в разном глобальном цикле
// время последнего обновления положения pacman
u8 pacmanLastUpdateTime = PACMAN_SPEED;

// время поседнего обновления RED
u8 redLastUpdateTime = 4; //RED_SPEED;

// время последнего обновления положения pacGirl
u8 pacGirlLastUpdateTime = 6; //PACGIRL_SPEED;

// время через которое появится вишня
u8 cherryTime = CHERRY_TIME;

// играет ли музыка призрака
u8 shadowLastSoundTime = 0;

// переменная через которую записываем и получаем значения из map
// через функции setValToMap getValFromMap
u8 val;

// координаты y, y привязанные к верхнему левому углу (в пикселях)
// используются для определения где рисовать объекты
s16 y = 0;
s16 x = 0;

// количество выбранных игроков для игры
u8 players = 1;

// счетчик циклов для защиты от 2го нажатия на стартовом экране 
// кнопки Select
u8 playersTime = 0;


// для ывода сколько точек белых съели их больше 256 поэтому
// храню в 3х счетчиках, ну и для вывода проще так
// единицы для съеденной еды
u8 food001 = 1;

// десятки для съеденной еды
u8 food010 = 0;

// сотни для съеденной еды
u8 food100 = 0;


// для вывода результата игры значение от 000 до 999
// единицы для итоговых очков
u8 score001 = 0;

// десятки для итоговых очков
u8 score010 = 0;

// сотни для итоговых очков
u8 score100 = 0;

// значение сколько раз съели призрака (используется клиентом 2-го игрока)
// (используется клиентом 2-го игрока при сетевом взаимодействии)
u8 redBonusVal = 0;

// значение двери (используется клиентом 2-го игрока)
// (используется клиентом 2-го игрока при сетевом взаимодействии)
u8 doorVal = 0;

// значение для черешни
// (используется клиентом 2-го игрока при сетевом взаимодействии)
u8 cherryVal = 0;

// режим игры основной приставки
// (используется клиентом 2-го игрока при сетевом взаимодействии)
u8 gameStateMaster = 1;

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

// спрайт с текстом 'PAUSE'
Sprite* pauseSprite;

// координаты где рисуем спрайт Соника
s16 sonicX = -95;
s16 sonicY = 83;

// скорость перемещения Соника
s16 dxSonic = 1;


/**
 * Функции
 */

/**
 * Создаем объект для синхронизации приставок через Link Cable Protocol: OBJECT_TYPE_MASTER
 * в байтовом массиве transferObject
 *
 * Фраза может быть любая, если ее получит другая приставка то она
 * будет ведомая (slave) и на экране будет отражено "LINK  SLAVE" у другого игрока.
 *
 * Наша станет ведущей (master) при работе через Link Cable Protocol
 * на экране будет фраза "LINK MASTER" в свою очередь мы получаем от другой приставки
 * другой бъект синхронизации OBJECT_TYPE_SLAVE с другой фразой "Pac-Man!"
 */
void masterToTransferObject();

/**
 * Создаем объект для синхронизации приставок через Link Cable Protocol: OBJECT_TYPE_SLAVE
 * в байтовом массиве transferObject
 *
 * Фраза может бфть любая, если ее получит другая приставка то она
 * будет ведущая (master) и на экране будет отражено "LINK MASTER" у другого игрока.
 *
 * Наша станет ведомой (slave) при работе через Link Cable Protocol
 * на экране будет фраза "LINK  SLAVE" в свою очередь мы получаем от другой приставки
 * другой бъект синхронизации OBJECT_TYPE_MASTER с другой фразой "Pac-Girl"
 */
void slaveToTransferObject();

/**
 * Сохраняем dx, dy, pacmanX, pacmanY, oldX, oldY в байтовом массиве transferObject
 * для передачи другой приставке в виде объекта OBJECT_TYPE_PAC_MAN_STATE
 */
void pacManStateToTransferObject();

/**
 * Получаем dx, dy, pacmanX, pacmanY , oldX, oldY из байтовового массива transferObject
 * в случае если получили от другой приставки объект OBJECT_TYPE_PAC_MAN_STATE
 */
void pacManStateFromTransferObject();

/**
 * Сохраняем dxPacGirl, dyPacGirl, pacGirlX, pacGirlY, oldPacGirlX, oldPacGirlY байтовом массиве transferObject
 * для передачи другой приставке в виде объекта OBJECT_TYPE_PAC_GIRL_STATE
 */
void pacGirlStateToTransferObject();

/**
 * Получаем dxPacGirl, dyPacGirl, pacGirlX, pacGirlY из байтовового массива transferObject
 * в случае если получили от другой приставки объект OBJECT_TYPE_PAC_GIRL_STATE
 */
void pacGirlStateFromTransferObject();

/**
 * Сохраняем dxRed, dyRed, redX, redY, redFlag байтовом массиве transferObject
 * для передачи другой приставке в виде объекта OBJECT_TYPE_RED_STATE
 */
void redStateToTransferObject();

/**
 * Получаем dxRed, dyRed, redX, redY, redFlag из байтовового массива transferObject
 * в случае если получили от другой приставки объект OBJECT_TYPE_RED_STATE
 */
void redStateFromTransferObject();

/**
 * В transferObject сохраняем объект содержащий информацию что было нажато на первом контроллере
 * нашей приставки для передачи другой приставке в виде объекта OBJECT_TYPE_JOY
 *
 * pad - информация о том что было нажато на контроллере
 */
void padToTransferObject(u16 pad);

/**
 * Из объекта что лежит в transferObject переданного через Link Cable получаем информацию что было нажато
 * на первом контроллере другой приставки в случае если получили от другой приставки объект OBJECT_TYPE_JOY
 *
 * return информация о том что было нажато на контроллере
 */
u16 getPadFromTransferObject();

/**
 * SEGA
 *
 * Определяем режим работы приставки при попытке играть вдвоем
 * смотрим что воткнуто в 2 порт контроллера.
 * Если играем через SEGA Link Cable определяем какая
 * приставка ведущая (master) а какая ведомая (slave)
 */
void initControllerPort2();

/**
 * Подсчет отчков с учетом всех бонусов
 */
void calcScore();

/**
 * Клетка по заданным координатам не стена (WALL)
 * i - строка в массиве карты
 * j - столбец в массиве карты
 * return val = 1 - не стена, 0 - стена
 */
u8 isNotWall(s16 y, s16 x);

/**
 * Клетка по заданным координатам не стена и не дверь (WALL, DOOR)
 * y - координата Y на карте (map[][])
 * x - координата X на карте (map[][])
 * return val = 1 - не стена и не дверь, 0 - стена или дверь
 */
u8 isNotWallOrDoor(s16 y, s16 x);

/**
 * Корректировка координат PAC-MAN, PAC-GIRL или Призрака
 * если вышел за поле (появление с другой стороны поля)
 * x - координата по X на карте (map[][])
 * y - координата по y на карте (map[][])
 * значение передаются по ссылке, по этому они меняются
 */
void moveBound(s16 *x, s16 *y);

/**
 * Открыть двери к вишне и дому призраков
 */
void openDoors();

/**
 * Закрыть двери к дому призраков
 */
void closeDoors();

/**
 * Сбрасываем все на начальные настройки по карте:
 * начальные значения счетчиков циклов
 * начальное положение персонажей
 * где будет еда и поверапы
 */
void resetGame();

/**
 * Съедена еда
 * пересчитать значения счетчиков 
 * food001 food010 food100
 */
void incFood();

/**
 * SEGA
 *
 * Проиграл ли PACMAN или он мог съесть призрака
 * и что было съеедено на месте призрака PACMAN или PACGIRL
 */
u8 pacmanLooser();

/**
 * SEGA
 *
 * Алгоритм обработки движения PAC-MAN на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
u8 pacManState();

/**
 * SEGA
 *
 * Алгоритм обработки движения PAC-GIRL на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
u8 pacGirlState();

/**
 * SEGA
 *
 * Алгоритм призрака гоняющегося за PAC-MAN
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
u8 redState();

/**
 *  SEGA
 *
 *  Перерисовать на бекграунде tile когда съели точку
 *  рисуем черный квадрат 8x8
 */
void drawBlackBox(s16 y, s16 x);

/**
 * SEGA
 *
 * Для отладки, в буфер text положить значение числа в виде символов
 * 4 символа типа char число выводится в шеснадцатиричном формате
 * например десятичное 65535 будет выведено как FFFF
 *
 * val чтосло которое хотим вывести на экран
 */
void printU16(u16 val);

/**
 *  SEGA
 *
 *  Нарисовать бонусы, очки
 *  результат игры (GAME OVER или YOU WINNER)
 */
void drawText();

/**
 * SEGA
 *
 * Нарисовать спрайты
 */
void drawSprites();

/**
 * Нарисовать только 1 объект с карты
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void draw(s16 i, s16 j);

/**
 * SEGA
 *
 * Нарисовать переданный объект в координатах
 * i - строка в массиве карты
 * j - столбец в массиве карты
 * val - Что нарисовать в координатах
 */
void drawSprite(s16 i, s16 j, u8 val);

/**
 * SEGA
 *
 * Обновить карту, персонажей, двери, черешню на экране
 */
void refreshGame();


/**
 * SEGA
 *
 * Нарисовать задний фон
 */
void drawBackground();

/**
 * SEGA
 *
 *  Обработка нажатых кнопок игроками во время заставки, когда поедают надпись SEGA
 *  для gameState == STATE_SCREENSAVER
 */
void actionsStateScreensaver();

/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры во время отоброжения экрана с меню выбора количества игроков
 * для gameState == STATE_SELECT
 */
void actionsStateSelectPlayers();

/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры во время паузы
 * для gameState == STATE_PAUSE
 */
void actionsStatePause();

/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры во время непосредственно игры
 * для gameState == STATE_GAME
 */
void actionsStateGame();

/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры когда показываем экран с результатом после окончаня игры
 * gameState == STATE_RESULT
 */
void actionsStateResult();

/**
 *  SEGA
 *
 *  Обработка нажатых кнопок игроком и основная логика игры на основании действий игроков
 */
void actions();

/**
 * SEGA
 *
 * сбрасываем значения переменных для отображения заставкии
 * и отрисовываем их в новых местах
 */
void initScreensaver();

/**
 * SEGA
 *
 * отображаем заставку
 * Sonic убегающий от Pac-Man и надпись SEGA которую съест Pac-Man и Pac-Girl
 * затем Pac-Man убегает от призрака
 */
void screensaver();


/**
 * SEGA
 *
 * Перевести игру в режим паузы
 */
void pause();


/**
 * SEGA
 *
 * Выходим из паузы, т.е. меняем состояние на продолжение игры
 */
void resumeGame();

/**
 * SEGA
 *
 * Играем на двух приставках и наша ведущая (master) приставка
 * отправляем ведомой (slave) приставке что было нажато нажато на нашем контроллере
 * в случае смены персонажей при выборе количества игроков, шлем кто за какого игрока играет
 * а так же шлем все объекты пакета которые еще не были отправлены
 * затем разбераем что было получено от ведомой (slave) приставки а точнее что было нажато на контролере
 * другой приставки
 */
void masterControls();

/**
 * SEGA
 *
 * Играем на двух приставках и наша ведомая (slave) приставка
 * сохраняем в пакет для отправки ведущей (master) приставке что было нажато нажато на нашем контроллере
 * пакет будет отправлен асинхронно при наступлении внешнего прерывания инициируемого ведущей (master) приставкой
 * в случайный момент времени для нашей приставки (обработчик внешнего прерывания - функция LCP_slaveCycle() из библиотеки link_cable.c)
 * разбераем что было получено от ведущей (master) приставки при прошлом вызове внешнего прерывания (LCP_slaveCycle())
 */
void slaveControls();

/**
 * SEGA
 *
 * Определение что было нажато игроками на контроллерах. В случае игры через Link Cable получаем что
 * было нажато на первом контролере подключеном у другой приставке.
 * Анализ объектов в пришедших пакетах по Link Cable, тоже происходит тут и для ведущей (master) и
 * для ведомой (slave) приставки.
 *
 * pad1 - игрок управляющий Pac-Man. Что нажато на 1 контролере при игре на одной приставке.
 *        При игре через Link cable по умолчанию в pad1 будет что нажато на ведущей приставке (master)
 *        на 1 контроллере, так же игра за Pac-Man.
 *
 * pad2 - игрок управляющий Pac-Girl. Что нажато на 2 контролере при игре на одной приставке.
 *        При игре через Link cable по умолчанию в pad2 будет что нажато на ведомой приставке (slave)
 *        на 1 контроллере, так же ига за Pac-Girl
 *
 * НО если в мню '2 PLAYERS' нажать вправо то pad1 и pad2 поменяются местами! Т.е. когда
 * switchPlayers == 1 и players == 2 игроки будут играть за противоположенных персонажей!
 *
 */
void controls();

/**
 * Определить победил ли ты в игре
 * return true - победил в игре
 */
u8 winner();

/**
 * SEGA
 *
 * Инициализация звуковых эффектов
 */
void initSound();

/**
 * SEGA
 *
 * Установка палитр
 */
void  initPaletts();

/**
 * SEGA
 *
 * Инициализация спрайтов
 *
 */
void initSprites();

/**
 * SEGA
 *
 * Инициализация и сброс переменных в значения по умолчанию
 * тут нужно также сбрасывать переменные после soft reset (нажатия на кнопку RESET)
 */
void initGame();

