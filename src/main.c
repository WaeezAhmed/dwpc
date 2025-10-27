/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"
#include "mgos_http_server.h"
#include "mgos_pwm.h"
#include "mgos_mqtt.h"
#include "mgos_wifi.h"
#include "mgos_rpc.h"
#include "mgos_ro_vars.h"
#include "mgos_sys_config.h"
#include "mgos_uart.h"
#include "mgos_cron.h"

#include "PCA9543A.h"
#include "task.h"
#include "FreeRTOS.h"
#include "KTD202X.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vl53l8cx_api.h"
#include "vl53l8cx_plugin_motion_indicator.h"
#include "vl53l8cx_plugin_xtalk.h"
#include "vl53l8cx_platform.h"

#include "uart_data.h"
#include "websocket.h"
#include "types.h"
#include "network.h"

#include "ppcl5.h"
#include "rpc_callbacks.h"

#include "ppcl5_globals.h"
#include "buzzer.h"

#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"

#include "driver/i2c.h"
#include "mgos_vfs.h"

/***DEVICE FUNCTIONMODE ***/
#define DWPC_SD 1
#define DWPC_DD 2
#define DWPC_DIAGNOSTIC 3

/**** DEVICE DATA SPECIFIC */
#define WHITE 0
#define RED 1
#define BUZZER_ON 1
#define BUZZER_OFF 0
#define DEVICE_VERSION 1
#define AGGREGATION_COUNT_EVENT 2
#define PERIODIC_COUNT_RESET_EVENT 4
#define PERIODIC_RESET_MIN 10
#define PERIODIC_RESET_MAX 180

#define PARALLELDETECTIONCOUNTTHRESHOLD 2

/*Buzzer pin*/
#define BUZZER 14
#define BUTTON 0

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t dataReady;
uint8_t xtalk_data[VL53L8CX_XTALK_BUFFER_SIZE];
uint8_t packet_counter_telemetry = 0;
uint8_t packet_counter_health = 0;
uint16_t algorithm_uptime_hours = 0;
uint16_t device_uptime_days = 0;
uint8_t sensor_type_dwpc = 30;
uint8_t event_type_telemetry = 10;
uint8_t event_type_device_health = 20;
uint8_t function_mode_periodic = 10;
uint8_t device_type_legacy_dwpc = 32;
uint8_t sensor_status_tof = 0;


/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t white[] = {255, 255, 255};
static uint8_t red[] = {255, 0, 0};
static uint8_t green[] = {0, 255, 0};
static uint8_t defaultColour[] = {0x34, 0xEF, 0xAB};
static uint8_t blue[] = {0, 0, 255};

static int i2c_master_port = 0;

int8_t PrevPeopleCount = 0;
static uint8_t CalibrationCompleted = 0;

/*Buzzer variables*/
static uint16_t buzzer_frequency = 5000;
static uint8_t buzzer_delay = 100;
static uint16_t buzzer_off_time = 5000;

// Wi-Fi Connection status check counter 
uint8_t wifi_connection_status_counter = 0;

/*********************************************************************
 * VL53L5CX LOCAL VARIABLES
 */
static VL53L8CX_Configuration Dev;
static VL53L8CX_ResultsData RangeResults;
static VL53L8CX_Motion_Configuration motion;
static uint8_t resolution, isAlive, status;
int events = 0;

nvs_handle_t nvsHandle;
esp_err_t err;

/*********************************************************************
 * VL53L5CX LOCAL FUNCTIONS
 */
uint8_t vl53l8x_init_ppcl5();
uint8_t error_wrapper(char *message, uint8_t error_code);
static void NV_write_wrapper();
static void NV_read_wrapper();
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void aggregated_data(void *arg);
static void periodicResetCount(void *arg);
static void PeopleCountingData();
int get_flash_utilization_percentage();
void cronJob(void *user_data, mgos_cron_id_t cron_id);
/******* Timer variables *******/
mgos_timer_id periodicResetCountID = MGOS_INVALID_TIMER_ID;
mgos_timer_id wifiChangeTimerID = MGOS_INVALID_TIMER_ID;

/******* MAC address variables *******/
const char *mac_string;
/*TIme variable*/
char time_bff[80];

