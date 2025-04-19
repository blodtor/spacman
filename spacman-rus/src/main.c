/**
 * Super Turbo MEGA Pac-Man 2.1 для Sega Mega Drive / Sega Genesis
 * для разработки использовал SGDK 1.90 (July 2023)
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
 * SGDK - свободный и открытый development kit для Sega Mega Drive / Sega Genesis
 * https://github.com/Stephane-D/SGDK.git
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

#include <genesis.h>

#include "main.h"
#include "resources.h"
#include "link_cable.h"


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
void masterToTransferObject() {
	memcpy(transferObject, "Pac-Girl", MASTER_OBJECT_LENGTH);
}


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
void slaveToTransferObject() {
	memcpy(transferObject, "Pac-Man!", SLAVE_OBJECT_LENGHT);
}


/**
 * Сохраняем dx, dy, pacmanX, pacmanY, oldX, oldY в байтовом массиве transferObject
 * для передачи другой приставке в виде объекта OBJECT_TYPE_PAC_MAN_STATE
 */
void pacManStateToTransferObject() {
	transferObject[0] = 0;
	if (dx > 0) {
		// если двигается впаво 6 бит = 1
		transferObject[0] = 0b01000000;
	} else if (dx < 0) {
		// если двигается влево 7 бит = 1
		transferObject[0] = 0b10000000;
	}

	// pacmanX имеет значение от 0 до 31 т.е. 11111 в двоичном виде
	transferObject[0]+= pacmanX;

	transferObject[1] = 0;
	if (dy > 0) {
		// если двигается вниз 6 бит = 1
		transferObject[1] = 0b01000000;
	} else if (dy < 0) {
		// если двигается верх 7 бит = 1
		transferObject[1] = 0b10000000;
	}

	// pacmanY имеет значение от 0 до 22 т.е. 10100 в двоичном виде
	transferObject[1]+= pacmanY;

	// старые координаты по x Pac-Man
	transferObject[2] = oldX;

	// старые координаты по y Pac-Man
	transferObject[3] = oldY;
}


/**
 * Получаем dx, dy, pacmanX, pacmanY , oldX, oldY из байтовового массива transferObject
 * в случае если получили от другой приставки объект OBJECT_TYPE_PAC_MAN_STATE
 */
void pacManStateFromTransferObject() {
	if (transferObject[0] & 0b01000000) {
		// если 6 бит = 1 то двигаемся вправо
		dx = 1;
	} else if (transferObject[0] & 0b10000000) {
		// если 7 бит = 1 то двигаемся влево
		dx = -1;
	} else {
		// иначе стоим на месте
		dx = 0;
	}

	// в 0 - 5 битах значение где находится PAC-MAN по X
	pacmanX = transferObject[0] & 0b00111111;

	if (transferObject[1] & 0b01000000) {
		// если 6 бит = 1 то двигаемся вправо
		dy = 1;
	} else if (transferObject[1] & 0b10000000) {
		// если 7 бит = 1 то двигаемся влево
		dy = -1;
	} else {
		// иначе стоим на месте
		dy = 0;
	}

	// в 0 - 5 битах значение где находится PAC-MAN по Y
	pacmanY = transferObject[1] & 0b00111111;

	// старые координаты по x Pac-Man
	oldX = transferObject[2];

	// старые координаты по y Pac-Man
	oldY = transferObject[3];


    if (map[oldY][oldX] != PACGIRL && map[oldY][oldX] != DOOR) {
    	// если в старых координатах была не Дверь то стираем значение в массиве
    	map[oldY][oldX] = EMPTY;
    }

	if(map[pacmanY][pacmanX] != PACGIRL) {
		// если в текущих координатах не находится PACGIRL
		// то занимаем эту клетку PACMAN
		map[pacmanY][pacmanX] = PACMAN;
	}
}



/**
 * Сохраняем dxPacGirl, dyPacGirl, pacGirlX, pacGirlY, oldPacGirlX, oldPacGirlY байтовом массиве transferObject
 * для передачи другой приставке в виде объекта OBJECT_TYPE_PAC_GIRL_STATE
 */
void pacGirlStateToTransferObject() {
	transferObject[0] = 0;
	if (dxPacGirl > 0) {
		// если двигается впаво 6 бит = 1
		transferObject[0] = 0b01000000;
	} else if (dxPacGirl < 0) {
		// если двигается влево 7 бит = 1
		transferObject[0] = 0b10000000;
	}

	// pacGirlX имеет значение от 0 до 31 т.е. 11111 в двоичном виде
	transferObject[0]+= pacGirlX;

	transferObject[1] = 0;
	if (dyPacGirl > 0) {
		// если двигается вниз 6 бит = 1
		transferObject[1] = 0b01000000;
	} else if (dyPacGirl < 0) {
		// если двигается верх 7 бит = 1
		transferObject[1] = 0b10000000;
	}

	// pacGirlY имеет значение от 0 до 22 т.е. 10100 в двоичном виде
	transferObject[1]+= pacGirlY;

	// старые координаты по x Pac-Girl
	transferObject[2] = oldPacGirlX;

	// старые координаты по y Pac-Girl
	transferObject[3] = oldPacGirlY;
}

/**
 * Получаем dxPacGirl, dyPacGirl, pacGirlX, pacGirlY из байтовового массива transferObject
 * в случае если получили от другой приставки объект OBJECT_TYPE_PAC_GIRL_STATE
 */
void pacGirlStateFromTransferObject() {
	if (transferObject[0] & 0b01000000) {
		// если 6 бит = 1 то двигаемся вправо
		dxPacGirl = 1;
	} else if (transferObject[0] & 0b10000000) {
		// если 7 бит = 1 то двигаемся влево
		dxPacGirl = -1;
	} else {
		// иначе стоим на месте
		dxPacGirl = 0;
	}

	// в 0 - 5 битах значение где находится PAC-GIRL по X
	pacGirlX = transferObject[0] & 0b00011111;

	if (transferObject[1] & 0b01000000) {
		// если 6 бит = 1 то двигаемся вправо
		dyPacGirl = 1;
	} else if (transferObject[1] & 0b10000000) {
		// если 7 бит = 1 то двигаемся влево
		dyPacGirl = -1;
	} else {
		// иначе стоим на месте
		dyPacGirl = 0;
	}

	// в 0 - 5 битах значение где находится PAC-GIRL по Y
	pacGirlY = transferObject[1] & 0b00011111;

	// старые координаты PAC-GIRL по X
	oldPacGirlX = transferObject[2];

	// старые координаты PAC-GIRL по Y
	oldPacGirlY = transferObject[3];

    if (map[oldPacGirlY][oldPacGirlX] != DOOR) {
    	// если в старых координатах не дверь очищаем клетку
    	map[oldPacGirlY][oldPacGirlX] = EMPTY;
    }

    // в новыех координатах PACGIRL отображаем
	map[pacGirlY][pacGirlX] = PACGIRL;
}



/**
 * Сохраняем dxRed, dyRed, redX, redY, redFlag байтовом массиве transferObject
 * для передачи другой приставке в виде объекта OBJECT_TYPE_RED_STATE
 */
void redStateToTransferObject() {
	transferObject[0] = 0;
	if (dxRed > 0) {
		// если двигается впаво 6 бит = 1
		transferObject[0] = 0b01000000;
	} else if (dxRed < 0) {
		// если двигается влево 7 бит = 1
		transferObject[0] = 0b10000000;
	}

	if (redFlag) {
		// если redFlag == 1 то 5 бит = 1
		transferObject[0] |= 0b00100000;
	}

	// redX имеет значение от 0 до 31 т.е. 11111 в двоичном виде
	transferObject[0]+= redX;

	transferObject[1] = 0;
	if (dyRed > 0) {
		// если двигается вниз 6 бит = 1
		transferObject[1] = 0b01000000;
	} else if (dyRed < 0) {
		// если двигается верх 7 бит = 1
		transferObject[1] = 0b10000000;
	}

	// redY имеет значение от 0 до 22 т.е. 10100 в двоичном виде
	transferObject[1]+= redY;
}


/**
 * Получаем dxRed, dyRed, redX, redY, redFlag из байтовового массива transferObject
 * в случае если получили от другой приставки объект OBJECT_TYPE_RED_STATE
 */
void redStateFromTransferObject() {
	if (transferObject[0] & 0b01000000) {
		// если 6 бит = 1 то двигаемся вправо
		dxRed = 1;
	} else if (transferObject[0] & 0b10000000) {
		// если 7 бит = 1 то двигаемся влево
		dxRed = -1;
	} else {
		// иначе стоим на месте
		dxRed = 0;
	}

	// в 0 - 5 битах значение где находится RED по X
	redX = transferObject[0] & 0b00011111;

	if (transferObject[1] & 0b01000000) {
		// если 6 бит = 1 то двигаемся вправо
		dyRed = 1;
	} else if (transferObject[1] & 0b10000000) {
		// если 7 бит = 1 то двигаемся влево
		dyRed = -1;
	} else {
		// иначе стоим на месте
		dyRed = 0;
	}

	// в 0 - 5 битах значение где находится RED по Y
	redY = transferObject[1] & 0b00011111;

	if (transferObject[0] & 0b00100000) {
		// если 5 бит == 1 то призрак не съедобен
		redFlag = 1;
		if (map[redY][redX] != PACGIRL) {
			map[redY][redX] = RED;
		}
	} else {
		// иначе съедобен
		redFlag = 0;
		if (map[redY][redX] != PACGIRL) {
			map[redY][redX] = SHADOW;
		}
	}
}


/**
 * В transferObject сохраняем объект содержащий информацию что было нажато на первом контроллере
 * нашей приставки для передачи другой приставке в виде объекта OBJECT_TYPE_JOY
 *
 * pad - информация о том что было нажато на контроллере
 */
void padToTransferObject(u16 pad) {
	transferObject[0] = pad >> 8;     // старший байт у pad сдвинули в младший байт, т.е. на 8 бит вправо
	transferObject[1] = pad & 0x00FF; // младший байт из pad в 2 элемент массива что передадим
}

/**
 * Из объекта что лежит в transferObject переданного через Link Cable получаем информацию что было нажато
 * на первом контроллере другой приставки в случае если получили от другой приставки объект OBJECT_TYPE_JOY
 *
 * return информация о том что было нажато на контроллере
 */
u16 getPadFromTransferObject() {
	u16 pad = transferObject[0]; // старший байт
	pad <<= 8;           // подвинули старший байт на место (влево на 8 бит)
	pad |= transferObject[1];    // младший байт
	return pad;
}


/**
 * SEGA
 *
 * Определяем режим работы приставки при попытке играть вдвоем
 * смотрим что воткнуто в 2 порт контроллера.
 * Если играем через SEGA Link Cable определяем какая
 * приставка ведущая (master) а какая ведомая (slave)
 */
