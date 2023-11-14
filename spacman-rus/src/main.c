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

#include <genesis.h>

#include "main.h"
#include "resources.h"

/**
 *  Подсчет отчков с учетом всех бонусов
 */
void calcScore() {
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
int isNotWell(s16 y, s16 x) {
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
int isNotWellOrDoor(s16 y, s16 x) {
	if (isNotWell(y, x) && map[y][x] != DOOR) {
		return 1;

	}
	return 0;
}


/**
 * Корректировка координат PACMAN или Призрака
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
	map[cherryY][cherryX] = CHERRY;

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
void init() {
	// счетчики циклов начинаются с разных значений
	// чтоб рендеринг каждого персонажа был в разном глобальном цикле
	pacmanLastUpdateTime = PACMAN_SPEED;
	redLastUpdateTime = 4;     //RED_SPEED;
	pacGirlLastUpdateTime = 6; //PACGIRL_SPEED;
	cherryTime = CHERRY_TIME

	cherryBonus = 0;
	powerBonus = 0;
	redBonus = 0;

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
}


/**
 * SEGA
 *
 * Съедена еда
 * пересчитать значения счетчиков
 * food001 food010 food100
 *
 * т.к. у 8 битной консоли максимальное число 256
 * то проще считать очки в 3х отдельных переменных
 * так можно запомнить число от 000 до 999
 * ну и выводить результат так проще
 */
void incFood() {
	// звук поедания точки
	XGM_startPlayPCM(66, 1, SOUND_PCM_CH2);

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
 * и что съел на месте призрака
 */
int pacmanLooser() {
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

			return 1;
		} else {
			// звук поедания призрака
			XGM_startPlayPCM(70, 15, SOUND_PCM_CH3);

			// RED съедобен в данный момент
			// Отправляем его в дом Приведений
			redY = 10;
			redX = 15;
			// бездвиживаем
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

			// проверяем что пакмен съел вместе с RED
			if (oldRedVal == FOOD) {
				// еду
				incFood();
			} else if (oldRedVal == POWER_FOOD) {
				// поверап
				++powerBonus;

				// звук поедания поверапа
				XGM_startPlayPCM(68, 15, SOUND_PCM_CH2);

				// обнавляем время когда RED стал съедобным
				redTime = RED_TIME;

			} else if (oldRedVal == CHERRY) {
				// вишню
				++cherryBonus;

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);

				// звук поедания черешни
				XGM_startPlayPCM(67, 15, SOUND_PCM_CH2);
			}

			oldRedVal = EMPTY;
		}
	} else if (redY == pacGirlY && redX == pacGirlX) {
		// проверяем что Pac-Girl съела на месте RED
		if (oldRedVal == FOOD) {
			// еду
			incFood();
		} else if (oldRedVal == POWER_FOOD) {
			// поверап
			++powerBonus;

			// звук поедания поверапа
			XGM_startPlayPCM(68, 15, SOUND_PCM_CH2);

			// обнавляем время когда RED стал съедобным
			redTime = RED_TIME;

			// RED становится съедобным
			redFlag = 0;
		} else if (oldRedVal == CHERRY) {
			// вишню
			++cherryBonus;

			// скрыть черешню
			SPR_setPosition(cherrySprite, -90, 100);

			//звук поедания черешни
			XGM_startPlayPCM(67, 15, SOUND_PCM_CH2);
		}

		map[pacGirlY][pacGirlX] = RED;

		oldRedVal = EMPTY;
	}
	return 0;
}


