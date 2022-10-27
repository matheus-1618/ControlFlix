/************************************************************************
* bluetooth.h

* 5 semestre - Eng. da Computacao - Insper
*
* _Authors_ = [Matheus,Nivea]
*
* 2022 - Controlflix
*
*/

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_
#include <asf.h>
#include "conf_board.h"
#include <string.h>

#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define USART_COM USART1
#define USART_COM_ID ID_USART1
#else
#define USART_COM USART0
#define USART_COM_ID ID_USART0
#endif

uint32_t usart_puts(uint8_t *pstring);
void usart_put_string(Usart *usart, char str[]);
int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms);
void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen,
char buffer_tx[], int timeout);
void config_usart0(void);
int hc05_init(void);


#endif /* BLUETOOTH_H_ */