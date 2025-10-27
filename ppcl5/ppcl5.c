/******************************************************************************
Copyright (C) 2015, STMicroelectronics International N.V. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of STMicroelectronics nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/
#include "mgos.h"
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "vl53l8cx_api.h"
#include "ppcl5.h"
#include "ppcl5_globals.h"
#include "websocket.h"
#include "esp_timer.h"
#include "rpc_callbacks.h"

#define UNINITIALIZED -1

#define PHASE_CONSISTENCY 4
#define RANGECOMPLETE 5				  // Normal ranging operation, everything was OK
#define RANGECOMPLETE_NO_WRAP_CHECK 6 // Very first status reported for a target only once, jsut after the device is started
#define RANGECOMPLETE_MERGED_PULSE 9  // Large number of consecutive bins of the histogram for a target. Can be a tilted target and 2 targets close to each other
#define PREV_RANGE_NO_TARGETS 10	  // status for a target that was not detected in the previous ranging. But no possible Wraparound error in case of people counting, it's a valid status
#define RANGE_STATUS_BLUR 12

// #if NB_ZONES == VL53L5CX_RESOLUTION_4X4
#if NB_ZONES == 16
#define SQRT_ZONES 4
// #elif NB_ZONES == VL53L5CX_RESOLUTION_8X8
#elif NB_ZONES == 64
#define SQRT_ZONES 8
#endif


// one way complete pathes-Entry
#define O_M_I 0x1B // expected path  00011011
#define O_I 0x1C   // less expected path 00011100
#define O_I_I 0x1F // less expected path 00011111
#define O_O_I 0x17 // less expected path

// one way partial pathes-Entry
#define M_I 0x2C   // unexpected path
#define O_M 0x18   // unexpected path
#define O_M_M 0x1A // unexpected path
#define M_I_I 0x2F // unexpected path
#define M_I_O 0x2D // unexpected path  ++// IN
#define M_M_I 0x2B // partial movement 00101011   ++
#define O_O_M 0x16 // partial movement 00010110  ++

// other way complete pathes - Exit 
#define I_M_O 0x39 // expected path
#define I_O 0x34   // less expected path
#define I_O_O 0x35 // less expected path
#define I_I_O 0x3D // less expected path

//Otherway partial pathes - Exit
#define M_O 0x24   // unexpected path
#define I_M 0x38   // unexpected path
#define I_M_M 0x3A // unexpected path
#define M_O_O 0x25 // unexpected path
#define M_M_O 0x29 // partial movement00101001  ++
#define I_I_M 0x3E // partial movement 00111110  ++
#define M_O_I 0x27 // unexpected path  ++// Out



// visible only 1 side
#define O_M_O 0x19
#define I_M_I 0x3B
#define O_ 0x10
#define I_ 0x30

// undertermined (wierd path)
#define M_O_M 0x26
#define M_I_M 0x2E
#define I_O_I 0x37
#define O_I_O 0x1D
#define O_I_M 0x1E // unexpected path  00011110
#define I_O_M 0x36 // unexpected path

#define IS_ZONE_TOP_LEFT(z) ((z % 16 < 8) && ((z % 16) % 2) == 0)
#define IS_ZONE_BOT_RIGHT(z) ((z % 16 > 7) && ((z % 16) % 2) == 1)
#define IS_ZONE_BOT_LEFT(z) ((z % 16 < 8) && ((z % 16) % 2) == 1)
#define IS_ZONE_TOP_RIGHT(z) ((z % 16 > 7) && ((z % 16) % 2) == 0)

// people counting status
static uint8_t Ppcl5Status = 0;
// Consecutive number of people detected
static uint8_t SomeoneHereCount = 0;

// Initialization and Calibration completed flags
static int InitializationCompleted = 0;
static int CalibrationCompleted = 0;

static uint16_t successfull_range_count[NB_ZONES];
static int16_t MinDistanceOfNEEDED, MinDistanceOf[NB_ZONES];
static int16_t MaxDistanceOfNEEDED, MaxDistanceOf[NB_ZONES];

#ifdef DEBUG_TRACES
typedef struct
{
	int ValidRangingNb;
	int UnvalidRangingNb;
	int NoTargetNb;
	int MinDistance;
	int MaxDistance;
	int MinSignal;
	int MaxSignal;
} CalibrationPerZoneStats_t;
#endif

// configuration of the device
int ppcl5_config(VL53L8CX_Configuration *pDev)
{

	uint8_t error;

	/* Set resolution. WARNING : As others settings depend to this
	 * one, it must be the first to use.
	 */
	error = vl53l8cx_set_resolution(pDev, NB_ZONES);
	if (error)
	{
#ifdef DEBUG
		printf("vl53l5cx_set_resolution failed, error %u\n", error);
#endif
		return error;
	}

	/* Set device in back to back (continuous mode)
	 */
	error = vl53l8cx_set_ranging_mode(pDev, VL53L8CX_RANGING_MODE_CONTINUOUS);
	if (error)
	{
#ifdef DEBUG
		printf("vl53l5cx_set_ranging_mode failed, error %u\n", error);
#endif
		return error;
	}

	/* Set ranging frequency to 10Hz.
	 * Using 4x4, min frequency is 1Hz and max is 60Hz
	 * Using 8x8, min frequency is 1Hz and max is 15Hz
	 */
	error = vl53l8cx_set_ranging_frequency_hz(pDev, sensor_frequency);
	if (error)
	{
#ifdef DEBUG
		printf("vl53l5cx_set_ranging_frequency_hz failed, error %u\n", error);
#endif
		return error;
	}

	/* Set target order to closest */
	error = vl53l8cx_set_target_order(pDev, VL53L8CX_TARGET_ORDER_CLOSEST);
	if (error)
	{
#ifdef DEBUG
		printf("vl53l5cx_set_target_order failed, error %u\n", error);
#endif
		return error;
	}

	return 0;
}

