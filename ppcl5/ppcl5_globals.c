

#include "ppcl5_globals.h"
#include "ppcl5.h"
#include "mgos_sys_config.h"
#include "mgos_gpio.h"
#include "types.h"
#include "uart_data.h"
#include "vl53l8cx_api.h"

#define STATIC_IP_RST_BUTTON 0

#define PHASE_CONSISTENCY 4
#define RANGECOMPLETE 5				  // Normal ranging operation, everything was OK
#define RANGECOMPLETE_NO_WRAP_CHECK 6 // Very first status reported for a target only once, jsut after the device is started
#define RANGECOMPLETE_MERGED_PULSE 9  // Large number of consecutive bins of the histogram for a target. Can be a tilted target and 2 targets close to each other
#define PREV_RANGE_NO_TARGETS 10	  // status for a target that was not detected in the previous ranging. But no possible Wraparound error in case of people counting, it's a valid status
#define RANGE_STATUS_BLUR 12

static uint8_t PathZonesCount;
bool previouslyOccupied = 0;

void loadConfigurablefileds()
{
	room_capacity = mgos_sys_config_get_dwpc_capacity();
	sensor_resolution = mgos_sys_config_get_dwpc_resolution();
	enable_two_person_count = mgos_sys_config_get_dwpc_person_count_threshold();
	enable_low_confidence_target = mgos_sys_config_get_dwpc_enable_low_confidence_target();

	superDirection = mgos_sys_config_get_dwpc_resolution();
	/*Calibration variables*/
	// minDistance = mgos_sys_config_get_dwpc_min_distance();
	minDistanceInit();
	minSignal = mgos_sys_config_get_dwpc_min_signal();
	Threshold = mgos_sys_config_get_dwpc_threshold();
	min_floor_distance = mgos_sys_config_get_dwpc_min_floor_distance();
	maxDistanceDelta = mgos_sys_config_get_dwpc_max_distance_delta();
	numberOfTargetPerZone = mgos_sys_config_get_dwpc_numberOfTargetPerZone();

	validSamplesForRangeCalibration = mgos_sys_config_get_dwpc_validSamplesForRangeCalibration();
	TotalSamplesForRangeCalibration = mgos_sys_config_get_dwpc_TotalSamplesForRangeCalibration();

	// Consecutive number of samples in which people detected
	min_consecutive_count = mgos_sys_config_get_dwpc_min_consecutive_count();

	enter_or_exit_middle_ok = mgos_sys_config_get_dwpc_enter_or_exit_middle_ok();

	/*cross talk variables*/
	distance_mm = mgos_sys_config_get_dwpc_distance_mm();
	nb_samples = mgos_sys_config_get_dwpc_nb_samples();
	reflectance_percent = mgos_sys_config_get_dwpc_reflectance_percent();

	/*Direction variables*/
	event_direction = mgos_sys_config_get_dwpc_direction();
#ifdef DEBUG
	printf("event_direction:%d\n", event_direction);
#endif
	if (event_direction == DEFAULT_DIRECTION)
	{
		event_one_way = 0x1;	// entry
		event_other_way = 0x10; // exit
	}
	else
	{
		event_one_way = 0x10;  // exit
		event_other_way = 0x1; // entry
	}
#ifdef DEBUG
	printf("Event_one_way:%d\n", event_one_way);
	printf("Event_other_way:%d\n", event_other_way);
#endif

	/*Zibgee packet data*/
	dwpcData.functionMode = enter_or_exit_middle_ok * 10 + event_direction; // appending values
	dwpcData.distanceThreshold = 0;
	// dwpcData.onepersoncountThreshold = mgos_sys_config_get_dwpc_interval();
	// dwpcData.twopersoncountThreshold = mgos_sys_config_get_dwpc_periodic_reset();
	dwpcData.onepersoncountThreshold = 0;
	dwpcData.twopersoncountThreshold = 0;
	sensor_frequency = mgos_sys_config_get_dwpc_sensor_freq();
#if NB_ZONES == 16
	if (dwpcData.sensorFrequency > 60)
	{
		mgos_sys_config_set_dwpc_sensor_freq(60);
		sensor_frequency = mgos_sys_config_get_dwpc_sensor_freq();
	}
#elif NB_ZONES == 64
	if (dwpcData.sensorFrequency > 15)
	{
		mgos_sys_config_set_dwpc_sensor_freq(15);
		dwpcData.sensorFrequency = mgos_sys_config_get_dwpc_sensor_freq();
	}
#endif

#ifdef DEBUG
	printf("PathZonesCount:%d\n FunctionMode:%d\n sensor_freq:%d\n", dwpcData.distanceThreshold, dwpcData.functionMode, dwpcData.sensorFrequency);
#endif
}

