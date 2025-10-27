#ifndef _RPC_CALLBACKS_H_
#define _RPC_CALLBACKS_H_

#include "mgos_rpc.h"
#include "mgos.h"


/********************************
 * GLOBAL VARIABLES 
 * ****/
/*Task handle*/
TaskHandle_t xHandleTask1 ,xHandleTask2 ,xHandleTask3 ;
uint8_t liveStream; // added by DC , livestream status
uint8_t debugEnable;

/******* RPC call back functions *******/
void rpcCallbacksInit();
void rebootSensor(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args);
void xtalkcalibration(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args);
void thresholdCalibration(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args);
void liveStreamData(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args);
void getSysInfo(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args);
void debugWrapper(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args);



/*RPC callback Tasks*/
void cross_talk_task(void *arg);
void calibration_task(void *arg);

#endif