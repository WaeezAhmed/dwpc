#ifndef TYPES_H_
#define TYPES_H_


#include <stdint.h>
#include <stdbool.h>
/*********************************************************************
 * GLOBAL variable
 */
char arrayofdistance[150];
char jsondata[400];


uint8_t incountAggregation;
uint8_t outcountAggregation;

typedef struct DWPC_CONFIG
{
    uint8_t toggleApMode;
    bool reboot;
}__attribute__((packed)) dwpc_config;

typedef struct DWPC_DATA
{
    uint8_t functionMode;
    uint8_t eventType;
    short PeopleCount;
    unsigned short inCount;
    unsigned short outCount;
    uint8_t ledColour;
    uint8_t buzzerState;
    uint16_t distanceThreshold;     //Added by Sharath
    uint16_t onepersoncountThreshold;  // originally distance 1
    uint16_t twopersoncountThreshold;    //originally distance 2
    uint16_t sensorFrequency;            // originally distance 3
    uint16_t sensingMode;                  // originally lastdetected distance
} __attribute__((packed)) dwpc_data;
dwpc_data dwpcData;
#endif