/*******************************************************************************
*@fn     PathZoneInit

*@brief   Read  the zone status NEDDED, DISCARD or OPTIONAL

*@param   None

*@return  None
*/
void PathZoneInit()
{
#if NB_ZONES == 16
	PathZones[0] = mgos_sys_config_get_pathzone_index0();
	PathZones[1] = mgos_sys_config_get_pathzone_index1();
	PathZones[2] = mgos_sys_config_get_pathzone_index2();
	PathZones[3] = mgos_sys_config_get_pathzone_index3();

	PathZones[4] = mgos_sys_config_get_pathzone_index4();
	PathZones[5] = mgos_sys_config_get_pathzone_index5();
	PathZones[6] = mgos_sys_config_get_pathzone_index6();
	PathZones[7] = mgos_sys_config_get_pathzone_index7();

	PathZones[8] = mgos_sys_config_get_pathzone_index8();
	PathZones[9] = mgos_sys_config_get_pathzone_index9();
	PathZones[10] = mgos_sys_config_get_pathzone_index10();
	PathZones[11] = mgos_sys_config_get_pathzone_index11();

	PathZones[12] = mgos_sys_config_get_pathzone_index12();
	PathZones[13] = mgos_sys_config_get_pathzone_index13();
	PathZones[14] = mgos_sys_config_get_pathzone_index14();
	PathZones[15] = mgos_sys_config_get_pathzone_index15();
#elif NB_ZONES == 64

	PathZones[0] = mgos_sys_config_get_pathzone_index0();
	PathZones[1] = mgos_sys_config_get_pathzone_index0();
	PathZones[8] = mgos_sys_config_get_pathzone_index0();
	PathZones[9] = mgos_sys_config_get_pathzone_index0();

	PathZones[2] = mgos_sys_config_get_pathzone_index1();
	PathZones[3] = mgos_sys_config_get_pathzone_index1();
	PathZones[10] = mgos_sys_config_get_pathzone_index1();
	PathZones[11] = mgos_sys_config_get_pathzone_index1();

	PathZones[4] = mgos_sys_config_get_pathzone_index2();
	PathZones[5] = mgos_sys_config_get_pathzone_index2();
	PathZones[12] = mgos_sys_config_get_pathzone_index2();
	PathZones[13] = mgos_sys_config_get_pathzone_index2();

	PathZones[6] = mgos_sys_config_get_pathzone_index3();
	PathZones[7] = mgos_sys_config_get_pathzone_index3();
	PathZones[14] = mgos_sys_config_get_pathzone_index3();
	PathZones[15] = mgos_sys_config_get_pathzone_index3();

	PathZones[16] = mgos_sys_config_get_pathzone_index4();
	PathZones[17] = mgos_sys_config_get_pathzone_index4();
	PathZones[24] = mgos_sys_config_get_pathzone_index4();
	PathZones[25] = mgos_sys_config_get_pathzone_index4();

	PathZones[18] = mgos_sys_config_get_pathzone_index5();
	PathZones[19] = mgos_sys_config_get_pathzone_index5();
	PathZones[26] = mgos_sys_config_get_pathzone_index5();
	PathZones[27] = mgos_sys_config_get_pathzone_index5();

	PathZones[20] = mgos_sys_config_get_pathzone_index6();
	PathZones[21] = mgos_sys_config_get_pathzone_index6();
	PathZones[28] = mgos_sys_config_get_pathzone_index6();
	PathZones[29] = mgos_sys_config_get_pathzone_index6();

	PathZones[22] = mgos_sys_config_get_pathzone_index7();
	PathZones[23] = mgos_sys_config_get_pathzone_index7();
	PathZones[30] = mgos_sys_config_get_pathzone_index7();
	PathZones[31] = mgos_sys_config_get_pathzone_index7();

	PathZones[32] = mgos_sys_config_get_pathzone_index8();
	PathZones[33] = mgos_sys_config_get_pathzone_index8();
	PathZones[40] = mgos_sys_config_get_pathzone_index8();
	PathZones[41] = mgos_sys_config_get_pathzone_index8();

	PathZones[34] = mgos_sys_config_get_pathzone_index9();
	PathZones[35] = mgos_sys_config_get_pathzone_index9();
	PathZones[42] = mgos_sys_config_get_pathzone_index9();
	PathZones[43] = mgos_sys_config_get_pathzone_index9();

	PathZones[36] = mgos_sys_config_get_pathzone_index10();
	PathZones[37] = mgos_sys_config_get_pathzone_index10();
	PathZones[44] = mgos_sys_config_get_pathzone_index10();
	PathZones[45] = mgos_sys_config_get_pathzone_index10();

	PathZones[38] = mgos_sys_config_get_pathzone_index11();
	PathZones[39] = mgos_sys_config_get_pathzone_index11();
	PathZones[46] = mgos_sys_config_get_pathzone_index11();
	PathZones[47] = mgos_sys_config_get_pathzone_index11();

	PathZones[48] = mgos_sys_config_get_pathzone_index12();
	PathZones[49] = mgos_sys_config_get_pathzone_index12();
	PathZones[56] = mgos_sys_config_get_pathzone_index12();
	PathZones[57] = mgos_sys_config_get_pathzone_index12();

	PathZones[50] = mgos_sys_config_get_pathzone_index13();
	PathZones[51] = mgos_sys_config_get_pathzone_index13();
	PathZones[58] = mgos_sys_config_get_pathzone_index13();
	PathZones[59] = mgos_sys_config_get_pathzone_index13();

	PathZones[52] = mgos_sys_config_get_pathzone_index14();
	PathZones[53] = mgos_sys_config_get_pathzone_index14();
	PathZones[60] = mgos_sys_config_get_pathzone_index14();
	PathZones[61] = mgos_sys_config_get_pathzone_index14();

	PathZones[54] = mgos_sys_config_get_pathzone_index15();
	PathZones[55] = mgos_sys_config_get_pathzone_index15();
	PathZones[62] = mgos_sys_config_get_pathzone_index15();
	PathZones[63] = mgos_sys_config_get_pathzone_index15();
#endif
	for (uint8_t i = 0; i < NB_ZONES; i++)
	{
		if (PathZones[i] == NEEDED)
		{
			PathZonesCount++;
		}
	}
#ifdef DEBUG
	printf("Zone importance configured from user input!\n");
#endif
}

