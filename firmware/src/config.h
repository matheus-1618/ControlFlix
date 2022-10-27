/*
 * config.h
 *
 * Created: 10/19/2022 12:13:52 AM
 *  Author: Matheus
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_
#include <asf.h>
#include "conf_board.h"

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX      11
#define BUT_IDX_MASK (1 << BUT_IDX)

// Botão Vermelho
#define BUT1_PIO      PIOC
#define BUT1_PIO_ID   ID_PIOC
#define BUT1_IDX      17
#define BUT1_IDX_MASK (1 << BUT1_IDX)

// Botão Azul
#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX      30
#define BUT2_IDX_MASK (1 << BUT2_IDX)

// Botão Verde
#define BUT3_PIO      PIOB
#define BUT3_PIO_ID   ID_PIOB
#define BUT3_IDX      2
#define BUT3_IDX_MASK (1 << BUT3_IDX)

// Botão Preto
#define BUT4_PIO      PIOA
#define BUT4_PIO_ID   ID_PIOA
#define BUT4_IDX      19
#define BUT4_IDX_MASK (1 << BUT4_IDX)


#define AFEC_POT AFEC0
#define AFEC_POT_ID ID_AFEC0
#define AFEC_POT_CHANNEL 0 // Canal do pino PD30
void vTimerCallback(TimerHandle_t xTimer);

void AFEC_pot_callback(void;

void vermelho_callback(void);

void azul_callback(void);

void verde_callback(void);
void preto_callback(void);

void io_init(void);

void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback);

void configure_console(void);

#endif /* CONFIG_H_ */