// Get ranging data
int ppcl5_get_data(VL53L8CX_Configuration *pDev, VL53L8CX_ResultsData *pRangeResults, uint16_t TimeOutms)
{
	uint8_t error, isReady, isAlive, ElapsedTime_ms = 0;
	// isReady =0;
	while (1)
	{

		error = vl53l8cx_check_data_ready(pDev, &isReady);

		if (error)
		{
#ifdef DEBUG
			printf("Error check data\n");
#endif
			if (debugEnable)
				send_message_to_webui("Error check data", error);
			return -1;
		}

		if (isReady)
		{

			vl53l8cx_get_ranging_data(pDev, pRangeResults);

			return 0;
		}

		/* Wait a few ms to avoid too high polling (function in platform
		 * file, not in API) */

		mgos_msleep(((1 / sensor_frequency) * 1000) / 2);
		// mgos_msleep(5);
		/* if TimeOut is set and elapsed time exceeds timeout then returns
		 */
		ElapsedTime_ms++;
		if ((TimeOutms > 0) && (ElapsedTime_ms > TimeOutms))
		{
#ifdef DEBUG
			printf("Time out\n");
#endif
			return -1;
		}
	}
}

// Initialization. Msut be called before ppcl5_calibrate
int ppcl5_init()
{
	int z;

	// init status and counters
	Ppcl5Status = 0;
	SomeoneHereCount = 0;

	// init super zones
	SuperZones[0].SuperZoneId = OUTER;
	SuperZones[0].NumberOfZones = 0;
	SuperZones[1].SuperZoneId = INNER;
	SuperZones[1].NumberOfZones = 0;
	for (z = 0; z < NB_ZONES; z++)
	{
		SuperZones[0].Zones[z] = UNINITIALIZED;
		SuperZones[1].Zones[z] = UNINITIALIZED;
	}

	InitializationCompleted = 1;
	// CalibrationCompleted = 0;
	// mgos_sys_config_set_dwpc_CalibrationCompleted(0);  // Added by sharath
	return 0;
}

