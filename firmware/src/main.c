/************************************************************************
* main.c

* 5 semestre - Eng. da Computacao - Insper
* 
* _Authors_ = [Matheus,Nivea]  
*
* 2022 - Controlflix
*
*/

#include <asf.h>
#include <string.h>
#include <math.h>
#include "conf_board.h"
#include "lcd.h"
#include "bluetooth.h"
#include "configs.h"
/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define START 'W'
#define EOP 'X'
#define SACRIFICE 'K'
#define HANDSHAKE 'Z'
#define VOLTAR10S 'R'
#define PAUSE 'P'
#define AVANCAR10S 'A'
#define RANDOM 'O'
#define ONOFF 'B'
#define VOLUME 'V'
#define END_STRING '#'

#define TASK_MEAN_STACK_SIZE (4096/ sizeof(portSTACK_TYPE))
#define TASK_MEAN_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define TASK_BLUETOOTH_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_READ_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_READ_STACK_PRIORITY        (tskIDLE_PRIORITY)

/************************************************************************/
/* GLOBALS                                                              */
/************************************************************************/
QueueHandle_t xQueueMean;
QueueHandle_t xQueueBluetooth;
QueueHandle_t xQueueLCD;
SemaphoreHandle_t xSemaphoreWrite;
TimerHandle_t xTimer;

typedef struct {
	uint value;
} adcData;

typedef struct {
	char type;
	int value;
} packet;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);


static void task_read(void *pvParameters);
static void task_bluetooth(void *pvParameters);
static void task_proc(void *pvParameters);
static void task_lcd(void *pvParameters);

static void update_lcd_info(lcd_info *new_lcd, int is_on, int is_connected, int value, char str[], int button);
static void write_COM(char msg);
static void send_handshake(void);
static int log_conversor(adcData adc);
static int turn_off(void);
static int turn_on(void);
static int read_string(Usart *usart, char buffer[]);
static void decide_button(lcd_info *new_lcd, char button);
static void media_movel(packet *pot_packet, int ln,int *i);
static void send_package(packet *packet_to_send, int volume_changed);
static void change_volume(packet *packet_to_send,lcd_info *new_lcd);
static void send_button_status(packet *packet_to_send,lcd_info *new_lcd);

/************************************************************************/
/* RTOS application HOOK                                                */
/************************************************************************/
/* Called if stack overflow during execution */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	* identify which task has overflowed its stack.
	*/
	for (;;) {
	}
}

/* This function is called by FreeRTOS idle task */
extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/* This function is called by FreeRTOS each tick */
extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* HANDLERS / CALLBACKS                                                 */
/************************************************************************/

void vTimerCallback(TimerHandle_t xTimer) {
	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC_POT, AFEC_POT_CHANNEL);
	afec_start_software_conversion(AFEC_POT);
}

static void AFEC_pot_callback(void) {
	adcData adc;
	adc.value = afec_channel_get_value(AFEC_POT, AFEC_POT_CHANNEL);
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueMean, &adc, &xHigherPriorityTaskWoken);
}

void voltar10s_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	volatile packet but_packet = {.type = VOLTAR10S, .value = 0};
	xQueueSendFromISR(xQueueBluetooth, (void *)&but_packet, &xHigherPriorityTaskWoken);
}

void pause_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	volatile packet but_packet = {.type = PAUSE, .value = 0};
	xQueueSendFromISR(xQueueBluetooth, (void *)&but_packet, &xHigherPriorityTaskWoken);
}

void avancar10s_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	volatile packet but_packet = {.type = AVANCAR10S, .value = 0};
	xQueueSendFromISR(xQueueBluetooth, (void *)&but_packet, &xHigherPriorityTaskWoken);
}

void randomovie_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	volatile packet but_packet = {.type = RANDOM, .value = 0};
	xQueueSendFromISR(xQueueBluetooth, (void *)&but_packet, &xHigherPriorityTaskWoken);
}