/*******************************************************************************
*@fn     InnerOuterZoneInit

*@brief   Read the zone definition INNER, OUTER or MIDDLE

*@param   None

*@return  None
*/
void InnerOuterZoneInit()
{
#if NB_ZONES == 16
	InnerOuterZones[0] = mgos_sys_config_get_InnerOuterZone_index0();
	InnerOuterZones[1] = mgos_sys_config_get_InnerOuterZone_index1();
	InnerOuterZones[2] = mgos_sys_config_get_InnerOuterZone_index2();
	InnerOuterZones[3] = mgos_sys_config_get_InnerOuterZone_index3();

	InnerOuterZones[4] = mgos_sys_config_get_InnerOuterZone_index4();
	InnerOuterZones[5] = mgos_sys_config_get_InnerOuterZone_index5();
	InnerOuterZones[6] = mgos_sys_config_get_InnerOuterZone_index6();
	InnerOuterZones[7] = mgos_sys_config_get_InnerOuterZone_index7();

	InnerOuterZones[8] = mgos_sys_config_get_InnerOuterZone_index8();
	InnerOuterZones[9] = mgos_sys_config_get_InnerOuterZone_index9();
	InnerOuterZones[10] = mgos_sys_config_get_InnerOuterZone_index10();
	InnerOuterZones[11] = mgos_sys_config_get_InnerOuterZone_index11();

	InnerOuterZones[12] = mgos_sys_config_get_InnerOuterZone_index12();
	InnerOuterZones[13] = mgos_sys_config_get_InnerOuterZone_index13();
	InnerOuterZones[14] = mgos_sys_config_get_InnerOuterZone_index14();
	InnerOuterZones[15] = mgos_sys_config_get_InnerOuterZone_index15();

#elif NB_ZONES == 64
	InnerOuterZones[0] = mgos_sys_config_get_InnerOuterZone_index0();
	InnerOuterZones[1] = mgos_sys_config_get_InnerOuterZone_index0();
	InnerOuterZones[8] = mgos_sys_config_get_InnerOuterZone_index0();
	InnerOuterZones[9] = mgos_sys_config_get_InnerOuterZone_index0();

	InnerOuterZones[2] = mgos_sys_config_get_InnerOuterZone_index1();
	InnerOuterZones[3] = mgos_sys_config_get_InnerOuterZone_index1();
	InnerOuterZones[10] = mgos_sys_config_get_InnerOuterZone_index1();
	InnerOuterZones[11] = mgos_sys_config_get_InnerOuterZone_index1();

	InnerOuterZones[4] = mgos_sys_config_get_InnerOuterZone_index2();
	InnerOuterZones[5] = mgos_sys_config_get_InnerOuterZone_index2();
	InnerOuterZones[12] = mgos_sys_config_get_InnerOuterZone_index2();
	InnerOuterZones[13] = mgos_sys_config_get_InnerOuterZone_index2();

	InnerOuterZones[6] = mgos_sys_config_get_InnerOuterZone_index3();
	InnerOuterZones[7] = mgos_sys_config_get_InnerOuterZone_index3();
	InnerOuterZones[14] = mgos_sys_config_get_InnerOuterZone_index3();
	InnerOuterZones[15] = mgos_sys_config_get_InnerOuterZone_index3();

	InnerOuterZones[16] = mgos_sys_config_get_InnerOuterZone_index4();
	InnerOuterZones[17] = mgos_sys_config_get_InnerOuterZone_index4();
	InnerOuterZones[24] = mgos_sys_config_get_InnerOuterZone_index4();
	InnerOuterZones[25] = mgos_sys_config_get_InnerOuterZone_index4();

	InnerOuterZones[18] = mgos_sys_config_get_InnerOuterZone_index5();
	InnerOuterZones[19] = mgos_sys_config_get_InnerOuterZone_index5();
	InnerOuterZones[26] = mgos_sys_config_get_InnerOuterZone_index5();
	InnerOuterZones[27] = mgos_sys_config_get_InnerOuterZone_index5();

	InnerOuterZones[20] = mgos_sys_config_get_InnerOuterZone_index6();
	InnerOuterZones[21] = mgos_sys_config_get_InnerOuterZone_index6();
	InnerOuterZones[28] = mgos_sys_config_get_InnerOuterZone_index6();
	InnerOuterZones[29] = mgos_sys_config_get_InnerOuterZone_index6();

	InnerOuterZones[22] = mgos_sys_config_get_InnerOuterZone_index7();
	InnerOuterZones[23] = mgos_sys_config_get_InnerOuterZone_index7();
	InnerOuterZones[30] = mgos_sys_config_get_InnerOuterZone_index7();
	InnerOuterZones[31] = mgos_sys_config_get_InnerOuterZone_index7();

	InnerOuterZones[32] = mgos_sys_config_get_InnerOuterZone_index8();
	InnerOuterZones[33] = mgos_sys_config_get_InnerOuterZone_index8();
	InnerOuterZones[40] = mgos_sys_config_get_InnerOuterZone_index8();
	InnerOuterZones[41] = mgos_sys_config_get_InnerOuterZone_index8();

	InnerOuterZones[34] = mgos_sys_config_get_InnerOuterZone_index9();
	InnerOuterZones[35] = mgos_sys_config_get_InnerOuterZone_index9();
	InnerOuterZones[42] = mgos_sys_config_get_InnerOuterZone_index9();
	InnerOuterZones[43] = mgos_sys_config_get_InnerOuterZone_index9();

	InnerOuterZones[36] = mgos_sys_config_get_InnerOuterZone_index10();
	InnerOuterZones[37] = mgos_sys_config_get_InnerOuterZone_index10();
	InnerOuterZones[44] = mgos_sys_config_get_InnerOuterZone_index10();
	InnerOuterZones[45] = mgos_sys_config_get_InnerOuterZone_index10();

	InnerOuterZones[38] = mgos_sys_config_get_InnerOuterZone_index11();
	InnerOuterZones[39] = mgos_sys_config_get_InnerOuterZone_index11();
	InnerOuterZones[46] = mgos_sys_config_get_InnerOuterZone_index11();
	InnerOuterZones[47] = mgos_sys_config_get_InnerOuterZone_index11();

	InnerOuterZones[48] = mgos_sys_config_get_InnerOuterZone_index12();
	InnerOuterZones[49] = mgos_sys_config_get_InnerOuterZone_index12();
	InnerOuterZones[56] = mgos_sys_config_get_InnerOuterZone_index12();
	InnerOuterZones[57] = mgos_sys_config_get_InnerOuterZone_index12();

	InnerOuterZones[50] = mgos_sys_config_get_InnerOuterZone_index13();
	InnerOuterZones[51] = mgos_sys_config_get_InnerOuterZone_index13();
	InnerOuterZones[58] = mgos_sys_config_get_InnerOuterZone_index13();
	InnerOuterZones[59] = mgos_sys_config_get_InnerOuterZone_index13();

	InnerOuterZones[52] = mgos_sys_config_get_InnerOuterZone_index14();
	InnerOuterZones[53] = mgos_sys_config_get_InnerOuterZone_index14();
	InnerOuterZones[60] = mgos_sys_config_get_InnerOuterZone_index14();
	InnerOuterZones[61] = mgos_sys_config_get_InnerOuterZone_index14();

	InnerOuterZones[54] = mgos_sys_config_get_InnerOuterZone_index15();
	InnerOuterZones[55] = mgos_sys_config_get_InnerOuterZone_index15();
	InnerOuterZones[62] = mgos_sys_config_get_InnerOuterZone_index15();
	InnerOuterZones[63] = mgos_sys_config_get_InnerOuterZone_index15();
#endif
#ifdef DEBUG
	printf("Zone direction configured from user input!\n");
#endif
}