void initControllerPort2() {
	// не известно есть ли соединение между приставками
    controllerPort2Mode = MODE_PORT2_UNKNOWN;

    // заполняем нулями текст типа игры
    memset(gameModeText, 0, GAME_MODE_TEXT_SIZE + 1);

	// определяем что воткнуто в второй порт приставки
	u8 pad2type = JOY_getJoypadType(JOY_2);
	if (pad2type == JOY_TYPE_PAD3 || pad2type == JOY_TYPE_PAD6) {
		// воткнут 3 или 6 кнопочный контроллер во 2 порт приставки
		controllerPort2Mode = MODE_MULTY_PLAYER;

		// вывести на экран тип игры '2 ИГРОКА НЕ СЕТЕВАЯ! ' - 2 player играет на контроллере подключенным
		// в 2 порт нашей же приставки, НЕ через SEGA Link Cable
		memcpy(gameModeText, TEXT_2P_NO_LINK, GAME_MODE_TEXT_SIZE);

		return;
	} else if (pad2type == JOY_TYPE_UNKNOWN) {
		// сброс ошибок при передаче данных в 0 которые показываем на экране через Link cabile
		linkCableErrors = 0;
		// сброс количества ошибок при передаче данных через Link cabile
		linkCableErrorsCount = 0;
		// сброс количества отресованных фреймов с начала создания соединения через Link cabile
		linkCableFrameCount = 0;

		memcpy(gameModeText, TEXT_TRY_MASTER, GAME_MODE_TEXT_SIZE);
		drawText();
		SYS_doVBlankProcess();

		// инициализация Link Cable Protocol
		LCP_init();

		// в переменную buffer положим объект типа данных OBJECT_TYPE_MASTER фразу "Pac-Girl"
		masterToTransferObject();
		// добавляем в пакет для передачи данных созданный объект в переменной buffer
		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_MASTER, LINK_TYPES_LENGHT);

		do {
			// пытаемся передать пакет LCP_sendPacket другой приставке
			// и получить от нее пакет LCP_recivePacke с данными
			// в качестве ведущей приставки (master)
			LCP_masterCycle();

			// если ошибок нет т.е. LCP_error == 0, значит успешно отправили и получили данные от другой приставки
			// выходим из цикла! Надо смотреть что было получено в LCP_recivePacke! Возможно наша приставка будет ведущей (master)
			// если ошибка 0x1A - значит другая приставка вообще не пыталась получать и отправлять данные,
			// выходим из цикла и будем пытаться стать ведомой приставкой (slave)
			// если любая другая ошибка - значит вторая приставка пыталась отправлять и получать данные, надо
			// попытаться обменятся данными еще раз!

			linkCableErrors = LCP_getError();
		} while (!( linkCableErrors == 0x1A || linkCableErrors == 0));

		// если ошибок при передачи и получении пакетов небыло
		// то LCP_getNextObjectFromRecivePacket() вернет объект полученный от другой приставки
		while ((objectType = LCP_getNextObjectFromRecivePacket(transferObject, LINK_TYPES_LENGHT))) {

			// Проверка что получили от другой приставки OBJECT_TYPE_SLAVE с фразой 'Pac-Man!'
			if (objectType == OBJECT_TYPE_SLAVE && LCP_getError() == 0) {
				// Наша приставка становется ведущей (master)
				// это значит далее при взаимодействии мы вызываем внешннее прерывание - External interrupt (EX-INT)
				// у дрогой приставки т.к. она будет ведомой (slave) отправляем пакет а затем получаем пакет.
				// теперь отправка и получение пакетов на нашей приставке с помащью метода LCP_masterCycle()
				// на нашей приставке не будет вызыватся внешннее прерывание работаем синхронно из основного кода игры
				// метода controls() в котором и формируем пакет и отправляем и получаем пакет от другой приставки
				controllerPort2Mode = MODE_PORT2_MASTER;

				// вывести на экран тип игры 'Pac-Man!' - 2 player при этом играет на контроллере подключенным
				// в 1 порт другой приставки через SEGA Link Cable воткнутый во 2 порт обоих приставок

				memcpy(gameModeText, TEXT_LINK_MASTER, GAME_MODE_TEXT_SIZE);

				// звук удалось создать соединение по Link cabile
				XGM_startPlayPCM(SFX_SOUND_CONNECT_LINK_CABLE, 15, SOUND_PCM_CH4);

				// надо немного подождать чтоб ведомая приставка (slave) успела обработать полученный пакет
				SYS_doVBlankProcess();

				// выходим из функции
				return;
			}

		}

		memcpy(gameModeText, TEXT_TRY_SLAVE, GAME_MODE_TEXT_SIZE);

		// так как не удалось получить от другой приставки OBJECT_TYPE_SLAVE с фразой 'Pac-Man!'
		// пробуем получить данные в качестве ведомой приставки (slave)

		// в переменную buffer положим объект типа данных OBJECT_TYPE_SLAVE фразу "Pac-Man!"
		slaveToTransferObject();
		// добавляем в пакет для передачи данных созданный объект в переменной buffer
		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_SLAVE , LINK_TYPES_LENGHT);
		// теперь у нас в пакете 2 объекта и OBJECT_TYPE_MASTER и OBJECT_TYPE_SLAVE
		// и в случае получания нашей приставкой пакета от другой мы ей отправим оба объекта в пакете
		// но ведущая приставка будет ожидать только OBJECT_TYPE_SLAVE, объект OBJECT_TYPE_MASTER будет ей
		// проигнорирован смотри код выше

		// открываем 2 порт приставки, что означает теперь наша приставка будет обрабатывать внешнние прерывания
		// External interrupt (EX-INT) и сначала получать пакеты при наступлении этого прерывания затем отправлять.
		// Происходить это будет асинхронно с основным кодом игры, потому что посути другая приставка
		// удаленно вызывает метод нашей приставки LCP_slaveCycle() останавливая текущее выполнение программы на нашей
		// в люой рандомный момент
		LCP_open();

		// таймер по истечении кторого считаем что нет соединения по SEGA Link Cable
		u16 timer = 0xFFFF;
		while (timer > 0) {
			drawText();
			SYS_doVBlankProcess();

			pad1 = JOY_readJoypad(JOY_1);
			if (pad1 & BUTTON_START) {
				// если на 1 контроллере нажат Start выход из цикла ожидания соединения с другой приставкой
				break;
			}

			// пытаемся получить объект из полученного объекта
			objectType = LCP_getNextObjectFromRecivePacket(transferObject, LINK_TYPES_LENGHT);

			// Проверка что получили от другой приставки OBJECT_TYPE_MASTER с фразой 'Pac-Girl'
			linkCableErrors = LCP_getError();
			if (objectType == OBJECT_TYPE_MASTER && linkCableErrors == 0) {
				// Наша приставка становется ведомой (slave)
				// это значит что далее при взаимоействии ведущая приставка (master) другого игрока
				// будет вызывать у нашей внешннее прерывание - External interrupt (EX-INT) в следнствии
				// чего основной код игры на нашей ведомой приставке (slave) будет приостановлен на время
				// выполняния функции обработчика внешнего прирывания.
				// теперь получение и отправка пакетов на нашей приставке будет происходить асинхронно
				// по средатвам вызова метода LCP_slaveCycle() в любой рандомный для нас момент другой приставкой
				// а разбор объектов из пакета будет в основном коде игры в методе controls() там же и подготовка
				// пакета на отправку нашей приставкой
				controllerPort2Mode = MODE_PORT2_SLAVE;

				// вывести на экран тип игры 'Pac-Girl' - 1 player при этом играет на контроллере подключенным
				// в 1 порт другой приставки через SEGA Link Cable воткнутый во 2 порт обоих приставок
				memcpy(gameModeText, TEXT_LINK_SLAVE, GAME_MODE_TEXT_SIZE);

				// звук что удалось создать соединение по Link cabile
				XGM_startPlayPCM(SFX_SOUND_CONNECT_LINK_CABLE, 15, SOUND_PCM_CH4);

				// выходим из функции
				return;
			}

			timer--;
		}

		// не удалось создать соединение через SEGA Link Cable
		// закрываем порт, что значит что больше не обрабатываем внешние прерывания - External interrupt (EX-INT)
		// в обработчике LCP_slaveCycle()
		LCP_close();
	}

	memcpy(gameModeText, TEXT_1P_NO_LINK, GAME_MODE_TEXT_SIZE);

	// Нет соединения по SEGA Link cabile и во 2ой порт не вткнут 3 или 6 кнопочный контроллер
	// игра в одного только возможна
	controllerPort2Mode = MODE_SINGLE_PLAYER;
}


/**
 *  Подсчет отчков с учетом всех бонусов
 */
void calcScore() {
	u16 i;
	score100 = food100;
	score010 = food010;
	score001 = food001;

	if (cherryBonus) {
		score100 += 2;
	}

	for (i = 0; i < powerBonus; i++) {
		score001 += 5;

		if (score001 >= 10) {
			score001 -= 10;
			++score010;
		}

		score010 += 2;

		if (score010 >=10) {
			score010 -= 10;
			++score100;
		}

	}


	for (i = 0; i < redBonus; i++) {
		score010 += 5;

		if (score010 >=10) {
			score010 -= 10;
			++score100;
		}
	}
}


/**
 * Клетка по заданным координатам не стена (WALL)
 * y - координата Y на карте (map[][])
 * x - координата X на карте (map[][])
 * return 1 - не стена, 0 - стена
 */
u8 isNotWall(s16 y, s16 x) {
	if (map[y][x] == PACMAN || map[y][x] == PACGIRL || map[y][x] == RED
			|| map[y][x] == CHERRY || map[y][x] == FOOD
			|| map[y][x] == POWER_FOOD || map[y][x] == EMPTY
			|| map[y][x] == SHADOW) {
		return 1;

	}
	return 0;
}


/**
 * Клетка по заданным координатам не стена и не дверь (WALL, DOOR)
 * y - координата Y на карте (map[][])
 * x - координата X на карте (map[][])
 * return 1 - не стена и не дверь, 0 - стена или дверь
 */
u8 isNotWallOrDoor(s16 y, s16 x) {
	if (isNotWall(y, x) && map[y][x] != DOOR) {
		return 1;

	}
	return 0;
}


/**
 * Корректировка координат PAC-MAN, PAC-GIRL или Призрака
 * если вышел за поле (появление с другой стороны поля)
 * x - координата по X на карте (map[][])
 * y - координата по y на карте (map[][])
 * значение передаются по ссылке, по этому они меняются
 */
void moveBound(s16 *x, s16 *y) {
	if (*x < 0) {
		*x = MAP_SIZE_X - 2;
	} else if (*x > MAP_SIZE_X - 2) {
		*x = 0;
	}

	if (*y < 0) {
		*y = MAP_SIZE_Y - 1;
	} else if (*y > MAP_SIZE_Y - 1) {
		*y = 0;
	}
}


/**
 * Открыть двери к вишне и дому призраков
 */
void openDoors() {
	map[doorY][doorX] = EMPTY;
	if (!cherryBonus) {
		map[cherryY][cherryX] = CHERRY;
	}

	cherryFlag = 1;
	refreshCherry = 1;
}


/**
 * Закрыть двери к дому призраков
 * если вишню не съел PACMAN она появится еще
 */
void closeDoors(void) {
	map[doorY][doorX] = DOOR;

	cherryFlag = 0;
	refreshDoor = 1;
	refreshCherry = 0;
}


/**
 * Сбрасываем все на начальные настройки по карте:
 * начальные значения счетчиков циклов
 * начальное положение персонажей
 * где будет еда и поверапы
 */
void resetGame() {
	u16 i, j;
	// счетчики циклов начинаются с разных значений
	// чтоб рендеринг каждого персонажа был в разном глобальном цикле
	pacmanLastUpdateTime = PACMAN_SPEED;
	redLastUpdateTime = 4;     //RED_SPEED;
	pacGirlLastUpdateTime = 6; //PACGIRL_SPEED;
	cherryTime = CHERRY_TIME;

	cherryBonus = 0;
	powerBonus = 0;
	redBonus = 0;
	redBonusVal = 0;

	food001 = 1;
	food010 = 0;
	food100 = 0;


	pacmanX = 15;
	pacmanY = 17;

	pacGirlX = 15;
	pacGirlY = 3;

	oldX = 15;
	oldY = 17;

	dx = 0;
	dy = 0;

	dxPacGirl = 0;
	dyPacGirl = 0;

	oldPacGirlX = 15;
	oldPacGirlY = 3;

	dxRed = 1;
	dyRed = 0;

	redX = 22;
	redY = 10;

	oldXRed = 22;
	oldYRed = 10;

	redFlag = 1;

	redTime = 0;

	cherryFlag = 0;
	refreshCherry = 0;
	refreshDoor = 1;


	oldRedVal = '.';
	oldPacGirlVal = '.';

	// расстовляем еду по карте (серые точки)
	for (i = 0; i < MAP_SIZE_X; i++) {
		for (j = 0; j < MAP_SIZE_Y; j++) {
			val = map[j][i];
			if (val == EMPTY || val == PACGIRL || val == PACMAN || val == RED || val == SHADOW) {
				map[j][i] = FOOD;
			}

		}
	}

	// расставляем поверапы
	map[2][1] = POWER_FOOD;
	map[2][29] = POWER_FOOD;
	map[17][1] = POWER_FOOD;
	map[17][29] = POWER_FOOD;

	// Pac-Man на начальную позицию
	map[pacmanY][pacmanX] = PACMAN;

	// Red на начальную позицию
	map[redY][redX] = RED;

    // Pac-Girl на начальную позицию
	map[pacGirlY][pacGirlX] = PACGIRL;

	// дверь в дом призраков
	// черешня и клетки вокруг
	// в начальное состояние на карте
	map[doorY][doorX] = DOOR;
	map[doorY + 1][doorX] = EMPTY;
	map[cherryY][cherryX - 2] = EMPTY;
	map[cherryY][cherryX - 1] = EMPTY;
	map[cherryY][cherryX] = EMPTY;
	map[cherryY][cherryX + 1] = EMPTY;
	map[cherryY][cherryX + 2] = EMPTY;

	// Надо сбросить т.к. когда начнем новою игру можем еще не успеть получить
	// пакет с состоянием игры ведущей приставки (master) и ведомая приставка (slave) подумает что игра завершена
	gameStateMaster = STATE_GAME;
}


/**
 * SEGA
 *
 * Съедена еда
 * пересчитать значения счетчиков
 * food001 food010 food100
 */
