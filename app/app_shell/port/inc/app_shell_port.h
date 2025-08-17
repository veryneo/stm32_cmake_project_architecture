#ifndef __APP_SHELL_PORT_H__
#define __APP_SHELL_PORT_H__

/*==============================================================================
 * Include
 *============================================================================*/

#include "stdint.h"


/*==============================================================================
 * External Function Declaration
 *============================================================================*/

extern short app_shell_port_write(char *data, unsigned short len);
extern short app_shell_port_read(char *data, unsigned short len);

#endif