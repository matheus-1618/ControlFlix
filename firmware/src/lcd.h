/************************************************************************
* lcd.h

* 5 semestre - Eng. da Computacao - Insper
*
* _Authors_ = [Matheus,Nivea]
*
* 2022 - Controlflix
*
*/

#ifndef LCD_H_
#define LCD_H_
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"
#include <asf.h>
#include <string.h>
/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

typedef struct {
	int value;
	int is_connected;
	int is_on;
	int button;
	char str[40];
} lcd_info;


//Global functions
void event_handler(lv_event_t * e);
void value_changed_event_cb(lv_event_t * e);
void first_screen(void);
void new_screen(lcd_info *new_lcd);
void configure_lcd(void);
void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void configure_lvgl(void);


#endif /* LCD_H_ */