#ifndef __BSP_SERIALPORT_ADAPTER_H__
#define __BSP_SERIALPORT_ADAPTER_H__

/*==============================================================================
 * Include
 *============================================================================*/

 #include "stdint.h"


/*==============================================================================
 * Enum
 *============================================================================*/

typedef enum
{
    E_SERIALPORT_ADAPTER_RET_STATUS_OK,
    E_SERIALPORT_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR,
    E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR,
    E_SERIALPORT_ADAPTER_RET_STATUS_TX_OVERFLOW,
} E_SERIALPORT_ADAPTER_RET_STATUS_T;


/*==============================================================================
 * External Function Declaration
 *============================================================================*/

extern E_SERIALPORT_ADAPTER_RET_STATUS_T serialport_adapter_init(void);
extern void* serialport_adapter_thread_entry_get(void);
extern E_SERIALPORT_ADAPTER_RET_STATUS_T serialport_adapter_transmit(const uint8_t* const, const uint16_t);
extern E_SERIALPORT_ADAPTER_RET_STATUS_T serialport_adapter_receive(uint8_t* const, uint16_t* const);



#endif /* __BSP_SERIALPORT_ADAPTER_H__ */