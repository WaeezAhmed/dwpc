#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_

#include <stdio.h>
#include "mgos_http_server.h"
#include "mgos_rpc.h"
#include "mgos.h"
#include "mgos_sys_config.h"
#include "mgos_ro_vars.h"
#include "frozen.h"


/******* EPOCH time variables *******/
time_t maintime;
uint64_t epochTime;

// Web sockets connection and communication Function Prototypes
void web_sockets_connection(void);
void web_sockets_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data);
void broadcast(void);
void broadcast_array(void);
void send_message_to_webui(char *message,uint8_t status);




#endif