/*LED and Buzzer variables*/
char *ledColour = "white";
char *buzzState = "OFF";
/******* Task functions *******/

void ppl_count_task(void *arg);
// ######################### Added by Jyoti Bhartiya ##########################
char modified_mac[18];
char *base_mac = NULL;
void incrementLastOctet(const char *macAddress, int num, char *result)
{
	uint64_t macInt = strtoull(macAddress, NULL, 16);
	macInt += num;
	sprintf(result, "%012llX", macInt);
	printf("Modified MAC : %s\n", result);
}
enum mgos_app_init_result mgos_app_init(void)
{
	// Added by Jyoti
	const char *macl = mgos_sys_ro_vars_get_mac_address();
	size_t mac_length = strlen(macl);
	base_mac = (char *)malloc(mac_length + 1); // declare base_mac
	if (base_mac == NULL)
	{
		printf("Error: Memory allocation failed\n");
		return MGOS_APP_INIT_ERROR;
	}
	/* Read mac address of the device */
	bool is_wifi_enabled = mgos_sys_config_get_wifi_sta_enable();
	printf("is_wifi_enabled ######################## %d", is_wifi_enabled);
	if (is_wifi_enabled)
	{
		// WIFI is enabled, fetch WIFI MAC address
		mac_string = mgos_sys_ro_vars_get_mac_address();
		strncpy(base_mac, mac_string, mac_length + 1);
		base_mac[mac_length] = '\0';
		printf("inside wifi condition mac_string%s\n", mac_string);
		printf("inside wifi condition base_mac%s\n", base_mac);
	}
	else
	{
		// fetch ethernet MAC address
		//printf("######################################################");
		mac_string = mgos_sys_ro_vars_get_mac_address();
		printf("default mac inside else condition %s\n", mac_string);
		// Add 3 to the last octet of the Ethernet MAC address
		incrementLastOctet(mac_string, 3, modified_mac);
		printf("modified mac inside ethernet %s\n", modified_mac + 1);
		strncpy(base_mac, modified_mac, mac_length);
		base_mac[mac_length] = '\0';
		//printf("base mac_string ***********%s\n", base_mac);
	}
	//############### ADDED BY JYOTI ###############################

	/*Clock streching*/
	// i2c_set_timeout(i2c_master_port, 12500 * 10);
	mgos_msleep(2000);
	uint8_t vlStatus;

	/*Initialize ESP32 NVS*/
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	/*Zone initialization Nedded/Discard*/
	PathZoneInit();

	loadConfigurablefileds();

	i2cScanner();

	/* Initilization LED blink */
	initLedDriver();
	/*Checking the sensor mode for VL53L5x */
	vlStatus = vl53l8x_init_ppcl5();
	if (vlStatus == EXIT_SUCCESS)
	{
		breathLeds(green);
		mgos_msleep(5000);
#ifdef DEBUG
		printf("VL53L8 Sensor detected at address 0x29 \n");
#endif
	}

	/*Set Led colur to default*/
	breathLeds(defaultColour);

	buzzer_init(BUZZER);
	/* Buzzer functionality configuration check */
	buzzer_buzz(buzzer_frequency, buzzer_delay, 3000);

	/* Variable to hold return status of created FREERTOS task*/

	if (strcmp(mgos_sys_config_get_admin_variant(), "ZIGBEE") == 0)
	{
		if (uartInit())
		{
			mgos_uart_set_dispatcher(UART_INTERFACE_ZERO, uart_dispatcher_cb, NULL);
			mgos_uart_flush(UART_INTERFACE_ZERO);
		}
	}
	/* Initilization web socket connection */
	web_sockets_connection();

	/*Init RPC callbacks*/
	rpcCallbacksInit();

	/*Define inner/outer zones*/
	InnerOuterZoneInit();

	/*Get distance threshold based on Prev_calibration Data*/
	getDistanceThreshold();
	/*Get superzones*/
	getSuperZones();

	/* aggragation data interval*/

	mgos_set_timer(mgos_sys_config_get_dwpc_interval() * 1000, MGOS_TIMER_REPEAT, aggregated_data, NULL);

