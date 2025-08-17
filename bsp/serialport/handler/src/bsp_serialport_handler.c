/*==============================================================================
 * Include
 *============================================================================*/

#include "bsp_serialport_handler.h"
#include "bsp_serialport_driver.h"

#include "osal.h"

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"


/*==============================================================================
 * Macro
 *============================================================================*/

#define D_SERIALPORT_HANDLER_TRANSMIT_TMP_BUFFER_SIZE   (512)


/*==============================================================================
 * Global Variable
 *============================================================================*/

static uint8_t gs_serialport_handler_tx_tmp_buffer[D_SERIALPORT_HANDLER_TRANSMIT_TMP_BUFFER_SIZE];

static S_SERIALPORT_HANDLER_T gs_serialport_handler = {0};


/*==============================================================================
 * Private Function Declaration
 *============================================================================*/

static bool _serialport_handler_init_conf_is_valid(const S_SERIALPORT_HANDLER_INIT_CONFIG_T* const);


/*==============================================================================
 * Public Function Implementation
 *============================================================================*/

extern void serialport_handler_thread(void* argument)
{
    (void)argument;

    if (E_SERIALPORT_HANDLER_INIT_STATUS_OK != gs_serialport_handler.is_inited)
    {
        return;
    }

    while (1)
    {
        /**
         * There are two cases that semaphore can be acquired:
         * 1. There is new data to transmit
         * 2. The last transmission is completed
         */
        if (E_OSAL_RET_STATUS_OK != osal_semaphore_acquire(gs_serialport_handler.p_tx_semaphore_handle, D_OSAL_CORE_TIMEOUT_FOREVER) )
        {
            continue;
        }

        while (1)
        {
            bool should_transmit = false;
            
            /* Enter critical section to protect status check and update */
            osal_critical_enter();
            
            /* Check if there is data to transmit */
            if (0 == gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_used_size_get())
            {
                /* There is no data to transmit, break the loop */
                osal_critical_exit();
                break;
            }

            /* Check if the handler is ready to transmit */
            if (E_SERIALPORT_HANDLER_TX_STATUS_READY != gs_serialport_handler.tx_status)
            {
                /* The handler is not ready to transmit, break the loop */
                osal_critical_exit();
                break;
            }

            /* Update status to BUSY and set flag */
            gs_serialport_handler.tx_status = E_SERIALPORT_HANDLER_TX_STATUS_BUSY;
            should_transmit = true;
            
            /* Exit critical section */
            osal_critical_exit();

            /* Start transmission outside critical section */
            if (should_transmit)
            {
                uint16_t read_size = gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_read(gs_serialport_handler.p_tx_tmp_buffer, D_SERIALPORT_HANDLER_TRANSMIT_TMP_BUFFER_SIZE);
                if (0 < read_size)
                {
                    /* Transmit data by driver */
                    E_SERIALPORT_DRIVER_RET_STATUS_T ret_status_drv = serialport_driver_transmit_dma_start(gs_serialport_handler.p_tx_tmp_buffer, read_size);
                    if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret_status_drv)
                    {
                        (void)ret_status_drv;
                        
                        /* Restore status on failure */
                        osal_critical_enter();
                        gs_serialport_handler.tx_status = E_SERIALPORT_HANDLER_TX_STATUS_READY;
                        osal_critical_exit();
                        
                        break;
                    }
                }
                else
                {
                    /* No data read, restore status */
                    osal_critical_enter();
                    gs_serialport_handler.tx_status = E_SERIALPORT_HANDLER_TX_STATUS_READY;
                    osal_critical_exit();
                }
            }
        }
    }
}

extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_init(const S_SERIALPORT_HANDLER_INIT_CONFIG_T* const p_init_config)
{
    /* Check input parameter */
    if (NULL == p_init_config || false == _serialport_handler_init_conf_is_valid(p_init_config))
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check handler initialization status */
    if (E_SERIALPORT_HANDLER_INIT_STATUS_NO != gs_serialport_handler.is_inited)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Define return status */
    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status = E_SERIALPORT_HANDLER_RET_STATUS_OK;

    /* Set ringbuffer interface */
    gs_serialport_handler.p_tx_ringbuf_intf = p_init_config->p_tx_ringbuf_intf;
    gs_serialport_handler.p_rx_ringbuf_intf = p_init_config->p_rx_ringbuf_intf;

    /* Initialize ringbuffer */
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_init() || 
        E_SERIALPORT_HANDLER_RET_STATUS_OK != gs_serialport_handler.p_rx_ringbuf_intf->pf_ringbuf_init() )
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
        goto cleanup_and_exit;
    }

    /* Create TX mutex */
    S_OSAL_MUTEX_CONFIG_T tx_mutex_conf = 
    {
        .p_name = "Serialport handler TX mutex"
    };
    
    if (E_OSAL_RET_STATUS_OK != osal_mutex_create(&(gs_serialport_handler.p_tx_mutex_handle), &tx_mutex_conf))
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
        goto cleanup_and_exit;
    }

    /* Create TX queue */
    S_OSAL_SEMAPHORE_CONFIG_T tx_semaphore_conf = 
    {
        .p_name = "Serialport handler TX semaphore"
    };

    if (E_OSAL_RET_STATUS_OK != osal_semaphore_create(&(gs_serialport_handler.p_tx_semaphore_handle), &tx_semaphore_conf, 1, 0) )
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;    
        goto cleanup_and_exit;
    }

    /* Create RX semaphore */
    S_OSAL_SEMAPHORE_CONFIG_T rx_semaphore_conf = 
    {
        .p_name = "Serialport handler RX semaphore"
    };

    if (E_OSAL_RET_STATUS_OK != osal_semaphore_create(&(gs_serialport_handler.p_rx_semaphore_handle), &rx_semaphore_conf, 1, 0) )
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
        goto cleanup_and_exit;
    }

    /* Update handler status */
    gs_serialport_handler.is_inited = E_SERIALPORT_HANDLER_INIT_STATUS_OK;
    gs_serialport_handler.tx_status = E_SERIALPORT_HANDLER_TX_STATUS_READY;

    /* Set handler transmit buffer */
    gs_serialport_handler.p_tx_tmp_buffer = gs_serialport_handler_tx_tmp_buffer;

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;

cleanup_and_exit:
    /* Delete RX semaphore */
    if (NULL != gs_serialport_handler.p_rx_semaphore_handle)
    {
        osal_semaphore_delete(gs_serialport_handler.p_rx_semaphore_handle);
    }

    /* Delete TX semaphore */
    if (NULL != gs_serialport_handler.p_tx_semaphore_handle)
    {
        osal_semaphore_delete(gs_serialport_handler.p_tx_semaphore_handle);
    }

    /* Delete TX mutex */
    if (NULL != gs_serialport_handler.p_tx_mutex_handle)
    {
        osal_mutex_delete(gs_serialport_handler.p_tx_mutex_handle);
    }

    /* Deinitialize TX ringbuffer */
    if (NULL != gs_serialport_handler.p_tx_ringbuf_intf)
    {
        gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_deinit();
    }

    /* Deinitialize RX ringbuffer */
    if (NULL != gs_serialport_handler.p_rx_ringbuf_intf)
    {
        gs_serialport_handler.p_rx_ringbuf_intf->pf_ringbuf_deinit();
    }

    /* Clear handler */
    memset(&gs_serialport_handler, 0, sizeof(S_SERIALPORT_HANDLER_T) );

    return ret_status;
}

extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_transmit(const uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    /* Note: data_size must not be 0 to ensure DMA is started and TX complete interrupt can be triggered */
    if (NULL == p_data || 0 == data_size)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check handler initialization status */
    if (E_SERIALPORT_HANDLER_INIT_STATUS_OK != gs_serialport_handler.is_inited)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Lock mutex to protect ringbuffer writing */
    if (E_OSAL_RET_STATUS_OK != osal_mutex_lock(gs_serialport_handler.p_tx_mutex_handle))
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    E_SERIALPORT_HANDLER_RET_STATUS_T ret_status = E_SERIALPORT_HANDLER_RET_STATUS_OK;

    /* Check ringbuffer max size */
    uint16_t max_size = gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_max_size_get();
    if (max_size < data_size)
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_TX_MAX_SIZE_EXCEED;
        goto unlock_and_exit;
    }

    /* Check ringbuffer free size */
    uint16_t free_size = gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_free_size_get();
    if (free_size < data_size)
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_TX_OVERFLOW;
        goto unlock_and_exit;
    }

    /* Write data to TX ringbuffer */
    uint16_t write_size = gs_serialport_handler.p_tx_ringbuf_intf->pf_ringbuf_write(p_data, data_size);
    if (data_size != write_size)
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
        goto unlock_and_exit;
    }

    /* Release semaphore */
    if (E_OSAL_RET_STATUS_OK != osal_semaphore_release(gs_serialport_handler.p_tx_semaphore_handle))
    {
        ret_status = E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
        goto unlock_and_exit;
    }

