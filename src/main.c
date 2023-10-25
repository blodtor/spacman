/**
 * Super Turbo MEGA Pac-Man 1.0
 * 
 * Чтобы собрать - надо запустить ./compile.sh
 * 
 * Разрешение у Sega Genesis / Sega Megadrive - 320 x 240 
 * 
 */

#include <genesis.h>

#include "resources.h"

// спрайт Соника
Sprite* sonicSprite;

// спрайт Pac-Man
Sprite* pacmanSprite;

// спрайт RED
Sprite* redSprite;

// спрайт Pac-Girl
Sprite* pacGirlSprite;


// координаты где рисуем спрайт Соника
s16 sonicX = -95;
s16 sonicY = 83;

// скорость перемещения Соника
s16 soincDx = 1;


// координаты где рисуем спрайт Pac-Man
s16 pacmanX = -100;
s16 pacmanY = 105;

// скорость перемещения Pac-Man
s16 pacmanDx = 1;


// координаты где рисуем спрайт RED
s16 redX = 400;
s16 redY = 103;

// скорость перемещения RED
s16 redDx = 0;

// координаты где рисуем спрайт Pac-Girl
s16 pacGirlX = -100;
s16 pacGirlY = 90;

// скорость перемещения Pac-Girl
s16 pacGirlDx = 0;


// Tile которым буду все закрашивать
const u32 tile[8] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

/**
 * сбрасываем положение спраутов персонажей для отображения заставкии
 * в началоное положение
 */
void initScreensaver() {
    sonicX = -95;
    sonicY = 83;
    soincDx = 1;
    pacmanX = -100;
    pacmanY = 105;
    pacmanDx = 1;
    redX = 400;
    redY = 103;
    redDx = 0;
    pacGirlX = -100;
    pacGirlY = 90;
    pacGirlDx = 0;

    // TODO а надоли то что ниже
    SPR_setAnim(pacmanSprite, 0);
    SPR_setAnim(redSprite, 0);
    SPR_setAnim(pacGirlSprite, 0);

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

    // Обновляет и отображает спрайты на экране
    SPR_update();

    //  делает всю закулисную обработку, нужен когда есть спрайты, музыка, джойстик.
    SYS_doVBlankProcess();

}

/**
 * отображаем заставку
 * return - 1 всегда после отрисовки заставки
 */
int screensaver() {
	// что нажато на контроллере 1
	u16 joy1 = 0;

	// что нажато на контроллере 2
	u16 joy2 = 0;

	initScreensaver();

    // рисуем задний фон (лого SEGA из sega.png)
    VDP_drawImage(BG_A, &sega_image, 0, 0);

    while(1) {
           SPR_setAnim(pacmanSprite, 0);
           SPR_setAnim(redSprite, 0);
           SPR_setAnim(pacGirlSprite, 0);

           // отоброзить по горизонтали спрайт RED
           SPR_setHFlip(redSprite, TRUE);

           if (pacmanDx < 0) {
               SPR_setHFlip(pacmanSprite, TRUE);
           } else {
               SPR_setHFlip(pacmanSprite, FALSE);
           }

           // задаем какую анимацию использовать
           if (sonicX <= 30)  {
               // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
               SPR_setAnim(sonicSprite, 2);
           } else if (sonicX <= 50)  {
               soincDx = 2;
               // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
               SPR_setAnim(sonicSprite, 2);
           } else if (sonicX <= 70) {
               soincDx = 3;
               // соник идет - 2 строчка анимации в файле sonic.png если считать с 0
               SPR_setAnim(sonicSprite, 2);
           } else if (sonicX <= 120) {
               soincDx = 5;
               // соник бежит - 3 строчка анимации в файле sonic.png
               SPR_setAnim(sonicSprite, 3);
           } else {
               soincDx = 6;
               // появляется RED
               redDx = -1;
               pacGirlDx = 1;
               // соник бежит - 3 строчка анимации в файле sonic.png
               SPR_setAnim(sonicSprite, 3);
           }

           // Обновляет и отображает спрайты на экране
           SPR_update();

           //  делает всю закулисную обработку, нужен когда есть спрайты, музыка, джойстик.
           SYS_doVBlankProcess();

           if (sonicX < 320) {
               // вычисляем новые координаты нахождения Соника
               sonicX += soincDx;
           }

           if (pacmanX < 210) {
               // вычисляем новые координаты нахождения Pac-Man
               pacmanX += pacmanDx;
           } else {
               // пора убегать от RED
               pacmanDx = -1;
               pacmanX = 209;
           }

           if (redX > -16) {
               // вычисляем новые координаты нахождения RED
               redX += redDx;
           }

           pacGirlX+= pacGirlDx;


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

           // что нажато на 1 джойстике
           joy1 = JOY_readJoypad(JOY_1);

           // что нажато на 2 джойстике
           joy2 = JOY_readJoypad(JOY_2);

           // условие окончания анимации заставки
           // тупо закончилась анимация
           // нажать любую кнопку на одном из Джойстиков (кроме Start)
           if ((pacGirlX >= 380)
        		 || (joy1 & BUTTON_A || joy1 & BUTTON_B || joy1 & BUTTON_C || joy1 & BUTTON_X || joy1 & BUTTON_Y || joy1 & BUTTON_Z)
				 || (joy2 & BUTTON_A || joy2 & BUTTON_B || joy2 & BUTTON_C || joy2 & BUTTON_X || joy2 & BUTTON_Y || joy2 & BUTTON_Z)
			   ) {
        	   // нужно всех персонажей убрать с экрана
        	   initScreensaver();
        	   return 1;
           }

       }
    return 1;
}