void incFood() {
	// звук поедания точки
	XGM_startPlayPCM(SFX_SOUND_EAT, 1, SOUND_PCM_CH2);

	++food001;
	if (food001 >= 10) {
		food001 -= 10;
		++food010;
	}
	if (food010 >= 10) {
		food010 -= 10;
		++food100;
	}
}


/**
 * SEGA
 *
 * Проиграл ли PACMAN или он мог съесть призрака
 * и что было съеедено на месте призрака PACMAN или PACGIRL
 */
u8 pacmanLooser() {
	// Если RED и PACMAN на одной клетке поля
	if (redY == pacmanY && redX == pacmanX) {
		// RED не съедобен
		if (redFlag) {
			// Конец игры - PACMAN съеден

			map[pacmanY][pacmanX] = RED;

			// убрать спрайт Pac-Man с экрана
			SPR_setPosition(pacmanSprite, -90, 90);

			// обездвижить Pac-Girl
			dxPacGirl = 0;
			dyPacGirl = 0;

			// останавливаем RED
			dxRed = 0;
			dyRed = 0;

	        calcScore();

	    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
	    		// отправляем состояние призрака ведомой (slave) приставке
	    		redStateToTransferObject();
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_RED_STATE, LINK_TYPES_LENGHT);

	    		// отправляем ведомой (slave) приставке что наступил конец игры
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_END_GAME, LINK_TYPES_LENGHT);
	    	}

			return 1;
		} else {
			// звук поедания призрака
			XGM_startPlayPCM(SFX_SOUND_EAT_SHADOW, 15, SOUND_PCM_CH3);

			// RED съедобен в данный момент
			// Отправляем его в дом Приведений
			redY = 10;
			redX = 15;
			//обездвиживаем
			dyRed = 0;
			dxRed = 0;
			// закрываем дверь в дом привидений
			closeDoors();

	    	// скрыть черешню
	    	SPR_setPosition(cherrySprite, -90, 100);

			// отображаем RED на карте как съедобного
			map[redY][redX] = RED;

	        redFlag = 1;

	       	// пусть сидит в домике дополнительное время
	        redTime = RED_TIME;

			// даем бонус за то что RED съели
			++redBonus;

	    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
	    		// отправляем состояние призрака ведомой (slave) приставке
	    		redStateToTransferObject();
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_RED_STATE, LINK_TYPES_LENGHT);

	    		// отправляем ведомой (slave) приставке что был съеден призрак
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_SHADOW, LINK_TYPES_LENGHT);
	    	}

			// проверяем что пакмен съел вместе с RED
			if (oldRedVal == FOOD) {
				// еду
				incFood();

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена белая точка (еда)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POINT, LINK_TYPES_LENGHT);
		    	}
			} else if (oldRedVal == POWER_FOOD) {
				// поверап
				++powerBonus;

				// звук поедания поверапа
				XGM_startPlayPCM(SFX_SOUND_POWERUP, 15, SOUND_PCM_CH2);

				// обнавляем время когда RED стал съедобным
				redTime = RED_TIME;

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что был съеден powerup (зеленая точка)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POWERUP, LINK_TYPES_LENGHT);
		    	}

			} else if (oldRedVal == CHERRY) {
				// вишню
				++cherryBonus;

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);

				// звук поедания черешни
				XGM_startPlayPCM(SFX_SOUND_CHERRY, 15, SOUND_PCM_CH2);

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена черешня
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_CHERRY, LINK_TYPES_LENGHT);
		    	}

			}

			if (cherryBonus) {
				oldRedVal = EMPTY;
			} else {
				oldRedVal = CHERRY;
			}
		}
	} else if (redY == pacGirlY && redX == pacGirlX) {
		// проверяем что Pac-Girl съела на месте RED
		if (oldRedVal == FOOD) {
			// еду
			incFood();
	    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
	    		// отправляем ведомой (slave) приставке что была съедена белая точка (еда)
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POINT, LINK_TYPES_LENGHT);
	    	}
		} else if (oldRedVal == POWER_FOOD) {
			// поверап
			++powerBonus;

			// звук поедания поверапа
			XGM_startPlayPCM(SFX_SOUND_POWERUP, 15, SOUND_PCM_CH2);

			// обнавляем время когда RED стал съедобным
			redTime = RED_TIME;

			// RED становится съедобным
			redFlag = 0;

	    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
	    		// отправляем ведомой (slave) приставке что был съеден powerup (зеленая точка)
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POWERUP, LINK_TYPES_LENGHT);
	    	}
		} else if (oldRedVal == CHERRY) {
			// вишню
			++cherryBonus;

			// скрыть черешню
			SPR_setPosition(cherrySprite, -90, 100);
			refreshCherry = 0;

			//звук поедания черешни
			XGM_startPlayPCM(SFX_SOUND_CHERRY, 15, SOUND_PCM_CH2);

	    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
	    		// отправляем ведомой (slave) приставке что была съедена черешня
	    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_CHERRY, LINK_TYPES_LENGHT);
	    	}
		}

		map[pacGirlY][pacGirlX] = RED;

		oldRedVal = EMPTY;
	}
	return 0;
}

/**
 * SEGA
 *
 * Алгоритм обработки движения PAC-MAN на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
u8 pacManState() {
	// проверяем, у PACMAN задоно ли направление движения
	if (dx != 0 || dy != 0) {

		// должен ли PACMAN переместиться на новую клетку
		if (pacmanLastUpdateTime == 0) {
			oldX = pacmanX;
			oldY = pacmanY;
			pacmanX = pacmanX + dx;
			pacmanY = pacmanY + dy;

			// сбрасываем счетчик времени
			pacmanLastUpdateTime = PACMAN_SPEED;

			// корректируем координаты PACMAN если надо (чтоб не вышел с поля)
			// если вышел за поле (появление с другой стороны поля)
			moveBound(&pacmanX, &pacmanY);

			// если текущая клетка с едой, увиличиваем счетчик съеденного
			val = map[pacmanY][pacmanX];
			if (val == FOOD) {
				incFood();
		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена белая точка (еда)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POINT, LINK_TYPES_LENGHT);
		    	}
			} else if (val == POWER_FOOD) {
				// RED становится съедобным
				redFlag = 0;
				// бежит в обратную сторону
				dxRed = -dxRed;
				dyRed = -dyRed;

				// RED стал съедобным
				redTime = RED_TIME;

				// и даем еще бонус
				++powerBonus;

				// звук поедания поверапа
				XGM_startPlayPCM(SFX_SOUND_POWERUP, 15, SOUND_PCM_CH2);

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что был съеден powerup (зеленая точка)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POWERUP, LINK_TYPES_LENGHT);
		    	}

			} else if (val == CHERRY) {
				++cherryBonus;

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);
				refreshCherry = 0;

				// звук поедания черешни
				XGM_startPlayPCM(SFX_SOUND_CHERRY, 15, SOUND_PCM_CH2);

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена черешня
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_CHERRY, LINK_TYPES_LENGHT);
		    	}
			}


			if (isNotWallOrDoor(pacmanY, pacmanX)) {
				// если в новой клетке не дверь то в старой делаем пустую клетку
				map[oldY][oldX] = EMPTY;
				drawBlackBox(oldY, oldX);
			} else {
				// если в новой клетке стена WALL или дверь DOOR
				// остаемся на прошлой клетке
				pacmanY = oldY;
				pacmanX = oldX;
				// вектор движения сбрасываем (PACMAN останавливается)
				dx = 0;
				dy = 0;
			}

			// рисуем пакмена в координатах текущей клетки карты
			map[pacmanY][pacmanX] = PACMAN;

			if (MODE_PORT2_MASTER == controllerPort2Mode) {
				// отправляем состояние Pac-Man ведомой (slave) приставке
				pacManStateToTransferObject();
				LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_PAC_MAN_STATE, LINK_TYPES_LENGHT);
			}

			// если съеденны все FOOD и POWER_FOOD - PACMAN выиграл
			if (winner()) {

				// звук выиграша
				XGM_startPlay(victory_vgm);


				dxPacGirl = 0;
				dyPacGirl = 0;
				dx = 0;
				dy = 0;
				dxRed = 0;
				dyRed = 0;

				calcScore();

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что наступил конец игры
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_END_GAME, LINK_TYPES_LENGHT);
		    	}

				return 0;
			}

			// сеъеи ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {

				// звук окончания игры
				XGM_startPlay(fatality_vgm);
				return 0;
			}

		 }

	}
	return 1;
}

/**
 * SEGA
 *
 * Алгоритм обработки движения PAC-GIRL на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
u8 pacGirlState() {
	// проверяем, у pacGirl задоно ли направление движения
	if (dxPacGirl != 0 || dyPacGirl != 0) {

		// если подключился 2 игрок
		if (players == 1) {
			players = 2;
			if (map[pacGirlY][pacGirlX] == FOOD) {
				incFood();

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена белая точка (еда)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POINT, LINK_TYPES_LENGHT);
		    	}
			}
		}

		if (pacGirlLastUpdateTime == 0) {
			oldPacGirlX = pacGirlX;
			oldPacGirlY = pacGirlY;
			pacGirlX = pacGirlX + dxPacGirl;
			pacGirlY = pacGirlY + dyPacGirl;

			pacGirlLastUpdateTime = PACGIRL_SPEED;

			// если вышел за поле (появление с другой стороны поля)
			moveBound(&pacGirlX, &pacGirlY);


			// если текущая клетка с едой, увиличиваем счетчик съеденного
			val = map[pacGirlY][pacGirlX];
			if (val == FOOD) {
				incFood();
		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена белая точка (еда)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POINT, LINK_TYPES_LENGHT);
		    	}
			} else if (val == POWER_FOOD) {
				// RED становится съедобным
				redFlag = 0;
				// бежит в обратную сторону
				dxRed = -dxRed;
				dyRed = -dyRed;


				// RED стал съедобным
				redTime = RED_TIME;

				// и даем еще бонус
				++powerBonus;

				// звук поедания поверапа
				XGM_startPlayPCM(SFX_SOUND_POWERUP, 15, SOUND_PCM_CH2);

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что был съеден powerup (зеленая точка)
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_POWERUP, LINK_TYPES_LENGHT);
		    	}

			} else if (val == CHERRY) {
				++cherryBonus;

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);
				refreshCherry = 0;

				// звук поедания черешни
				XGM_startPlayPCM(SFX_SOUND_CHERRY, 15, SOUND_PCM_CH2);

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что была съедена черешня
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_EAT_CHERRY, LINK_TYPES_LENGHT);
		    	}
			}

			if (isNotWallOrDoor(pacGirlY, pacGirlX)) {
				// если в новой клетке не дверь то в старой делаем пустую клетку
				oldPacGirlVal = val;
				map[oldPacGirlY][oldPacGirlX] = EMPTY;
				drawBlackBox(oldPacGirlY, oldPacGirlX);
			} else {
				// если в новой клетке стена WALL или дверь DOOR
				// остаемся на прошлой клетке
				pacGirlY = oldPacGirlY;
				pacGirlX = oldPacGirlX;
				// вектор движения сбрасываем (PACMAN останавливается)
				dxPacGirl = 0;
				dyPacGirl = 0;
			}

			// рисуем PAC-GIRL в координатах текущей клетки карты
			map[pacGirlY][pacGirlX] = PACGIRL;

			if (MODE_PORT2_MASTER == controllerPort2Mode) {
				// отправляем состояние Pac-Girl ведомой (slave) приставке
				pacGirlStateToTransferObject();
				LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_PAC_GIRL_STATE, LINK_TYPES_LENGHT);
			}

			// если съеденны все FOOD и POWER_FOOD - PACMAN выиграл
			if (winner()) {

				// звук когда выиграли
				XGM_startPlay(victory_vgm);

				dxPacGirl = 0;
				dyPacGirl = 0;
				dx = 0;
				dy = 0;
				dxRed = 0;
				dyRed = 0;

				calcScore();

		    	if (MODE_PORT2_MASTER == controllerPort2Mode) {
		    		// отправляем ведомой (slave) приставке что наступил конец игры
		    		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_END_GAME, LINK_TYPES_LENGHT);
		    	}

				return 0;
			}

			// сеъел ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {
				// звук окончания игры
				XGM_startPlay(fatality_vgm);
				return 0;
			}

		}
	}

	return 1;
}


/**
 * SEGA
 *
 * Алгоритм призрака гоняющегося за PAC-MAN
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
u8 redState() {

	// надо ли RED перейти в режим погони
	if (redTime == 0 ) {
		redFlag = 1;
		// если не двигается, пусть идет вверх
		if (dyRed == 0 && dxRed == 0) {
			dyRed = -1;
			dxRed = 0;
		}
	} else if (redLastUpdateTime == 0 && dyRed == 0 && dxRed == 0) {
		// пока призрак не может двигаться
		redLastUpdateTime = RED_SPEED;
	}

	// проверяем, у RED задоно ли направление движения
	if (dxRed != 0 || dyRed != 0) {
		if (redLastUpdateTime == 0) {
			// можно обновить координаты призрака
			// запоминаем старые координаты призрака
			oldXRed = redX;
			oldYRed = redY;

			// меняем координаты призрака (одеовременно dxRed и dyRed не могут быть больше 0!)
			redX = redX + dxRed;
			redY = redY + dyRed;

			redLastUpdateTime = RED_SPEED;

			// вышли за границы
			moveBound(&redX, &redY);

			if (isNotWall(redY, redX)) {
				// текущие координаты не препядствие
				map[oldYRed][oldXRed] = oldRedVal;
				oldRedVal = map[redY][redX];

				if (redX == 15 && redY >= 7 && redY <= 10) {
					// призрак из дома призрака всегда выходит вверх
					// ну и зайти назад не может!
					dyRed = -1;
					dxRed = 0;
				} else if (dxRed != 0) {
					// призрак двигается по оси X
					if (redFlag && redY != pacmanY) {
						// призрак не съедобен и не догнал Pac-Man
						if (isNotWallOrDoor(redY + 1, redX)	&& isNotWallOrDoor(redY - 1, redX)) {
							// есть альтернативный путь по оси x
							if (abs(redY + 1 - pacmanY) < abs(redY - 1 - pacmanY)) {
								// путь до Pac-Man вниз короче до Pac-Man по Y
								dyRed = 1;
							} else {
								// путь до Pac-Man вверх короче до Pac-Man по Y
								dyRed = -1;
							}
						} else if (isNotWallOrDoor(redY + 1, redX)) {
							// есть путь вниз
							if (abs(redY + 1 - pacmanY) < abs(redY - pacmanY)) {
								// путь вниз короче до Pac-Man по Y
								dyRed = 1;
							}
						} else if (isNotWallOrDoor(redY - 1, redX)) {
							// есть путь вверх
							if (abs(redY - 1 - pacmanY) < abs(redY - pacmanY)) {
								// путь вверх короче до Pac-Man по Y
								dyRed = -1;
							}
						}
					} else {
						// Призрак съедобен
						if (isNotWallOrDoor(redY + 1, redX)) {
							// если есть другой путь выбираем случайно куда пойдет призрак
							dyRed = random() % 2;
						}

						if (isNotWallOrDoor(redY - 1, redX)) {
							// если есть другой путь выбираем случайно куда пойдет призрак
							dyRed = -1 * (random() % 2);
						}
					}

					if (dyRed != 0) {
						// если меняется направление движения с x на y надо обнулить смещение по x
						dxRed = 0;
					}

				} else if (dyRed != 0) {
					// призрак двигается по оси Y
					if (redFlag && redX != pacmanX) {
						// призрак не съедобен и не догнал Pac-Man
						if (isNotWallOrDoor(redY, redX + 1)	&& isNotWallOrDoor(redY, redX - 1)) {
							// есть альтернативный путь выбираем какой короче по X
							if (abs(redX + 1 - pacmanX) < abs(redX - 1 - pacmanX)) {
								// путь впаво короче до Pac-Man по X
								dxRed = 1;
							} else {
								// путь влево короче до Pac-Man по X
								dxRed = -1;
							}
						} else if (isNotWallOrDoor(redY, redX + 1)) {
							// есть альтернативный путь вправо
							if (abs(redX + 1 - pacmanX) < abs(redX - pacmanX)) {
								// путь вправо короче до Pac-Man по X
								dxRed = 1;
							}
						} else if (isNotWallOrDoor(redY, redX - 1)) {
							// слква есть альтернативный путь для призрака
							if (abs(redX - 1 - pacmanX) < abs(redX - pacmanX)) {
								// путь влево короче до Pac-Man по X
								dxRed = -1;
							}

						}
					} else {
						// Призрак съедобен
						if (isNotWallOrDoor(redY, redX + 1)) {
							// если есть другой путь выбираем случайно куда пойдет призрак
							dxRed = random() % 2;
						}

						if (isNotWallOrDoor(redY, redX - 1)) {
							// если есть другой путь выбираем случайно куда пойдет призрак
							dxRed = -1 * (random() % 2);
						}

					}

					if (dxRed != 0) {
						// если меняется направление движения с y на x надо обнулить смещение по y
						dyRed = 0;
					}
				}
			} else {
				// текущие координаты у призрака - припятствие, надо изменить направление движения
				if (redX == 15 && redY >= 7 && redY <= 10) {
					// из дома призрака выходит он всегда вверх
					dyRed = -1;
					dxRed = 0;
				} else {
					// возвращаем призорака в предидущие координаты
					redX = oldXRed;
					redY = oldYRed;

					if (dxRed != 0) {
						// если призрак двигался по оси x надо его остановить
						dxRed = 0;
						if (isNotWallOrDoor(redY + 1, redX)) {
							// можно двигатся вниз
							dyRed = 1;
						} else if (isNotWallOrDoor(redY - 1, redX)) {
							// можно двигаться вверх
							dyRed = -1;
						}
					} else {
						// если двигались по оси y
						dyRed = 0;
						if (isNotWallOrDoor(redY, redX + 1)) {
							// можем двигаться вправо
							dxRed = 1;
						} else if (isNotWallOrDoor(redY, redX - 1)) {
							// можем двигаться влево
							dxRed = -1;
						}
					}
				}

			}

			if (MODE_PORT2_MASTER == controllerPort2Mode) {
				// отправляем состояние призрака ведомой (slave) приставке
				redStateToTransferObject();
				LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_RED_STATE, LINK_TYPES_LENGHT);
			}

			// сеъеи ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {
				// музыка оканчания игры
				XGM_startPlay(fatality_vgm);
				// Pac-Man съеден, игра закончена
				return 0;
			}

		}
	}

	if (redFlag) {
		// отображаем спрайт красного призрака
		map[redY][redX] = RED;
	} else {
		// отображаем спрайт убегающего призрака
		map[redY][redX] = SHADOW;
	}

	// Pac-Man не съеден, игра продолжается
	return 1;
}


/**
 *  SEGA
 *
 *  Перерисовать на бекграунде tile когда съели точку
 *  рисуем черный квадрат 8x8
 */
