#ifndef __MCU_UART_H__
#define __MCU_UART_H__


#include "stdint.h"


#define D_MCU_UART_TRANSMIT_SIZE_MAX   512    /* Byte */
#define D_MCU_UART_RECEIVE_SIZE_MAX    256    /* Byte */


typedef enum
{
   E_MCU_UART_RET_STATUS_OK = 0,
   E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR,
   E_MCU_UART_RET_STATUS_INIT_STATUS_ERR,
   E_MCU_UART_RET_STATUS_RESOURCE_ERR, 
   E_MCU_UART_RET_STATUS_TX_BUSY,
} E_MCU_UART_RET_STATUS_T;

typedef enum
{
    E_MCU_UART_INIT_STATUS_NO = 0,
    E_MCU_UART_INIT_STATUS_OK,
} E_MCU_UART_INIT_STATUS_T;

typedef enum
{
    E_MCU_UART_TX_STATUS_NONE = 0,
    E_MCU_UART_TX_STATUS_READY,
    E_MCU_UART_TX_STATUS_BUSY,
} E_MCU_UART_TX_STATUS_T;

typedef enum
{
    E_MCU_UART_RX_STATUS_NONE = 0,
    E_MCU_UART_RX_STATUS_READY,
    E_MCU_UART_RX_STATUS_BUSY,
} E_MCU_UART_RX_STATUS_T;

typedef void (*PF_MCU_UART_TRANSMIT_COMPLETE_CALLBACK_T)(void);
typedef void (*PF_MCU_UART_RECEIVE_COMPLETE_CALLBACK_T)(void);
typedef void (*PF_MCU_UART_RECEIVE_PROCESS_CALLBACK_T)(const uint8_t* const p_data, const uint16_t data_size);


extern E_MCU_UART_RET_STATUS_T mcu_uart_init(void);
extern E_MCU_UART_RET_STATUS_T mcu_uart_deinit(void);
extern E_MCU_UART_RET_STATUS_T mcu_uart_init_status_get(E_MCU_UART_INIT_STATUS_T* const);

extern E_MCU_UART_RET_STATUS_T mcu_uart_transmit_dma_start(const uint8_t* const, const uint16_t);
extern E_MCU_UART_RET_STATUS_T mcu_uart_transmit_status_get(E_MCU_UART_TX_STATUS_T* const);
extern E_MCU_UART_RET_STATUS_T mcu_uart_transmit_complete_callback_register(PF_MCU_UART_TRANSMIT_COMPLETE_CALLBACK_T);

extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_dma_idle_enable(void);
extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_status_get(E_MCU_UART_RX_STATUS_T* const);
extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_complete_callback_register(PF_MCU_UART_RECEIVE_COMPLETE_CALLBACK_T);
extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_process_callback_register(PF_MCU_UART_RECEIVE_PROCESS_CALLBACK_T);

#endif /* __MCU_UART_H__ */