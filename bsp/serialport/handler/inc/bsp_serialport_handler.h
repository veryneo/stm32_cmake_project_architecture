#ifndef __BSP_SERIALPORT_HANDLER_H__
#define __BSP_SERIALPORT_HANDLER_H__

/*==============================================================================
 * Include
 *============================================================================*/

#include "stdint.h"


/*==============================================================================
 * Enumeration
 *============================================================================*/

typedef enum
{
    E_SERIALPORT_HANDLER_RET_STATUS_OK = 0,
    E_SERIALPORT_HANDLER_RET_STATUS_INPUT_PARAM_ERR,
    E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR,
    E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR,
    E_SERIALPORT_HANDLER_RET_STATUS_TX_MAX_SIZE_EXCEED,
    E_SERIALPORT_HANDLER_RET_STATUS_TX_OVERFLOW,
} E_SERIALPORT_HANDLER_RET_STATUS_T;

typedef enum
{
    E_SERIALPORT_HANDLER_INIT_STATUS_NO = 0,
    E_SERIALPORT_HANDLER_INIT_STATUS_OK
} E_SERIALPORT_HANDLER_INIT_STATUS_T;

typedef enum
{
    E_SERIALPORT_HANDLER_TX_STATUS_NONE = 0,
    E_SERIALPORT_HANDLER_TX_STATUS_READY,
    E_SERIALPORT_HANDLER_TX_STATUS_BUSY,
} E_SERIALPORT_HANDLER_TX_STATUS_T;


/*==============================================================================
 * Structure
 *============================================================================*/

typedef E_SERIALPORT_HANDLER_RET_STATUS_T (*PF_SERIALPORT_HANDLER_RINGBUF_INIT_T)(void);
typedef E_SERIALPORT_HANDLER_RET_STATUS_T (*PF_SERIALPORT_HANDLER_RINGBUF_DEINIT_T)(void);
typedef uint16_t (*PF_SERIALPORT_HANDLER_RINGBUF_WRITE_T)(const uint8_t* const, const uint16_t);
typedef uint16_t (*PF_SERIALPORT_HANDLER_RINGBUF_READ_T)(uint8_t* const, const uint16_t);
typedef uint16_t (*PF_SERIALPORT_HANDLER_RINGBUF_USED_SIZE_GET_T)(void);
typedef uint16_t (*PF_SERIALPORT_HANDLER_RINGBUF_FREE_SIZE_GET_T)(void);
typedef uint16_t (*PF_SERIALPORT_HANDLER_RINGBUF_MAX_SIZE_GET_T)(void);

typedef struct 
{
    PF_SERIALPORT_HANDLER_RINGBUF_INIT_T     pf_ringbuf_init;
    PF_SERIALPORT_HANDLER_RINGBUF_DEINIT_T   pf_ringbuf_deinit;

    PF_SERIALPORT_HANDLER_RINGBUF_WRITE_T    pf_ringbuf_write;
    PF_SERIALPORT_HANDLER_RINGBUF_READ_T     pf_ringbuf_read;
    
    PF_SERIALPORT_HANDLER_RINGBUF_USED_SIZE_GET_T   pf_ringbuf_used_size_get;
    PF_SERIALPORT_HANDLER_RINGBUF_FREE_SIZE_GET_T   pf_ringbuf_free_size_get;
    PF_SERIALPORT_HANDLER_RINGBUF_MAX_SIZE_GET_T    pf_ringbuf_max_size_get;
} S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T;

typedef struct
{
    S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T* p_tx_ringbuf_intf;
    S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T* p_rx_ringbuf_intf;
} S_SERIALPORT_HANDLER_INIT_CONFIG_T;

typedef struct
{
    E_SERIALPORT_HANDLER_INIT_STATUS_T is_inited;
    volatile E_SERIALPORT_HANDLER_TX_STATUS_T tx_status;

    void* p_tx_mutex_handle;
    void* p_tx_semaphore_handle;
    void* p_rx_semaphore_handle;

    uint8_t* p_tx_tmp_buffer;

    S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T* p_tx_ringbuf_intf; /* Multi entry, single exit */
    S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T* p_rx_ringbuf_intf; /* Single entry, single exit */
} S_SERIALPORT_HANDLER_T;


/*==============================================================================
 * External Function Declaration
 *============================================================================*/

extern void serialport_handler_thread(void* argument);

extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_init(const S_SERIALPORT_HANDLER_INIT_CONFIG_T* const);

extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_transmit(const uint8_t* const, const uint16_t);
extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_on_transmit_complete(void);

extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_receive(uint8_t* const, uint16_t* const);
extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_on_hw_receive_process(const uint8_t* const, const uint16_t);
extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_on_hw_receive_complete(void);


#endif /* __BSP_SERIALPORT_HANDLER_H__ */