int ppcl5_calibrate_max_min(VL53L8CX_Configuration *pDev, VL53L8CX_ResultsData *pRangeResults, uint8_t *pPathZones)
{

#ifdef DEBUG_TRACES
	CalibrationPerZoneStats_t CalibrationStats[NB_ZONES];
	uint8_t target_id;
#endif
	uint8_t target_status, target_status_ok;
	int16_t distance;
	uint32_t signal;
	int i, z, error;

	// Initialization must be completed
	// if (!InitializationCompleted)
	if (!InitializationCompleted) // Added by sharath
	{
#ifdef DEBUG
		printf("Error in ppcl5_calibrate. Initialization not done. Please call ppcl5_init \n");
#endif
		mgos_sys_config_set_service_thresholdCalibration_status("calibrationFailed at 01");
		return -1;
	}
	if (debugEnable)
		send_message_to_webui("----- Start ppcl5_calibrate_max_min  ----", 0);
#ifdef DEBUG
	printf("----- Start ppcl5_calibrate_max_min  ----\n");
#endif
	// init MaxDistanceOf array
	MinDistanceOfNEEDED = INT16_MAX;
	MaxDistanceOfNEEDED = 0;
	for (z = 0; z < NB_ZONES; z++)
	{
		successfull_range_count[z] = 0;
		MinDistanceOf[z] = INT16_MAX;
		MaxDistanceOf[z] = 0;
		DistanceThresholdOf[z] = 0;

		/*Erase superzones which are configured for Prev Calibrations*/
		SuperZones[0].NumberOfZones = 0;
		SuperZones[1].NumberOfZones = 0;
		SuperZones[0].SuperZoneId = 0;
		SuperZones[1].SuperZoneId = 0;
		SuperZones[0].Zones[z] = 0;
		SuperZones[1].Zones[z] = 0;
	}

#ifdef DEBUG_TRACES
	for (z = 0; z < NB_ZONES; z++)
	{
		CalibrationStats[z].ValidRangingNb = 0;
		CalibrationStats[z].UnvalidRangingNb = 0;
		CalibrationStats[z].NoTargetNb = 0;
		CalibrationStats[z].MinDistance = INT16_MAX;
		CalibrationStats[z].MaxDistance = 0;
		CalibrationStats[z].MinSignal = INT16_MAX;
		CalibrationStats[z].MaxSignal = 0;
	}

#endif

	// fill MaxDistanceOf
	// for (i = 0; i < NB_OF_RANGING_CALIBRATION; i++)
	for (i = 0; i < TotalSamplesForRangeCalibration; i++) // Added by sharath
	{
		// printf("%d\n", i);
		error = ppcl5_get_data(pDev, pRangeResults, GET_DATA_TIME_OUT_MS);
		if (error)
		{
			if (debugEnable)
				send_message_to_webui("In ppcl5_calibrate_max_min - ppcl5_get_data error", error);

			printf("\n In ppcl5_calibrate_max_min - ppcl5_get_data error  %u\n", error);
			return -1;
		}

		for (z = 0; z < NB_ZONES; z++)
		{
			if (pRangeResults->nb_target_detected[z] > 0)
			{ // if at least one target detected
#ifdef DEBUG_TRACES
				target_id = 0;
#endif
				target_status = pRangeResults->target_status[VL53L8CX_NB_TARGET_PER_ZONE * z];
				if ((target_status == RANGE_STATUS_BLUR) && (pRangeResults->nb_target_detected[z] > 1))
				{
#ifdef DEBUG_TRACES
					target_id = 1;
#endif
					target_status = pRangeResults->target_status[VL53L8CX_NB_TARGET_PER_ZONE * z + 1];
					distance = pRangeResults->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * z + 1];
					signal = pRangeResults->signal_per_spad[VL53L8CX_NB_TARGET_PER_ZONE * z + 1];
				}
				else
				{
					distance = pRangeResults->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * z];
					signal = pRangeResults->signal_per_spad[VL53L8CX_NB_TARGET_PER_ZONE * z];
				}
#ifndef DEBUG_TRACES
				printf("|%d,%2d,%4d,%4d,", target_id, target_status, distance, signal); // print sample for threshold calculation
#endif
				if (enable_low_confidence_target)
				{

					target_status_ok = ((target_status == RANGECOMPLETE) ||
										(target_status == RANGECOMPLETE_MERGED_PULSE) ||
										(target_status == PREV_RANGE_NO_TARGETS) ||
										(target_status == RANGECOMPLETE_NO_WRAP_CHECK) ||
										(target_status == PHASE_CONSISTENCY));
				}
				else
				{
					target_status_ok = ((target_status == RANGECOMPLETE) ||
										(target_status == RANGECOMPLETE_MERGED_PULSE) ||
										(target_status == RANGECOMPLETE_NO_WRAP_CHECK));
				}

#ifdef DEBUG_TRACES
				if (target_status_ok)
				{
					CalibrationStats[z].ValidRangingNb++;
					if (distance < CalibrationStats[z].MinDistance)
						CalibrationStats[z].MinDistance = distance;
					if (distance > CalibrationStats[z].MaxDistance)
						CalibrationStats[z].MaxDistance = distance;
					if (signal < CalibrationStats[z].MinSignal)
						CalibrationStats[z].MinSignal = signal;
					if (signal > CalibrationStats[z].MaxSignal)
						CalibrationStats[z].MaxSignal = signal;
				}
				else
					CalibrationStats[z].UnvalidRangingNb++;
#endif

				if (target_status_ok &&
					(distance >= minDistance[z]) &&
					(signal >= minSignal))
				{
					if (distance < MinDistanceOf[z])
						MinDistanceOf[z] = distance;
					if (distance > MaxDistanceOf[z])
						MaxDistanceOf[z] = distance;

					successfull_range_count[z]++;

					// remember as well the min and max distance of all NEEDED zones together
					if (pPathZones[z] == NEEDED)
					{
						if (distance < MinDistanceOfNEEDED)
							MinDistanceOfNEEDED = distance;
						if (distance > MaxDistanceOfNEEDED)
							MaxDistanceOfNEEDED = distance;
					}
				}
			}
#ifdef DEBUG_TRACES
			else
			{
				CalibrationStats[z].NoTargetNb++;
				//				printf("| ,  ,    ,    ,");
			}
			//			if (z%ZONES_ARRAY_SIDE_SIZE==(ZONES_ARRAY_SIDE_SIZE-1))
			//				printf("|\n");

#endif
		}
	}
	/*Added by sharath */
	// WaitMs(&pDev->platform,10);