	/*Periodic reset timer*/
	if ((mgos_sys_config_get_dwpc_periodic_reset() >= PERIODIC_RESET_MIN) && (mgos_sys_config_get_dwpc_periodic_reset() <= PERIODIC_RESET_MAX))
	{
		/*Periodic Reset timer*/
		mgos_set_timer(((mgos_sys_config_get_dwpc_periodic_reset() * 60) * 1000), MGOS_TIMER_REPEAT, periodicResetCount, NULL);
	}
	else
	{
		/*setup the cronjob to run at 3am everyday */
		mgos_cron_add("* 0 3 * * *", cronJob, NULL);
	}

	mgos_sys_config_set_service_thresholdCalibration_status("Not started");

	mgos_sys_config_set_service_xtalkcalibration_status("Not started");

	mgos_sys_config_set_service_livestream_toggle_status("Not started");

	/*Create a task for People count MAIN TASK*/
	xTaskCreate(ppl_count_task, "Peoplecounttask", 2600, NULL, 3, &xHandleTask1);

	mgos_msleep(1000);
	return MGOS_APP_INIT_SUCCESS;
}

/*******************************************************************************
 * @fn      ppl_count_task
 *
 * @brief   initialize vl53l5cx and runs the people counting algorithm
 *
 * @param   arg -> unused.
 *
 * @return  None.
 */
void ppl_count_task(void *arg)
{
	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
	status = vl53l8cx_start_ranging(&Dev);
	sensor_status_tof = status;
	mgos_msleep(1000);
#ifdef DEBUG
	printf("\n In people count begin - start Ranging status %u\n", status);
#endif

	for (;;)
	{
		status = ppcl5_get_data(&Dev, &RangeResults, GET_DATA_TIME_OUT_MS);
		sensor_status_tof = status;
		if (status)
		{
			if (debugEnable)
				send_message_to_webui("Error in getting ranging data", status);
#ifdef DEBUG
			printf("Error in getting ranging data %u\n", status);
#endif
		}
		if (liveStream != 1)
		{
			getpixelcount(&RangeResults, PathZones);
			events = ppcl5_process_results(&RangeResults, PathZones);
			if (events != 0)
			{
				if (event_direction == DEFAULT_DIRECTION)
				{

					if (events & 0xF) // 0x1
					{
#ifdef DEBUG
						printf("Entry detected!");
#endif
						events -= event_one_way;
						if (enable_two_person_count)
						{

							if (inParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD && outParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD)
							{
								dwpcData.inCount += 2;
								incountAggregation += 2;
								dwpcData.PeopleCount += 2;
							}
							else
							{

								dwpcData.inCount += 1;
								incountAggregation += 1;
								dwpcData.PeopleCount += 1;
							}
							inParallelDetectioncount = 0;
							outParallelDetectioncount = 0;
							// innerCount =0;
						}
						else
						{

							dwpcData.inCount += 1;
							incountAggregation += 1;
							dwpcData.PeopleCount += 1;
						}
					}
					if (events & 0xF0) // 0x16
					{
#ifdef DEBUG
						printf("Exit detected!");
#endif
						events -= event_other_way;
						if (enable_two_person_count)
						{
							if (inParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD && outParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD)
							{
								dwpcData.outCount += 2;
								outcountAggregation += 2;
								dwpcData.PeopleCount -= 2;
							}
							else
							{

								dwpcData.outCount += 1;
								outcountAggregation += 1;
								dwpcData.PeopleCount -= 1;
							}
							inParallelDetectioncount = 0;
							outParallelDetectioncount = 0;
							// outerCount =0;
						}
						else
						{

							dwpcData.outCount += 1;
							outcountAggregation += 1;
							dwpcData.PeopleCount -= 1;
						}
					}
				}
				/*If sensor placed wrong direction*/ // Added by sharath
				else
				{
					if (events & 0xF0)
					{
#ifdef DEBUG
						printf("Exit detected!");
#endif
						events -= event_one_way;
						if (enable_two_person_count)
						{
							if (inParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD && outParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD)
							{
								dwpcData.outCount += 2;
								outcountAggregation += 2;
								dwpcData.PeopleCount -= 2;
							}
							else
							{

								dwpcData.outCount += 1;
								outcountAggregation += 1;
								dwpcData.PeopleCount -= 1;
							}
							inParallelDetectioncount = 0;
							outParallelDetectioncount = 0;
							// outerCount =0;
						}
						else
						{

							dwpcData.outCount += 1;
							outcountAggregation += 1;
							dwpcData.PeopleCount -= 1;
						}
					}
					if (events & 0xF)
					{
#ifdef DEBUG
						printf("Entry detected!");
#endif
						events -= event_other_way;
						if (enable_two_person_count)
						{

							if (inParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD && outParallelDetectioncount >= PARALLELDETECTIONCOUNTTHRESHOLD)
							{
								dwpcData.inCount += 2;
								incountAggregation += 2;
								dwpcData.PeopleCount += 2;
							}
							else
							{

								dwpcData.inCount += 1;
								incountAggregation += 1;
								dwpcData.PeopleCount += 1;
							}
							inParallelDetectioncount = 0;
							outParallelDetectioncount = 0;
							// innerCount =0;
						}
						else
						{

							dwpcData.inCount += 1;
							incountAggregation += 1;
							dwpcData.PeopleCount += 1;
						}
					}
				} // Added by sharath
			}
			if (PrevPeopleCount != dwpcData.PeopleCount)
			{
				broadcast();
				PeopleCountingData();
				PrevPeopleCount = dwpcData.PeopleCount;
			}
		}
		else
		{
			mgos_msleep(100);
			getDistanceJsonPacket(RangeResults.distance_mm, NB_ZONES);
			broadcast_array();
		}
		if (CalibrationCompleted)
		{
			getDistanceThreshold();
			InnerOuterZoneInit();

			status = vl53l8cx_start_ranging(&Dev);
			sensor_status_tof = status;
			if (debugEnable)
				send_message_to_webui("In people count loop - Resume Ranging status", status);
#ifdef DEBUG
			printf("\n In people count loop - Resume Ranging status  %u\n", status);
#endif
			mgos_msleep(1000);

			CalibrationCompleted = 0;
		}
		vTaskDelay(xDelay);
	}
	(void)arg;
}