void setDistanceThreshold(int16_t DistancePtr[])
{
#if NB_ZONES == 16
	mgos_sys_config_set_distanceThreshold_index0(DistancePtr[0]);
	mgos_sys_config_set_distanceThreshold_index1(DistancePtr[1]);
	mgos_sys_config_set_distanceThreshold_index2(DistancePtr[2]);
	mgos_sys_config_set_distanceThreshold_index3(DistancePtr[3]);

	mgos_sys_config_set_distanceThreshold_index4(DistancePtr[4]);
	mgos_sys_config_set_distanceThreshold_index5(DistancePtr[5]);
	mgos_sys_config_set_distanceThreshold_index6(DistancePtr[6]);
	mgos_sys_config_set_distanceThreshold_index7(DistancePtr[7]);

	mgos_sys_config_set_distanceThreshold_index8(DistancePtr[8]);
	mgos_sys_config_set_distanceThreshold_index9(DistancePtr[9]);
	mgos_sys_config_set_distanceThreshold_index10(DistancePtr[10]);
	mgos_sys_config_set_distanceThreshold_index11(DistancePtr[11]);

	mgos_sys_config_set_distanceThreshold_index12(DistancePtr[12]);
	mgos_sys_config_set_distanceThreshold_index13(DistancePtr[13]);
	mgos_sys_config_set_distanceThreshold_index14(DistancePtr[14]);
	mgos_sys_config_set_distanceThreshold_index15(DistancePtr[15]);

#elif NB_ZONES == 64

	mgos_sys_config_set_distanceThreshold_index0(DistancePtr[0]);
	mgos_sys_config_set_distanceThreshold_index0(DistancePtr[1]);
	mgos_sys_config_set_distanceThreshold_index0(DistancePtr[8]);
	mgos_sys_config_set_distanceThreshold_index0(DistancePtr[9]);

	mgos_sys_config_set_distanceThreshold_index1(DistancePtr[2]);
	mgos_sys_config_set_distanceThreshold_index1(DistancePtr[3]);
	mgos_sys_config_set_distanceThreshold_index1(DistancePtr[10]);
	mgos_sys_config_set_distanceThreshold_index1(DistancePtr[11]);

	mgos_sys_config_set_distanceThreshold_index2(DistancePtr[4]);
	mgos_sys_config_set_distanceThreshold_index2(DistancePtr[5]);
	mgos_sys_config_set_distanceThreshold_index2(DistancePtr[12]);
	mgos_sys_config_set_distanceThreshold_index2(DistancePtr[13]);

	mgos_sys_config_set_distanceThreshold_index3(DistancePtr[6]);
	mgos_sys_config_set_distanceThreshold_index3(DistancePtr[7]);
	mgos_sys_config_set_distanceThreshold_index3(DistancePtr[14]);
	mgos_sys_config_set_distanceThreshold_index3(DistancePtr[15]);

	mgos_sys_config_set_distanceThreshold_index4(DistancePtr[16]);
	mgos_sys_config_set_distanceThreshold_index4(DistancePtr[17]);
	mgos_sys_config_set_distanceThreshold_index4(DistancePtr[24]);
	mgos_sys_config_set_distanceThreshold_index4(DistancePtr[25]);

	mgos_sys_config_set_distanceThreshold_index5(DistancePtr[18]);
	mgos_sys_config_set_distanceThreshold_index5(DistancePtr[19]);
	mgos_sys_config_set_distanceThreshold_index5(DistancePtr[26]);
	mgos_sys_config_set_distanceThreshold_index5(DistancePtr[27]);

	mgos_sys_config_set_distanceThreshold_index6(DistancePtr[20]);
	mgos_sys_config_set_distanceThreshold_index6(DistancePtr[21]);
	mgos_sys_config_set_distanceThreshold_index6(DistancePtr[28]);
	mgos_sys_config_set_distanceThreshold_index6(DistancePtr[29]);

	mgos_sys_config_set_distanceThreshold_index7(DistancePtr[22]);
	mgos_sys_config_set_distanceThreshold_index7(DistancePtr[23]);
	mgos_sys_config_set_distanceThreshold_index7(DistancePtr[30]);
	mgos_sys_config_set_distanceThreshold_index7(DistancePtr[31]);

	mgos_sys_config_set_distanceThreshold_index8(DistancePtr[32]);
	mgos_sys_config_set_distanceThreshold_index8(DistancePtr[33]);
	mgos_sys_config_set_distanceThreshold_index8(DistancePtr[40]);
	mgos_sys_config_set_distanceThreshold_index8(DistancePtr[41]);

	mgos_sys_config_set_distanceThreshold_index9(DistancePtr[34]);
	mgos_sys_config_set_distanceThreshold_index9(DistancePtr[35]);
	mgos_sys_config_set_distanceThreshold_index9(DistancePtr[42]);
	mgos_sys_config_set_distanceThreshold_index9(DistancePtr[43]);

	mgos_sys_config_set_distanceThreshold_index10(DistancePtr[36]);
	mgos_sys_config_set_distanceThreshold_index10(DistancePtr[37]);
	mgos_sys_config_set_distanceThreshold_index10(DistancePtr[44]);
	mgos_sys_config_set_distanceThreshold_index10(DistancePtr[45]);

	mgos_sys_config_set_distanceThreshold_index11(DistancePtr[38]);
	mgos_sys_config_set_distanceThreshold_index11(DistancePtr[39]);
	mgos_sys_config_set_distanceThreshold_index11(DistancePtr[46]);
	mgos_sys_config_set_distanceThreshold_index11(DistancePtr[47]);

	mgos_sys_config_set_distanceThreshold_index12(DistancePtr[48]);
	mgos_sys_config_set_distanceThreshold_index12(DistancePtr[49]);
	mgos_sys_config_set_distanceThreshold_index12(DistancePtr[56]);
	mgos_sys_config_set_distanceThreshold_index12(DistancePtr[57]);

	mgos_sys_config_set_distanceThreshold_index13(DistancePtr[50]);
	mgos_sys_config_set_distanceThreshold_index13(DistancePtr[51]);
	mgos_sys_config_set_distanceThreshold_index13(DistancePtr[58]);
	mgos_sys_config_set_distanceThreshold_index13(DistancePtr[59]);

	mgos_sys_config_set_distanceThreshold_index14(DistancePtr[52]);
	mgos_sys_config_set_distanceThreshold_index14(DistancePtr[53]);
	mgos_sys_config_set_distanceThreshold_index14(DistancePtr[60]);
	mgos_sys_config_set_distanceThreshold_index14(DistancePtr[61]);

	mgos_sys_config_set_distanceThreshold_index15(DistancePtr[54]);
	mgos_sys_config_set_distanceThreshold_index15(DistancePtr[55]);
	mgos_sys_config_set_distanceThreshold_index15(DistancePtr[62]);
	mgos_sys_config_set_distanceThreshold_index15(DistancePtr[63]);

#endif

#ifdef DEBUG
	printf("New Distance threshold set to NV !\n");
#endif
}