void turnoff_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	volatile packet but_packet = {.type = ONOFF, .value = 0};
	xQueueSendFromISR(xQueueBluetooth, (void *)&but_packet, &xHigherPriorityTaskWoken);
}
/************************************************************************/
/* CONFIG FUNCTIONS                                                     */
/************************************************************************/
void configure_lcd(void) {
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	ili9341_init();
	ili9341_backlight_on();
}

void io_init(void) {
	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
	config_but(BUT1_PIO,BUT1_PIO_ID,BUT1_IDX_MASK,voltar10s_callback);
	config_but(BUT2_PIO,BUT2_PIO_ID,BUT2_IDX_MASK,pause_callback);
	config_but(BUT3_PIO,BUT3_PIO_ID,BUT3_IDX_MASK,avancar10s_callback);
	config_but(BUT4_PIO,BUT4_PIO_ID,BUT4_IDX_MASK,randomovie_callback);
	config_but(BUT5_PIO,BUT5_PIO_ID,BUT5_IDX_MASK,turnoff_callback);
}

static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
afec_callback_t callback) {
	afec_enable(afec);
	struct afec_config afec_cfg;
	afec_get_config_defaults(&afec_cfg);
	afec_init(afec, &afec_cfg);
	afec_set_trigger(afec, AFEC_TRIG_SW);
	struct afec_ch_config afec_ch_cfg;
	afec_ch_get_config_defaults(&afec_ch_cfg);
	afec_ch_cfg.gain = AFEC_GAINVALUE_0;
	afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);
	afec_channel_set_analog_offset(afec, afec_channel, 0x200);
	struct afec_temp_sensor_config afec_temp_sensor_cfg;
	afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
	afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);
	afec_set_callback(afec, afec_channel, callback, 1);
	NVIC_SetPriority(afec_id, 4);
	NVIC_EnableIRQ(afec_id);
}
/************************************************************************/
/* UTILS                                                                */
/************************************************************************/