/*******************************************************************************
 * @fn      cross_talk_task
 *
 * @brief   run the xtalk calibration
 *
 * @param   arg -> unused.
 *
 * @return  None.
 */

void cross_talk_task(void *arg)
{
	vTaskSuspend(xHandleTask1);
	mgos_msleep(10000);

	status = vl53l8cx_stop_ranging(&Dev);
	sensor_status_tof = status;
	if (debugEnable)
		send_message_to_webui("In xtalk begin - stop Ranging status", status);
#ifdef DEBUG
	printf("\n In xtalk begin - stop Ranging status %d\n", status);
#endif
	mgos_msleep(5000);

	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
	/*   Xtalk calibration	   */
	/*********************************/
	/* Start Xtalk calibration with a 50% reflective target at 1000mm for the
	 * sensor, using 8 samples. Detect a too good coverglass checking the
	 * error VL53L5CX_ERROR_CG_TOO_GOOD.
	 */
	for (;;)
	{
		mgos_sys_config_set_service_xtalkcalibration_status("In Progress");

		status = vl53l8cx_calibrate_xtalk(&Dev, reflectance_percent, nb_samples, distance_mm);
		// status = vl53l5cx_calibrate_xtalk(&Dev, 50, 8, 1000);
		if (status)
		{
			if (debugEnable)
				send_message_to_webui("vl53l8cx_calibrate_xtalk failed", status);
#ifdef DEBUG
			printf("vl53l8cx_calibrate_xtalk failed:%d\n", status);
#endif
			mgos_sys_config_set_service_xtalkcalibration_status("Failed");
		}
		else
		{

			/* Get Xtalk calibration data, in order to use them later */
			status = vl53l8cx_get_caldata_xtalk(&Dev, xtalk_data);

			/* Set Xtalk calibration data */
			status = vl53l8cx_set_caldata_xtalk(&Dev, xtalk_data);
			if (debugEnable)
				send_message_to_webui("vl53l8cx_calibrate_xtalk done", status);
#ifdef DEBUG
			printf("\nvl53l8cx_calibrate_xtalk done\n");
#endif
			mgos_sys_config_set_service_xtalkcalibration_status("Done");
			/*Writing data into ESP NV*/
			NV_write_wrapper();
			mgos_msleep(1000);
			/*Clear the buffer*/
			memset(xtalk_data, 0, VL53L8CX_XTALK_BUFFER_SIZE);
		}
		vTaskDelay(xDelay);
		break;
	}
	CalibrationCompleted = 1;
	vTaskResume(xHandleTask1);
	mgos_msleep(1000);
	vTaskDelete(xHandleTask3);
	(void)arg;
}