void getDistanceThreshold()
{
#if NB_ZONES == 16

	DistanceThresholdOf[0] = mgos_sys_config_get_distanceThreshold_index0();
	DistanceThresholdOf[1] = mgos_sys_config_get_distanceThreshold_index1();
	DistanceThresholdOf[2] = mgos_sys_config_get_distanceThreshold_index2();
	DistanceThresholdOf[3] = mgos_sys_config_get_distanceThreshold_index3();

	DistanceThresholdOf[4] = mgos_sys_config_get_distanceThreshold_index4();
	DistanceThresholdOf[5] = mgos_sys_config_get_distanceThreshold_index5();
	DistanceThresholdOf[6] = mgos_sys_config_get_distanceThreshold_index6();
	DistanceThresholdOf[7] = mgos_sys_config_get_distanceThreshold_index7();

	DistanceThresholdOf[8] = mgos_sys_config_get_distanceThreshold_index8();
	DistanceThresholdOf[9] = mgos_sys_config_get_distanceThreshold_index9();
	DistanceThresholdOf[10] = mgos_sys_config_get_distanceThreshold_index10();
	DistanceThresholdOf[11] = mgos_sys_config_get_distanceThreshold_index11();

	DistanceThresholdOf[12] = mgos_sys_config_get_distanceThreshold_index12();
	DistanceThresholdOf[13] = mgos_sys_config_get_distanceThreshold_index13();
	DistanceThresholdOf[14] = mgos_sys_config_get_distanceThreshold_index14();
	DistanceThresholdOf[15] = mgos_sys_config_get_distanceThreshold_index15();

// 64 zones
#elif NB_ZONES == 64
	DistanceThresholdOf[0] = mgos_sys_config_get_distanceThreshold_index0();
	DistanceThresholdOf[1] = mgos_sys_config_get_distanceThreshold_index0();
	DistanceThresholdOf[8] = mgos_sys_config_get_distanceThreshold_index0();
	DistanceThresholdOf[9] = mgos_sys_config_get_distanceThreshold_index0();

	DistanceThresholdOf[2] = mgos_sys_config_get_distanceThreshold_index1();
	DistanceThresholdOf[3] = mgos_sys_config_get_distanceThreshold_index1();
	DistanceThresholdOf[10] = mgos_sys_config_get_distanceThreshold_index1();
	DistanceThresholdOf[11] = mgos_sys_config_get_distanceThreshold_index1();

	DistanceThresholdOf[4] = mgos_sys_config_get_distanceThreshold_index2();
	DistanceThresholdOf[5] = mgos_sys_config_get_distanceThreshold_index2();
	DistanceThresholdOf[12] = mgos_sys_config_get_distanceThreshold_index2();
	DistanceThresholdOf[13] = mgos_sys_config_get_distanceThreshold_index2();

	DistanceThresholdOf[6] = mgos_sys_config_get_distanceThreshold_index3();
	DistanceThresholdOf[7] = mgos_sys_config_get_distanceThreshold_index3();
	DistanceThresholdOf[14] = mgos_sys_config_get_distanceThreshold_index3();
	DistanceThresholdOf[15] = mgos_sys_config_get_distanceThreshold_index3();

	DistanceThresholdOf[16] = mgos_sys_config_get_distanceThreshold_index4();
	DistanceThresholdOf[17] = mgos_sys_config_get_distanceThreshold_index4();
	DistanceThresholdOf[24] = mgos_sys_config_get_distanceThreshold_index4();
	DistanceThresholdOf[25] = mgos_sys_config_get_distanceThreshold_index4();

	DistanceThresholdOf[18] = mgos_sys_config_get_distanceThreshold_index5();
	DistanceThresholdOf[19] = mgos_sys_config_get_distanceThreshold_index5();
	DistanceThresholdOf[26] = mgos_sys_config_get_distanceThreshold_index5();
	DistanceThresholdOf[27] = mgos_sys_config_get_distanceThreshold_index5();

	DistanceThresholdOf[20] = mgos_sys_config_get_distanceThreshold_index6();
	DistanceThresholdOf[21] = mgos_sys_config_get_distanceThreshold_index6();
	DistanceThresholdOf[28] = mgos_sys_config_get_distanceThreshold_index6();
	DistanceThresholdOf[29] = mgos_sys_config_get_distanceThreshold_index6();

	DistanceThresholdOf[22] = mgos_sys_config_get_distanceThreshold_index7();
	DistanceThresholdOf[23] = mgos_sys_config_get_distanceThreshold_index7();
	DistanceThresholdOf[30] = mgos_sys_config_get_distanceThreshold_index7();
	DistanceThresholdOf[31] = mgos_sys_config_get_distanceThreshold_index7();

	DistanceThresholdOf[32] = mgos_sys_config_get_distanceThreshold_index8();
	DistanceThresholdOf[33] = mgos_sys_config_get_distanceThreshold_index8();
	DistanceThresholdOf[40] = mgos_sys_config_get_distanceThreshold_index8();
	DistanceThresholdOf[21] = mgos_sys_config_get_distanceThreshold_index8();

	DistanceThresholdOf[34] = mgos_sys_config_get_distanceThreshold_index9();
	DistanceThresholdOf[35] = mgos_sys_config_get_distanceThreshold_index9();
	DistanceThresholdOf[42] = mgos_sys_config_get_distanceThreshold_index9();
	DistanceThresholdOf[43] = mgos_sys_config_get_distanceThreshold_index9();

	DistanceThresholdOf[36] = mgos_sys_config_get_distanceThreshold_index10();
	DistanceThresholdOf[37] = mgos_sys_config_get_distanceThreshold_index10();
	DistanceThresholdOf[44] = mgos_sys_config_get_distanceThreshold_index10();
	DistanceThresholdOf[45] = mgos_sys_config_get_distanceThreshold_index10();

	DistanceThresholdOf[38] = mgos_sys_config_get_distanceThreshold_index11();
	DistanceThresholdOf[39] = mgos_sys_config_get_distanceThreshold_index11();
	DistanceThresholdOf[46] = mgos_sys_config_get_distanceThreshold_index11();
	DistanceThresholdOf[47] = mgos_sys_config_get_distanceThreshold_index11();

	DistanceThresholdOf[48] = mgos_sys_config_get_distanceThreshold_index12();
	DistanceThresholdOf[49] = mgos_sys_config_get_distanceThreshold_index12();
	DistanceThresholdOf[56] = mgos_sys_config_get_distanceThreshold_index12();
	DistanceThresholdOf[57] = mgos_sys_config_get_distanceThreshold_index12();

	DistanceThresholdOf[50] = mgos_sys_config_get_distanceThreshold_index13();
	DistanceThresholdOf[51] = mgos_sys_config_get_distanceThreshold_index13();
	DistanceThresholdOf[58] = mgos_sys_config_get_distanceThreshold_index13();
	DistanceThresholdOf[59] = mgos_sys_config_get_distanceThreshold_index13();

	DistanceThresholdOf[52] = mgos_sys_config_get_distanceThreshold_index14();
	DistanceThresholdOf[53] = mgos_sys_config_get_distanceThreshold_index14();
	DistanceThresholdOf[60] = mgos_sys_config_get_distanceThreshold_index14();
	DistanceThresholdOf[61] = mgos_sys_config_get_distanceThreshold_index14();

	DistanceThresholdOf[54] = mgos_sys_config_get_distanceThreshold_index15();
	DistanceThresholdOf[55] = mgos_sys_config_get_distanceThreshold_index15();
	DistanceThresholdOf[62] = mgos_sys_config_get_distanceThreshold_index15();
	DistanceThresholdOf[63] = mgos_sys_config_get_distanceThreshold_index15();
#endif

#ifdef DEBUG
	printf("Distance threshold set from past callibration results!\n");
#endif
}

