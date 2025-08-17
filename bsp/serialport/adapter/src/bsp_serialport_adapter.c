/*==============================================================================
 * Include
 *============================================================================*/

#include "bsp_serialport_adapter.h"
#include "bsp_serialport_handler.h"
#include "bsp_serialport_driver.h"

#include "mcu.h"

#include "lwrb.h"


/*==============================================================================
 * Macro
 *============================================================================*/

/* Ringbuffer for transmit, size depends on MCU UART transmit size */
#define D_SERIALPORT_ADAPTER_TRANSMIT_RINGBUFFER_CPPACITY_SIZE  (D_MCU_UART_TRANSMIT_SIZE_MAX)
#define D_SERIALPORT_ADAPTER_TRANSMIT_RINGBUFFER_STORAGE_SIZE   (D_MCU_UART_TRANSMIT_SIZE_MAX + 1)

/* Ringbuffer for receive, size depends on MCU UART receive size */
#define D_SERIALPORT_ADAPTER_RECEIVE_RINGBUFFER_CPPACITY_SIZE   (D_MCU_UART_RECEIVE_SIZE_MAX)
#define D_SERIALPORT_ADAPTER_RECEIVE_RINGBUFFER_STORAGE_SIZE    (D_MCU_UART_RECEIVE_SIZE_MAX + 1)


/*==============================================================================
 * Private Function Declaration
 *============================================================================*/

 /**
  * @brief  Driver layer hardware interface function
  */
static E_SERIALPORT_DRIVER_RET_STATUS_T _serialport_adapter_drv_hw_transmit_dma_start(const uint8_t* const p_data, const uint16_t data_size);
static E_SERIALPORT_DRIVER_RET_STATUS_T _serialport_adapter_drv_hw_receive_dma_idle_enable(void);

/**
 * @brief  Handler layer ringbuffer interface function
 */
static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_tx_ringbuf_init(void);
static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_tx_ringbuf_deinit(void);
static uint16_t _serialport_adapter_hdl_tx_ringbuf_write(const uint8_t* const, const uint16_t);
static uint16_t _serialport_adapter_hdl_tx_ringbuf_read(uint8_t* const, const uint16_t);
static uint16_t _serialport_adapter_hdl_tx_ringbuf_used_size_get(void);
static uint16_t _serialport_adapter_hdl_tx_ringbuf_free_size_get(void);
static uint16_t _serialport_adapter_hdl_tx_ringbuf_max_size_get(void);

/**
 * @brief  Handler layer ringbuffer interface function
 */
static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_rx_ringbuf_init(void);
static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_rx_ringbuf_deinit(void);
static uint16_t _serialport_adapter_hdl_rx_ringbuf_write(const uint8_t* const, const uint16_t);
static uint16_t _serialport_adapter_hdl_rx_ringbuf_read(uint8_t* const, const uint16_t);
static uint16_t _serialport_adapter_hdl_rx_ringbuf_used_size_get(void);
static uint16_t _serialport_adapter_hdl_rx_ringbuf_free_size_get(void);
static uint16_t _serialport_adapter_hdl_rx_ringbuf_max_size_get(void);

/**
 * @brief  MCU layer callback function
 */
static void _serialport_adapter_mcu_uart_to_drv_on_transmit_complete(void);
static void _serialport_adapter_mcu_uart_to_hdl_on_hw_receive_complete(void);
static void _serialport_adapter_mcu_uart_to_hdl_on_hw_receive_process(const uint8_t* const, const uint16_t);

/**
 * @brief  Driver layer callback function
 */
static void _serialport_adapter_drv_to_hdl_on_transmit_complete(void);


/*==============================================================================
 * Variable
 *============================================================================*/

static lwrb_t gs_serialport_adapter_tx_ringbuf_handle;
static uint8_t gs_serialport_adapter_tx_ringbuf_buffer[D_SERIALPORT_ADAPTER_TRANSMIT_RINGBUFFER_STORAGE_SIZE];

static lwrb_t gs_serialport_adapter_rx_ringbuf_handle;
static uint8_t gs_serialport_adapter_rx_ringbuf_buffer[D_SERIALPORT_ADAPTER_RECEIVE_RINGBUFFER_STORAGE_SIZE];

static S_SERIALPORT_DRIVER_HW_INTERFACE_T gs_serialport_driver_hw_intf = 
{
    .pf_hw_transmit_dma_start       = _serialport_adapter_drv_hw_transmit_dma_start,
    .pf_hw_receive_dma_idle_enable  = _serialport_adapter_drv_hw_receive_dma_idle_enable,
};