unlock_and_exit:
    /* Unlock mutex */
    if (E_OSAL_RET_STATUS_OK != osal_mutex_unlock(gs_serialport_handler.p_tx_mutex_handle))
    {
        /* If unlock fails, return this error instead */
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    return ret_status;
}

/**
 * @brief   Callback function for transmit complete
 * @note    This function is used to notify the handler that the transmit is complete.
 *          Called by external caller.
 */
extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_on_transmit_complete(void)
{
    /* Check handler initialization status */
    if (E_SERIALPORT_HANDLER_INIT_STATUS_OK != gs_serialport_handler.is_inited)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR;
    }
    
    /* Set handler tx status to ready */
    gs_serialport_handler.tx_status = E_SERIALPORT_HANDLER_TX_STATUS_READY;

    /* Release semaphore */
    if (E_OSAL_RET_STATUS_OK != osal_semaphore_release(gs_serialport_handler.p_tx_semaphore_handle) )
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_receive(uint8_t* const p_data, uint16_t* const p_data_size)
{
    /* Check input parameter */
    if (NULL == p_data || NULL == p_data_size || 0 == *p_data_size)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check handler initialization status */
    if (E_SERIALPORT_HANDLER_INIT_STATUS_OK != gs_serialport_handler.is_inited)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Acquire semaphore */
    E_OSAL_RET_STATUS_T ret_status = osal_semaphore_acquire(gs_serialport_handler.p_rx_semaphore_handle, D_OSAL_CORE_TIMEOUT_FOREVER);
    if (E_OSAL_RET_STATUS_OK != ret_status)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    /* Read data from ringbuffer */
    uint16_t need_size = *p_data_size;
    uint16_t read_size = gs_serialport_handler.p_rx_ringbuf_intf->pf_ringbuf_read(p_data, need_size);

    /* Update data size */
    *p_data_size = read_size;

    /* If ringbuffer is still not empty, release semaphore again */
    if (0 < gs_serialport_handler.p_rx_ringbuf_intf->pf_ringbuf_used_size_get() )
    {
        if (E_OSAL_RET_STATUS_OK != osal_semaphore_release(gs_serialport_handler.p_rx_semaphore_handle) )
        {
            return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
        }
    }

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

/**
 * @brief   Callback function for hardware receive process
 * @note    This function is used to notify the handler to process hardware receive data.
 *          Called by external caller.
 */
extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_on_hw_receive_process(const uint8_t* const p_data, const uint16_t data_size)
{
    /* Check input parameter */
    if (NULL == p_data || 0 == data_size)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check handler initialization status */
    if (E_SERIALPORT_HANDLER_INIT_STATUS_OK != gs_serialport_handler.is_inited)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Write data to receive ringbuffer */
    if (E_SERIALPORT_HANDLER_RET_STATUS_OK != gs_serialport_handler.p_rx_ringbuf_intf->pf_ringbuf_write(p_data, data_size))
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}

/**
 * @brief   Callback function for hardware receive complete
 * @note    This function is used to notify the handler that the hardware receive is complete.
 *          Called by external caller.
 */
extern E_SERIALPORT_HANDLER_RET_STATUS_T serialport_handler_on_hw_receive_complete(void)
{
    /* Check handler initialization status */
    if (E_SERIALPORT_HANDLER_INIT_STATUS_OK != gs_serialport_handler.is_inited)
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Release semaphore */
    if (E_OSAL_RET_STATUS_OK != osal_semaphore_release(gs_serialport_handler.p_rx_semaphore_handle) )
    {
        return E_SERIALPORT_HANDLER_RET_STATUS_RESOURCE_ERR;
    }

    return E_SERIALPORT_HANDLER_RET_STATUS_OK;
}


/*==============================================================================
 * Private Function Implementation
 *============================================================================*/

static bool _serialport_handler_init_conf_is_valid(const S_SERIALPORT_HANDLER_INIT_CONFIG_T* const p_init_config)
{
    /* Check input parameter */
    if (NULL == p_init_config)
    {
        return false;
    }

    /* Check TX ringbuffer interface */
    if (NULL == p_init_config->p_tx_ringbuf_intf                            ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_init           ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_deinit         ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_write          ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_read           ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_free_size_get  ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_used_size_get  ||
        NULL == p_init_config->p_tx_ringbuf_intf->pf_ringbuf_max_size_get)
    {
        return false;
    }

    /* Check RX ringbuffer interface */
    if (NULL == p_init_config->p_rx_ringbuf_intf                            ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_init           ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_deinit         ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_write          ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_read           ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_free_size_get  ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_used_size_get  ||
        NULL == p_init_config->p_rx_ringbuf_intf->pf_ringbuf_max_size_get)
    {
        return false;
    }

    return true;
}