/* Write on the serial port                                               */
static void write_COM(char msg){
	while(!usart_is_tx_ready(USART_COM)) {
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	usart_write(USART_COM,msg);
}
/* Send a sacrifice byte while dont receive the handshake from the client */
static void send_handshake(void){
	while(1){
		char rx;
		if (usart_read(USART_COM, &rx) == 0) {
			if (rx==HANDSHAKE){
				write_COM(HANDSHAKE);
				if (xTaskCreate(task_read, "Read", TASK_READ_STACK_SIZE, NULL,	TASK_READ_STACK_PRIORITY, NULL)!= pdPASS){
					printf("Failed to create Read task\r\n");
				}
				break;
			}
		}
		write_COM(SACRIFICE);
	}
}
/* Logaritmic conversor to adc data                           */
static int log_conversor(adcData adc){
	double ln_double = log((double) adc.value +2000);
	int ln = (1000) * ln_double;
	return ln;
}
/* Put the OS in sleep_mode, while reset the main tasks and LCD message       */
static int turn_off(void){
	xQueueReset(xQueueBluetooth);
	xQueueReset(xQueueMean);
	lcd_info new_lcd;
	update_lcd_info(&new_lcd,0,1,0,"",0);
	xQueueSend(xQueueLCD,&new_lcd,0);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	return 0;
}
/* Awaken the OS, and update the status of LCD                   */
static int turn_on(void){	
	lcd_info new_lcd;
	update_lcd_info(&new_lcd,1,1,0,"Ligando...",0);
	xQueueSend(xQueueLCD,&new_lcd,0);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	update_lcd_info(&new_lcd,1,1,0,"      ",0);
	xQueueSend(xQueueLCD,&new_lcd,0);
	return 1;
}
/* Read a string from serial port until it's end                                  */
static int read_string(Usart *usart, char buffer[]) {
	uint32_t rx = HANDSHAKE;
	uint32_t counter = 0;
	uint32_t time_ms = 100000;

	while(rx != END_STRING && time_ms > 0) {
		if(usart_read(usart, &rx) == 0) {
			if (counter == 0 && rx == HANDSHAKE){
				vTaskDelay(1);
			}
			else{
				buffer[counter++] = rx;
				vTaskDelay(1);
			}
		}
		else{
			time_ms--;
			vTaskDelay(1);
		}
	}
	buffer[counter] = 0x00;
	return counter;
}
/* Decide wich action to take and update  the LCD                   */
static void decide_button(lcd_info *new_lcd, char button){
	switch (button){
		case AVANCAR10S:
			update_lcd_info(new_lcd,1,1,0,"",2);
			xQueueSend(xQueueLCD, new_lcd, 0);
			break;
		case PAUSE:
			update_lcd_info(new_lcd,1,1,0,"",3);
			xQueueSend(xQueueLCD, new_lcd, 0);
			break;
		case VOLTAR10S:
			update_lcd_info(new_lcd,1,1,0,"",1);
			xQueueSend(xQueueLCD, new_lcd, 0);
			break;
		default:
			break;	
	}
	
}
/* Update the LCD Screen                 */
static void update_lcd_info(lcd_info *new_lcd, int is_on, int is_connected, int value, char str[], int button){
	new_lcd->value = value;
	new_lcd->is_on = is_on;
	new_lcd->is_connected = is_connected;
	new_lcd->button = button;
	strcpy(new_lcd->str,str);
}
/* Calculate the moving average of the volume data           */
static void media_movel(packet *pot_packet, int ln,int *i){
	static int pot_antigo = 0;
	static int media = 0;
	if ((*i) >= 3){
		media = media / 3;
		pot_packet->type = VOLUME;
		pot_packet->value = media;
		if (abs(media-pot_antigo)>50){
			xQueueSend(xQueueBluetooth, pot_packet, 10);
			pot_antigo = media;
		}
		*i = 0;
		media = 0;
	}
	else{
		(*i)++;
		media += ln;
	}
}

/* Send a package to the client                 */
static void send_package(packet *packet_to_send, int volume_changed){
	write_COM(START);
	if (volume_changed){
		write_COM(3);
		write_COM(packet_to_send->type);
		write_COM(packet_to_send->value);
		write_COM(packet_to_send->value >> 8);
	}
	else{
		write_COM(1);	
		write_COM(packet_to_send->type);
	}
	write_COM(EOP);
}

/* Update the volume arc in the screen  and in the client         */
static void change_volume(packet *packet_to_send,lcd_info *new_lcd){
	int alvo = ((double) 10.0/128.0)*(packet_to_send->value-7500);
	update_lcd_info(new_lcd,1,1,alvo,"",0);
	xQueueSend(xQueueLCD,new_lcd,0);
	send_package(packet_to_send,1);
	vTaskDelay(500 / portTICK_PERIOD_MS);
}

/*  Update the LCD and send the status by the pressed button           */
static void send_button_status(packet *packet_to_send,lcd_info *new_lcd){
	if (packet_to_send->type == RANDOM){
		xSemaphoreGive(xSemaphoreWrite);
		update_lcd_info(new_lcd,1,1,0,"Escolhendo seu longa...",0);
		xQueueSend(xQueueLCD,new_lcd,0);
	}
	decide_button(new_lcd,packet_to_send->type);
	send_package(packet_to_send,0);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	update_lcd_info(new_lcd,1,1,0,"",0);
	xQueueSend(xQueueLCD, new_lcd, 0);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

/* Task to read informations from client                 */
static void task_read(void *pvParameters) {
	lcd_info new_lcd;
	char str[40];
	for (;;)  {
		 if (xSemaphoreTake(xSemaphoreWrite,0)){
			if (read_string(USART_COM,str)){
				update_lcd_info(&new_lcd,1,1,0,str,0);
				xQueueSend(xQueueLCD, &new_lcd, 0);
			}
		}
	}
}

/* Task to update the LCD screen                */
static void task_lcd(void *pvParameters) {
	lcd_info new_lcd;
	first_screen();
	char buffer[40]="\0";
	for (;;)  {
		if (xQueueReceive(xQueueLCD, &new_lcd, 0)){
			if (strlen(new_lcd.str) > 2){
				strcpy(buffer,new_lcd.str);
			}
			else{
				strcpy(new_lcd.str,buffer);
			}
			new_screen(&new_lcd);
		}
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}

/* Process task to send the volume data                */
static void task_proc(void *pvParameters){
	config_AFEC_pot(AFEC_POT, AFEC_POT_ID, AFEC_POT_CHANNEL, AFEC_pot_callback);
	xTimer = xTimerCreate("Timer",100,pdTRUE,(void *)0, vTimerCallback);
	xTimerStart(xTimer, 0);
    adcData adc;
	int i = 0;
	int ln;
	packet pot_packet;
    for (;;) {
	    if (xQueueReceive(xQueueMean, &(adc), 1000)) {
			ln = log_conversor(adc);
			media_movel(&pot_packet,ln,&i);
		} 
    }
}

/* Main task, control and decide the information to send to the client           */
static void task_bluetooth(void *pvParameters) {
	config_usart0();
	hc05_init();
	io_init();
	first_screen();
	send_handshake();
	int is_on = 1;
	packet packet_to_send;
	lcd_info new_lcd;
	for (;;) {
		if (xQueueReceive(xQueueBluetooth, &packet_to_send, (TickType_t) 0)) {		
			if (packet_to_send.type == ONOFF){
				send_package(&packet_to_send,0);
				vTaskDelay(500 / portTICK_PERIOD_MS);
				if (is_on){
					is_on = turn_off();
					vTaskDelay(50 / portTICK_PERIOD_MS);
				}
				else{
					is_on = turn_on();
					vTaskDelay(50 / portTICK_PERIOD_MS);
				}
			}
			else if (packet_to_send.type == VOLUME ){
 			    	change_volume(&packet_to_send,&new_lcd);
			 }
			else if(packet_to_send.value == 0) {
				send_button_status(&packet_to_send,&new_lcd);
			}
		}
	}
}

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void) {
	/************************************************************************/
	/* Inicializações                                                       */
	/************************************************************************/
	sysclk_init();
	board_init();
	configure_console();
	configure_lcd();
	configure_touch();
	configure_lvgl();
	
	/************************************************************************/
	/* Filas e Semaforo                                                     */
	/************************************************************************/
	 xQueueMean = xQueueCreate(10, sizeof(adcData));
	 if (xQueueMean == NULL)
	 printf("falha em criar a queue xQueueMean \n");
	 
	 xQueueLCD = xQueueCreate(10, sizeof(lcd_info));
	 if (xQueueLCD == NULL)
	 printf("falha em criar a queue xQueueLCD \n");
	 
	 xQueueBluetooth = xQueueCreate(10, sizeof(packet));
	 if (xQueueBluetooth == NULL)
	 printf("falha em criar a queue xQueueBluetooth \n");
	 
	 xSemaphoreWrite = xSemaphoreCreateBinary();
	 if (xSemaphoreWrite == NULL)
	 printf("falha em criar o semaforo xSemaphoreWrite \n");
	
	/************************************************************************/
	/* Criação de tasks                                                     */
	/************************************************************************/	
	if(xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL) != pdPASS){
		printf("Failed to create bluetooth task\r\n");
	}
		
	if (xTaskCreate(task_proc, "Proc", TASK_MEAN_STACK_SIZE, NULL,TASK_MEAN_STACK_PRIORITY, NULL) != pdPASS) {
		 printf("Failed to create test PROC task\r\n");
	 }
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create lcd task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){}
	return 0;
}
