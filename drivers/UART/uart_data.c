#include "uart_data.h"
#include "types.h"
#include "mgos.h"
#include "mgos_system.h"
#include "network.h"

// CRC16-CCITT polynomial
#define CRC16_POLY 0x1021

/* uart configuartion structure */
static struct mgos_uart_config ucfg;
static uint8_t uart_flag = 1;
static struct DWPC_CONFIG dwpcConfig;
static struct mgos_config_wifi_ap ap_cfg;   // nwk sta mode config
static struct mgos_config_wifi_sta sta_cfg; // nwk ap mode config

/*********************************************************************
 * @fn    uartInit
 *
 * @brief configures the uart communication with TX,RX buffer sizes and sets the communication buad rate, returns true if success else false
 *
 *
 * @return bool.
 */
bool uartInit()
{
    mgos_uart_config_set_defaults(UART_INTERFACE_ZERO, &ucfg);
    ucfg.baud_rate = 115200;
    ucfg.rx_buf_size = sizeof(dwpc_config);
    ucfg.tx_buf_size = sizeof(dwpc_data);
    mgos_uart_configure(UART_INTERFACE_ZERO, &ucfg);
    return mgos_uart_config_get(UART_INTERFACE_ZERO, &ucfg);
}

/*********************************************************************
 * @fn    uartReadData
 *
 * @brief reads data from the uart
 *
 *
 * @return size_t.
 */
size_t uartReadData(dwpc_config databuff)
{
    return mgos_uart_read(UART_INTERFACE_ZERO, &databuff, sizeof(databuff));
}

void uart_dispatcher_cb(int uart_no, void *arg)
{
    if (mgos_uart_read_avail(UART_INTERFACE_ZERO))
    {

        mgos_uart_read(UART_INTERFACE_ZERO, &dwpcConfig, sizeof(dwpcConfig));
        if (dwpcConfig.toggleApMode == AP_MODE_ON)
        {
            toggleApmode(1);
        }
        else if (dwpcConfig.toggleApMode == AP_MODE_OFF)
        {
            toggleApmode(0);
        }

        if (dwpcConfig.reboot == true)
        {
            mgos_system_restart_after(100);
        }
    }
}

// Calculate CRC16
uint16_t calculate_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ CRC16_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Append CRC to data structure
void append_crc_to_data(dwpc_data *data) {
    uint16_t crc = calculate_crc16((const uint8_t *)data, (sizeof(dwpc_data) - sizeof(data->sensingMode)));
    data->sensingMode = crc;
}