/*******************************************************************************
 * @fn      calibration_task
 *
 * @brief  runs ppcl5_calibration and get distance thresholds
 *
 * @param   arg -> unused.
 *
 * @return  None.
 */
void calibration_task(void *arg)
{
	vTaskSuspend(xHandleTask1);
	mgos_msleep(10000);

	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
	for (;;)
	{
		mgos_sys_config_set_service_thresholdCalibration_status("In Progress");
		if (debugEnable)
			send_message_to_webui("Looking at what Distance the floor is ...", 0);
#ifdef DEBUG
		printf("Looking at what Distance the floor is ...\n");
#endif
		status = ppcl5_calibrate_max_min(&Dev, &RangeResults, PathZones);
		sensor_status_tof = status;
		if (status)
		{
			if (debugEnable)
				send_message_to_webui("Error in ppcl5_calibrate_max_min", status);

#ifdef DEBUG
			printf("Error in ppcl5_calibrate_max_min %u\n", status);
#endif
		}
		status = ppcl5_calibrate_threshold(PathZones, InnerOuterZones);
		sensor_status_tof = status;
		if (status)
		{
			if (debugEnable)
				send_message_to_webui("Error in ppcl5_calibrate_threshold ", status);
#ifdef DEBUG
			printf("Error in ppcl5_calibrate_threshold %u\n", status);
#endif
		}
		else
		{
			mgos_sys_config_set_service_thresholdCalibration_status("Done");
			setDistanceThreshold(DistanceThresholdOf);
			breathLeds(blue);
			mgos_msleep(5000);
			breathLeds(defaultColour);
			if (debugEnable)
				send_message_to_webui("Start Counting", 0);
#ifdef DEBUG
			printf("Start Counting\n");
#endif
		}
		vTaskDelay(xDelay);
		break;
	}

	status = vl53l8cx_stop_ranging(&Dev);
	sensor_status_tof = status;
	if (debugEnable)
		send_message_to_webui("In calibration end - stop Ranging status", status);
#ifdef DEBUG
	printf("\n In calibration end - stop Ranging status %u\n", status);
#endif
	mgos_msleep(10000);
	CalibrationCompleted = 1;
	vTaskResume(xHandleTask1);
	mgos_msleep(1000);

	vTaskDelete(xHandleTask2);
	(void)arg;
}

uint8_t error_wrapper(char *message, uint8_t error_code)
{
	printf(message);
	return error_code;
}
/*******************************************************************************
 * @fn      aggregated_data
 *
 * @brief   The Data from the device is periodically published
 *
 * @param   arg -> unused.
 *
 * @return  None.
 */
