/*******************************************************************************
* Copyright (c) 2020, STMicroelectronics - All Rights Reserved
*
* This file is part of the VL53L5CX Ultra Lite Driver and is dual licensed,
* either 'STMicroelectronics Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, the VL53L5CX Ultra Lite Driver may be distributed under the
* terms of 'BSD 3-clause "New" or "Revised" License', in which case the
* following provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
*******************************************************************************/

#ifndef _PPCL5_H_
#define _PPCL5_H_

#define DEBUG_TRACES 1

#define ENABLED 1
#define DISABLED 0

#define OUTER 1
#define MIDDLE 2
#define INNER 3

#define OPTIONAL 1
#define NEEDED 2
#define DISCARD 3

#define GET_DATA_TIME_OUT_MS 1000

// Zones number and array side size
#define NB_ZONES 16 // MUST be 16 or 64
#if NB_ZONES == 16
#define	ZONES_ARRAY_SIDE_SIZE 4
#elif NB_ZONES == 64
#define	ZONES_ARRAY_SIDE_SIZE 8
#endif

#define PATTERN_TL_BR_BL_TR 1
#define PATTERN_TLBR_BLTR 2
#define PATTERN PATTERN_TLBR_BLTR

#define SUPER_ZONES_NB 2

/*GLOBAL VARIABLE*/
typedef struct
{
	int SuperZoneId;
	int NumberOfZones;
	int Zones[NB_ZONES];
} SuperZone_t;
SuperZone_t SuperZones[SUPER_ZONES_NB];

uint8_t innerCount;
uint8_t outerCount;
uint8_t inParallelDetectioncount;
uint8_t outParallelDetectioncount;


#include "vl53l8cx_api.h"


// This array stores the distance under any range under meams a person is in the fied of view
int16_t DistanceThresholdOf[NB_ZONES];
int16_t minDistance[NB_ZONES];


/**
 * @brief Function responsible for configuring the VL53L5 device in a mode fitting people counting algorithm
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @return int status. 0 : success. -1 : failed
 */
int ppcl5_config(VL53L8CX_Configuration * pDev);

/**
 * @brief
 * @param (VL53L5CX_Configuration)  pDev : VL53L5CX configuration structure.
 * @param (VL53L5CX_ResultsData) *pRangeResults : pointer on a ranging results data structure
 * @return int status. 0 : success. -1 : failed
 */
int ppcl5_get_data(VL53L8CX_Configuration * pDev, VL53L8CX_ResultsData * pRangeResults, uint16_t TimeOutms);

/**
 * @brief Initialization of the people counting
 * @return int status. 0 : success. -1 : failed
 */
int ppcl5_init();

/**
 * @brief Calibration of and scene analysis
 * @param (VL53L5CX_Configuration) *p_dev : VL53L5CX configuration structure.
 * @param (VL53L5CX_ResultsData) *pInitResults : Preliminary ranging data
 * @param (uint8_t) *pPathZones : Table of zones status telling the zones that must, should and are not supposed to range the floor
 * @param (uint8_t) *pInnerOuterZones : Table of zones defining what super zone each zone belongs to
 * @return int status. 0 : success. -1 : failed
 */
int ppcl5_calibrate(VL53L8CX_Configuration * pDev, VL53L8CX_ResultsData * pRangeResults, uint8_t * pPathZones, uint8_t * pInnerOuterZones);
int ppcl5_calibrate_max_min(VL53L8CX_Configuration * pDev, VL53L8CX_ResultsData * pRangeResults, uint8_t * pPathZones);
int ppcl5_calibrate_threshold(uint8_t * pPathZones, uint8_t * pInnerOuterZones);

/**
 * @brief Sending the new ranging data to the people counting
 * @param (VL53L5CX_ResultsData) *pRangeResults : pointer on a ranging results data structure
 * @param (uint8_t) *pPathZones : Table of zones status telling the zones that must, should and are not supposed to range the floor
 * @param (uint8_t) count : number of successful ranging data extraction before returning data
 * @return int status. 0 : success. -1 : failed
 */
int ppcl5_process_results(VL53L8CX_ResultsData * pRangeResults, uint8_t * pPathZones);

#endif