void drawBlackBox(s16 y, s16 x) {
	// отображаем путой квадрат из Русских Tile
	VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 1, FALSE, FALSE, FIRST_INDEX_RUSSIAN_TILE), x + 4, y);
}


/**
 * SEGA
 *
 * Рисуем Русский текст
 *
 * plane - слой, на котором будет расположен тайл
 * vramOffsets - массив смещений относительно FIRST_INDEX_RUSSIAN_TILE
 * length - количество символов которые нужно отобразить на экране (размер массива vramOffsets)
 * xTile - x координата где нарисовать текст
 * yTile - y координата где нарисовать текст
 */
void drawRussianText(VDPPlane plane, const u8 *vramOffsets, u8 length, u8 xTile, u8 yTile) {
	// полчаем размер plane по горизонтали
    u16 pw = (plane == WINDOW) ? windowWidth : planeWidth;

    // размер по вертикали
    u16 ph = (plane == WINDOW) ? 32 : planeHeight;


    if ((xTile >= pw) || (yTile >= ph)) {
    	// не нужно рисовать текст в не видимой области экрана
        return;
    }


    // сколько можем отрисовать на экране
    u16 n = length;

    // если строка полностью не влезит на экране, обризаем ее
    // на столько чтоб влезла
    if (n > (pw - xTile)) {
        n = pw - xTile;
    }


    // смещение по x
    u16 curX = xTile;

    // номер тайла в VRAM
	u16 curTileInd = 0;


    for(u16 i = 0; i < n; i++, curX++) {

    	// вычисляем номер Tile в VRAM
        curTileInd = FIRST_INDEX_RUSSIAN_TILE - 1 + vramOffsets[i];

        // рисуем Tile используя палитру PAL0,
        // curTileInd - номер Tile в VRAM
        // в позиции curX, y
		VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL0, 1, FALSE, FALSE, curTileInd), curX, yTile);
	}
}


/**
 * SEGA
 *
 * Для отладки, в буфер text положить значение числа в виде символов
 * 4 символа типа char число выводится в шеснадцатиричном формате
 * например десятичное 65535 будет выведено как FFFF
 *
 * val чтосло которое хотим вывести на экран
 */
void printU16(u16 val) {
	u8 ch = '0';

	for (s16 j = 3; j >= 0; j--) {
		ch = (val & LCP_LO_BITS);
		switch (ch) {
		case 10:
			ch = 'A';
			break;
		case 11:
			ch = 'B';
			break;
		case 12:
			ch = 'C';
			break;
		case 13:
			ch = 'D';
			break;
		case 14:
			ch = 'E';
			break;
		case 15:
			ch = 'F';
			break;
		default:
			ch = ch + '0';
		}
		text[j] = ch;

		// сдвиг на 4 бита в право
		val >>= 4;
	}
}


/**
 *  SEGA
 *
 *  Нарисовать бонусы, очки
 *  результат игры (GAME OVER или YOU WINNER)
 */
void drawText() {

	switch (showLinkCableErrors) {
		case SHOW_LINK_CABLE_LAST_ERROR:
			// выводим на экран ошибки при передаче через Link Cadle
			printU16(linkCableErrors);
			VDP_drawText(text, 28, 25);
		break;


		case SHOW_LINK_CABLE_ERROS_COUNT:
			// выводим на экран количество ошибок при передаче через Link Cadle
			printU16(linkCableErrorsCount);
			VDP_drawText(text, 28, 25);
		break;


		case SHOW_LINK_CABLE_FRAME_COUNT:
			//  количество отресованных фреймов с начала создания соединения через Link Cadle
			printU16(linkCableFrameCount);
			VDP_drawText(text, 28, 25);
		break;
	}

	if (STATE_SCREENSAVER != gameState) {
		// задаем цвет текста для функции drawRussianText() БЕЛЫЙ
		PAL_setColor(1, RGB24_TO_VDPCOLOR(0xffffff));

		// играем ли мы через Link Cable на двух приставках или играем на одной приставке одним или двумя контроллерами
		// "ВЕДУЩАЯ ПРИСТАВКА    " - (TEXT_LINK_MASTER) игра через Link Cable на двух приставках и наша приставка ведущая (master)
		// "LВЕДОМАЯ ПРИСТАВКА   " - (TEXT_LINK_SLAVE) игра через Link Cable на двух приставках и наша приставка ведомая (slave)
		// "2 ИГРОКА НЕ СЕТЕВАЯ! " - (TEXT_2P_NO_LINK) игра не через Link Cable и в 2 порту подключен контроллер, играет 2 игрока на одной приставке
		// "1 ИГРОК НЕ СЕТЕВАЯ!  " - (TEXT_1P_NO_LINK) игра не через Link Cable и в 2 порту нет контроллера, играет 1 игрок
		// "ПЫТАЕМСЯ БЫТЬ ВЕДУЩЕЙ" - (TEXT_TRY_MASTER) попытка создать соединение с другой приставкой в качестве ведущей (master)
		// "ПЫТАЕМСЯ БЫТЬ ВЕДОМОЙ" - (TEXT_TRY_SLAVE) попытка создать соединение с другой приставкой в качестве ведомой (slave)
		drawRussianText(BG_A, gameModeText, GAME_MODE_TEXT_SIZE, 11, 25);
	}

	memset(text, 0, 4);

	if (STATE_GAME == gameState || STATE_RESULT == gameState) {
		// идет игра или отображаем результат игры

		// задаем цвет текста для функции VDP_drawText() БЕЛЫЙ
		PAL_setColor(15,RGB24_TO_VDPCOLOR(0xffffff));
		// задаем цвет текста для функции drawRussianText() БЕЛЫЙ
		PAL_setColor(1, RGB24_TO_VDPCOLOR(0xffffff));

		// количество съеденых черешень
		text[0] = cherryBonus + '0';
		text[1] = 0;
		VDP_drawText(text, 10, 26);

		// количество съеденных призраков
		text[0] = redBonus + '0';
		VDP_drawText(text, 10, 24);

        // количество съеденных поверапов
		text[0] = powerBonus + '0';
		VDP_drawText(text, 27, 26);


		// количество съеденных серых точек (еды)
		text[0] = food100 + '0';
		text[1] = food010 + '0';
		text[2] = food001 + '0';
		VDP_drawText(text, 27, 24);
		VDP_drawText("/271", 30, 24);
	}

	if (STATE_RESULT == gameState) {
		SYS_doVBlankProcess();
		// отображаем результат игры
		if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {
			// задаем цвет текста для функции VDP_drawText() ЗЕЛЕНЫЙ
			PAL_setColor(15,RGB24_TO_VDPCOLOR(0x00ff00));
			// задаем цвет текста для функции drawRussianText() ЗЕЛЕНЫЙ
			PAL_setColor(1, RGB24_TO_VDPCOLOR(0x00ff00));

			// если победили пишем 'ТЫ ВЫИГРАЛ'
			drawRussianText(BG_A, TEXT_YOU_WINNER, TEXT_GAME_OVER_SIZE, 13, 24);
		} else {
			// задаем цвет текста для функции VDP_drawText() КРАСНЫЙ
			PAL_setColor(15,RGB24_TO_VDPCOLOR(0xff0000));
			// задаем цвет текста для функции drawRussianText() КРАСНЫЙ
			PAL_setColor(1, RGB24_TO_VDPCOLOR(0xff0000));

			// если проиграли пишем 'КОНЕЦ ИГРЫ'
			drawRussianText(BG_A, TEXT_GAME_OVER, TEXT_GAME_OVER_SIZE, 13, 24);
		}
		// пишем 'ОЧКИ'
		drawRussianText(BG_A, TEXT_SCORE, TEXT_SCORE_SIZE, 13, 26);


		// отображаем количество полученных очков
		// с учетом всех бонусов
		// черешня 200 очков
		// съеденный призрак 50 очков
		// поверап 25 очков
		// серая точка (еда) 1 очко


		text[0] = score100 + '0';
		text[1] = score010 + '0';
		text[2] = score001 + '0';
		VDP_drawText(text, 20, 26);
	}

}