static void aggregated_data(void *arg)
{
	dwpcData.eventType = AGGREGATION_COUNT_EVENT;
	maintime = time(0);
	epochTime = (uint64_t)time(&maintime);
	epochTime = epochTime * 1000;

	if (mgos_mqtt_global_is_connected())
	{
		mgos_sys_config_set_mqtt_status("Connected");

		if (dwpcData.inCount > 0 || dwpcData.outCount > 0)
		{
		packet_counter_telemetry++;
		algorithm_uptime_hours = mgos_uptime() / 3600.0;
        //mgos_mqtt_pubf(mgos_sys_config_get_mqtt_pub(), 0, false, "{DeviceVersion:\"%s\",Data:AggregationCount,Time: %llu, Incount: %d, Outcount: %d, Absolute:%d, PktId:32, DeviceType:32, MAC:%Q, BuzzerState:%Q, LedColour:%Q}", mgos_sys_config_get_admin_version(), epochTime, dwpcData.inCount, dwpcData.outCount, dwpcData.PeopleCount, base_mac, buzzState, ledColour);
        mgos_mqtt_pubf(mgos_sys_config_get_mqtt_pub(), 0, false, "{DeviceVersion: \"%s\", DeviceType: %d, SensorType: %d, EventType: %d, TimeStamp: %llu, Time:%llu, FunctionMode: %d, PacketCounterTelemetry: %d, AlgorithmUptimeHours: %d, ResidualCount: %d, Incount: %d, Outcount: %d, MAC:%Q, LedColour:%Q, BuzzerState:%Q}", mgos_sys_config_get_admin_version(), device_type_legacy_dwpc, sensor_type_dwpc, event_type_telemetry, epochTime, epochTime, function_mode_periodic, packet_counter_telemetry, algorithm_uptime_hours, dwpcData.PeopleCount, dwpcData.inCount, dwpcData.outCount, base_mac, ledColour, buzzState);
	}
	}
	else
	{
		packet_counter_telemetry++;
		mgos_mqtt_global_connect(); // This function will force immediate connection attempt if disconnected from broker
		mgos_sys_config_set_mqtt_status("Not Connected");
	}
	if (debugEnable)
	{
		send_message_to_webui("entered one side and exited same side", dwpcData.sensorFrequency);
		send_message_to_webui("Really unexpected Path", dwpcData.functionMode);
		send_message_to_webui("Occupancy Count:", dwpcData.distanceThreshold);
	}
	if (strcmp(mgos_sys_config_get_admin_variant(), "ZIGBEE") == 0)
	{

		if (dwpcData.inCount > 0 || dwpcData.outCount > 0)
		{
			if (mgos_uart_write_avail(UART_INTERFACE_ZERO))
			{
				append_crc_to_data(&dwpcData);
				mgos_uart_write(UART_INTERFACE_ZERO, &dwpcData, sizeof(dwpc_data));
				mgos_uart_flush(UART_INTERFACE_ZERO);
			}
			dwpcData.distanceThreshold = 0;
		}
	}

	dwpcData.functionMode = 0;
	dwpcData.sensorFrequency = 0;

	dwpcData.inCount = 0;
	dwpcData.outCount = 0;
#ifdef DEBUG
	printf("Aggregated data...!!\n");
#endif
}
/*******************************************************************************
*@fn     periodicResetCount
*@brief   Resets the inCount, outCount, AbslouteCount values based on the timer interval set
*@param   None
*@return  None
*/
static void periodicResetCount(void *arg)
{
	dwpcData.eventType = PERIODIC_COUNT_RESET_EVENT;
	dwpcData.buzzerState = BUZZER_OFF;
	dwpcData.ledColour = WHITE;

	bool is_wifi_enabled = mgos_sys_config_get_wifi_sta_enable();
	if (is_wifi_enabled)
	{	
		printf("Wi-Fi is enabled , plz check for wifi connection staus");
		int wifi_connection_status = mgos_wifi_get_status();
		if (wifi_connection_status == 3)
		{
		printf("\n wifi connection status : MGOS_WIFI_EV_STA_CONNECTED");
		wifi_connection_status_counter = 0;
		}
		else 
		{
		printf("\n wifi connection status : %d",wifi_connection_status);
		mgos_wifi_connect();
		wifi_connection_status_counter++;

		}
		printf("\n wifi_connection_status_counter: %d", wifi_connection_status_counter); 
		if (wifi_connection_status_counter > 2)
		{
		printf("\n mgos_system_restart "); 
		mgos_system_restart();
		}
	}
	else 
	{
	printf("Wi-Fi is disabled,so no need to check wifi connection staus");
	}


	if (mgos_mqtt_global_is_connected())
	{
		mgos_sys_config_set_mqtt_status("Connected");

		maintime = time(0);
		epochTime = (uint64_t)time(&maintime);
		epochTime = epochTime * 1000;
		device_uptime_days = mgos_uptime() / 86400.0;
		packet_counter_health++;
		int heap_utilization = mgos_get_heap_size() - mgos_get_free_heap_size();
		int max_heap_utilization = mgos_get_min_free_heap_size();
		uint8_t flash_utilization = get_flash_utilization_percentage();
		//mgos_mqtt_pubf(mgos_sys_config_get_mqtt_device_health(), 0, false, "{DeviceVersion:\"%s\",Data:PeriodicReset,Time: %llu, Incount: %d, Outcount: %d, Absolute:%d, PktId:32, DeviceType:32, MAC:%Q, BuzzerState:%Q, LedColour:%Q}", mgos_sys_config_get_admin_version(), epochTime, dwpcData.inCount, dwpcData.outCount, dwpcData.PeopleCount, base_mac, buzzState, ledColour);
        mgos_mqtt_pubf(mgos_sys_config_get_mqtt_device_health(), 0, false, "{DeviceVersion: \"%s\",DeviceType: %d, SensorType: %d, EventType: %d, TimeStamp: %llu, Time: %llu, FunctionMode: %d, PacketCounterHealth: %d, UptimeDays: %d, SensorStatus: %d, HeapUtilization: %d, MaxHeapUtilization: %d, FilesystemUtilization: %d, MAC: %Q}", mgos_sys_config_get_admin_version(), device_type_legacy_dwpc, sensor_type_dwpc, event_type_device_health, epochTime, epochTime, function_mode_periodic, packet_counter_health, device_uptime_days, sensor_status_tof, heap_utilization, max_heap_utilization, flash_utilization, base_mac);
	}
	else
	{
		packet_counter_health++;
		mgos_mqtt_global_connect(); // This function will force immediate connection attempt if disconnected from broker
		mgos_sys_config_set_mqtt_status("Not Connected");
	}
	if (dwpcData.PeopleCount > 0) // More In detected
	{
		dwpcData.onepersoncountThreshold = dwpcData.PeopleCount; // Distance1 Outcount
	}
	else if (dwpcData.PeopleCount < 0) // More Out detected
	{
		dwpcData.twopersoncountThreshold = abs(dwpcData.PeopleCount); // Distance2 Incount
	}

	if (debugEnable)
	{

		send_message_to_webui("dwpcData.onepersoncountThreshold", dwpcData.onepersoncountThreshold);
		send_message_to_webui("dwpcData.twopersoncountThreshold", dwpcData.twopersoncountThreshold);
	}

	if (strcmp(mgos_sys_config_get_admin_variant(), "ZIGBEE") == 0)
	{

		if (mgos_uart_write_avail(UART_INTERFACE_ZERO))
		{
			append_crc_to_data(&dwpcData);
			mgos_uart_write(UART_INTERFACE_ZERO, &dwpcData, sizeof(dwpc_data));
			mgos_uart_flush(UART_INTERFACE_ZERO);
		}
	}
	broadcast();
	/*Turon off APmode */
	turnOffApMode();
	PrevPeopleCount = 0;
	dwpcData.inCount = 0;
	dwpcData.outCount = 0;
	dwpcData.PeopleCount = 0;
	incountAggregation = 0;
	outcountAggregation = 0;
	dwpcData.onepersoncountThreshold = 0;
	dwpcData.twopersoncountThreshold = 0;
	dwpcData.functionMode = 0;
	dwpcData.sensorFrequency = 0;

#ifdef DEBUG
	printf("Count is cleared by periodic reset...!!\n");
#endif
}