static S_SERIALPORT_DRIVER_INIT_CONFIG_T gs_serialport_driver_init_conf = 
{
    .p_hw_intf = &gs_serialport_driver_hw_intf,
};

static S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T gs_serialport_handler_tx_ringbuf_interface = 
{
    .pf_ringbuf_init             = _serialport_adapter_hdl_tx_ringbuf_init,
    .pf_ringbuf_deinit           = _serialport_adapter_hdl_tx_ringbuf_deinit,
    .pf_ringbuf_write            = _serialport_adapter_hdl_tx_ringbuf_write,
    .pf_ringbuf_read             = _serialport_adapter_hdl_tx_ringbuf_read,
    .pf_ringbuf_used_size_get    = _serialport_adapter_hdl_tx_ringbuf_used_size_get,
    .pf_ringbuf_free_size_get    = _serialport_adapter_hdl_tx_ringbuf_free_size_get,
    .pf_ringbuf_max_size_get     = _serialport_adapter_hdl_tx_ringbuf_max_size_get,
};

static S_SERIALPORT_HANDLER_RINGBUF_INTERFACE_T gs_serialport_handler_rx_ringbuf_interface = 
{
    .pf_ringbuf_init             = _serialport_adapter_hdl_rx_ringbuf_init,
    .pf_ringbuf_deinit           = _serialport_adapter_hdl_rx_ringbuf_deinit,
    .pf_ringbuf_write            = _serialport_adapter_hdl_rx_ringbuf_write,
    .pf_ringbuf_read             = _serialport_adapter_hdl_rx_ringbuf_read,
    .pf_ringbuf_used_size_get    = _serialport_adapter_hdl_rx_ringbuf_used_size_get,
    .pf_ringbuf_free_size_get    = _serialport_adapter_hdl_rx_ringbuf_free_size_get,
    .pf_ringbuf_max_size_get     = _serialport_adapter_hdl_rx_ringbuf_max_size_get,
};

static S_SERIALPORT_HANDLER_INIT_CONFIG_T gs_serialport_handler_init_conf = 
{
    .p_tx_ringbuf_intf = &gs_serialport_handler_tx_ringbuf_interface,
    .p_rx_ringbuf_intf = &gs_serialport_handler_rx_ringbuf_interface,
};


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

 /**
  * @brief  Initialize adapter layer (handler layer and driver layer will be initialized in adapter layer)
  * @return E_SERIALPORT_ADAPTER_RET_STATUS_T
  */
