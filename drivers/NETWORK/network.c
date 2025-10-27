
#include "network.h"
#include <stdbool.h>
#include "mgos.h"
#include "mgos_system.h"

static struct mgos_config_wifi_ap ap_cfg;   // nwk sta mode config
static struct mgos_config_wifi_sta sta_cfg; // nwk ap mode config

void toggleApmode(bool state)
{
    /* Enable/Disable access point mode with default configurations(configurations in mos.yml) */
    ap_cfg.enable = state;
    mgos_wifi_setup_ap(&ap_cfg);
}

bool nwkStatus()
{
    return (ap_cfg.enable || sta_cfg.enable);
}

/*******************************************************************************
 * @fn      turnOffApMode
 *
 * @brief   turns off ap mode
 *
 * @param   None
 *
 * @return  None
 */
void turnOffApMode()
{
    /* copy the existing configuration of access point mode */
    memcpy(&ap_cfg, mgos_sys_config_get_wifi_ap(), sizeof(ap_cfg));
    if (mgos_sys_config_get_wifi_ap_enable() == AP_MODE_ENABLE)
    {
        /* Disable access point mode */
        ap_cfg.enable = AP_MODE_DISABLE;
        mgos_wifi_setup_ap(&ap_cfg);
    }
}