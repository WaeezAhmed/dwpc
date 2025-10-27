#ifndef UART_DATA_H_
#define UART_DATA_H_

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "mgos_uart.h"
#include "types.h"


/*********************************************************************
 * mACROS
 */
/* as per data sheet TXD0 and RXD0 is used and as per mongoose uart interface 0 */
/* UART pins: RXIO = GPIO3,  TXIO = GPIO1,  CTS= N/A, RTS= N/A */
#define UART_INTERFACE_ZERO 0

// #define DEBUG

/*********************************************************************
 * GLOBAL VARIABLES
 */

bool uartInit();
size_t uartReadData(dwpc_config databuff);
void uart_dispatcher_cb(int uart_no, void *arg);
uint16_t calculate_crc16(const uint8_t *data, size_t len);
void append_crc_to_data(dwpc_data *data);
#endif