void cronJob(void *user_data, mgos_cron_id_t cron_id)
{
	periodicResetCount(NULL);
}

/*******************************************************************************
*@fn     PeopleCountingData

*@brief   cheking for capacity in DWPC_SD Mode

*@param   None

*@return  None
*/
static void PeopleCountingData()
{
	if (room_capacity > 0)
	{
		// check if PeopleCount > capacity
		if (dwpcData.PeopleCount > room_capacity)
		{
			buzzer_buzz(buzzer_frequency, buzzer_delay, buzzer_off_time);
			// dwpcData.buzzerState = BUZZER_ON;
		}
		else if (dwpcData.PeopleCount == room_capacity)
		{

			/* check if led is already blinking in red or set to red and blink */
			buzzer_buzz(0, 0, 0);
			breathLeds(red);
			ledColour = "RED";
			buzzState = "OFF";
			// dwpcData.ledColour = RED;
			// dwpcData.buzzerState = BUZZER_OFF;
		}
		else
		{
			buzzer_buzz(0, 0, 0);
			// stop blinking
			breathLeds(white);
			ledColour = "WHITE";
			buzzState = "OFF";
			// dwpcData.ledColour = WHITE;
			// dwpcData.buzzerState = BUZZER_OFF;
		}
	}
#ifdef DEBUG
	printf("\nincount:%d,outcount:%d,Peoplecount: %d\n", dwpcData.inCount, dwpcData.outCount, dwpcData.PeopleCount);
#endif
}