/**
 * SEGA
 *
 * Нарисовать спрайты
 */
void drawSprites() {
	switch (gameState) {
		case STATE_SCREENSAVER:
			// заставка

			screensaver();
		break;
		case STATE_SELECT:
			// стартовый экран

			if (players == 1) {
				// надо спрятать Pac-Girl
				SPR_setPosition(pacGirlSprite, -100, 90);

				// если выбрана игра за одного (только Pac-Man)
				// надо нарисовать спрайт PAC-MAN перед 1 PLAYER
				SPR_setAnim(pacmanSprite, 0);
				SPR_setHFlip(pacmanSprite, FALSE);
				SPR_setPosition(pacmanSprite, 100, 100);
			} else {
				// если выбрана игра на 2х игроков
				if (P1_PACGIRL__P2_PACMAN == switchPlayers) {
					// (1 игрок за Pac-Girl, 2 игрок за Pac-Man)

					// нарисовать спрайт PAC-Girl перед 2 PLAYERS
					SPR_setAnim(pacGirlSprite, 0);
					SPR_setHFlip(pacGirlSprite, FALSE);
					SPR_setPosition(pacGirlSprite, 100, 113);

					// нарисовать спрайт PAC-MAN после 2 PLAYERS
					SPR_setAnim(pacmanSprite, 0);
					SPR_setHFlip(pacmanSprite, TRUE);
					SPR_setPosition(pacmanSprite, 190, 113);
				} else {
					 // (1 игрок за Pac-Man, 2 игрок за Pac-Girl)

					// нарисовать спрайт PAC-MAN перед 2 PLAYERS
					SPR_setAnim(pacmanSprite, 0);
					SPR_setHFlip(pacmanSprite, FALSE);
					SPR_setPosition(pacmanSprite, 100, 113);

					// нарисовать спрайт PAC-Girl после 2 PLAYERS
					SPR_setAnim(pacGirlSprite, 0);
					SPR_setHFlip(pacGirlSprite, TRUE);
					SPR_setPosition(pacGirlSprite, 190, 113);
				}
			}
		break;
		case STATE_GAME:
		case STATE_RESULT:
			// если идет игра или показываем результаты игры

			// отрисовываем спрайты игры
			// PAC-MAN, Pac-Girl, RED или SHADOW, дверь, черешню
			refreshGame();
		break;
		case STATE_PAUSE:
			// пауза игры

			// нарисвовать спрайт с словом PAUSE
			SPR_setAnim(pauseSprite, 0);
			SPR_setPosition(pauseSprite, pauseX, pauseY);

			// нарисавать спрайт бегущего Соника
			SPR_setAnim(sonicSprite, 3);
			SPR_setPosition(sonicSprite, sonicX, sonicY);
		break;
	}
}


/**
 * SEGA
 *
 * Нарисовать только объект из карты map[i][j]
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void draw(s16 i, s16 j) {
	drawSprite(i, j,  map[i][j]);
}


/**
 * SEGA
 *
 * Нарисовать переданный объект в координатах
 * i - строка в массиве карты
 * j - столбец в массиве карты
 * val - Что нарисовать в координатах
 */
void drawSprite(s16 i, s16 j, u8 val) {
	// x = i * 8 и на 29 пиксела вправо
    x = (j << 3) + 29;

    // y = j * 8 и на 2 пиксела вверх
    y = (i << 3) - 2;

    if (val == PACMAN) {
		if (dx < 0) {
			// движение налево PACMAN
			SPR_setAnim(pacmanSprite, 0);
			SPR_setHFlip(pacmanSprite, TRUE);
			SPR_setPosition(pacmanSprite, x, y);
		} else if (dx > 0) {
			// движение направо PACMAN
			SPR_setAnim(pacmanSprite, 0);
			SPR_setHFlip(pacmanSprite, FALSE);
			SPR_setPosition(pacmanSprite, x, y);
		} else if (dy < 0) {
			// движение вверх PACMAN
			SPR_setAnim(pacmanSprite, 1);
			SPR_setVFlip(pacmanSprite, FALSE);
			SPR_setPosition(pacmanSprite, x, y);
		} else if (dy > 0) {
			// движение вниз PACMAN
			SPR_setAnim(pacmanSprite, 1);
			SPR_setVFlip(pacmanSprite, TRUE);
			SPR_setPosition(pacmanSprite, x, y);
		} else {
			// стоит на месте PACMAN
			SPR_setAnim(pacmanSprite, 2);
			SPR_setPosition(pacmanSprite, x, y);
		}
    } else if (val == RED) {
        if (dxRed < 0) {
          	// движение налево RED
        	SPR_setAnim(redSprite, 0);
        	SPR_setHFlip(redSprite, TRUE);
        	SPR_setPosition(redSprite, x, y);
		} else if (dxRed > 0) {
			// движение направо  RED
        	SPR_setAnim(redSprite, 0);
        	SPR_setHFlip(redSprite, FALSE);
        	SPR_setPosition(redSprite, x, y);
		} else if (dyRed > 0) {
			// движение вниз RED
        	SPR_setAnim(redSprite, 2);
        	SPR_setPosition(redSprite, x, y);
		} else if (dyRed < 0) {
			// движение вверх RED
			SPR_setAnim(redSprite, 1);
        	SPR_setPosition(redSprite, x, y);
		} else {
			// стоит на месте RED
			SPR_setAnim(redSprite, 4);
        	SPR_setPosition(redSprite, x, y);
		}

    } else if (val == SHADOW) {
		// логика для проигрования звука когда можно есть
    	// призрака
    	if (shadowLastSoundTime == 0) {
    		// звук когда призрак съедобен
    		XGM_startPlayPCM(SFX_SOUND_SHADOW, 15, SOUND_PCM_CH3);
    		shadowLastSoundTime = 20;
    	}

    	if (dxRed != 0 || dyRed != 0) {
    		// движение призрака
    		SPR_setAnim(redSprite, 3);
    		SPR_setPosition(redSprite, x, y);
    	} else {
    		// призрак стоит
    		SPR_setAnim(redSprite, 5);
    		SPR_setPosition(redSprite, x, y);
    	}
    } else if (val == PACGIRL) {
		if (dxPacGirl < 0) {
			// движение налево PACGIRL
			SPR_setAnim(pacGirlSprite, 0);
			SPR_setHFlip(pacGirlSprite, TRUE);
			SPR_setVFlip(pacGirlSprite, FALSE);
			SPR_setPosition(pacGirlSprite, x, y);
		} else if (dxPacGirl > 0) {
			// движение направо PACGIRL
			SPR_setAnim(pacGirlSprite, 0);
			SPR_setHFlip(pacGirlSprite, FALSE);
			SPR_setVFlip(pacGirlSprite, FALSE);
			SPR_setPosition(pacGirlSprite, x, y);
		} else if (dyPacGirl < 0) {
			// движение вверх PACGIRL
			SPR_setAnim(pacGirlSprite, 1);
			SPR_setVFlip(pacGirlSprite, FALSE);
			SPR_setHFlip(pacGirlSprite, FALSE);
			SPR_setPosition(pacGirlSprite, x, y);
		} else if (dyPacGirl > 0) {
			// движение вниз PACGIRL
			SPR_setAnim(pacGirlSprite, 1);
			SPR_setVFlip(pacGirlSprite, TRUE);
			SPR_setHFlip(pacGirlSprite, FALSE);
			SPR_setPosition(pacGirlSprite, x, y);
		} else {
			// стоит на месте PACGIRL
			SPR_setAnim(pacGirlSprite, 2);
			SPR_setHFlip(pacGirlSprite, FALSE);
			SPR_setVFlip(pacGirlSprite, FALSE);
			SPR_setPosition(pacGirlSprite, x, y);
		}


    } else if (val == CHERRY) {
    	// черешня
    	SPR_setPosition(cherrySprite, x, y);
    } else if (val == DOOR) {
    	// двеь
    	SPR_setPosition(doorSprite, x, y);
    }

}


/**
 * SEGA
 *
 * Обновить карту, персонажей, двери, черешню на экране
 */
void refreshGame() {
	if (!cherryFlag && redTime == 0 && !cherryBonus && cherryTime == 0) {
		// открыть двери
		openDoors();

		if (MODE_PORT2_MASTER == controllerPort2Mode) {
			transferObject[0] = 0xAF;
			transferObject[1] = 0xFA;
			LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_OPEN_DOOR, LINK_TYPES_LENGHT);
		}

	}

    if (refreshDoor) {
		if (map[doorY][doorX] != DOOR) {
			refreshDoor = 0;
			SPR_setPosition(doorSprite, -90, 100);

			if (MODE_PORT2_MASTER == controllerPort2Mode) {
				transferObject[0] = 0xAF;
				transferObject[1] = 0xFA;
				LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_OPEN_DOOR, LINK_TYPES_LENGHT);
			}

		} else {
			// рисуем дверь
			draw(doorY, doorX);
		}
    }

    if (refreshCherry) {
		if (map[cherryY][cherryX] != CHERRY) {
			SPR_setPosition(cherrySprite, -90, 100);
		} else {
		    // рисуем черешню
	    	drawSprite(cherryY, cherryX, CHERRY);
		}
    }

    // рисуем призрака RED
    draw(redY, redX);


    // рисуем Pac-Man
    draw(pacmanY, pacmanX);


    // рисуем Pac-Girl
    draw(pacGirlY, pacGirlX);
}


/**
 * SEGA
 *
 * Нарисовать задний фон
 */
void drawBackground() {
	switch(gameState) {
		case STATE_SCREENSAVER:
			// заставка

			// рисуем задний фон лого SEGA из sega.png
			VDP_drawImage(BG_A, &sega_image, 0, 0);
		break;
		case STATE_SELECT:
			// стартовый бекграунд

			// рисуем в качестве заднего фона меню выбора количества игроков из menu.png
			VDP_drawImage(BG_A, &menu_image, 0, 0);
		break;
		default:
			// любой другой экран

			// карта уровня с лабиринтом
			// рисуем в качестве заднего фона карту уровня из map.png
			VDP_drawImage(BG_A, &map_image, 0, 0);
	}
}


/**
 * SEGA
 *
 *  Обработка нажатых кнопок игроками во время заставки, когда поедают надпись SEGA
 *  для gameState == STATE_SCREENSAVER
 */
void actionsStateScreensaver() {
	// заставка
	// условие окончания анимации заставки
	// тупо закончилась анимация: Pac-Girl X координата >= 380
	// или нажали любую кнопку на одном из Джойстиков (кроме Start)
	if ((pacGirlX >= 380)
			|| (pad1 & BUTTON_A)  || (pad1 & BUTTON_B) || (pad1 & BUTTON_C)
			|| (pad1 & BUTTON_X)  || (pad1 & BUTTON_Y) || (pad1 & BUTTON_Z)
			|| (pad2 & BUTTON_A)  || (pad2 & BUTTON_B) || (pad2 & BUTTON_C)
			|| (pad2 & BUTTON_X)  || (pad2 & BUTTON_Y) || (pad2 & BUTTON_Z)) {
		// нужно всех персонажей убрать с экрана
		// для этого ставим их в начальное состояние перед запуском заставки
		initScreensaver();
		// выбор количества игроков для игры
		gameState = STATE_SELECT;
		// рисуем задний фон с меню выбора игроков
		drawBackground();
		// останавливаем проигрывание SEGA
		XGM_stopPlayPCM(SOUND_PCM_CH2);
		// музыка играющая когда отображаем меню выбора количества игроков
		XGM_startPlay(contrah_vgm);
	}
}


