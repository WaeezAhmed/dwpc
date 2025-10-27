
#include "websocket.h"
#include "types.h"

//  JSON message which is sent to web page
char *timeString = "{\"time\":%llu,\"incount\":%d,\"outcount\":%d,\"absloute\":%d}";
char *distaneArray = "{\"grid\":%s}";

/******* HTTP server, response variables *******/
struct mg_serve_http_opts http_opts = {.document_root = "."};
struct mg_connection *nc;
struct mg_connection *s_conn;

/*******************************************************************************
 * @fn     web_sockets_connection
 *
 *@brief   Register the http endpoint for web sockets
 *
 *@param   None
 *
 *@return  None
 */

void web_sockets_connection(void)
{

    void *user_data = "";

    //  Get the server handle.
    if ((nc = mgos_get_sys_http_server()) == NULL)
    {
        puts("The value of nc is NULL");
    }
    //  Bind the event handler to the HTTP server.
    mgos_register_http_endpoint("/", web_sockets_handler, user_data);
}

/*******************************************************************************
 *@fn     web_sockets_handler
 *
 *@brief   Handles all the web sockets events
 *
 *@param   nc -> connection, ev -> event
 *
 *@return  None
 */
void web_sockets_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data)
{
    struct http_message *hm = (struct http_message *)ev_data;
    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
    {
        mg_serve_http(nc, hm, http_opts);

        break;
    }
    case MG_EV_SEND:
    {
        // printf("Event_send\n");
        break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
    {
        s_conn = nc;
        break;
    }
    case MG_EV_WEBSOCKET_FRAME:
    {
        break;
    }
    case MG_EV_CLOSE:
    {
        // printf("Event closed\n");
        break;
    }
    case MG_EV_TIMER:
    {
        break;
    }
    }
}
/*******************************************************************************
 *@fn     broadcast
 *
 *@brief   broadcasts the data to webpage
 *
 *@param   nc -> connection, ev -> event
 *
 *@return  None
 */
void broadcast(void)
{
    struct mg_mgr *mgr = mgos_get_mgr();

    for (struct mg_connection *c = mg_next(mgr, NULL); c != NULL;
         c = mg_next(mgr, c))
    {
        if (c->flags & MG_F_IS_WEBSOCKET)
        {
            // time_t epochTime = time(0);
            maintime = time(0);
            epochTime = (uint64_t)time(&maintime);
            epochTime = epochTime * 1000;
            // printf("broadcast\n");
            mg_printf_websocket_frame(c, WEBSOCKET_OP_TEXT, timeString, epochTime, incountAggregation, outcountAggregation, dwpcData.PeopleCount);
        }
    }
}

/*******************************************************************************
 *@fn     broadcast
 *
 *@brief   broadcasts the data to webpage
 *
 *@param   nc -> connection, ev -> event
 *
 *@return  None
 */
void broadcast_array(void)
{
    struct mg_mgr *mgr_1 = mgos_get_mgr();

    for (struct mg_connection *c = mg_next(mgr_1, NULL); c != NULL;
         c = mg_next(mgr_1, c))
    {
        if (c->flags & MG_F_IS_WEBSOCKET)
        {
            mg_printf_websocket_frame(c, WEBSOCKET_OP_TEXT, distaneArray, jsondata);
            strcpy(jsondata, "\0");
            strcpy(arrayofdistance, "\0");
        }
    }
}

void send_message_to_webui(char *message, uint8_t status)
{
    // printf("Data sending to webui\n");
    if (s_conn != NULL)
    {
        mg_printf_websocket_frame(s_conn, WEBSOCKET_OP_TEXT, "{\"Message\":\"%s:%d\"}", message, status);
    }
}