#ifdef DEBUG_TRACES
	for (z = 0; z < NB_ZONES; z++)
	{
		if (CalibrationStats[z].MinDistance == INT16_MAX)
			CalibrationStats[z].MinDistance = -1;
		if (CalibrationStats[z].MaxDistance == 0)
			CalibrationStats[z].MaxDistance = -1;
		if (CalibrationStats[z].MinSignal == INT16_MAX)
			CalibrationStats[z].MinSignal = -1;
		if (CalibrationStats[z].MaxSignal == 0)
			CalibrationStats[z].MaxSignal = -1;
	}
#ifdef DEBUG
	printf("-- Number ranges for calibration : %d\n", TotalSamplesForRangeCalibration);
	printf("-- Number of valid ranging :%d\n", validSamplesForRangeCalibration);
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%2d ", CalibrationStats[z].ValidRangingNb);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("-- Number of unvalid ranges\n");
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%2d ", CalibrationStats[z].UnvalidRangingNb);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("-- Number of ranges with no target detected\n");
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%2d ", CalibrationStats[z].NoTargetNb);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("-- Min distance\n");
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%4d ", CalibrationStats[z].MinDistance);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("-- Max distance\n");
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%4d ", CalibrationStats[z].MaxDistance);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("-- Min Signal\n");
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%4d ", CalibrationStats[z].MinSignal);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("-- Max signal\n");
	for (z = 0; z < NB_ZONES; z++)
	{
		printf("%4d ", CalibrationStats[z].MaxSignal);
		if (z % ZONES_ARRAY_SIDE_SIZE == (ZONES_ARRAY_SIDE_SIZE - 1))
			printf("\n");
	}
	printf("----- End of calibration data ----\n");
#endif
#endif

	return 0;
}