extern E_SERIALPORT_ADAPTER_RET_STATUS_T serialport_adapter_init(void)
{
    /* Initialize handler */
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status_hdl = E_SERIALPORT_HANDLER_RET_STATUS_OK;

    ret_status_hdl = serialport_handler_init(&gs_serialport_handler_init_conf);
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        (void)ret_status_hdl;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Initialize driver */
    E_SERIALPORT_DRIVER_RET_STATUS_T ret_status_drv = E_SERIALPORT_DRIVER_RET_STATUS_OK;

    ret_status_drv = serialport_driver_init(&gs_serialport_driver_init_conf);
    if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret_status_drv)
    {
        (void)ret_status_drv;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Register mcu layer callback function */
    E_MCU_UART_RET_STATUS_T ret_status_mcu = E_MCU_UART_RET_STATUS_OK;

    ret_status_mcu = mcu_uart_transmit_complete_callback_register(_serialport_adapter_mcu_uart_to_drv_on_transmit_complete);
    if (E_MCU_UART_RET_STATUS_OK != ret_status_mcu)
    {
        (void)ret_status_mcu;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    ret_status_mcu = mcu_uart_receive_complete_callback_register(_serialport_adapter_mcu_uart_to_hdl_on_hw_receive_complete);
    if (E_MCU_UART_RET_STATUS_OK != ret_status_mcu)
    {
        (void)ret_status_mcu;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    ret_status_mcu = mcu_uart_receive_process_callback_register(_serialport_adapter_mcu_uart_to_hdl_on_hw_receive_process);
    if (E_MCU_UART_RET_STATUS_OK != ret_status_mcu)
    {
        (void)ret_status_mcu;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Register driver callback function */
    ret_status_drv = serialport_driver_transmit_complete_callback_register(_serialport_adapter_drv_to_hdl_on_transmit_complete);
    if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret_status_drv)
    {
        (void)ret_status_drv;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Enable receive DMA idle */
    ret_status_drv = serialport_driver_receive_dma_idle_enable();
    if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret_status_drv)
    {
        (void)ret_status_drv;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    return E_SERIALPORT_ADAPTER_RET_STATUS_OK;
}

extern void* serialport_adapter_thread_entry_get(void)
{
    return (void*)serialport_handler_thread;
}

extern E_SERIALPORT_ADAPTER_RET_STATUS_T serialport_adapter_transmit(const uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data || 0 == data_size)
    {
        return E_SERIALPORT_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Transmit data to handler layer */
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status_hdl = serialport_handler_transmit(p_data, data_size);
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        (void)ret_status_hdl;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    return E_SERIALPORT_ADAPTER_RET_STATUS_OK;
}

extern E_SERIALPORT_ADAPTER_RET_STATUS_T serialport_adapter_receive(uint8_t* const p_data, uint16_t* const p_data_size)
{
    /* Check input parameter */
    if (NULL == p_data || NULL == p_data_size || 0 == *p_data_size)
    {
        return E_SERIALPORT_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Receive data from handler layer */
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status_hdl = serialport_handler_receive(p_data, p_data_size);
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        (void)ret_status_hdl;

        return E_SERIALPORT_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    return E_SERIALPORT_ADAPTER_RET_STATUS_OK;
}


/*==============================================================================
 * Private Function Implementation
 *============================================================================*/

static E_SERIALPORT_DRIVER_RET_STATUS_T _serialport_adapter_drv_hw_transmit_dma_start(const uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Transmit data to MCU */
    E_MCU_UART_RET_STATUS_T ret_status_mcu = mcu_uart_transmit_dma_start(p_data, data_size);
    if (E_MCU_UART_RET_STATUS_OK != ret_status_mcu)
    {
        (void)ret_status_mcu;

        return E_SERIALPORT_DRIVER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}

static E_SERIALPORT_DRIVER_RET_STATUS_T _serialport_adapter_drv_hw_receive_dma_idle_enable(void)
{
    /* Enable receive DMA idle */
    E_MCU_UART_RET_STATUS_T ret_status_mcu = mcu_uart_receive_dma_idle_enable();
    if (E_MCU_UART_RET_STATUS_OK != ret_status_mcu)
    {
        (void)ret_status_mcu;

        return E_SERIALPORT_DRIVER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}

static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_tx_ringbuf_init(void)
{
    /* Initialize transmit ringbuffer */
    if (1 != lwrb_init(&gs_serialport_adapter_tx_ringbuf_handle, gs_serialport_adapter_tx_ringbuf_buffer, D_SERIALPORT_ADAPTER_TRANSMIT_RINGBUFFER_STORAGE_SIZE))
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_tx_ringbuf_deinit(void)
{
    /* No implementation */
    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

static uint16_t _serialport_adapter_hdl_tx_ringbuf_write(const uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data || D_SERIALPORT_ADAPTER_TRANSMIT_RINGBUFFER_CPPACITY_SIZE < data_size)
    {
        return 0;
    }

    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_tx_ringbuf_handle) )
    {
        return 0;
    }

    /* Check if ringbuffer has enough space */
    uint16_t free_size = lwrb_get_free(&gs_serialport_adapter_tx_ringbuf_handle);
    if (free_size < data_size)
    {
        return 0;
    }

    /* Write data to ringbuffer */
    uint16_t write_size = 0;
    write_size = (uint16_t)lwrb_write(&gs_serialport_adapter_tx_ringbuf_handle, p_data, (lwrb_sz_t)data_size);

    return write_size;
}

static uint16_t _serialport_adapter_hdl_tx_ringbuf_read(uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data)
    {
        return 0;
    }

    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_tx_ringbuf_handle) )
    {
        return 0;
    }

    /* Read data from ringbuffer */
    uint16_t read_size = 0;
    read_size = (uint16_t)lwrb_read(&gs_serialport_adapter_tx_ringbuf_handle, p_data, (lwrb_sz_t)data_size);
    
    return read_size;
}

static uint16_t _serialport_adapter_hdl_tx_ringbuf_used_size_get(void)
{
    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_tx_ringbuf_handle) )
    {
        return 0;
    }

    /* Get used size of ringbuffer */
    uint16_t used_size = 0;
    used_size = (uint16_t)lwrb_get_full(&gs_serialport_adapter_tx_ringbuf_handle);

    return used_size;
}

static uint16_t _serialport_adapter_hdl_tx_ringbuf_free_size_get(void)
{
    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_tx_ringbuf_handle) )
    {
        return 0;
    }

    /* Get free size of ringbuffer */
    uint16_t free_size = 0;
    free_size = (uint16_t)lwrb_get_free(&gs_serialport_adapter_tx_ringbuf_handle);

    return free_size;
}