void getSuperZones()
{
	int z;
	// check nb of successfull ranging and add to super zones
	for (z = 0; z < NB_ZONES; z++)
	{
		// the middle and should range the floor
		if (PathZones[z] == NEEDED)
		{
			if (InnerOuterZones[z] == INNER)
			{
				// add the zone of some as part of the super zone it belongs to
				SuperZones[1].Zones[SuperZones[1].NumberOfZones] = z;
				SuperZones[1].NumberOfZones++;
			}
			else if (InnerOuterZones[z] == OUTER)
			{
				// add the zone of some as part of the super zone it belongs to
				SuperZones[0].Zones[SuperZones[0].NumberOfZones] = z;
				SuperZones[0].NumberOfZones++;
			}
			else
			{
#ifdef DEBUG
				printf("\n FATAL Error. InnerOuter table not initialized properly at index : %d\n", z);
				printf(" , and the actual value is : %d\n", InnerOuterZones[z]);
#endif
			}
		}
	}
#ifdef DEBUG
	printf("Super Zones configured for the processing - Forcefully !\n");
	printf("\nInner Zones\n");
	for (z = 0; z < SuperZones[1].NumberOfZones; z++)
	{
		printf("%d ", SuperZones[1].Zones[z]);
	}
	printf("\nOuter Zones\n");
	for (z = 0; z < SuperZones[0].NumberOfZones; z++)
	{
		printf("%d ", SuperZones[0].Zones[z]);
	}
#endif
}