/**
 * SEGA
 *
 * Алгоритм обработки движения PACMAN на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacManState() {
	// проверяем, у PACMAN задоно ли направление движения
	if (dx != 0 || dy != 0) {

		// должен ли PACMAN переместиться на новую клетку
		if (pacmanLastUpdateTime == 0) {
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
				XGM_startPlayPCM(68, 15, SOUND_PCM_CH2);

			} else if (val == CHERRY) {
				++cherryBonus;

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);

				// звук поедания черешни
				XGM_startPlayPCM(67, 15, SOUND_PCM_CH2);
			}


			if (isNotWellOrDoor(pacmanY, pacmanX)) {
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

			// если съеденны все FOOD и POWER_FOOD - PACMAN выиграл
			if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {

				// звук выиграша
				XGM_startPlay(victory_vgm);


				dxPacGirl = 0;
				dyPacGirl = 0;
				dx = 0;
				dy = 0;
				dxRed = 0;
				dyRed = 0;

				calcScore();
				return 0;
			}

			// сеъеи ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {

				// звук окончания игры
				XGM_startPlay(fatality_vgm);
				return 0;
			}

			oldX = pacmanX;
			oldY = pacmanY;

		 }

	}
	return 1;
}

/**
 * SEGA
 *
 * Алгоритм обработки движения PACGIRL на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacGirlState() {
	// проверяем, у pacGirl задоно ли направление движения
	if (dxPacGirl != 0 || dyPacGirl != 0) {

		// если подключился 2 игрок
		if (players == 1) {
			players = 2;
			if (map[pacGirlY][pacGirlX] == FOOD) {
				incFood();
			}
		}

		if (pacGirlLastUpdateTime == 0) {
			pacGirlX = pacGirlX + dxPacGirl;
			pacGirlY = pacGirlY + dyPacGirl;

			pacGirlLastUpdateTime = PACGIRL_SPEED;

			// если вышел за поле (появление с другой стороны поля)
			moveBound(&pacGirlX, &pacGirlY);


			// если текущая клетка с едой, увиличиваем счетчик съеденного
			val = map[pacGirlY][pacGirlX];
			if (val == FOOD) {
				incFood();
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
				XGM_startPlayPCM(68, 15, SOUND_PCM_CH2);
			} else if (val == CHERRY) {
				++cherryBonus;

				// скрыть черешню
				SPR_setPosition(cherrySprite, -90, 100);

				// звук поедания черешни
				XGM_startPlayPCM(67, 15, SOUND_PCM_CH2);
			}

			if (isNotWellOrDoor(pacGirlY, pacGirlX)) {
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

			// если съеденны все FOOD и POWER_FOOD - PACMAN выиграл
			if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {

				// звук когда выиграли
				XGM_startPlay(victory_vgm);

				dxPacGirl = 0;
				dyPacGirl = 0;
				dx = 0;
				dy = 0;
				dxRed = 0;
				dyRed = 0;

				calcScore();
				return 0;
			}

			// сеъел ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {
				// звук окончания игры
				XGM_startPlay(fatality_vgm);
				return 0;
			}


			oldPacGirlX = pacGirlX;
			oldPacGirlY = pacGirlY;
		}
	}

	return 1;
}


/**
 * SEGA
 *
 * Алгоритм призрака гоняющегося за PACMAN
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int redState() {

	// надо ли RED перейти в режим погони
	if (redTime == 0 ) {
		redFlag = 1;
		// если не двигается, пусть идет вверх
		if (dyRed == 0 && dxRed == 0) {
			dyRed = -1;
		}
	} else if (redLastUpdateTime == 0 && dyRed == 0 && dxRed == 0){
		redLastUpdateTime = RED_SPEED;
	}

	// проверяем, у RED задоно ли направление движения
	if (dxRed != 0 || dyRed != 0) {
		if (redLastUpdateTime == 0) {
			redX = redX + dxRed;
			redY = redY + dyRed;

			redLastUpdateTime = RED_SPEED;

			// вышли за границы
			moveBound(&redX, &redY);

			if (isNotWell(redY, redX)) {
				map[oldYRed][oldXRed] = oldRedVal;
				oldRedVal = map[redY][redX];

				if (redX == 15 && redY >= 7 && redY <= 10) {
					dyRed = -1;
					dxRed = 0;
				} else if (dxRed != 0) {
					if (redFlag && redY != pacmanY) {
						if (isNotWellOrDoor(redY + 1, redX)
								&& isNotWellOrDoor(redY - 1, redX)) {
							if (abs(redY + 1 - pacmanY) < abs(redY - 1 - pacmanY)) {
								dyRed = 1;
							} else {
								dyRed = -1;
							}
						} else if (isNotWellOrDoor(redY + 1, redX)) {
							if (abs(redY + 1 - pacmanY) < abs(redY - pacmanY)) {
								dyRed = 1;
							}
						} else if (isNotWellOrDoor(redY - 1, redX)) {
							if (abs(redY - 1 - pacmanY) < abs(redY - pacmanY)) {
								dyRed = -1;
							}
						}
					} else {
						if (isNotWellOrDoor(redY + 1, redX)) {
							dyRed = random() % 2;
						}

						if (isNotWellOrDoor(redY - 1, redX)) {
							dyRed = -1 * (random() % 2);
						}
					}

					if (dyRed != 0) {
						dxRed = 0;
					}

				} else if (dyRed != 0) {
					if (redFlag && redX != pacmanX) {
						if (isNotWellOrDoor(redY, redX + 1)
								&& isNotWellOrDoor(redY, redX - 1)) {
							if (abs(redX + 1 - pacmanX) < abs(redX - 1 - pacmanX)) {
								dxRed = 1;
							} else {
								dxRed = -1;
							}
						} else if (isNotWellOrDoor(redY, redX + 1)) {
							if (abs(redX + 1 - pacmanX) < abs(redX - pacmanX)) {
								dxRed = 1;
							}
						} else if (isNotWellOrDoor(redY - 1, redX)) {
							if (abs(redX - 1 - pacmanX) < abs(redX - pacmanX)) {
								dxRed = -1;
							}

						}
					} else {

						if (isNotWellOrDoor(redY, redX + 1)) {
							dxRed = random() % 2;
						}

						if (isNotWellOrDoor(redY, redX - 1)) {
							dxRed = -1 * (random() % 2);
						}

					}

					if (dxRed != 0) {
						dyRed = 0;
					}
				}
			} else {
				if (redX == 15 && redY >= 7 && redY <= 10) {
					dyRed = -1;
					dxRed = 0;
				} else {

					redX = oldXRed;
					redY = oldYRed;

					if (dxRed != 0) {
						dxRed = 0;
						if (isNotWellOrDoor(redY + 1, redX)) {
							dyRed = 1;
						} else if (isNotWellOrDoor(redY - 1, redX)) {
							dyRed = -1;
						}
					} else {
						dyRed = 0;
						if (isNotWellOrDoor(redY, redX + 1)) {
							dxRed = 1;
						} else if (isNotWellOrDoor(redY, redX - 1)) {
							dxRed = -1;
						}
					}
				}

			}


			// сеъеи ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {
				// музыка оканчания игры
				XGM_startPlay(fatality_vgm);
				return 0;
			}

			oldXRed = redX;
			oldYRed = redY;

		}
	}

	if (redFlag) {
		map[redY][redX] = RED;
	} else {
		map[redY][redX] = SHADOW;
	}

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
void drawRussianText(VDPPlane plane, const u16 *vramOffsets, u16 length, u16 xTile, u16 yTile) {
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
 *  SEGA
 *
 *  Нарисовать бонусы, очки
 *  результат игры (GAME OVER или YOU WINNER)
 */