static uint16_t _serialport_adapter_hdl_tx_ringbuf_max_size_get(void)
{
    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_tx_ringbuf_handle) )
    {
        return 0;
    }

    return D_SERIALPORT_ADAPTER_TRANSMIT_RINGBUFFER_CPPACITY_SIZE;
}

static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_rx_ringbuf_init(void)
{
    if (1 != lwrb_init(&gs_serialport_adapter_rx_ringbuf_handle, gs_serialport_adapter_rx_ringbuf_buffer, D_SERIALPORT_ADAPTER_RECEIVE_RINGBUFFER_STORAGE_SIZE))
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

static E_SERIALPORT_HANDLER_RET_STATUS_T _serialport_adapter_hdl_rx_ringbuf_deinit(void)
{
    /* No implementation */
    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

static uint16_t _serialport_adapter_hdl_rx_ringbuf_write(const uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data || D_SERIALPORT_ADAPTER_RECEIVE_RINGBUFFER_CPPACITY_SIZE < data_size)
    {
        return 0;
    }

    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_rx_ringbuf_handle) )
    {
        return 0;
    }

    /* Get free size of ringbuffer */
    uint16_t free_size = 0;
    free_size = lwrb_get_free(&gs_serialport_adapter_rx_ringbuf_handle);

    /* Check if ringbuffer has enough space */
    if (free_size < data_size)
    {
        return 0;
    }

    /* Write data to ringbuffer */
    uint16_t write_size = 0;
    write_size = (uint16_t)lwrb_write(&gs_serialport_adapter_rx_ringbuf_handle, p_data, (lwrb_sz_t)data_size);

    return write_size;
}

static uint16_t _serialport_adapter_hdl_rx_ringbuf_read(uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data)
    {
        return 0;
    }

    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_rx_ringbuf_handle) )
    {
        return 0;
    }

    /* Read data from ringbuffer */
    uint16_t read_size = 0;
    read_size = (uint16_t)lwrb_read(&gs_serialport_adapter_rx_ringbuf_handle, p_data, (lwrb_sz_t)data_size);

    return read_size;
}

static uint16_t _serialport_adapter_hdl_rx_ringbuf_used_size_get(void)
{
    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_rx_ringbuf_handle) )
    {
        return 0;    
    }

    /* Get used size of ringbuffer */
    uint16_t used_size = 0;
    used_size = (uint16_t)lwrb_get_full(&gs_serialport_adapter_rx_ringbuf_handle);

    return used_size;
}

static uint16_t _serialport_adapter_hdl_rx_ringbuf_free_size_get(void)
{
    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_rx_ringbuf_handle) )
    {
        return 0;
    }

    /* Get free size of ringbuffer */
    uint16_t free_size = 0;
    free_size = (uint16_t)lwrb_get_free(&gs_serialport_adapter_rx_ringbuf_handle);

    return free_size;
}

static uint16_t _serialport_adapter_hdl_rx_ringbuf_max_size_get(void)
{
    /* Check if ringbuffer is ready */
    if (1 != lwrb_is_ready(&gs_serialport_adapter_rx_ringbuf_handle) )
    {
        return 0;
    }

    return D_SERIALPORT_ADAPTER_RECEIVE_RINGBUFFER_CPPACITY_SIZE;
}

static void _serialport_adapter_mcu_uart_to_drv_on_transmit_complete(void)
{
    E_SERIALPORT_DRIVER_RET_STATUS_T ret_status_drv = E_SERIALPORT_DRIVER_RET_STATUS_OK;

    ret_status_drv = serialport_driver_on_transmit_complete();
    if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret_status_drv)
    {
        (void)ret_status_drv;
    }
}

static void _serialport_adapter_mcu_uart_to_hdl_on_hw_receive_complete(void)
{
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status_hdl = E_SERIALPORT_HANDLER_RET_STATUS_OK;

    ret_status_hdl = serialport_handler_on_hw_receive_complete();
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        (void)ret_status_hdl;
    }
}

static void _serialport_adapter_mcu_uart_to_hdl_on_hw_receive_process(const uint8_t* const p_data, const uint16_t data_size)
{
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status_hdl = E_SERIALPORT_HANDLER_RET_STATUS_OK;

    ret_status_hdl = serialport_handler_on_hw_receive_process(p_data, data_size);
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        (void)ret_status_hdl;
    }
}

static void _serialport_adapter_drv_to_hdl_on_transmit_complete(void)
{
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status_hdl = E_SERIALPORT_HANDLER_RET_STATUS_OK;

    ret_status_hdl = serialport_handler_on_transmit_complete();
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        (void)ret_status_hdl;
    }
}