// точка входа в программу
int main() {
    // рисуем задний фон (лого SEGA из sega.png)
    //VDP_drawImage(BG_A, &img, 0, 0);

    // загружаем в tile из VDP 
    VDP_loadTileData(tile, 2, 1, 0);
    // инициализируем спрайтовый движок (выделяем место в VRAM под спрайты)
    SPR_init();

    // задали цвета в 4-ой палитре (отсчет начинается с нуля), цветами взятыми из спрайта соника, 
    // и выбрали в качестве способа передачи DMA.
    // Sega поддерживает 4 палитры по 16 цветов (PAL0-PAL3), и хранит их в CRAM.
    PAL_setPalette(PAL3, sonic_sprite.palette->data, DMA);
    PAL_setPalette(PAL2, red_sprite.palette->data, DMA);
    PAL_setPalette(PAL1, pacgirl_sprite.palette->data, DMA);

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
                                                , 0      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
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
                                                , 0      // приоритет спрайта (спрайт с меньшим числом, будет перекрывать спрайт с большим)
                                                , FALSE  // перевернуть по вертикали
                                                , FALSE  // перевернуть по горизонтали
                                              )
                                );


    // заставка SEGA которую съест Pac-Man и Pac-Girl
    screensaver();

    // рисуем в качестве заднего фона меню выбора количества игроков
    VDP_drawImage(BG_A, &menu_image, 0, 0);

	// что нажато на контроллере 1
	u16 joy1 = 0;

	// что нажато на контроллере 2
	u16 joy2 = 0;

    while(1) {
    	// что нажато на 1 джойстике
        joy1 = JOY_readJoypad(JOY_1);

        // что нажато на 2 джойстике
        joy2 = JOY_readJoypad(JOY_2);

        // Обновляет и отображает спрайты на экране
        SPR_update();

        //  делает всю закулисную обработку, нужен когда есть спрайты, музыка, джойстик.
        SYS_doVBlankProcess();

        // условие окончания выбора в меню
        // нажат start на одном из Джойстиков
        if ((joy1 & BUTTON_START)
				 || (joy2 & BUTTON_START)
			   ) {
     	   break;
        }
    }
    
    // рисуем в качестве заднего фона карту уровня
    VDP_drawImage(BG_A, &map_image, 0, 0);

    // цикл анимации игры
    while(1) {

    }

    return (0);
}