void drawText() {
	if (STATE_GAME == gameState || STATE_RESULT == gameState) {
		// идет игра или отображаем результат игры
		PAL_setColor(15,RGB24_TO_VDPCOLOR(0xffffff));
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
			// если победили
			// пишем 'ТЫ ВЫИГРАЛ'
			PAL_setColor(15,RGB24_TO_VDPCOLOR(0x00ff00));
			PAL_setColor(1, RGB24_TO_VDPCOLOR(0x00ff00));
			drawRussianText(BG_A, textYouWinner, TEXT_GAME_OVER_SIZE, 13, 24);
		} else {
			// если проиграли
			// пишем 'КОНЕЦ ИГРЫ'
			PAL_setColor(15,RGB24_TO_VDPCOLOR(0xff0000));
			PAL_setColor(1, RGB24_TO_VDPCOLOR(0xff0000));
			drawRussianText(BG_A, textGameOver, TEXT_GAME_OVER_SIZE, 13, 24);
		}
		// пишем 'ОЧКИ'
		drawRussianText(BG_A, textScore, TEXT_SCORE_SIZE, 13, 26);


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
	if (STATE_SCREENSAVER == gameState) {
		screensaver();
	} else if (STATE_SELECT == gameState) {
		// если находимся на стартовом экране
		if (players == 1) {
			// надо спрятать Pac-Girl
			SPR_setPosition(pacGirlSprite, -100, 90);

			// если выбрана игра за одного (только Pac-Man)
			// надо нарисовать спрайт PAC-MAN перед 1 PLAYER
			SPR_setAnim(pacmanSprite, 0);
			SPR_setHFlip(pacmanSprite, FALSE);
		    SPR_setPosition(pacmanSprite, 100, 100);
		} else {
			// если выбрана игра на 2х игроков (1 игрок за Pac-Man, 2 игрок за Pac-Girl)
			// нарисовать спрайт PAC-Girl перед 2 PLAYERS
			SPR_setAnim(pacGirlSprite, 0);
			SPR_setPosition(pacGirlSprite, 100, 113);

			// нарисовать спрайт PAC-MAN после 2 PLAYERS
			SPR_setAnim(pacmanSprite, 0);
			SPR_setHFlip(pacmanSprite, TRUE);
		    SPR_setPosition(pacmanSprite, 187, 113);
		}
	} else if (STATE_GAME == gameState || STATE_RESULT == gameState) {
		// если идет игра или показываем результаты игры
		// отрисовываем спрайты игры
		// PAC-MAN, Pac-Girl, RED или SHADOW, дверь, черешню

		refreshGame();
	}
}