/*******************************************************************************
*@fn     vl53l8x_init_ppcl5
*@brief   initializing all necessary functions
*@param   None
*@return  uint8_t
*/
uint8_t vl53l8x_init_ppcl5()
{
	status = VL53L8CX_STATUS_OK;
	Dev.platform.address = VL53L8CX_DEFAULT_I2C_ADDRESS;
	/* (Optional) Check if there is a VL53L5CX sensor connected */
	status = vl53l8cx_is_alive(&Dev, &isAlive);
	if (!isAlive || status)
	{
		return error_wrapper("VL53L8CX not detected at requested address:%d\n", status);
	}

	/* (Mandatory) Init VL53L5CX sensor */
	status = vl53l8cx_init(&Dev);
	if (status)
	{
		return error_wrapper("VL53L8CX ULD Loading failed:%d\n", status);
	}
#ifdef DEBUG
	printf("VL53L8CX ULD ready ! (Version : %s)\n", VL53L8CX_API_REVISION);
#endif
/* disable charge pump : option to reduce power consumption only if ADD=3.3V */
#ifdef DEBUG
	printf("vl53l8cx_disable_internal_cp ....\n");
#endif
	/*status = vl53l8cx_disable_internal_cp(&Dev);
	if (status)
	{
		return error_wrapper("vl53l8cx_disable_internal_cp failed\n", status);
	}
#ifdef DEBUG
	printf("vl53l8cx_disable_internal_cp done\n");
#endif*/
	NV_read_wrapper();
	/* Set Xtalk calibration data */
	status = vl53l8cx_set_caldata_xtalk(&Dev, xtalk_data);
	if (status)
	{
		return error_wrapper("Error in setting Xtalk Data\n", status);
	}
	status = ppcl5_init();
	if (status)
	{
		return error_wrapper("Error in initialization of VL53L8 people counting\n", status);
	}
	status = ppcl5_config(&Dev);
	if (status)
	{
		return error_wrapper("Error in configuring VL53L8\n", status);
	}
	return status;
}

static void NV_write_wrapper()
{
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
	if (err != ESP_OK)
	{
#ifdef DEBUG
		printf("Failed to OPEN ESP32 NV:%d\n", err);
#endif
		if (debugEnable)
		{
			send_message_to_webui("Failed to OPEN ESP32 NV", err);
			return;
		}
	}
	err = nvs_set_blob(nvsHandle, "XtalkData", xtalk_data, VL53L8CX_XTALK_BUFFER_SIZE);
	if (err != ESP_OK)
	{
#ifdef DEBUG
		printf("Failed to set data into ESP32 NV:%d\n", err);
#endif
		if (debugEnable)
		{
			send_message_to_webui("Failed to set data into ESP32 NV", err);
			return;
		}
	}
	err = nvs_commit(nvsHandle);
	if (err != ESP_OK)
	{
#ifdef DEBUG
		printf("Failed to save into ESP32 NV:%d\n", err);
#endif
		if (debugEnable)
		{
			send_message_to_webui("Failed to save into ESP32 NV", err);
			return;
		}
	}
}

static void NV_read_wrapper()
{
	size_t required_size = 0;
	// Open
	err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
	if (err != ESP_OK)
	{
#ifdef DEBUG
		printf("Failed to open NV:%d\n", err);
#endif
	}
	err = nvs_get_blob(nvsHandle, "XtalkData", NULL, &required_size);

	if (required_size == 0)
	{
		// printf("Nothing Saved Yet\n");
		breathLeds(blue);
		return;
	}
	else
	{
		err = nvs_get_blob(nvsHandle, "XtalkData", xtalk_data, &required_size);
		if (err != ESP_OK)
		{
#ifdef DEBUG
			printf("Failed to read:%d\n", err);
#endif
			return;
		}
	}
}


int get_flash_utilization_percentage() {
    size_t total = mgos_vfs_get_space_total("/");
    size_t free = mgos_vfs_get_space_free("/");

    if (total > 0) {
        size_t used = total - free; // Calculate used space
        return (used * 100) / total;  // Return integer percentage
    }
    return -1; // Return -1 if retrieval fails
}