int ppcl5_calibrate_threshold(uint8_t *pPathZones, uint8_t *pInnerOuterZones)
{
	int i, j, z, sz, add_zone_to_sz;
	char ptr[110] = "";
#ifdef DEBUG
	printf("\n In ppcl5_calibrate_threshold begin \n");
#endif
	if (debugEnable)
		send_message_to_webui("In ppcl5_calibrate_threshold begin", 0);
	// check is the needed zones see the floor at a minimal expected distance
	if (MinDistanceOfNEEDED < min_floor_distance)
	{
#ifdef DEBUG
		printf("Calibration failed :\n");
		printf("    Floor detected at a too close distance. Expected minimal distance:%d , got:%d\n", min_floor_distance, MinDistanceOfNEEDED);
#endif
		if (debugEnable)
		{

			sprintf(ptr, "Calibration failed Floor detected at a too close distance. Expected minimal distance:%d , got:%d", min_floor_distance, MinDistanceOfNEEDED);
			send_message_to_webui(ptr, 0);
		}
		mgos_sys_config_set_service_thresholdCalibration_status("calibrationFailed at 02");

		return -1;
	}

	// check delta of all middle zones together
	if ((MaxDistanceOfNEEDED - MinDistanceOfNEEDED) > maxDistanceDelta)
	{
#ifdef DEBUG
		printf("Calibration failed :\n");
		printf("    distance delta in mandatory zones too large. max:%d - min:%d = %d > %d\n", MaxDistanceOfNEEDED, MinDistanceOfNEEDED, MaxDistanceOfNEEDED - MinDistanceOfNEEDED, maxDistanceDelta);
#endif
		if (debugEnable)
		{

			sprintf(ptr, "Calibration failed distance delta in mandatory zones too large. max:%d - min:%d = %d > %d", MaxDistanceOfNEEDED, MinDistanceOfNEEDED, MaxDistanceOfNEEDED - MinDistanceOfNEEDED, maxDistanceDelta);
			send_message_to_webui(ptr, 0);
		}
		mgos_sys_config_set_service_thresholdCalibration_status("calibrationFailed at 03");
		return -1;
	}

	// check nb of successfull ranging and add to super zones
	for (z = 0; z < NB_ZONES; z++)
	{
		add_zone_to_sz = 0;

		// for now let's consider that these zones are mandattory since they are the ones in
		// the middle and should range the floor
		if (pPathZones[z] == NEEDED)
		{
			if (successfull_range_count[z] >= validSamplesForRangeCalibration)
				// delta already checked
				add_zone_to_sz = 1;
			else
			{
#ifdef DEBUG
				printf("Calibration failed. zone %d has only %d successfull ranging\n", z, successfull_range_count[z]);
#endif
				if (debugEnable)
				{

					sprintf(ptr, "Calibration failed. zone %d has only %d successfull ranging", z, successfull_range_count[z]);
					send_message_to_webui(ptr, 0);
				}
				mgos_sys_config_set_service_thresholdCalibration_status("calibrationFailed at 04");
				return -1;
			}
		}
		else if (pPathZones[z] == OPTIONAL)
		{
			// printf("size zone. z:%d sfr:%d maxdist:%d mindist:%d maxdist_mzs:%d mindist_mzs:%d\n",
			//  z, successfull_range_count[z], MaxDistanceOf[z], MinDistanceOf[z], MaxDistanceOfNEEDED, MinDistanceOfNEEDED);
			if ((successfull_range_count[z] >= validSamplesForRangeCalibration) &&
				(MinDistanceOf[z] <= MaxDistanceOfNEEDED) &&
				(MaxDistanceOf[z] >= MinDistanceOfNEEDED) &&
				((MaxDistanceOf[z] - MinDistanceOf[z]) <= maxDistanceDelta))
				add_zone_to_sz = 1;
		}

		if (add_zone_to_sz)
		{
			// DistanceThresholdOf[z] = MinDistanceOf[z] - mgos_sys_config_get_dwpc_threshold();
			DistanceThresholdOf[z] = MinDistanceOf[z] - Threshold;
			if (pInnerOuterZones[z] == INNER)
			{
				// add the zone of some as part of the super zone it belongs to
				SuperZones[1].Zones[SuperZones[1].NumberOfZones] = z;
				SuperZones[1].NumberOfZones++;
			}
			else if (pInnerOuterZones[z] == OUTER)
			{
				// add the zone of some as part of the super zone it belongs to
				SuperZones[0].Zones[SuperZones[0].NumberOfZones] = z;
				SuperZones[0].NumberOfZones++;
			}
			else
			{
#ifdef DEBUG
				printf("\n FATAL Error. InnerOuter table not initialized properly at index : %d\n", z);
				printf(" , and the actual value is : %d\n", pInnerOuterZones[z]);
#endif
			}
		}
		else
		{
			DistanceThresholdOf[z] = 0;
		}
	}
#ifdef DEBUG
	printf("super zones :\n");
	for (sz = 0; sz < SUPER_ZONES_NB; sz++)
	{
		printf("%2d,", SuperZones[sz].SuperZoneId);
		printf("%2d,", SuperZones[sz].NumberOfZones);
		for (z = 0; (z < SuperZones[sz].NumberOfZones); z++)
			if (SuperZones[sz].Zones[z] == UNINITIALIZED)
				printf("\nFATAL Error : Implementation error. This Zone should be not be UNINITIALIZED\n");
			else
				printf("%2d,", SuperZones[sz].Zones[z]);
		printf("\n");
	}

	printf("Threshold table :\n");
	for (i = 0; i < SQRT_ZONES; i++)
	{
		for (j = 0; j < SQRT_ZONES; j++)
		{
			printf("%2d-%s:%5d  ", i * SQRT_ZONES + j, ((pInnerOuterZones[i * SQRT_ZONES + j] == INNER) ? "I" : "O"), DistanceThresholdOf[i * SQRT_ZONES + j]);
		}
		printf("\n");
	}

	printf("\n");
#endif
	if (debugEnable)
	{

		send_message_to_webui("Threshold Table:", 0);
		getDistanceJsonPacket(DistanceThresholdOf, NB_ZONES);
		broadcast_array();
	}

	return 0;
}
// Check is a person crossed the area under tracking
int ppcl5_process_results(VL53L8CX_ResultsData *pRangeResults, uint8_t *pPathZones)
{

	static uint32_t count = 0;
	uint32_t signal;
	uint8_t target_status, target_status_ok, signal_too_low = 0;
	int16_t distance, threshold, min_distance;
	int i, z, sz;
	int position = 0, inner = 0, outer = 0, event = 0;
	innerCount = 0;
	outerCount = 0;

	// Initialization must be completed-  by passed

//-------------------------------------------------------------------
// #if NB_ZONES == VL53L5CX_RESOLUTION_8X8
#if NB_ZONES == 64
	// in 64 zones mode each the VL53L5 actually runs 4 sub zones ranings in a TopLeft - BotomRight - BottomLeft - TopRight order
	// Meaning for example for that the sub zones are ranged in this order 0;9;1;8
	int pattern;
#if PATTERN == PATTERN_TL_BR_BL_TR
	for (pattern = 0; pattern < 4; pattern++)
	{
#endif
#if PATTERN == PATTERN_TLBR_BLTR
		for (pattern = 0; pattern < 2; pattern++)
		{
#endif
#endif
			//------------------------------------------------------------------------
			for (sz = 0; sz < SUPER_ZONES_NB; sz++)
			{
				// run all the zones of the super zones
				for (i = 0; i < SuperZones[sz].NumberOfZones; i++)
				{
					z = SuperZones[sz].Zones[i];

// #if NB_ZONES == VL53L5CX_RESOLUTION_8X8
//-----------------------------------------------------------------------------
#if NB_ZONES == 64
#if PATTERN == PATTERN_TL_BR_BL_TR
					if ((((pattern == 0) && IS_ZONE_TOP_LEFT(z)) ||	  // zones 0,2,4,6,16,18,20,22,32, ...
						 ((pattern == 1) && IS_ZONE_BOT_RIGHT(z)) ||  // zones 9,11,13,15,25,27,29,31,41,43, ...
						 ((pattern == 2) && IS_ZONE_BOT_LEFT(z)) ||	  // zones 1,3,5,7,17,19,21,23,33,35, ...
						 ((pattern == 3) && IS_ZONE_TOP_RIGHT(z))) && // zones 8,10,12,14,24,26,28,30,40,44, ....
						(pRangeResults->nb_target_detected[z] > 0))
					{
#endif
#if PATTERN == PATTERN_TLBR_BLTR
						if ((((pattern == 0) && (IS_ZONE_TOP_LEFT(z) || IS_ZONE_BOT_RIGHT(z))) || // zones 0,2,4,6,9,11,13,15, 1816,18,20,22,32
							 ((pattern == 1) && (IS_ZONE_BOT_LEFT(z) || IS_ZONE_TOP_RIGHT(z)))) &&
							(pRangeResults->nb_target_detected[z] > 0))
						{
#endif
#else
			if (pRangeResults->nb_target_detected[z] > 0)
			{
#endif
							//-------------------------------------------------------------------------------------------
							target_status = pRangeResults->target_status[VL53L8CX_NB_TARGET_PER_ZONE * z];
							if ((target_status == RANGE_STATUS_BLUR) && (pRangeResults->nb_target_detected[z] > 1))
							{
								target_status = pRangeResults->target_status[VL53L8CX_NB_TARGET_PER_ZONE * z + 1];
								distance = pRangeResults->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * z + 1];
								signal = pRangeResults->signal_per_spad[VL53L8CX_NB_TARGET_PER_ZONE * z + 1];
							}
							else
							{
								distance = pRangeResults->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * z];
								signal = pRangeResults->signal_per_spad[VL53L8CX_NB_TARGET_PER_ZONE * z];
							}

							threshold = DistanceThresholdOf[z];
							if (enable_low_confidence_target)
							{

								target_status_ok = ((target_status == RANGECOMPLETE) ||
													(target_status == RANGECOMPLETE_MERGED_PULSE) ||
													(target_status == PREV_RANGE_NO_TARGETS) ||
													(target_status == RANGECOMPLETE_NO_WRAP_CHECK) ||
													(target_status == PHASE_CONSISTENCY));
							}
							else
							{
								target_status_ok = ((target_status == RANGECOMPLETE) ||
													(target_status == RANGECOMPLETE_MERGED_PULSE) ||
													(target_status == RANGECOMPLETE_NO_WRAP_CHECK));
							}
							min_distance = minDistance[z];
							if (target_status_ok &&
								(distance >= min_distance) && // at least some cms !
								(distance < threshold))
							{
								// some one is under the field of view
								// printf("someone ! dist = %d. threshold = %d\n", pRangeResults->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE*z], DistanceThresholdOf[z]);
								if (signal >= minSignal)
								{
									if (sz == 0)
									{ // outer
										outer = 1;
										outerCount += 1;
										// printf("someone in outer zone. z=%d\n", z);
									}
									else
									{
										inner = 1;
										innerCount += 1;
										// printf("someone in inner zone. z=%d\n", z);
									}
								}
								else
								{
									signal_too_low = 1;
								}
								// printf("\n|z:%d t:%d d:%d s:%d|", z, threshold, distance, signal);
							}
						}
						if (enable_two_person_count)
						{

							if (innerCount >= numberOfTargetPerZone)  // 4 is No of zones which we are intrested in 
							{
								inParallelDetectioncount++;
								if (debugEnable)
									send_message_to_webui("INParalleldetectionCount", inParallelDetectioncount);
								// printf("parallelDetectioncount:%d\n",parallelDetectioncount);
							}
							
							if (outerCount >= numberOfTargetPerZone)
							{
								outParallelDetectioncount++;
								if (debugEnable)
									send_message_to_webui("OUTParalleldetectionCount", outParallelDetectioncount);
							}
						}
					}

					// if someone seen in both super zones, no need lo look further
					if ((outer) && (inner))
						goto process;
				}

			process:

				if ((inner) || (outer))
				{
					if (SomeoneHereCount < min_consecutive_count)
						SomeoneHereCount++;

					if ((inner) && (outer))
					{
						position = MIDDLE;
#ifdef DEBUF
						printf("m");
#endif
					}
					else if (inner)
					{
						position = INNER;
#ifdef DEBUG
						printf("i");
#endif
					}
					else
					{
						position = OUTER;
#ifdef DEBUG
						printf("o");
#endif
					}

					if (Ppcl5Status == 0)
					{
						// first time a person is being seen. lets assigne bits [5-4]
						Ppcl5Status = position << 4;
					}
					else if ((Ppcl5Status & 0xF) == 0)
					{
						if ((Ppcl5Status & 0x30) != (position << 4))
						{
							// if in a different position, lets assign bits [3-2]
							Ppcl5Status |= (position << 2);
						}
						// else nothing to change
					}
					else if ((Ppcl5Status & 0x3) == 0)
					{
						if ((Ppcl5Status & 0xC) != (position << 2))
						{
							// if in a different position, lets assign bits [3-2]
							Ppcl5Status |= position;
						}
						// else nothing to change
					}
					else
					{
						// if seen in all all possible zones keep track of the latest in [1-0] bits
						Ppcl5Status &= 0x3C;
						Ppcl5Status |= position;
					}
				}
				else
				{ // nobody on the stage
					if (SomeoneHereCount >= min_consecutive_count)
					{
						if ((Ppcl5Status == O_M_I) || (Ppcl5Status == O_I) || (Ppcl5Status == O_I_I) || (Ppcl5Status == O_O_I))
						{
#ifdef DEBUG
							printf("+");
#endif
							event += event_one_way; // Default Entry: 1
						}
						else if ((Ppcl5Status == I_M_O) || (Ppcl5Status == I_O) || (Ppcl5Status == I_O_O) || (Ppcl5Status == I_I_O))
						{
#ifdef DEBUG
							printf("-");
#endif
							event += event_other_way; // Default Exit: 16
						}

						else if ((Ppcl5Status == M_I) ||
								 (Ppcl5Status == O_M) ||
								 (Ppcl5Status == O_M_M) ||
								 (Ppcl5Status == M_I_I) ||
								 (Ppcl5Status == M_M_I) || 
								 (Ppcl5Status == O_O_M)||
								 (Ppcl5Status == M_I_O)) // ++Lakki
						{

							if (enter_or_exit_middle_ok)
							{
								event += event_one_way;
							}
#ifdef DEBUG
							printf("U"); // unexpected path. should not happen. the definition of the INNER and OUTER zones should be double checked
#endif
						}
						else if ((Ppcl5Status == M_O) ||
								 (Ppcl5Status == I_M) ||
								 (Ppcl5Status == I_M_M) ||
								 (Ppcl5Status == M_O_O) ||
								 (Ppcl5Status == M_M_O) || 
								 (Ppcl5Status == I_I_M)||
								 (Ppcl5Status == M_O_I))  //++Lakki
						{

							if (enter_or_exit_middle_ok)
							{

								event += event_other_way;
							}
#ifdef DEBUG
							printf("U"); // unexpected path. should not happen. the definition of the INNER and OUTER zones should be double checked
#endif
						}
						else if ((Ppcl5Status == O_) ||
								 (Ppcl5Status == O_M_O) ||
								 (Ppcl5Status == I_) ||
								 (Ppcl5Status == I_M_I))
						{
							if (dwpcData.sensorFrequency < 65520)
							{

								dwpcData.sensorFrequency++; // oneway counter
							}
							else
							{
								dwpcData.sensorFrequency = 65525;
							}
							
#ifdef DEBUG
							printf(":"); // entered one side and exited same side
#endif
						}
						else if ((Ppcl5Status == M_O_M) ||
								 (Ppcl5Status == M_I_M) ||
								 (Ppcl5Status == I_O_I) ||
								 (Ppcl5Status == O_I_O) ||
								 (Ppcl5Status == I_O_M))
						{
							if (dwpcData.functionMode < 250)
							{

								dwpcData.functionMode++; //
							}
							else
							{
								dwpcData.functionMode = 255;
							}
#ifdef DEBUG
							printf("E"); // a really, really unexpected path. the settings of the device are likely wrong.
#endif
						}
						else
						{
							printf("\nFATAL : implementation error\n"); // unexpected path
						}
					}
					else
					{
						if (signal_too_low)
							printf("x");
						// else
						// 	printf(".");
					}
					Ppcl5Status = 0;
					SomeoneHereCount = 0;
				}

				count++;
				if (count == 150)
				{
					count = 0;
					// printf("\n");
				}

// #if NB_ZONES == VL53L5CX_RESOLUTION_8X8
#if NB_ZONES == 64
			} // end of for (pattern=0; pattern<2 (or 4); pattern++)
#endif

			return event;
		}