/**
 * SEGA
 *
 * Нарисовать только 1 объект с карты
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void draw(s16 i, s16 j) {
    val = map[i][j] ;

	 // x = i * 8 и на 29 пиксела вправо
    x = j * 8 + 29;

    // y = j * 8 и на 2 пиксела вверх
    y = i * 8 - 2;

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
    		// звук когда RED съедобен
    		XGM_startPlayPCM(69, 15, SOUND_PCM_CH3);
    		shadowLastSoundTime = 20;
    	}

    	if (dxRed != 0 && dy != 0) {
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
 * Обновить карту / персонажей, двери, черешню
 */
void refreshGame() {
	if (!cherryFlag && redTime == 0 && !cherryBonus && cherryTime == 0) {
		// открыть двери
		openDoors();
	}

    if (refreshDoor) {
		if (map[doorY][doorX] != DOOR) {
			refreshDoor = 0;
			SPR_setPosition(doorSprite, -90, 100);
		} else {
			// рисуем дверь
			draw(doorY, doorX);
		}
    }

    if (refreshCherry) {
		if (map[cherryY][cherryX] != CHERRY) {
			refreshCherry = 0;
			SPR_setPosition(cherrySprite, -90, 100);
		} else {
		    // рисуем черешню
	    	draw(cherryY, cherryX);
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
	if (STATE_SCREENSAVER == gameState) {
		// заставка
	    // рисуем задний фон (лого SEGA из sega.png)
	    VDP_drawImage(BG_A, &sega_image, 0, 0);
	} else if (STATE_SELECT == gameState) {
		// стартовый бекграунд
	    // рисуем в качестве заднего фона меню выбора количества игроков
	    VDP_drawImage(BG_A, &menu_image, 0, 0);
	} else {
		// карта уровня с лабиринтом
	    // рисуем в качестве заднего фона карту уровня
	    VDP_drawImage(BG_A, &map_image, 0, 0);
	}
}

/**
 *  SEGA
 *
 *  Обработка нажатых кнопок игроком
 *  передвижение персонажей во время игры
 */
void actions() {
	if (STATE_SCREENSAVER == gameState) {
        // условие окончания анимации заставки
        // тупо закончилась анимация: Pac-Girl X координата >= 380
        // или нажали любую кнопку на одном из Джойстиков (кроме Start)
        if ((pacGirlX >= 380)
        		 || (pad1 & BUTTON_A || pad1 & BUTTON_B || pad1 & BUTTON_C || pad1 & BUTTON_X || pad1 & BUTTON_Y || pad1 & BUTTON_Z)
				 || (pad2 & BUTTON_A || pad2 & BUTTON_B || pad2 & BUTTON_C || pad2 & BUTTON_X || pad2 & BUTTON_Y || pad2 & BUTTON_Z)
		    ) {

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
	} else if (STATE_SELECT == gameState) {
		// стартовый экран (выбор количества игроков)
		if (((pad1 & BUTTON_START) || (pad2 & BUTTON_START)) && playersTime == 0) {
			// нажат Start на 1 или 2 джойстике

			// сбросить игру в стартовое состояние
			// начальное положение персонажей, обнулить очки, и т.д.
			init();

			if (players == 1) {
				// выбрана игра за 1го (1 PLAYER)

				// игрок 1 убираем с карты PAC-GIRL
				map[pacGirlY][pacGirlX] = FOOD;
			} else {
				// выбрана игра на 2их (2 PLAYERS)

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

		// задержка для обработки нажатия SELECT
		if (playersTime > 0) {
			// защита от двойного нажатия,
			// когда playersTime станет равным 0 обработчик заработает  опять
			--playersTime;
		}
	} else if (STATE_GAME == gameState) {
		// идет ира

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

		if (pacmanLastUpdateTime > 0 ) {
			// счетчик для анимации Pac-Man
			--pacmanLastUpdateTime;
		}

		if (redLastUpdateTime > 0) {
			// счетчик для анимации RED и SHADOW
			--redLastUpdateTime;
		}

		if (pacGirlLastUpdateTime > 0) {
			// счетчик для анимации Pac-Girl
			--pacGirlLastUpdateTime;
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

		if (shadowLastSoundTime > 0) {
			--shadowLastSoundTime;
		}
	} else if (STATE_RESULT == gameState && ((pad1 & BUTTON_START) || (pad2 & BUTTON_START))) {
		if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {
			// TODO
			// логика если выиграли в игре
			// пока незнаю что тут  напишу
		}
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
		XGM_startPlayPCM(64, 15, SOUND_PCM_CH2);
	}

	return;

}


/**
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
 * отображаем заставку
 * SEGA которую съест Pac-Man и Pac-Girl
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

   if (pacmanX > 70 && pacmanX < 180 && dx > 0) {
	   if (pacmanLastUpdateTime <= 0) {
		   XGM_startPlayPCM(66, 15, SOUND_PCM_CH2);
		   pacmanLastUpdateTime = 10;
	   } else {
		   pacmanLastUpdateTime--;
	   }
   }

   if (pacGirlX > 70 && pacGirlX < 180) {
	   if (pacGirlLastUpdateTime <= 0) {
		   XGM_startPlayPCM(66, 15, SOUND_PCM_CH2);
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

// точка входа в программу
int main() {
	// звук SEGA
	XGM_setPCM(64, sega_sfx, sizeof(sega_sfx));
	XGM_setPCM(66, eat_sfx, sizeof(eat_sfx));
	XGM_setPCM(67, cherry_sfx, sizeof(cherry_sfx));
	XGM_setPCM(68, powerup_sfx, sizeof(powerup_sfx));
	XGM_setPCM(69, shadow_sfx, sizeof(shadow_sfx));
	XGM_setPCM(70, eatred_sfx, sizeof(eatred_sfx));

    // инициализируем спрайтовый движок (выделяем место в VRAM под спрайты)
    SPR_init();

    // задали цвета в 4-ой палитре (отсчет начинается с нуля), цветами взятыми из спрайта соника, 
    // и выбрали в качестве способа передачи DMA.
    // Sega поддерживает 4 палитры по 16 цветов (PAL0-PAL3), и хранит их в CRAM.
    PAL_setPalette(PAL3, sonic_sprite.palette->data, DMA);
    PAL_setPalette(PAL2, red_sprite.palette->data, DMA);
    PAL_setPalette(PAL1, pacgirl_sprite.palette->data, DMA);

	// загружаем русские буквы в память в виде Tile
    VDP_loadTileSet(&font_rus, FIRST_INDEX_RUSSIAN_TILE, DMA);

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

    // заставка (надо обязательно в main выставить т.к. есть soft reset у SEGA)
    gameState = STATE_SCREENSAVER;

    // инициализируем положение персонажей для заставки
    initScreensaver();

    // рисуем в качестве заднего фона SEGA
    drawBackground();

	// звук SEGA при старте игры !
	XGM_startPlayPCM(64, 15, SOUND_PCM_CH2);

    // цикл анимации игры
    while(1) {
    	// что нажато на 1 джойстике
    	pad1 = JOY_readJoypad(JOY_1);

        // что нажато на 2 джойстике
    	pad2 = JOY_readJoypad(JOY_2);

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
    }

    return (0);
}