/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры во время отоброжения экрана с меню выбора количества игроков
 * для gameState == STATE_SELECT
 */
void actionsStateSelectPlayers() {
	// экран выбор количества игроков

	if (((pad1 & BUTTON_START) || (pad2 & BUTTON_START)) && playersTime == 0) {
		// нажат Start на 1 или 2 джойстике
		// останавливаем проигрывание музыки на экране выбора игроков
		XGM_pausePlay();

		// сбросить игру в стартовое состояние
		// начальное положение персонажей, обнулить очки, и т.д.
		resetGame();

		if (players != 2) {
			// выбрана игра за 1го (1 PLAYER)

			// первый игрок всегда в оденочную игру управляет Pac-Man
			switchPlayers = P1_PACMAN__P2_PACGIRL;

			// сбрасываем все что нажато на 2 контроллере
			pad2 = 0;

			// выводим на экран что соединения с другой притавкой нет, играем в одного
			memcpy(gameModeText, TEXT_1P_NO_LINK, GAME_MODE_TEXT_SIZE);

			if ((MODE_PORT2_MASTER == controllerPort2Mode || MODE_PORT2_SLAVE == controllerPort2Mode)) {
				// устанавливем режим работы второго порта - не участвует в игре, игрок нашей приставки
				// играет в одного на своей приставке контроллером подключенным в первый порт
				controllerPort2Mode = MODE_SINGLE_PLAYER;

				// есть соединение по Link cable
				// закрываем порт, данные больше не будут пересылатся через Link cable
				LCP_close();

				// защита от двойного нажатия на start
				playersTime = 30;

				// звук разъединения соединения
				XGM_startPlayPCM(SFX_SOUND_DISCONNECT_LINK_CABLE, 15, SOUND_PCM_CH3);

				// включаем музыку когда отображаем меню выбора количества игроков
				XGM_startPlay(contrah_vgm);

				// выходим, игру пока не начинаем надо еще раз нажать на start
				return;
			}
			// игрок 1 убираем с карты PAC-GIRL
			map[pacGirlY][pacGirlX] = FOOD;
		} else {
			// выбрана игра на 2их (2 PLAYERS)

			if (!(MODE_PORT2_MASTER == controllerPort2Mode || MODE_PORT2_SLAVE == controllerPort2Mode)) {
				// если еще не определили что у нас воткнуто во 2 порт приставки
				// или это режимы отличные от игры в 2м по Link cable
				// определяем что воткнуто (контроллер, Link cable или ничего)
				// и кем будет какая приставка (ведущей - master, ведомой - slave)
				initControllerPort2();
			}

			if (controllerPort2Mode == MODE_SINGLE_PLAYER) {
				// не удалось запустить игру на 2х
				// нет соединения через Link cabile
				// и не вставлен 2 контроллер в 2 порт SEGA

			   // включаем музыку когда отображаем меню выбора количества игроков
			   XGM_startPlay(contrah_vgm);

			   return;
			}

			// надо дать очки за точку
			// которая на месте PAC-GIRL была
			incFood();
		}

		// начинаем иру
		gameState = STATE_GAME;

		// рисуем задний фон с лабиринтом для игры
		drawBackground();

		// музыка играющая во время игры
		XGM_startPlay(comicszone_vgm);

		// повторное нажатие на разрешено только через 30 секунд (чтоб сразу не нажалась пауза)
		playersTime = 30;
		return;
	}


	// 1 или 2 игрока будут играть - выбор стрелочками
	if (((pad1 & BUTTON_DOWN) || (pad2 & BUTTON_DOWN)) && (players == 1)) {
		// Нажата кнопка вверх на 1 или 2 джойстике
		players = 2;
		return;
	}


	if (((pad1 & BUTTON_UP) || (pad2 & BUTTON_UP)) && (players == 2)) {
		// Нажата кнопка вниз на 1 или 2 джойстике
		players = 1;
		return;
	}

	if (((pad1 & BUTTON_RIGHT) || (pad2 & BUTTON_RIGHT)) && (switchPlayers == P1_PACMAN__P2_PACGIRL) && (players == 2)) {
		// Нажата кнопка вправо на 1 или 2 джойстике
		switchPlayers = P1_PACGIRL__P2_PACMAN;
		return;
	}

	if (((pad1 & BUTTON_LEFT) || (pad2 & BUTTON_LEFT)) && (switchPlayers == P1_PACGIRL__P2_PACMAN) && (players == 2)) {
		// Нажата кнопка влево на 1 или 2 джойстике
		switchPlayers = P1_PACMAN__P2_PACGIRL;
		return;
	}
}


/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры во время паузы
 * для gameState == STATE_PAUSE
 */
void actionsStatePause() {
	if (((pad1 & BUTTON_START) || (pad2 & BUTTON_START)) && playersTime == 0 && MODE_PORT2_SLAVE != controllerPort2Mode) {
		resumeGame();
		if (MODE_PORT2_MASTER == controllerPort2Mode) {
			// отправляем ведомой (slave) приставке сообщение что надо выйти из режима паузы
			LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_RESUME_GAME, LINK_TYPES_LENGHT);
		}
		return;
	}

	sonicX+=dxSonic;

	if (sonicX > 340) {
		dxSonic = 0;

		pauseX+=pauseDX;
		pauseY+=pauseDY;

		if (pauseY <= -18 || pauseY >= 200) {
			pauseDY = -pauseDY;
		}

		if (pauseX <= 1 || pauseX >= 280) {
			pauseDX = -pauseDX;
		}

	} else {
		pauseX+= (dxSonic - 1);
	}
}


/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры во время непосредственно игры
 * для gameState == STATE_GAME
 */
void actionsStateGame() {
	if (MODE_PORT2_SLAVE != controllerPort2Mode) {
		// если не ведомая приставка (slave), то нужно чтоб отработала логика игры
		if (((pad1 & BUTTON_START) || (pad2 & BUTTON_START)) && playersTime == 0) {
			// нажат start на 1 или 2 контроллере. с защитой от многократного нажатия
			pause();
			if (MODE_PORT2_MASTER == controllerPort2Mode) {
				// отправляем ведомой (slave) приставке сообщение что надо перейти в режим паузы
				LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_PAUSE, LINK_TYPES_LENGHT);
			}

			return;
		}

		if (pad1 &  BUTTON_LEFT) {
			// нажата кнопка влеао на 1 джойстике
			dx = -1;
			dy = 0;
		}

		if (pad1 & BUTTON_RIGHT) {
			// нажата кнопка вправо на 1 джойстике
			dx = 1;
			dy = 0;

		}

		if (pad1 & BUTTON_UP) {
			// нажата кнопка вверх на 1 джойстике
			dy = -1;
			dx = 0;
		}

		if (pad1 & BUTTON_DOWN) {
			// нажата кнопка вниз на 1 джойстике
			dy = 1;
			dx = 0;
		}

		if (pad2 & BUTTON_UP) {
			// нажата кнопка вверх на 2 джойстике
			dyPacGirl = -1;
			dxPacGirl = 0;
		}

		if (pad2 & BUTTON_DOWN) {
			// нажата кнопка вниз на 2 джойстике
			dyPacGirl = 1;
			dxPacGirl = 0;
		}

		if (pad2 & BUTTON_LEFT) {
			// нажата кнопка влево на 2 джойстике
			dxPacGirl = -1;
			dyPacGirl = 0;
		}

		if (pad2 & BUTTON_RIGHT) {
			// нажата кнопка вправо на 2 джойстике
			dxPacGirl = 1;
			dyPacGirl = 0;
		}

		// двигаем Pac-Man
		if (!pacManState()) {
			// игра окончена
			gameState = STATE_RESULT;
			return;
		}

		// двигаем RED
		if (!redState()) {
			// игра окончена
			gameState = STATE_RESULT;
			return;
		}


		// двигаем Pac-Girl
		if (!pacGirlState()) {
			// игра окончена
			gameState = STATE_RESULT;
			return;
		}


		if (redLastUpdateTime > 0) {
			// счетчик для анимации RED и SHADOW
			--redLastUpdateTime;
		}


		if (cherryTime > 0) {
			// счетчик когда надо показать черешню
			// и отрыть к ней дверь
			--cherryTime;
		}

		if (redTime > 0) {
			// счетчик когда SHADOW вновь станет RED
			--redTime;
		}
	}

	if (pacmanLastUpdateTime > 0 ) {
		// счетчик для анимации Pac-Man
		--pacmanLastUpdateTime;
	}

	if (pacGirlLastUpdateTime > 0) {
		// счетчик для анимации Pac-Girl
		--pacGirlLastUpdateTime;
	}

	if (shadowLastSoundTime > 0) {
		--shadowLastSoundTime;
	}
}


/**
 * SEGA
 *
 * Обработка нажатых кнопок игроками и логика игры когда показываем экран с результатом после окончаня игры
 * gameState == STATE_RESULT
 */
void actionsStateResult() {
	if ((pad1 & BUTTON_START) || (pad2 & BUTTON_START)) {
		// показываем результат игры  и на этом экране
		// нажат Start на 1 или 2 джойстике
		// переходим на экран заставки
		gameState = STATE_SCREENSAVER;
		// защита от 2го нажатия кнопки Start
		playersTime = 30;
		// нужно всех персонажей убрать с экрана
		// для этого ставим их в начальное состояние перед запуском заставки
		initScreensaver();
		// нарисовать SEGA на заднем фоне
		drawBackground();
		// останавливаем проигрывание музыки
		XGM_stopPlay();
		// проигрываем звук SEGA!
		XGM_startPlayPCM(SFX_SOUND_SEGA, 15, SOUND_PCM_CH2);
	}
}


/**
 *  SEGA
 *
 *  Обработка нажатых кнопок игроком и основная логика игры на основании действий игроков
 */
void actions() {

	// задержка для обработки нажатия кнопок (если нужна)
	if (playersTime > 0) {
		// защита от двойного нажатия,
		// когда playersTime станет равным 0 обработчик заработает  опять
		--playersTime;
	}


	if (((pad1 & BUTTON_A) && (pad1 & BUTTON_C)) || ((pad2 & BUTTON_A) && (pad2 & BUTTON_C))) {
		// вывод на экран ошибок при работе с Link Cable Protocol
		showLinkCableErrors = SHOW_LINK_CABLE_LAST_ERROR;
	}

	if (((pad1 & BUTTON_B) && (pad1 & BUTTON_C)) || ((pad2 & BUTTON_B) && (pad2 & BUTTON_C))) {
		// вывод на экран количества ошибок при работе с Link Cable Protocol
		showLinkCableErrors = SHOW_LINK_CABLE_ERROS_COUNT;
	}

	if (((pad1 & BUTTON_A) && (pad1 & BUTTON_X)) || ((pad2 & BUTTON_A) && (pad2 & BUTTON_X))) {
		// вывод на экран количества ошибок при работе с Link Cable Protocol
		showLinkCableErrors = SHOW_LINK_CABLE_FRAME_COUNT;
	}

	if (((pad1 & BUTTON_A) && (pad1 & BUTTON_B)) || ((pad2 & BUTTON_A) && (pad2 & BUTTON_B))) {
		// сброс ошибок при передаче данных в 0 на экране
		linkCableErrors = 0;
		// сброс количества ошибок
		linkCableErrorsCount = 0;
		// сброс количества кадров
		linkCableFrameCount = 0;
	}

	switch (gameState) {
		case STATE_SCREENSAVER:
				// заставка
				actionsStateScreensaver();
		break;
		case STATE_SELECT:
				// экран выбор количества игроков
				actionsStateSelectPlayers();
		break;
		case STATE_PAUSE:
				// пауза (при нажатии start во время игры)
				actionsStatePause();
		break;
		case STATE_GAME:
				// идет ира
				actionsStateGame();
		break;
		case STATE_RESULT:
				// результат игры
				actionsStateResult();
		break;
	}
}


/**
 * SEGA
 *
 * сбрасываем значения переменных для отображения заставкии
 * и отрисовываем их в новых местах
 */
