
#ifndef PCA9543A_H_
#define PCA9543A_H_
#include <stdint.h>
#include <stdbool.h>
#include "mgos_i2c.h"

#define PCA9543A_ADDRESS 0x77
#define TOF_SENSOR_MUX 0x77 // TOF sensors
#define LED_DRIVER_MUX 0x70 // Led

/*Reset pins for Mux*/
#define RESET_MUX1 4
#define RESET_MUX2 16

uint8_t selectMuxAndChannel(uint8_t muxAddress, uint8_t channel);
uint8_t selectChannel(uint8_t channel);
void muxReset();
void i2cScanner();

#endif