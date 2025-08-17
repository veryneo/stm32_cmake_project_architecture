#include "bsp_serialport_driver.h"

#include "osal.h"

#include "stddef.h"
#include "stdbool.h"

static S_SERIALPORT_DRIVER_T gs_bsp_serialport_driver =
{
    .is_inited = E_SERIALPORT_DRIVER_INIT_STATUS_NO,
    .tx_status = E_SERIALPORT_DRIVER_TX_STATUS_NONE,
    .rx_status = E_SERIALPORT_DRIVER_RX_STATUS_NONE,
    .p_hw_intf = NULL,
    .pf_transmit_complete_callback = NULL,
};


static bool _serialport_driver_init_config_check(const S_SERIALPORT_DRIVER_INIT_CONFIG_T* const p_init_config)
{
    /* Check input parameters */
    if (NULL == p_init_config)
    {
        return false;
    }

    /* Check hardware interface */
    if (NULL == p_init_config->p_hw_intf ||
        NULL == p_init_config->p_hw_intf->pf_hw_transmit_dma_start ||
        NULL == p_init_config->p_hw_intf->pf_hw_receive_dma_idle_enable)
    {
        return false;
    }

    return true;
}

extern E_SERIALPORT_DRIVER_RET_STATUS_T serialport_driver_init(const S_SERIALPORT_DRIVER_INIT_CONFIG_T* const p_init_config)
{
    /* Check input parameters */
    if (false == _serialport_driver_init_config_check(p_init_config))
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check driver initialization status */
    if (E_SERIALPORT_DRIVER_INIT_STATUS_NO != gs_bsp_serialport_driver.is_inited)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Set driver hardware interface */
    gs_bsp_serialport_driver.p_hw_intf = p_init_config->p_hw_intf;

    /* Update driver transmission and reception status */
    gs_bsp_serialport_driver.tx_status = E_SERIALPORT_DRIVER_TX_STATUS_READY;
    gs_bsp_serialport_driver.rx_status = E_SERIALPORT_DRIVER_RX_STATUS_READY;

    /* Update driver initialization status */
    gs_bsp_serialport_driver.is_inited = E_SERIALPORT_DRIVER_INIT_STATUS_OK;

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}

extern E_SERIALPORT_DRIVER_RET_STATUS_T serialport_driver_deinit(void)
{
    /* Check driver initialization status */
    if (E_SERIALPORT_DRIVER_INIT_STATUS_NO == gs_bsp_serialport_driver.is_inited)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_OK;
    }

    /* Reset driver hardware interface */
    gs_bsp_serialport_driver.p_hw_intf = NULL;

    /* Reset driver transmission and reception status */
    gs_bsp_serialport_driver.tx_status = E_SERIALPORT_DRIVER_TX_STATUS_NONE;
    gs_bsp_serialport_driver.rx_status = E_SERIALPORT_DRIVER_RX_STATUS_NONE;

    /* Reset driver initialization status */
    gs_bsp_serialport_driver.is_inited = E_SERIALPORT_DRIVER_INIT_STATUS_NO;

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}

extern E_SERIALPORT_DRIVER_RET_STATUS_T serialport_driver_transmit_dma_start(const uint8_t* const p_data, const uint32_t data_size)
{
    /* Check input parameters */
    if (NULL == p_data || 0 == data_size)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check driver initialization status */
    if (E_SERIALPORT_DRIVER_INIT_STATUS_OK != gs_bsp_serialport_driver.is_inited)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Define return status */
    E_SERIALPORT_DRIVER_RET_STATUS_T ret = E_SERIALPORT_DRIVER_RET_STATUS_OK;

    /* Enter critical section */
    osal_critical_enter();

    do
    {
        /* Check driver transmit status */
        if (E_SERIALPORT_DRIVER_TX_STATUS_READY != gs_bsp_serialport_driver.tx_status)
        {
            ret = (E_SERIALPORT_DRIVER_TX_STATUS_BUSY == gs_bsp_serialport_driver.tx_status) ?
            E_SERIALPORT_DRIVER_RET_STATUS_TX_STATUS_BUSY : E_SERIALPORT_DRIVER_RET_STATUS_INTERNAL_ERR;
           
            break;
        }
            
        /* Transmit data */
        /* Note: The transmit function must be non-blocking, otherwise it will cause the critical section to be too long */
        ret = gs_bsp_serialport_driver.p_hw_intf->pf_hw_transmit_dma_start(p_data, data_size);
        if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret)
        {
            break;
        }

        /* Update driver transmit status */
        gs_bsp_serialport_driver.tx_status = E_SERIALPORT_DRIVER_TX_STATUS_BUSY;
    } while (0);

    /* Exit critical section */
    osal_critical_exit();

    return ret;
}

extern E_SERIALPORT_DRIVER_RET_STATUS_T serialport_driver_transmit_complete_callback_register(PF_SERIALPORT_DRIVER_TRANSMIT_COMPLETE_CALLBACK_T pf_callback)
{
    /* Check input parameters */
    if (NULL == pf_callback)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check driver initialization status */
    if (E_SERIALPORT_DRIVER_INIT_STATUS_OK != gs_bsp_serialport_driver.is_inited)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Update driver transmit complete callback */
    gs_bsp_serialport_driver.pf_transmit_complete_callback = pf_callback;

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}

extern E_SERIALPORT_DRIVER_RET_STATUS_T serialport_driver_on_transmit_complete(void)
{
    /* Check driver initialization status */
    if (E_SERIALPORT_DRIVER_INIT_STATUS_OK != gs_bsp_serialport_driver.is_inited)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Update driver transmit status */
    if (E_SERIALPORT_DRIVER_TX_STATUS_READY != gs_bsp_serialport_driver.tx_status)
    {
        gs_bsp_serialport_driver.tx_status = E_SERIALPORT_DRIVER_TX_STATUS_READY;
    }

    /* Call transmit complete callback */
    if (NULL != gs_bsp_serialport_driver.pf_transmit_complete_callback)
    {
        gs_bsp_serialport_driver.pf_transmit_complete_callback();
    }

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}

extern E_SERIALPORT_DRIVER_RET_STATUS_T serialport_driver_receive_dma_idle_enable(void)
{
    /* Check driver initialization status */
    if (E_SERIALPORT_DRIVER_INIT_STATUS_OK != gs_bsp_serialport_driver.is_inited)
    {
        return E_SERIALPORT_DRIVER_RET_STATUS_INIT_STATUS_ERR;
    }

    /* Enable driver receive DMA idle */
    E_SERIALPORT_DRIVER_RET_STATUS_T ret = gs_bsp_serialport_driver.p_hw_intf->pf_hw_receive_dma_idle_enable();
    if (E_SERIALPORT_DRIVER_RET_STATUS_OK != ret)
    {
        return ret;
    }

    /* Update driver reception status */
    gs_bsp_serialport_driver.rx_status = E_SERIALPORT_DRIVER_RX_STATUS_BUSY;

    return E_SERIALPORT_DRIVER_RET_STATUS_OK;
}










