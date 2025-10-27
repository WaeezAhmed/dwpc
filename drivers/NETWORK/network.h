#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdbool.h>
/*********************************************************************
 * MACORS
 */
#define AP_MODE_ON 1
#define AP_MODE_OFF 2
#define AP_MODE_ENABLE 1
#define AP_MODE_DISABLE 0


/*********************************************************************
 * GLOBAL FUNCTIONS
 */
void toggleApmode(bool state);
bool nwkStatus();
void turnOffApMode();
#endif