void initScreensaver() {
    sonicX = -95;
    sonicY = 83;
    dxSonic = 1;
    pacmanX = -100;
    pacmanY = 105;
    dx = 1;
    redX = 400;
    redY = 103;
    dxRed = 0;
    pacGirlX = -100;
    pacGirlY = 90;
    dxPacGirl = 0;
    doorVal = DOOR;
    cherryVal = EMPTY;

    SPR_setAnim(pacmanSprite, 0);
    SPR_setAnim(redSprite, 0);
    SPR_setAnim(pacGirlSprite, 0);

	SPR_setHFlip(redSprite, FALSE);
	SPR_setVFlip(redSprite, FALSE);
	SPR_setHFlip(pacmanSprite, FALSE);
	SPR_setVFlip(pacmanSprite, FALSE);
	SPR_setHFlip(pacGirlSprite, FALSE);
	SPR_setVFlip(pacGirlSprite, FALSE);

    // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
    SPR_setAnim(sonicSprite, 2);

    // изменяем позицию спрайта Соника
    SPR_setPosition(sonicSprite, sonicX, sonicY);

    // изменяем позицию спрайта Pac-Man
    SPR_setPosition(pacmanSprite, pacmanX, pacmanY);

    // изменяем позицию спрайта RED
    SPR_setPosition(redSprite, redX, redY);

    // изменяем позицию спрайта Pac-Girl
    SPR_setPosition(pacGirlSprite, pacGirlX, pacGirlY);

	// скрыть черешню
	SPR_setPosition(cherrySprite, -90, 100);

	// скрыть дверь
	SPR_setPosition(doorSprite, -90, 100);
}


/**
 * SEGA
 *
 * отображаем заставку
 * Sonic убегающий от Pac-Man и надпись SEGA которую съест Pac-Man и Pac-Girl
 * затем Pac-Man убегает от призрака
 */
void screensaver() {
   SPR_setAnim(pacmanSprite, 0);
   SPR_setAnim(redSprite, 0);
   SPR_setAnim(pacGirlSprite, 0);

   // отоброзить по горизонтали спрайт RED
   SPR_setHFlip(redSprite, TRUE);

   if (dx < 0) {
	   SPR_setHFlip(pacmanSprite, TRUE);
   } else {
	   SPR_setHFlip(pacmanSprite, FALSE);
   }

   // задаем какую анимацию использовать
   if (sonicX <= 30)  {
	   // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
	   SPR_setAnim(sonicSprite, 2);
   } else if (sonicX <= 50)  {
	   dxSonic = 2;
	   // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
	   SPR_setAnim(sonicSprite, 2);
   } else if (sonicX <= 70) {
	   dxSonic = 3;
	   // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
	   SPR_setAnim(sonicSprite, 2);
	   XGM_startPlay(sonic_vgm);
   } else if (sonicX <= 120) {
	   dxSonic = 5;
	   // соник бежит - 3 строчка анимации в файле sonic.png
	   SPR_setAnim(sonicSprite, 3);
   } else {
	   dxSonic = 6;
	   // появляется RED
	   dxRed = -1;
	   dxPacGirl = 1;
	   // соник бежит - 3 строчка анимации в файле sonic.png
	   SPR_setAnim(sonicSprite, 3);
   }

   if (pacmanX > 90 && pacmanX < 190 && dx > 0) {
	   // звук поедания
	   if (pacmanLastUpdateTime <= 0) {
		   XGM_startPlayPCM(SFX_SOUND_EAT, 15, SOUND_PCM_CH2);
		   pacmanLastUpdateTime = 10;
	   } else {
		   pacmanLastUpdateTime--;
	   }
   }

   if (pacGirlX > 90 && pacGirlX < 190) {
	   if (pacGirlLastUpdateTime <= 0) {
		   XGM_startPlayPCM(SFX_SOUND_EAT, 15, SOUND_PCM_CH2);
		   pacGirlLastUpdateTime = 10;
	   } else {
		   pacGirlLastUpdateTime--;
	   }
   }

   if (sonicX < 320) {
	   // вычисляем новые координаты нахождения Соника
	   sonicX += dxSonic;
   }

   if (pacmanX < 210) {
	   // вычисляем новые координаты нахождения Pac-Man
	   pacmanX += dx;
   } else {
	   // пора убегать от RED
	   dx = -1;
	   pacmanX = 209;
   }

   if (redX > -16) {
	   // вычисляем новые координаты нахождения RED
	   redX += dxRed;
   }

   pacGirlX+= dxPacGirl;


   // изменяем позицию спрайта Соника
   SPR_setPosition(sonicSprite, sonicX, sonicY);

   // изменяем позицию спрайта Pac-Man
   SPR_setPosition(pacmanSprite, pacmanX, pacmanY);

   // изменяем позицию спрайта RED
   SPR_setPosition(redSprite, redX, redY);

   // изменяем позицию спрайта Pac-Girl
   SPR_setPosition(pacGirlSprite, pacGirlX, pacGirlY);


   // стираем на фоне то что съел Pac-Man
   if (pacmanX >=0 && pacmanX <= 320) {
	   // рисуем tile на бакграунде в координатах Pac-Man (он съест SEGA)
	   VDP_setTileMapXY(BG_A, 1, (pacmanX/8), (pacmanY/8));
	   VDP_setTileMapXY(BG_A, 1, (pacmanX/8), (pacmanY/8) + 1);
   }

   if (pacGirlX >= 0 && pacGirlX <= 320) {
	   // рисуем tile на бакграунде в координатах Pac-Girl (он съест SEGA)
	   VDP_setTileMapXY(BG_A, 1, (pacGirlX/8), (pacGirlY/8));
	   VDP_setTileMapXY(BG_A, 1, (pacGirlX/8), (pacGirlY/8) + 1);
   }

}


/**
 * SEGA
 *
 * Перевести игру в режим паузы
 */
void pause() {
	gameState = STATE_PAUSE;
	playersTime = 30;
	SPR_setPosition(pacGirlSprite, -100, -100);
	SPR_setPosition(pacmanSprite, -100, -100);
	SPR_setPosition(redSprite, -100, -100);
	dxSonic = 8;
	sonicX = -10;
	pauseX = -60;
	pauseY = sonicY;
}


/**
 * SEGA
 *
 * Выходим из паузы, т.е. меняем состояние на продолжение игры
 */
void resumeGame() {
	gameState = STATE_GAME;
	playersTime = 30;
	SPR_setPosition(pauseSprite, -100, -100);
	SPR_setPosition(sonicSprite, -100, -100);
}


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
void masterControls() {
	// считаем что на 2 контроллере ничего не нажато
	pad2 = 0;

	if (pad1) {
		// в transferObject положим объект содержащий информацию о нажатых кнопках на 1 контроллере нашей приставки
		// т.е. там будет лежать объект OBJECT_TYPE_JOY в виде байтового массива (если что то было нажато)
		padToTransferObject(pad1);


		// добавим объект OBJECT_TYPE_JOY в пакет который будет передан другой приставке при вызове LCP_masterCycle()
		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_JOY, LINK_TYPES_LENGHT);
	}

	if (STATE_SELECT == gameState) {
		// отправляем информацию какой игрок кем играет
		// если switchPlayers == 0 - master это Pac-Man,  slave это Pac-Girl
		// если switchPlayers == 1 - master это Pac-Girl, slave это Pac-Man
		transferObject[0] = switchPlayers;

		// добавим объект OBJECT_TYPE_SWITCH_PLAYERS в пакет, будет передан другой приставке при вызове LCP_masterCycle()
		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_SWITCH_PLAYERS, LINK_TYPES_LENGHT);
	}

	// передаем пакет с даннми от нашей ведущей приставки (master) ведомой приставке (slave)
	// через Link cable (у ведомой приставки произойдет внешнее прерывание EX-INT - External interrupt)
	// также в этом же методе получаем от ведомой приставки (slave) пакет с данными для нашей
	LCP_masterCycle();

	do {
		// пытаемся из полученного пакета данных от ведомой приставки (slave) достать очередной объект в transferObject
		objectType = LCP_getNextObjectFromRecivePacket(transferObject, LINK_TYPES_LENGHT);
		if (OBJECT_TYPE_JOY == objectType) {
			// тот кто играет на другой приставке, ведомой (slave) - второй игрок по умолчанию играет за Pac-Girl
			// поэтому что было нажато на первом контролере другой приставки сохоаняем в pad2
			pad2 = getPadFromTransferObject();

			// НО если в меню '2 PLAYERS' было нажато ВПРАВО перед игрой (switchPlayers == 1), то второй
			// игрок будет играть за Pac-Man т.к. в конце этой функции pad1 и pad2 будут поменяны в этом случае местами
		}
	} while (objectType != 0);

}

/**
 * SEGA
 *
 * Играем на двух приставках и наша ведомая (slave) приставка
 * сохраняем в пакет для отправки ведущей (master) приставке что было нажато нажато на нашем контроллере
 * пакет будет отправлен асинхронно при наступлении внешнего прерывания инициируемого ведущей (master) приставкой
 * в случайный момент времени для нашей приставки (обработчик внешнего прерывания - функция LCP_slaveCycle() из библиотеки link_cable.c)
 * разбераем что было получено от ведущей (master) приставки при прошлом вызове внешнего прерывания (LCP_slaveCycle())
 */
