/**
 * @file lv_demo_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_widgets.h"
#include "lv_events/lv_events.h"


#if LV_USE_DEMO_WIDGETS

#if LV_MEM_CUSTOM == 0 && LV_MEM_SIZE < (38ul * 1024ul)
    #error Insufficient memory for lv_demo_widgets. Please set LV_MEM_SIZE to at least 38KB (38ul * 1024ul).  48KB is recommended. 
#endif

#ifndef USE_IMGBTN_EVENT
	#define USE_IMGBTN_EVENT				0
#endif


/**********************
 *      Components
 **********************/

LV_IMG_DECLARE(lvgl_shallwing);
LV_IMG_DECLARE(lvgl_temper);

lv_obj_t              *sw_led;
lv_obj_t              *sw_buzzer;
lv_obj_t              *image_temper;
lv_obj_t              *label_temper;
lv_obj_t              *label_humidity;

lv_timer_t            *timer_sht20;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void led_event_cb(lv_event_t *e){

    lv_event_code_t code = lv_event_get_code(e);
    if(LV_EVENT_VALUE_CHANGED == code){
        if(lv_obj_has_state(sw_led, LV_STATE_CHECKED))
            led(true);
        else
            led(false);
    }
    return ;
}

static void buzzer_event_cb(lv_event_t *e){

    lv_event_code_t code = lv_event_get_code(e);
    if(LV_EVENT_VALUE_CHANGED == code){
        if(lv_obj_has_state(sw_buzzer, LV_STATE_CHECKED))
            buzzer(true);
        else
            buzzer(false);
    }
    return ;
}

#if USE_IMGBTN_EVENT
static void sht20_event_cb(lv_event_t *e){

    double               temperature, humidity;
	char				 buffer[32] = {0};

    lv_event_code_t code = lv_event_get_code(e);

    if(LV_EVENT_PRESSED == code){
    	sht20(&temperature, &humidity);
    	sprintf(buffer, "%.2lf 'C", temperature);
		lv_label_set_text(label_temper, buffer);
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%.2lf %%", humidity);
    	lv_label_set_text(label_humidity, buffer);
	}
    return ;
}
#endif

static void sht20_task(lv_timer_t *timer){

    double               temperature, humidity;
	char				 buffer[32] = {0};

    sht20(&temperature, &humidity);
    sprintf(buffer, "%.2lf 'C", temperature);
	lv_label_set_text(label_temper, buffer);
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%.2lf %%", humidity);
    lv_label_set_text(label_humidity, buffer);
    
    return ;
}


void lv_demo_widgets(void){

//lv_obj_create()         -- create a obj(blank component)
//lv_switch_create()      -- create a switch
//lv_btn_create()         -- create a button

//lv_obj_set_size(lv_obj_t *, int, int)       -- set the size
//lv_obj_set_width()                          -- set the width
//lv_obj_set_height()                         -- set the height

//lv_obj_set_pos()        -- set the position
//lv_obj_set_x()          -- set the x pos
//lv_obj_set_y()          -- set the y pos

    lv_obj_t              *image_shallwing = lv_img_create(lv_scr_act());
    lv_obj_t              *image_temper = lv_imgbtn_create(lv_scr_act());

    lv_obj_t              *label_lvgl_demo = lv_label_create(lv_scr_act());
    lv_obj_t              *label_author = lv_label_create(lv_scr_act());
    lv_obj_t              *label_studio = lv_label_create(lv_scr_act());

    lv_obj_t              *label_led = lv_label_create(lv_scr_act());
    lv_obj_t              *label_buzzer = lv_label_create(lv_scr_act());

    lv_obj_t              *calendar = lv_calendar_create(lv_scr_act());
    lv_obj_t              *temper_chart = lv_chart_create(lv_scr_act());

    sw_led = lv_switch_create(lv_scr_act());
    sw_buzzer = lv_switch_create(lv_scr_act());
    label_temper = lv_label_create(lv_scr_act());
    label_humidity = lv_label_create(lv_scr_act());

    //Set the image size and the position
    lv_img_set_src(image_shallwing, &lvgl_shallwing);
    lv_obj_set_pos(image_shallwing, 54, 52);

    //"Donald Shallwing"
    lv_obj_set_pos(label_author, 193, 61);
    lv_label_set_text(label_author, "Donald Shallwing");
    lv_obj_set_style_text_font(label_author, &lv_font_montserrat_24, LV_STATE_DEFAULT);

    //"LVGL for Demo"
    lv_obj_set_pos(label_lvgl_demo, 193, 108);
    lv_label_set_text(label_lvgl_demo, "donaldshallwing@gmail.com");
    lv_obj_set_style_text_font(label_lvgl_demo, &lv_font_montserrat_14, LV_STATE_DEFAULT);

    //"IoT-Yun"
    lv_obj_set_pos(label_studio, 193, 135);
    lv_label_set_text(label_studio, "IoT-Yun / CCNU");
    lv_obj_set_style_text_font(label_studio, &lv_font_montserrat_14, LV_STATE_DEFAULT);

    //Calendar
    lv_obj_set_pos(calendar, 511, 32);
    lv_obj_set_size(calendar, 205, 155);

    //Chart
    lv_obj_set_pos(temper_chart, 245, 222);
    lv_obj_set_size(temper_chart, 254, 190);

    //Switch for LED and buzzer
    lv_obj_set_pos(sw_led, 83, 234);
    lv_obj_set_size(sw_led, 60, 30);
    lv_obj_add_event_cb(sw_led, led_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_pos(sw_buzzer, 83, 335);
    lv_obj_set_size(sw_buzzer, 60, 30);
    lv_obj_add_event_cb(sw_buzzer, buzzer_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    //Label for LED and buzzer
    lv_obj_set_pos(label_led, 90, 270);
    lv_label_set_text(label_led, "LED");
    lv_obj_set_style_text_font(label_led, &lv_font_montserrat_20, LV_STATE_DEFAULT);
    lv_obj_set_pos(label_buzzer, 80, 382);
    lv_label_set_text(label_buzzer, "Buzzer");
    lv_obj_set_style_text_font(label_buzzer, &lv_font_montserrat_20, LV_STATE_DEFAULT);


    //Image for temperature & humidity
    lv_imgbtn_set_src(image_temper, LV_IMGBTN_STATE_RELEASED,
                      NULL, &lvgl_temper, NULL);
    lv_obj_set_size(image_temper, 80, 100);
    lv_obj_set_pos(image_temper, 557, 267);
	#if USE_IMGBTN_EVENT
		lv_obj_add_event_cb(image_temper, sht20_event_cb, LV_EVENT_PRESSED, NULL);
	#else
		timer_sht20 = lv_timer_create(sht20_task, 1000*20, NULL);
	#endif
	

    lv_obj_set_pos(label_temper, 662, 284);
    lv_label_set_text(label_temper, " ");
    lv_obj_set_style_text_font(label_temper, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_set_pos(label_humidity, 662, 329);
    lv_label_set_text(label_humidity, " ");
    lv_obj_set_style_text_font(label_humidity, &lv_font_montserrat_22, LV_STATE_DEFAULT);

    return ;
}

#endif
