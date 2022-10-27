/************************************************************************
* lcd.c

* 5 semestre - Eng. da Computacao - Insper
*
* _Authors_ = [Matheus,Nivea]
*
* 2022 - Controlflix
*
*/
#include "lcd.h"
#include "controlflix.h"
#include "background.h"
void event_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

void first_screen(void){
			lv_obj_t * label = lv_label_create(lv_scr_act());
			lv_obj_t *img1 = lv_img_create(lv_scr_act());
			lv_img_set_src(img1, &white);
			lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
			
			lv_obj_t *img = lv_img_create(lv_scr_act());
			lv_img_set_src(img, &controlflix);
			lv_obj_align(img, LV_ALIGN_CENTER, 0, -30);
					
			label = lv_label_create(lv_scr_act());
			lv_label_set_recolor(label, true);
			lv_label_set_text(label, "#ff0000 Mais controle para assistir Netflix#");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);
			
			label = lv_label_create(lv_scr_act());
			lv_label_set_recolor(label, true);
			lv_label_set_text(label, "Authors: Matheus&Nivea");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, 80);
			
			label = lv_label_create(lv_scr_act());
			lv_label_set_recolor(label, true);
			lv_label_set_text(label, "Engenharia da Computacao - 5 semestre");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, 100);
			
			label = lv_label_create(lv_scr_act());
			lv_label_set_recolor(label, true);
			lv_label_set_text(label, "Desconectado");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, -100);
}

void new_screen(lcd_info *new_lcd){
	static int value = 0;
	static play = 0;
	lv_obj_clean(lv_scr_act());
	
	lv_obj_t *img1 = lv_img_create(lv_scr_act());
	lv_img_set_src(img1, &white);
	lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
	
	lv_obj_t * label = lv_label_create(lv_scr_act());
	if (new_lcd->is_on) {
		lv_obj_t * label_per = lv_label_create(lv_scr_act());
		
		lv_obj_t *img = lv_img_create(lv_scr_act());
		lv_img_set_src(img, &controlflix);
		lv_obj_align(img, LV_ALIGN_CENTER, 0, 80);
		
		/*Create an Arc*/
		lv_obj_t * arc = lv_arc_create(lv_scr_act());
		lv_obj_set_size(arc, 150, 150);
		lv_arc_set_rotation(arc, 135);
		lv_arc_set_bg_angles(arc, 0, 270);
		
		if (new_lcd->button == 1){
			lv_obj_t * btn = lv_btn_create(lv_scr_act());
			lv_obj_align(btn, LV_ALIGN_CENTER, -120, -10);

			label = lv_label_create(btn);
			lv_label_set_text(label, "-10s");
		}
		else if (new_lcd->button == 2){
			lv_obj_t * btn = lv_btn_create(lv_scr_act());
			lv_obj_align(btn, LV_ALIGN_CENTER, 120, -10);

			label = lv_label_create(btn);
			lv_label_set_text(label, "+10s");
		}
		else if (new_lcd->button == 3){
			lv_obj_t * btn = lv_btn_create(lv_scr_act());
			lv_obj_align(btn, LV_ALIGN_CENTER, 0, 25);

			label = lv_label_create(btn);
			if (play){
				lv_label_set_text(label, "Play");
				play = !play;
			}
			else{
				lv_label_set_text(label, "Pause");
				play = !play;
			}	
		}
		if (new_lcd->value ==0){
			lv_arc_set_value(arc, value);
		}
		else{
			lv_arc_set_value(arc, new_lcd->value);
			value = new_lcd->value;
		}
		lv_obj_center(arc);
		lv_obj_add_event_cb(arc, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, label);
	
		if (new_lcd->is_connected){
			label = lv_label_create(lv_scr_act());
			lv_label_set_recolor(label, true);
			lv_label_set_text(label, "#ff0000 Conectado#");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, -110);
		}
		else{
			lv_label_set_recolor(label, true);
			lv_label_set_text(label, "Desconectado");
			lv_obj_align(label, LV_ALIGN_CENTER, 0, -110);
		}
		label = lv_label_create(lv_scr_act());
		lv_label_set_recolor(label, true);
		lv_label_set_text(label, new_lcd->str);
		lv_obj_align(label, LV_ALIGN_CENTER, 0, -90);
	
		label_per = lv_label_create(lv_scr_act());
		char ok[70];
		sprintf(ok,"%d%%",value);
		lv_label_set_text(label_per, ok);
		lv_obj_align(label_per, LV_ALIGN_CENTER, 0, -10);


		/*Manually update the label for the first time*/
		lv_event_send(arc, LV_EVENT_VALUE_CHANGED, NULL);
	}
	else{
		lv_obj_t * label = lv_label_create(lv_scr_act());
		lv_obj_t *img = lv_img_create(lv_scr_act());
		lv_img_set_src(img, &controlflix);
		lv_obj_align(img, LV_ALIGN_CENTER, 0, -30);
				
		label = lv_label_create(lv_scr_act());
		lv_label_set_recolor(label, true);
		lv_label_set_text(label,"#ff0000 Desligado#");
		lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);
		
		label = lv_label_create(lv_scr_act());
		lv_label_set_recolor(label, true);
		lv_label_set_text(label, "Authors: Matheus&Nivea");
		lv_obj_align(label, LV_ALIGN_CENTER, 0, 80);
		
		label = lv_label_create(lv_scr_act());
		lv_label_set_recolor(label, true);
		lv_label_set_text(label, "Engenharia da Computacao - 5 semestre");
		lv_obj_align(label, LV_ALIGN_CENTER, 0, 100);
	}
}


void value_changed_event_cb(lv_event_t * e)
{
	lv_obj_t * arc = lv_event_get_target(e);
	

	//lv_label_set_text_fmt(label, "%d%%", lv_arc_get_value(arc));

	/*Rotate the label to the current position of the arc*/
	//lv_arc_rotate_obj_to_angle(arc, label, 25);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/
void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void configure_lvgl(void) {
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