void slaveControls() {
	// мы играем за второго игрока, по этому что нажато на первом контролере
	// сохраняем в переменную 2 го контроллера
	pad2 = pad1;

	if (pad1) {
		// в transferObject положим объект содержащий информацию о нажатых кнопках на 1 контроллере нашей приставки
		// т.е. там будет лежать объект OBJECT_TYPE_JOY в виде байтового массива (если что то было нажато)
		padToTransferObject(pad1);

		// добавим объект OBJECT_TYPE_JOY в пакет который будет передан другой приставке при вызове LCP_slaveCycle()
		// в момент получения внешнего прерывания вызванного ведущей приставкой (master) в рандомный  момент времени для нас
		LCP_objectToPacketForSend(transferObject, OBJECT_TYPE_JOY, LINK_TYPES_LENGHT);
	}

	// счтаем что на 1 контроллере на данный момент ничего не нажато
	pad1 = 0;

	do {
		// пытаемся из полученного пакета данных от ведущей приставки (master) достать очередной объект в transferObject
		objectType = LCP_getNextObjectFromRecivePacket(transferObject, LINK_TYPES_LENGHT);
		switch (objectType) {
			case OBJECT_TYPE_JOY:
				// тот кто играет на другой приставке, ведущей (master) - первый игрок по умолчанию играет за Pac-Man
				// поэтому что было нажато на первом контролере другой приставки сохоаняем в pad1
				pad1 = getPadFromTransferObject();

				// НО если в меню '2 PLAYERS' было нажато ВПРАВО перед игрой (switchPlayers == 1), то первый
				// игрок будет играть за Pac-Girl т.к. в конце этой функции pad1 и pad2 будут поменяны в этом случае местами
			break;
			case OBJECT_TYPE_SWITCH_PLAYERS:
				if (STATE_SELECT == gameState) {
					// пришла информация о том какой игрок кем играет
					switchPlayers = transferObject[0];
				}
			break;
			case OBJECT_TYPE_PAC_MAN_STATE:
				// пришли dx, dy, pacmanX, pacmanY, oldX, oldY от ведущей приставки (master)
				pacManStateFromTransferObject();
				drawSprite(pacmanY, pacmanX, PACMAN);
				drawBlackBox(oldY, oldX);
			break;
			case OBJECT_TYPE_PAC_GIRL_STATE:
				// пришли dxPacGirl, dyPacGirl, pacGirlX, pacGirlY, oldPacGirlX, oldPacGirlY от ведущей приставки (master)
				pacGirlStateFromTransferObject();
				drawSprite(pacGirlY, pacGirlX, PACGIRL);
				drawBlackBox(oldPacGirlY, oldPacGirlX);
			break;
			case OBJECT_TYPE_RED_STATE:
				// пришли dxRed, dyRed, redX, redY, redFlag от ведущей приставки (master)
				redStateFromTransferObject();
				if (redFlag) {
					drawSprite(redY, redX, RED);
				} else {
					drawSprite(redY, redX, SHADOW);
				}
			break;
			case OBJECT_TYPE_END_GAME:
				// от ведущей приставки пришло событие окончания игры
				// если состояние что наша приставка еще в игре, нужно проиграть звук выигрыша или проигрыша

				// всех обездвиживаем
				dxRed = 0;
				dyRed = 0;
				dx = 0;
				dy = 0;
				dxPacGirl = 0;
				dyPacGirl =0;

				if (winner()) {
					// звук окончания игры - выиграли
					XGM_startPlay(victory_vgm);
				} else {
					// Pac-Man съели
					map[pacmanY][pacmanX] = RED;
					// убрать спрайт Pac-Man с экрана (нас съели)
					SPR_setPosition(pacmanSprite, -90, 90);
					// звук окончания игры - проиграли
					XGM_startPlay(fatality_vgm);
				}

				// подсчитать набранные очки
				calcScore();

				// изменяем состояние игры на показ результатов (игра окончена)
				gameState = STATE_RESULT;
			break;
			case OBJECT_TYPE_PAUSE:
				// ведущая приставка (master) сообщила что надо уйти в режим паузы
				pause();
			break;
			case OBJECT_TYPE_RESUME_GAME:
				// ведущая приставка (master) сообщила что надо продолжить игру
				resumeGame();
			break;
			case OBJECT_TYPE_EAT_POINT:
				// ведущая приставка (master) сообщила что cъедена еда
				incFood();
			break;
			case OBJECT_TYPE_EAT_POWERUP:
				// ведущая приставка (master) сообщила что cъеден powerup
				// звук поедания поверапа
				XGM_startPlayPCM(SFX_SOUND_POWERUP, 15, SOUND_PCM_CH2);

				// RED стал съедобным
				redTime = RED_TIME;

				// и даем еще бонус
				++powerBonus;
			break;
			case OBJECT_TYPE_EAT_SHADOW:
				// ведущая приставка (master) сообщила что cъеден призрак
				// звук поедания призрака
				XGM_startPlayPCM(SFX_SOUND_EAT_SHADOW, 15, SOUND_PCM_CH2);

				// закрываем дверь в дом привидений
				closeDoors();

		    	// скрыть черешню
		    	SPR_setPosition(cherrySprite, -90, 100);

		       	// пусть сидит в домике дополнительное время
		        redTime = RED_TIME;

				// даем бонус за то что RED съели
				++redBonus;
			break;
			case OBJECT_TYPE_EAT_CHERRY:
				// ведущая приставка (master) сообщила что cъедена черешня
				// звук поедания черешни
				XGM_startPlayPCM(SFX_SOUND_CHERRY, 15, SOUND_PCM_CH2);

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);

				// даем бонус за черешню
				++cherryBonus;
			break;
			case OBJECT_TYPE_OPEN_DOOR:
				// ведущая приставка (master) сообщила что надо открыть дверь в дом призрака
				openDoors();
			break;
		}
	} while (objectType != 0);
}

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
void controls() {
	// буферная переменная используется в случае емли игроки поменяли кто кем будет играть в меню '2 PLAYERS'
	// нужна чтоб поменять местами pad1 и pad2
	u16 switchPad;

	// ошибка при передачи данных
	u16 lcpError = 0;

	// что нажато на 1 джойстике
	pad1 = JOY_readJoypad(JOY_1);

	// определяем режим в которой работает приставка
	switch (controllerPort2Mode) {
		case MODE_MULTY_PLAYER:
			// multiplayer нет соединения между приставками по Link cable  но в втором порту 3 или 6
			// кнопочний контроллер. Второй игрок по умолчанию играет за PAC-GIGL этим контроллером
			// НО если в меню '2 PLAYERS' было нажато ВПРАВО перед игрой (switchPlayers == 1), то за Pac-Man
			// т.к. в конце этой функции pad1 и pad2 будут поменяны в этом случае

			// что нажато на 2 контроллере в pad2
			pad2 = JOY_readJoypad(JOY_2);
		break;
		case MODE_PORT2_MASTER:
			// master - наша приставка ведущая, игра в двоем через Link cable (сетевая ига на двух приставках SEGA)
			// играем за первого игрока PAC-MAN но если в меню '2 PLAYERS' сменили персонажей то за PAC-Girl

			masterControls();
		break;
		case MODE_PORT2_SLAVE:
			// slave - наша приставка ведомая, игра в двоем через Link cable (сетевая ига на двух приставках SEGA)
			// играем за второго игрока PAC-GIRL но если в меню '2 PLAYERS' сменили персонажей то за PAC-MAN

			slaveControls();
		break;
		case MODE_SINGLE_PLAYER:
			// singlepayer - нет соединения через Link cable и в 2 порту нет контроллера
		  	// основной и единственный игрок тот кто нажимает кнопки на первом контроллере, по этому
			// ничего не делаем мы и так уже все положили в pad1
		case MODE_PORT2_UNKNOWN:
			// не знаем что с вторым контроллером,
			// ничего не делаем
		default:
			// не нужно обрабатывать события от 2го порта контроллера
			// ничего не делаем
		break;
	}

	// выводим на экран только реальные ошибки произошедшие при передаче
	// через Link cable
	lcpError = LCP_getError();
	if (lcpError != 0 && lcpError != 0x1000) {
		linkCableErrors = lcpError;
		linkCableErrorsCount++;
	}

	if (P1_PACGIRL__P2_PACMAN == switchPlayers && players == 2) {
		// switchPlayers == P1_PACGIRL__P2_PACMAN - первый игрок играет Pac-Girl а не Pac-Man, при игре вдвоем на одной приставке
	    // если играем через Link cable - master играет за Pac-Girl а не за Pac-Man как изначально
		// а второй игрок играет за Pac-Man а не за  Pac-Girl, при игре вдвоем на одной приставке
		// ну и если играем через Link cable - slave играет за Pac-Man а не за  Pac-Girl
		// для этого просто меняем местами значения у pad1 и pad2
		switchPad = pad1;
		pad1 = pad2;
		pad2 = switchPad;
	}
}


/**
 * Определить победил ли ты в игре
 *
 * return true - победил в игре
 */
u8 winner() {
	return food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4;
}


/**
 * SEGA
 *
 * Инициализация звуковых эффектов
 */
void initSound() {
	// голос во время заставки произносящий слово SEGA
	XGM_setPCM(SFX_SOUND_SEGA, sega_sfx, sizeof(sega_sfx));
	// звук поедания белой точки - еды
	XGM_setPCM(SFX_SOUND_EAT, eat_sfx, sizeof(eat_sfx));
	// звук поедания черешни
	XGM_setPCM(SFX_SOUND_CHERRY, cherry_sfx, sizeof(cherry_sfx));
	// звук поедания зеленой точки - powerup
	XGM_setPCM(SFX_SOUND_POWERUP, powerup_sfx, sizeof(powerup_sfx));
	// звук когда призрак съедобен
	XGM_setPCM(SFX_SOUND_SHADOW, shadow_sfx, sizeof(shadow_sfx));
	// звук когда съели призрака
	XGM_setPCM(SFX_SOUND_EAT_SHADOW, eatred_sfx, sizeof(eatred_sfx));
	// звук создания соединения через Link cable
	XGM_setPCM(SFX_SOUND_CONNECT_LINK_CABLE, connect_sfx, sizeof(connect_sfx));
	// звук отключения соединения через Link cable
	XGM_setPCM(SFX_SOUND_DISCONNECT_LINK_CABLE, disconnect_sfx, sizeof(disconnect_sfx));
}


/**
 * SEGA
 *
 * Установка палитр
 */
void  initPaletts() {
    // задали цвета в 4-ой палитре (отсчет начинается с нуля), цветами взятыми из спрайта соника,
    // и выбрали в качестве способа передачи DMA.
    // Sega поддерживает 4 палитры по 16 цветов (PAL0-PAL3), и хранит их в CRAM.
    PAL_setPalette(PAL3, sonic_sprite.palette->data, DMA);
    PAL_setPalette(PAL2, red_sprite.palette->data, DMA);
    PAL_setPalette(PAL1, pacgirl_sprite.palette->data, DMA);
}


/**
 * SEGA
 *
 * Инициализация спрайтов
 *
 */
void initSprites() {
    // добавляем спрайт соника на экран
    sonicSprite = SPR_addSprite(&sonic_sprite, sonicX, sonicY, 
                                    TILE_ATTR(PAL3       // палитра
                                                , 0      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
                                                , FALSE  // перевернуть по вертикали
                                                , FALSE  // перевернуть по горизонтали
                                              )
                                );

    // добавляем спрайт Pac-Man на экран
    pacmanSprite = SPR_addSprite(&pacman_sprite, pacmanX, pacmanY, 
                                    TILE_ATTR(PAL1       // палитра
                                                , 1      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
                                                , FALSE  // перевернуть по вертикали
                                                , FALSE  // перевернуть по горизонтали
                                              )
                                );

    // добавляем спрайт Red на экран
    redSprite = SPR_addSprite(&red_sprite, redX, redY, 
                                    TILE_ATTR(PAL2       // палитра
                                                , 0      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
                                                , FALSE  // перевернуть по вертикали
                                                , FALSE  // перевернуть по горизонтали
                                              )
                                );


    // добавляем спрайт Pac-Girl на экран
    pacGirlSprite = SPR_addSprite(&pacgirl_sprite, pacGirlX, pacGirlY, 
                                    TILE_ATTR(PAL1       // палитра
                                                , 1      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
                                                , FALSE  // перевернуть по вертикали
                                                , FALSE  // перевернуть по горизонтали
                                              )
                                );


    // добавляем спрайт черешни  на экран
    cherrySprite = SPR_addSprite(&cherry_sprite, cherryX, cherryY,
                                    TILE_ATTR(PAL1       // палитра
                                                , 1      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
                                                , FALSE  // перевернуть по вертикали
                                                , FALSE  // перевернуть по горизонтали
                                              )
                                );
    // добавляем спрайт двери на экран
    doorSprite = SPR_addSprite(&door_sprite, doorX, doorY,
									TILE_ATTR(PAL1       // палитра
												, 1      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
												, FALSE  // перевернуть по вертикали
												, FALSE  // перевернуть по горизонтали
											  )
								);

    pauseSprite = SPR_addSprite(&pause_sprite, -100, -100,
									TILE_ATTR(PAL1       // палитра
												, 1      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
												, FALSE  // перевернуть по вертикали
												, FALSE  // перевернуть по горизонтали
											  )
								);

}


/**
 * SEGA
 *
 * Инициализация и сброс переменных в значения по умолчанию
 * тут нужно также сбрасывать переменные после soft reset (нажатия на кнопку RESET)
 */
void initGame() {
    // инициализируем спрайтовый движок (выделяем место в VRAM под спрайты)
    SPR_init();

    // установка палитр
    initPaletts();

   	// загружаем русские буквы в память в виде Tile
    VDP_loadTileSet(&font_rus, FIRST_INDEX_RUSSIAN_TILE, DMA);

    // инициализация спрайтов
    initSprites();

	// инициализация звуковых эффектов
	initSound();


    if (MODE_PORT2_MASTER == controllerPort2Mode || MODE_PORT2_SLAVE == controllerPort2Mode) {
    	// если был soft reset (нажали на RESET), переменные не сброшены надо в том числе
    	// сбросить состояние переменных SEGA Link Cable Protocol
    	LCP_close();
    }

    // сбрасываем режим работы приставки с вторым портом для контроллероллера
    controllerPort2Mode = MODE_PORT2_UNKNOWN;

    // ни чего не отображаем на экране по поводу режима игры
    memset(gameModeText, 0, GAME_MODE_TEXT_SIZE);

    // поумолчанию первый игрок управляет Pac-Man, второй игрок Pac-Girl
    switchPlayers = P1_PACMAN__P2_PACGIRL;

    // поумолчанию выбрана одиночная игра
    players = 1;

    // заставка (надо обязательно в main выставить т.к. есть soft reset у SEGA)
    gameState = STATE_SCREENSAVER;

    // инициализируем положение персонажей для заставки
    initScreensaver();
}


// точка входа в программу
int main() {

	// Инициализация и сброс переменных в значения по умолчанию
	initGame();

    // Рисуем в качестве заднего фона SEGA
    drawBackground();

	// Звук SEGA при старте игры
	XGM_startPlayPCM(SFX_SOUND_SEGA, 15, SOUND_PCM_CH2);

    // цикл анимации игры
    while(1) {

    	// определение нажатых кнопок на контроллерах
    	// передача данных по Link Cable от ведущей приставки (master)
    	// ведомой приставке (slave) и рекция на полученные объекты по Link Cable
    	controls();

		// нарисовать бонусы, очки или результат игры
		drawText();

		// обработать действия игроков (нажатие кнопок контроллеров)
		// подвинуть персонажи в зависимости от того что нажато на крате (map)
		actions();

		// нарисовать спрайты согласно расположению на карте (map)
		drawSprites();

        // Обновляет и отображает спрайты на экране
        SPR_update();

        //  делает всю закулисную обработку, нужен когда есть спрайты, музыка, джойстик.
        SYS_doVBlankProcess();

        // количество отресованных фреймов
        ++linkCableFrameCount;
    }

    return 0;
}