/*******************************************************************************
*@fn     PathZoneInit

*@brief   Read  the zone status NEDDED, DISCARD or OPTIONAL

*@param   None

*@return  None
*/
void minDistanceInit()
{
	minDistance[0] = mgos_sys_config_get_mindistance_index0();
	minDistance[1] = mgos_sys_config_get_mindistance_index1();
	minDistance[2] = mgos_sys_config_get_mindistance_index2();
	minDistance[3] = mgos_sys_config_get_mindistance_index3();

	minDistance[4] = mgos_sys_config_get_mindistance_index4();
	minDistance[5] = mgos_sys_config_get_mindistance_index5();
	minDistance[6] = mgos_sys_config_get_mindistance_index6();
	minDistance[7] = mgos_sys_config_get_mindistance_index7();

	minDistance[8] = mgos_sys_config_get_mindistance_index8();
	minDistance[9] = mgos_sys_config_get_mindistance_index9();
	minDistance[10] = mgos_sys_config_get_mindistance_index10();
	minDistance[11] = mgos_sys_config_get_mindistance_index11();

	minDistance[12] = mgos_sys_config_get_mindistance_index12();
	minDistance[13] = mgos_sys_config_get_mindistance_index13();
	minDistance[14] = mgos_sys_config_get_mindistance_index14();
	minDistance[15] = mgos_sys_config_get_mindistance_index15();
}
/*******************************************************************************
*@fn     getDistanceJsonPacket

*@brief   Convert array data into Json packet

*@param 1 DistancePtr

*@param 2 size

*@return  none
*/
void getDistanceJsonPacket(int16_t DistancePtr[], uint8_t size)
{
	strcpy(jsondata, "\0");
	strcpy(arrayofdistance, "\0");
	uint8_t i = 0;
	for (i = 0; i < size; i++)
	{
		if (i == 0)
		{
			sprintf(arrayofdistance, "[%d,", DistancePtr[i]);
			strcat(jsondata, arrayofdistance);
		}
		else if (i < size - 1)
		{
			sprintf(arrayofdistance, "%d,", DistancePtr[i]);
			strcat(jsondata, arrayofdistance);
		}
		else
		{
			sprintf(arrayofdistance, "%d]", DistancePtr[i]);
			strcat(jsondata, arrayofdistance);
		}
	}
}

