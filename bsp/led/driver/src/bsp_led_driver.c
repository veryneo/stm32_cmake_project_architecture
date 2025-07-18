/*==============================================================================
 * INCLUDES
 *============================================================================*/

#include "bsp_led_driver.h" 

#include "stdbool.h"


/*==============================================================================
 * Internal Function Declaration
 *============================================================================*/

static bool _led_driver_init_config_is_valid(const S_LED_DRIVER_INIT_CONFIG_T* const);


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

extern E_LED_DRIVER_RET_STATUS_T led_driver_init(S_LED_DRIVER_T* const p_led_drv, 
                                                 const S_LED_DRIVER_INIT_CONFIG_T* const p_led_drv_init_conf)
{
    /* Check input parameter */
    if (NULL == p_led_drv || false == _led_driver_init_config_is_valid(p_led_drv_init_conf) )
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED driver is already initialized */
    if (E_LED_DRIVER_INIT_STATUS_OK == p_led_drv->is_inited)
    {
        return E_LED_DRIVER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Set operation interface */
    p_led_drv->p_disp_op_intf = p_led_drv_init_conf->p_disp_op_intf;

    /* Set initialization status */
    p_led_drv->is_inited = E_LED_DRIVER_INIT_STATUS_OK;

    return E_LED_DRIVER_RET_STATUS_OK;
}

extern E_LED_DRIVER_RET_STATUS_T led_driver_deinit(S_LED_DRIVER_T* const p_led_drv)
{
    /* Check input parameter */
    if (NULL == p_led_drv)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED driver is initialized */
    if (E_LED_DRIVER_INIT_STATUS_NO == p_led_drv->is_inited)
    {
        /* LED driver has not been initialized, return OK */
        return E_LED_DRIVER_RET_STATUS_OK;
    }

    /* Turn off LED */
    if (NULL != p_led_drv->p_disp_op_intf &&
        NULL != p_led_drv->p_disp_op_intf->pf_disp_off)
    {   
        p_led_drv->p_disp_op_intf->pf_disp_off(p_led_drv); /* Ignore return value on purpose for best-effort cleanup */
    }

    /* Reset operation interface */
    p_led_drv->p_disp_op_intf = NULL;

    /* Reset initialization status */
    p_led_drv->is_inited = E_LED_DRIVER_INIT_STATUS_NO;

    return E_LED_DRIVER_RET_STATUS_OK;
}

extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_on(const S_LED_DRIVER_T* const p_led_driver)
{
    /* Check input parameter */
    if (NULL == p_led_driver)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED driver is initialized */
    if (E_LED_DRIVER_INIT_STATUS_NO == p_led_driver->is_inited)
    {
        return E_LED_DRIVER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Turn on LED */
    if (NULL == p_led_driver->p_disp_op_intf ||
        NULL == p_led_driver->p_disp_op_intf->pf_disp_on)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    return p_led_driver->p_disp_op_intf->pf_disp_on(p_led_driver);
}

extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_off(const S_LED_DRIVER_T* const p_led_driver)
{
    /* Check input parameter */
    if (NULL == p_led_driver)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED driver is initialized */
    if (E_LED_DRIVER_INIT_STATUS_NO == p_led_driver->is_inited)
    {
        return E_LED_DRIVER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Turn off LED */
    if (NULL == p_led_driver->p_disp_op_intf ||
        NULL == p_led_driver->p_disp_op_intf->pf_disp_off)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    return p_led_driver->p_disp_op_intf->pf_disp_off(p_led_driver);
}

extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_toggle(const S_LED_DRIVER_T* const p_led_driver)
{
    /* Check input parameter */
    if (NULL == p_led_driver)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED driver is initialized */
    if (E_LED_DRIVER_INIT_STATUS_NO == p_led_driver->is_inited)
    {
        return E_LED_DRIVER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Toggle LED */
    if (NULL == p_led_driver->p_disp_op_intf ||
        NULL == p_led_driver->p_disp_op_intf->pf_disp_toggle)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    return p_led_driver->p_disp_op_intf->pf_disp_toggle(p_led_driver);
}

extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_status_get(const S_LED_DRIVER_T* const p_led_driver, E_LED_DRIVER_DISP_STATUS_T* const p_led_status)
{
    /* Check input parameter */
    if (NULL == p_led_driver ||
        NULL == p_led_status)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED driver is initialized */
    if (E_LED_DRIVER_INIT_STATUS_NO == p_led_driver->is_inited)
    {
        return E_LED_DRIVER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Get LED status */
    if (NULL == p_led_driver->p_disp_op_intf ||
        NULL == p_led_driver->p_disp_op_intf->pf_disp_status_get)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }
    
    return p_led_driver->p_disp_op_intf->pf_disp_status_get(p_led_driver, p_led_status);
}


/*==============================================================================
 * Internal Function Implementation
 *============================================================================*/

static bool _led_driver_init_config_is_valid(const S_LED_DRIVER_INIT_CONFIG_T* const p_led_driver_init_conf)
{
    if (NULL == p_led_driver_init_conf)
    {
        return false;
    }

    if (NULL == p_led_driver_init_conf->p_disp_op_intf                  ||
        NULL == p_led_driver_init_conf->p_disp_op_intf->pf_disp_on      ||
        NULL == p_led_driver_init_conf->p_disp_op_intf->pf_disp_off     ||
        NULL == p_led_driver_init_conf->p_disp_op_intf->pf_disp_toggle  ||
        NULL == p_led_driver_init_conf->p_disp_op_intf->pf_disp_status_get)
    {
        return false;
    }

    return true;
}

















