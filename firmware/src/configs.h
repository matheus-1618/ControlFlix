/************************************************************************
* configs.h

* 5 semestre - Eng. da Computacao - Insper
*
* _Authors_ = [Matheus,Nivea]
*
* 2022 - Controlflix
*
*/


#ifndef CONFIGS_H_
#define CONFIGS_H_

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

//Botão amarelo
#define BUT5_PIO      PIOA
#define BUT5_PIO_ID   ID_PIOA
#define BUT5_IDX      4
#define BUT5_IDX_MASK (1 << BUT5_IDX)


#define AFEC_POT AFEC0
#define AFEC_POT_ID ID_AFEC0
#define AFEC_POT_CHANNEL 0 // Canal do pino PD30


void configure_console(void);
void config_but(Pio *pio_t, uint32_t id, const uint32_t ul_mask, void (*p_handler)(uint32_t,uint32_t)); 


#endif /* CONFIGS_H_ */