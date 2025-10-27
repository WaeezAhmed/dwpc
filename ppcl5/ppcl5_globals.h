#ifndef _GLOBALS_H_
#define _GLOBALS_H_
#include <stdint.h>
#include "types.h"
#include "ppcl5.h"
#include "vl53l8cx_api.h"

/*Direction */
#define DEFAULT_DIRECTION 0

/******************************
 * GLOBAL VARIBALES
 * */

uint8_t enable_two_person_count;
uint8_t minSignal;
uint8_t enable_low_confidence_target;
uint8_t Threshold; // mm
uint8_t numberOfTargetPerZone;
uint8_t min_consecutive_count;
uint8_t event_one_way;
uint8_t event_other_way;
uint8_t event_direction;
uint8_t enter_or_exit_middle_ok;
uint8_t	reflectance_percent;
uint8_t nb_samples;
uint8_t sensor_resolution;
uint8_t room_capacity;
uint16_t distance_mm;
uint16_t maxDistanceDelta;
uint16_t validSamplesForRangeCalibration;
uint16_t TotalSamplesForRangeCalibration;
uint16_t min_floor_distance;
uint8_t superDirection;
uint8_t occupancy;
uint8_t sensor_frequency;


// #if NB_ZONES == 16
//     0  1        2    3
//     4  5        6    7
//     8  9       10    11
//     12 13      14    15
//     outer      inner

// uint8_t PathZones[NB_ZONES] 
// {
// 	DISCARD, DISCARD, NEEDED, NEEDED,
// 	DISCARD, DISCARD, NEEDED, NEEDED,
// 	DISCARD, DISCARD, NEEDED, NEEDED,
// 	DISCARD, DISCARD, NEEDED, NEEDED,
// };

// uint8_t InnerOuterZones[NB_ZONES] 
// {
// 	OUTER, OUTER,      INNER, OUTER,
// 	OUTER, OUTER,      INNER, OUTER,
// 	OUTER, OUTER,      INNER, OUTER,
// 	OUTER, OUTER,      INNER, OUTER,
// };
// array of the zones


// #elif NB_ZONES == 64
//     0  1  2  3        4  5  6  7
//     8  9  10 11       12 13 14 15
//     16 17 18 19       20 21 22 23
//     24 25 16 27       28 29 30 31
//     32 33 34 35       36 37 38 39
//     40 41 42 43       44 45 46 47
//     48 49 50 51       52 53 54 55
//     56 57 58 59       60 61 62 63
//           outer       inner
uint8_t PathZones[NB_ZONES];// =
// {
// 	DISCARD,  DISCARD,  DISCARD,  DISCARD,       DISCARD,  DISCARD,  DISCARD,  DISCARD,//
// 	DISCARD,  DISCARD,  DISCARD,  DISCARD,       DISCARD,  DISCARD,  DISCARD,  DISCARD,//
// 	OPTIONAL, OPTIONAL, OPTIONAL, OPTIONAL,      OPTIONAL, OPTIONAL, OPTIONAL, OPTIONAL,//
// 	NEEDED,   NEEDED,   NEEDED,   NEEDED,        NEEDED,   NEEDED,   NEEDED,   NEEDED,//
// 	NEEDED,   NEEDED,   NEEDED,   NEEDED,        NEEDED,   NEEDED,   NEEDED,   NEEDED,//
// 	OPTIONAL, OPTIONAL, OPTIONAL, OPTIONAL,      OPTIONAL, OPTIONAL, OPTIONAL, OPTIONAL,//
// 	DISCARD,  DISCARD,  DISCARD,  DISCARD,       DISCARD,  DISCARD,  DISCARD,  DISCARD,//
// 	DISCARD,  DISCARD,  DISCARD,  DISCARD,       DISCARD,  DISCARD,  DISCARD,  DISCARD,//
// };
uint8_t InnerOuterZones[NB_ZONES];// =
// {
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// 	OUTER, OUTER, OUTER, OUTER,     INNER, INNER, INNER, INNER,//
// };


// #endif
/******************
 * GLOBAL FUNCTIONS 
 * */
void PathZoneInit();
void InnerOuterZoneInit();
void setDistanceThreshold(int16_t DistancePtr[]);
void getDistanceThreshold();
void getSuperZones();
void getDistanceJsonPacket(int16_t *DistancePtr, uint8_t size);
void gpio_init();
void controller_static_ip_reset();
void loadConfigurablefileds();
void minDistanceInit();
void getpixelcount(VL53L8CX_ResultsData * pRangeResults, uint8_t * pPathZones);
#endif