void getpixelcount(VL53L8CX_ResultsData *pRangeResults, uint8_t *pPathZones)
{
	uint16_t newFrame[16];
	uint8_t target_status_ok, target_status;
	int16_t distance, threshold, min_distance;
	uint32_t signal;
	uint8_t count = 0;
	for (uint8_t z = 0; z < NB_ZONES; z++)
	{
		if (pRangeResults->nb_target_detected[z] > 0)
		{

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

			if (pPathZones[z] == NEEDED)
			{
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
					(distance < threshold) && signal > minSignal)
				{
					newFrame[z] = distance;
					count++;
				}
				else
				{
					newFrame[z] = 0;
				}
			}
			else
			{

				newFrame[z] = 0;
			}
		}
	}
	if ((count < superDirection) && (previouslyOccupied))
	{
		previouslyOccupied = 0;
	}
	else if ((count >= superDirection) && (previouslyOccupied == 0))
	{
		occupancy = 1;
		previouslyOccupied = 1;
	}
	if (previouslyOccupied)
	{
		dwpcData.distanceThreshold += occupancy;
		// printf("Count:%d,occupancy:%d\n",count,occupancy);
		// printf("dwpcData.distanceThreshold:%d\n",dwpcData.distanceThreshold);
		occupancy = 0;
	}
}
/*******************************************************************************
 * @fn      gpio_init
 *
 * @brief   defines the GPIO pins and register the ISR and ISR callback function
 *
 * @param   None.
 *
 * @return  None.
 */
void gpio_init()
{
	/* Buzzer initialization as output */;
	mgos_gpio_setup_input(STATIC_IP_RST_BUTTON, MGOS_GPIO_PULL_UP);

	mgos_gpio_enable_int(STATIC_IP_RST_BUTTON);

	mgos_gpio_set_int_handler(STATIC_IP_RST_BUTTON, MGOS_GPIO_INT_EDGE_NEG, controller_static_ip_reset, NULL);
}

/*******************************************************************************
@fn     controller_static_ip_reset

@brief   The function performs reset static ip of the controller

@param   none

@return  None
*/
void controller_static_ip_reset()
{
	static uint8_t button_value;
	button_value += 1;
#ifndef DEBUG
	printf("Reset Button Value: %d\n", button_value);
#endif
	if (button_value == 5)
	{
		button_value = 0;
		mgos_config_reset(MGOS_CONFIG_LEVEL_USER);
#ifndef DEBUG
		printf("Static ip was Reset successfully!!");
#endif
	}
}
