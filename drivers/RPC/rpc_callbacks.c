
#include "rpc_callbacks.h"
#include <stdio.h>
#include "mgos_system.h"
#include <stdint.h>
#include "websocket.h"

char *data = "{result:success}";
/******* MAC address variables *******/
const char *mac_string;

char *health = "{\"time\":%llu,\"heapSize\":%d,\"Freeheap_Size\":%d,\"min_Freeheap_Size\":%d,\"Fs_size\":%d,\"free_fs_size\":%d,\"cpu_freq\":%d,\"uptime\":%f,\"sdkversion\":\"%s\",\"MAC\":\"%s\",\"AP_MODE_MAC\":\"%s\",\"ETHERNET_MAC\":\"%s\"}";
void rpcCallbacksInit()
{
	/* Registering RPC callback for resetting the sensor count*/
	mg_rpc_add_handler(mgos_rpc_get_global(), "thresholdCalibration", "{}", thresholdCalibration, NULL);
	// /* Registering RPC callback for rebooting the sensor */
	mg_rpc_add_handler(mgos_rpc_get_global(), "rebootSensor", "{}", rebootSensor, NULL);
	// /* Registering RPC callback for sensor calibration */
	mg_rpc_add_handler(mgos_rpc_get_global(), "xtalkCalibration", "{}", xtalkcalibration, NULL);
	/* Registering RPC callback for sensor Livestream */
	mg_rpc_add_handler(mgos_rpc_get_global(), "liveStream", "{}", liveStreamData, NULL);
	/* Registering RPC callback for getting system info*/
	mg_rpc_add_handler(mgos_rpc_get_global(), "sysGetInfo", "{}", getSysInfo, NULL);
	/*Registering RPC Enable/Disable debug console*/
	mg_rpc_add_handler(mgos_rpc_get_global(), "debug", "{}", debugWrapper, NULL);
}
/*******************************************************************************
 * @fn      rebootSensor
 *
 * @brief   The device will be rebooted
 *
 * @param   None.
 *
 * @return  None.
 */
void rebootSensor(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{

	mgos_system_restart_after(100);
	mg_rpc_send_responsef(ri, data);
	(void)cb_arg;
	(void)fi;
}

/*******************************************************************************
 * @fn      calibration
 *
 * @brief   Run xtalk_calibration from UI
 *
 * @param   None.
 *
 * @return  None.
 */
void xtalkcalibration(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{
	/*Create a task for cross_talk*/
	xTaskCreate(cross_talk_task, "crossTalktask", 3600, NULL, 1, &xHandleTask3);
	mg_rpc_send_responsef(ri, data);
	mgos_msleep(1000);
	(void)cb_arg;
	(void)fi;
}

void liveStreamData(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{

	liveStream ^= 1;
	if (liveStream)
	{
		mgos_sys_config_set_service_livestream_toggle_status("Enabled");
	}
	else
	{
		mgos_sys_config_set_service_livestream_toggle_status("Disabled");
	}
	mgos_msleep(10);
	mg_rpc_send_responsef(ri, data);
	(void)cb_arg;
	(void)fi;
}
/*******************************************************************************
 * @fn      resetSensorCount
 *
 * @brief   Resets the sensor count(inCount_c1, outCount_c1, PrevPeopleCount, PeopleCount) to zero and turns off buzzer
 *
 * @param   None.
 *
 * @return  None.
 */
void thresholdCalibration(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{
	xTaskCreate(calibration_task, "CalibrationTask", 3600, NULL, 1, &xHandleTask2);
	mg_rpc_send_responsef(ri, data);
	mgos_msleep(1000);
	(void)cb_arg;
	(void)fi;
}

/*******************************************************************************
 * @fn      resetSensorCount
 *
 * @brief   Resets the sensor count(inCount_c1, outCount_c1, PrevPeopleCount, PeopleCount) to zero and turns off buzzer
 *
 * @param   None.
 *
 * @return  None.
 */
void getSysInfo(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{
	int heapSize, Freeheap_Size, min_Freeheap_Size, Fs_size, free_fs_size, cpu_freq;
	float uptime;
	struct mg_mgr *mgr = mgos_get_mgr();

	heapSize = mgos_get_heap_size();
	Freeheap_Size = mgos_get_free_heap_size();
	min_Freeheap_Size = mgos_get_min_free_heap_size();
	Fs_size = mgos_get_fs_size();
	free_fs_size = mgos_get_free_fs_size();
	uptime = mgos_uptime();
	cpu_freq = mgos_get_cpu_freq();
	/* Read mac address of the device */
	mac_string = mgos_sys_ro_vars_get_mac_address();
	/* Set mac address as device ID */
	mgos_sys_config_set_device_id(mac_string);
	/* Creating char buffer to store mac*/
	char str[20];
	/* copy the mac string */
	sprintf(str, "/%s/rpc", mac_string);
	// ############## ADDED BY JYOTI ##############
	void incrementLastOctet(const char *macAddress, int num, char *result)
	{
		uint64_t macInt = strtoull(macAddress, NULL, 16);
		macInt += num;
		sprintf(result, "%012llX", macInt);
	}
	char apStr[20];
	char ethStr[20];
	char modified_mac_string[13];
	incrementLastOctet(mac_string, 1, modified_mac_string);
	strcpy((char *)apStr, modified_mac_string);
	incrementLastOctet(mac_string, 3, modified_mac_string);
	strcpy((char *)ethStr, modified_mac_string);
	// ############ ADDED BY JYOTI ############################
#ifdef DEBUG
	// const char *response = mgos_rpc_call("Sys.GetInfo", NULL);
	printf("\nheapSize:%d\n", mgos_get_heap_size());
	printf("\nFreeheap_Size:%d\n", mgos_get_free_heap_size());
	printf("\nmin_Freeheap_Size:%d\n", mgos_get_min_free_heap_size());

	printf("\nFs_size:%d\n", mgos_get_fs_size());
	printf("\nfree_fs_size:%d\n", mgos_get_free_fs_size());
	printf("\nuptime:%f\n", mgos_uptime());
#endif

	for (struct mg_connection *c = mg_next(mgr, NULL); c != NULL;
		 c = mg_next(mgr, c))
	{
		if (c->flags & MG_F_IS_WEBSOCKET)
		{
			// time_t epochTime = time(0);
			maintime = time(0);
			epochTime = (uint64_t)time(&maintime);
			epochTime = epochTime * 1000;
			mg_printf_websocket_frame(c, WEBSOCKET_OP_TEXT, health, epochTime, heapSize, Freeheap_Size, min_Freeheap_Size, Fs_size, free_fs_size, cpu_freq, uptime, mgos_sys_config_get_admin_version(), mac_string, apStr, ethStr);
		}
	}

	mg_rpc_send_responsef(ri, data);
	(void)cb_arg;
	(void)fi;
}

void debugWrapper(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{

	debugEnable ^= 1;
	mgos_msleep(10);
	mg_rpc_send_responsef(ri, data);
	(void)cb_arg;
	(void)fi;
}
