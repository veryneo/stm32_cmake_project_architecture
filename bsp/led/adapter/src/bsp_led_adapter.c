/*==============================================================================
 * FILE INFORMATION
 *============================================================================*/
/**
 * @file    bsp_led_adapter.c
 * @brief   BSP LED Adapter implementation
 * @author  Chenwei Gu
 * @date    2025-07-02
 * @version 1.0.0
 * 
 * @details 
 * 
 * @note   
 * 
 * @copyright Copyright (c) 2025 Chenwei Gu. All rights reserved.
 */


/*==============================================================================
 * Include
 *============================================================================*/

#include "bsp_led_adapter.h"
#include "bsp_led_handler.h"
#include "bsp_led_driver.h"

#include "mcu.h"

#include "stddef.h"
#include "string.h"


/*==============================================================================
 * Macro
 *============================================================================*/

/* LED configuration list - centralized management of all LED configurations */
#define D_LED_ADAPTER_LED_CONFIG_LIST \
    X(BLUE, E_MCU_GPIO_PIN_LED_0, E_LED_ADAPTER_LED_ACTIVE_HIGH) \
    X(GREEN, E_MCU_GPIO_PIN_LED_1, E_LED_ADAPTER_LED_ACTIVE_HIGH) \
    X(RED, E_MCU_GPIO_PIN_LED_2, E_LED_ADAPTER_LED_ACTIVE_HIGH) \

/* Count the number of items in LED configuration list */
#define D_LED_ADAPTER_LED_CONFIG_COUNT(...)  (+1) 
#define X(...)  D_LED_ADAPTER_LED_CONFIG_COUNT(__VA_ARGS__),  /* Don't remove the comma */
#define D_LED_ADAPTER_LED_CONFIG_NUM_ACTUAL (sizeof((uint8_t[]){ D_LED_ADAPTER_LED_CONFIG_LIST }) / sizeof(uint8_t))

/* Compile-time check: Ensure led config and led id match one-to-one */
_Static_assert(E_LED_ADAPTER_LED_ID_NUM_MAX == D_LED_ADAPTER_LED_CONFIG_NUM_ACTUAL,
    "FATAL: Mismatch between LED ID enum and LED config list in adapter!");

/* Compile-time check: Ensure the number of LEDs in adapter does not exceed the handler limit */
_Static_assert((uint8_t)E_LED_ADAPTER_LED_ID_NUM_MAX <= (uint8_t)E_LED_HANDLER_LED_ID_NUM_MAX,
    "Number of LEDs in adapter exceeds handler's capacity!");

#undef  X


/*==============================================================================
 * Enum
 *============================================================================*/

typedef enum
{
    E_LED_ADAPTER_LED_ACTIVE_LOW,   /* GPIO output low to turn on LED */
    E_LED_ADAPTER_LED_ACTIVE_HIGH   /* GPIO output high to turn on LED */
} E_LED_ADAPTER_LED_ACTIVE_T;


/*==============================================================================
 * Structure
 *============================================================================*/

typedef struct
{
    const char*                 led_name;
    S_LED_DRIVER_T              led_driver;

    E_MCU_GPIO_PIN_T            gpio_pin;
    E_LED_ADAPTER_LED_ACTIVE_T  active_level;
} S_LED_ADAPTER_LED_CONFIG_T;


/*==============================================================================
 * Private Function Declaration
 *============================================================================*/

static E_LED_ADAPTER_LED_ID_T _led_adapter_led_drv_to_led_id_adp(S_LED_DRIVER_T* const p_led_drv);

static E_LED_HANDLER_LED_ID_T _led_adapter_led_id_adp_to_hdl(const E_LED_ADAPTER_LED_ID_T led_id_adp);
static E_LED_HANDLER_DISP_PATTERN_TYPE_T _led_adapter_led_disp_ptn_type_adp_to_hdl(const E_LED_ADAPTER_DISP_PATTERN_TYPE_T disp_ptn_type_adp);

static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_on(S_LED_DRIVER_T* const p_led_drv);
static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_off(S_LED_DRIVER_T* const p_led_drv);
static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_toggle(S_LED_DRIVER_T* const p_led_drv);
static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_status_get(S_LED_DRIVER_T* const p_led_drv, E_LED_DRIVER_DISP_STATUS_T* const p_status);


/*==============================================================================
 * Private Variable
 *============================================================================*/

static S_LED_ADAPTER_LED_CONFIG_T gs_led_adp_led_config[E_LED_ADAPTER_LED_ID_NUM_MAX] = 
{
#define X(name, pin, active) \
    [E_LED_ADAPTER_LED_ID_##name] = { \
        .led_name           = "LED_" #name, \
        .led_driver         = {0}, \
        .gpio_pin           = pin, \
        .active_level       = active, \
    },
    D_LED_ADAPTER_LED_CONFIG_LIST
#undef X
};

static S_LED_DRIVER_T* gs_led_adp_led_drv_ptr_array[E_LED_HANDLER_LED_ID_NUM_MAX] = 
{
#define X(name, pin, active) &gs_led_adp_led_config[E_LED_ADAPTER_LED_ID_##name].led_driver,
    D_LED_ADAPTER_LED_CONFIG_LIST
#undef X
};

static S_LED_HANDLER_TIMEBASE_INTERFACE_T gs_led_hdl_timebase_interface = 
{
    .pf_time_ms_get     =   mcu_time_tick_get
};

static S_LED_DRIVER_DISP_OPERATION_INTERFACE_T gs_led_drv_disp_op_interface = 
{
    .pf_disp_on         =   _led_adapter_led_drv_disp_on,
    .pf_disp_off        =   _led_adapter_led_drv_disp_off,
    .pf_disp_toggle     =   _led_adapter_led_drv_disp_toggle,
    .pf_disp_status_get =   _led_adapter_led_drv_disp_status_get,
};

static const S_LED_DRIVER_INIT_CONFIG_T gs_led_drv_init_conf = 
{
    .p_disp_op_intf     =   &gs_led_drv_disp_op_interface
};

static const S_LED_HANDLER_INIT_CONFIG_T gs_led_hdl_init_conf = 
{
    .led_drv_num            =   E_LED_ADAPTER_LED_ID_NUM_MAX,
    .pp_led_drv             =   gs_led_adp_led_drv_ptr_array,
    .p_led_drv_init_conf    =   &gs_led_drv_init_conf,
    .p_timebase_intf        =   &gs_led_hdl_timebase_interface,
};


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

extern E_LED_ADAPTER_RET_STATUS_T led_adapter_init(void)
{
    /* Initialize handler return status */
    E_LED_HANDLER_RET_STATUS_T ret_status_hdl = E_LED_HANDLER_RET_STATUS_OK;

    /* Initialize handler */
    ret_status_hdl = led_handler_init(&gs_led_hdl_init_conf);
    if (E_LED_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    return E_LED_ADAPTER_RET_STATUS_OK;
}

extern void* led_adapter_thread_entry_get(void)
{
    return (void*)led_handler_thread;
}

extern E_LED_ADAPTER_RET_STATUS_T led_adapter_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_T led_id, E_LED_ADAPTER_DISP_PATTERN_TYPE_T disp_ptn_type)
{
    /* Check input parameter */
    if (0 > led_id || E_LED_ADAPTER_LED_ID_NUM_MAX <= led_id)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (0 > disp_ptn_type || E_LED_ADAPTER_DISP_PATTERN_TYPE_NUM_MAX <= disp_ptn_type)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get LED ID in handler layer */
    E_LED_HANDLER_LED_ID_T led_id_hdl = _led_adapter_led_id_adp_to_hdl(led_id);
    if (E_LED_HANDLER_LED_ID_NUM_MAX == led_id_hdl)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Get pattern type in adapter layer */
    E_LED_HANDLER_DISP_PATTERN_TYPE_T disp_ptn_type_hdl = _led_adapter_led_disp_ptn_type_adp_to_hdl(disp_ptn_type);
    if (E_LED_HANDLER_DISP_PATTERN_TYPE_NUM_MAX == disp_ptn_type_hdl)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Create event*/
    S_LED_HANDLER_EVENT_T led_hdl_event = {0};
    
    led_hdl_event.event_type = E_LED_HANDLER_EVENT_TYPE_DISP_PATTERN_PRESET_SET;
    led_hdl_event.event_data.disp_ptn_preset.led_id    = led_id_hdl;
    led_hdl_event.event_data.disp_ptn_preset.ptn_type  = disp_ptn_type_hdl;

    /* Initialize handler return status */
    E_LED_HANDLER_RET_STATUS_T ret_status_hdl = E_LED_HANDLER_RET_STATUS_OK;

    /* Send event to handler */
    ret_status_hdl = led_handler_disp_ptn_set(&led_hdl_event);
    if (E_LED_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    return E_LED_ADAPTER_RET_STATUS_OK;
}

extern E_LED_ADAPTER_RET_STATUS_T led_adapter_disp_ptn_custom_set(E_LED_ADAPTER_LED_ID_T led_id, S_LED_ADAPTER_DISP_PATTERN_CONFIG_T* p_disp_ptn_conf)
{
    /* Check input parameter */
    if (0 > led_id || E_LED_ADAPTER_LED_ID_NUM_MAX <= led_id)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (NULL == p_disp_ptn_conf)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (0 == p_disp_ptn_conf->step_num || D_LED_ADAPTER_DISP_PATTERN_STEP_NUM_MAX < p_disp_ptn_conf->step_num)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    for (uint8_t i = 0; i < p_disp_ptn_conf->step_num; i++)
    {
        if (E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_OFF != p_disp_ptn_conf->steps[i].step_state &&
            E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_ON != p_disp_ptn_conf->steps[i].step_state)
        {
            return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
        }

        if (0 == p_disp_ptn_conf->steps[i].dur_ms)
        {
            return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
        }
    }

    if (D_LED_ADAPTER_DISP_PATTERN_EXEC_NO == p_disp_ptn_conf->exec_times)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (p_disp_ptn_conf->step_num <= p_disp_ptn_conf->exec_loop_start_idx)
    {
        return E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get LED ID in handler layer */
    E_LED_HANDLER_LED_ID_T led_id_hdl = _led_adapter_led_id_adp_to_hdl(led_id);
    if (led_id_hdl >= E_LED_HANDLER_LED_ID_NUM_MAX)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }
    
    /* Check conversion parameter */
    if (D_LED_HANDLER_DISP_PATTERN_STEP_NUM_MAX < p_disp_ptn_conf->step_num)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Create event */
    S_LED_HANDLER_EVENT_T led_hdl_event = {0};
    
    led_hdl_event.event_type = E_LED_HANDLER_EVENT_TYPE_DISP_PATTERN_CUSTOM_SET;
    led_hdl_event.event_data.disp_ptn_custom.led_id = led_id_hdl;
    led_hdl_event.event_data.disp_ptn_custom.ptn_config.step_num = p_disp_ptn_conf->step_num;
    led_hdl_event.event_data.disp_ptn_custom.ptn_config.exec_times = p_disp_ptn_conf->exec_times;
    led_hdl_event.event_data.disp_ptn_custom.ptn_config.exec_loop_start_idx = p_disp_ptn_conf->exec_loop_start_idx;

    for (uint8_t i = 0; i < p_disp_ptn_conf->step_num; i++)
    {
        switch (p_disp_ptn_conf->steps[i].step_state)
        {
            case E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_OFF:
                led_hdl_event.event_data.disp_ptn_custom.ptn_config.steps[i].step_state = E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF;
                break;
            case E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_ON:
                led_hdl_event.event_data.disp_ptn_custom.ptn_config.steps[i].step_state = E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON;
                break;
            default:
                return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
        }
        led_hdl_event.event_data.disp_ptn_custom.ptn_config.steps[i].dur_ms = p_disp_ptn_conf->steps[i].dur_ms;
    }

    /* Initialize handler return status */
    E_LED_HANDLER_RET_STATUS_T ret_status_hdl = E_LED_HANDLER_RET_STATUS_OK;

    /* Send event to handler */
    ret_status_hdl = led_handler_disp_ptn_set(&led_hdl_event);
    if (E_LED_HANDLER_RET_STATUS_OK != ret_status_hdl)
    {
        return E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR;
    }
    
    return E_LED_ADAPTER_RET_STATUS_OK;
}


/*==============================================================================
 * Private Function Implementation
 *============================================================================*/

static E_LED_ADAPTER_LED_ID_T _led_adapter_led_drv_to_led_id_adp(S_LED_DRIVER_T* const p_led_drv)
{
    /* Check input parameter */
    if (NULL == p_led_drv)
    {
        return E_LED_ADAPTER_LED_ID_NUM_MAX; /* Invalid ID */
    }
    
    /* Find LED ID by comparing driver addresses */
    for (uint8_t i = 0; i < E_LED_ADAPTER_LED_ID_NUM_MAX; i++)
    {
        if (&gs_led_adp_led_config[i].led_driver == p_led_drv)
        {
            return (E_LED_ADAPTER_LED_ID_T)i;
        }
    }
    
    return E_LED_ADAPTER_LED_ID_NUM_MAX; /* Invalid ID - driver not found */
}

static E_LED_HANDLER_LED_ID_T _led_adapter_led_id_adp_to_hdl(E_LED_ADAPTER_LED_ID_T led_id_adp)
{
    /* Check input parameter */
    if (0 > led_id_adp || E_LED_ADAPTER_LED_ID_NUM_MAX <= led_id_adp)
    {
        return E_LED_HANDLER_LED_ID_NUM_MAX;
    }
    
    /* 
     * The LED IDs in the adapter layer and handler layer are designed to be
     * one-to-one mapped. A static assertion ensures that the number of LEDs
     * in the adapter does not exceed the handler's capacity. Therefore, a
     * direct cast is sufficient and safe for the conversion.
     */
    return (E_LED_HANDLER_LED_ID_T)led_id_adp;
}

static E_LED_HANDLER_DISP_PATTERN_TYPE_T _led_adapter_led_disp_ptn_type_adp_to_hdl(E_LED_ADAPTER_DISP_PATTERN_TYPE_T disp_ptn_type_adp)
{
    /* Check input parameter */
    if (0 > disp_ptn_type_adp || E_LED_ADAPTER_DISP_PATTERN_TYPE_NUM_MAX <= disp_ptn_type_adp)
    {
        return E_LED_HANDLER_DISP_PATTERN_TYPE_NUM_MAX;
    }

    E_LED_HANDLER_DISP_PATTERN_TYPE_T disp_ptn_type_hdl;

    switch (disp_ptn_type_adp)
    {
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_OFF:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_OFF;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_ON:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_ON;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_SLOW:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_SLOW;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_NORMAL:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_NORMAL;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_FAST:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_FAST;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_PULSE_SHORT:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_PULSE_SHORT;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_PULSE_LONG:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_PULSE_LONG;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_HEARTBEAT:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_HEARTBEAT;
            break;
        case E_LED_ADAPTER_DISP_PATTERN_TYPE_CUSTOM:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_CUSTOM;
            break;
        default:
            disp_ptn_type_hdl = E_LED_HANDLER_DISP_PATTERN_TYPE_NUM_MAX;
            break;
    }

    return disp_ptn_type_hdl;
}

static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_on(S_LED_DRIVER_T* const p_led_drv)
{
    /* Check input parameter */
    if (NULL == p_led_drv)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get LED ID */
    E_LED_ADAPTER_LED_ID_T led_id_adp = _led_adapter_led_drv_to_led_id_adp(p_led_drv);
    if (E_LED_ADAPTER_LED_ID_NUM_MAX == led_id_adp)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Turn on LED according to active level */
    E_MCU_GPIO_RET_STATUS_T mcu_gpio_ret_status = E_MCU_GPIO_RET_STATUS_OK;
    if (E_LED_ADAPTER_LED_ACTIVE_HIGH == gs_led_adp_led_config[led_id_adp].active_level)
    {
        mcu_gpio_ret_status = mcu_gpio_write_pin(gs_led_adp_led_config[led_id_adp].gpio_pin, E_MCU_GPIO_PIN_STATE_SET);
    }
    else
    {
        mcu_gpio_ret_status = mcu_gpio_write_pin(gs_led_adp_led_config[led_id_adp].gpio_pin, E_MCU_GPIO_PIN_STATE_RESET);
    }

    return (E_MCU_GPIO_RET_STATUS_OK == mcu_gpio_ret_status) ? E_LED_DRIVER_RET_STATUS_OK : E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
}

static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_off(S_LED_DRIVER_T* const p_led_drv)
{
    /* Check input parameter */
    if (NULL == p_led_drv)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get LED ID */
    E_LED_ADAPTER_LED_ID_T led_id_adp = _led_adapter_led_drv_to_led_id_adp(p_led_drv);
    if (E_LED_ADAPTER_LED_ID_NUM_MAX == led_id_adp)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Turn off LED according to active level */
    E_MCU_GPIO_RET_STATUS_T mcu_gpio_ret_status = E_MCU_GPIO_RET_STATUS_OK;
    if (E_LED_ADAPTER_LED_ACTIVE_HIGH == gs_led_adp_led_config[led_id_adp].active_level)
    {
        mcu_gpio_ret_status = mcu_gpio_write_pin(gs_led_adp_led_config[led_id_adp].gpio_pin, E_MCU_GPIO_PIN_STATE_RESET);
    }
    else
    {
        mcu_gpio_ret_status = mcu_gpio_write_pin(gs_led_adp_led_config[led_id_adp].gpio_pin, E_MCU_GPIO_PIN_STATE_SET);
    }

    return (E_MCU_GPIO_RET_STATUS_OK == mcu_gpio_ret_status) ? E_LED_DRIVER_RET_STATUS_OK : E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
}

static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_toggle(S_LED_DRIVER_T* const p_led_drv)
{
    /* Check input parameter */
    if (NULL == p_led_drv)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get LED ID */
    E_LED_ADAPTER_LED_ID_T led_id_adp = _led_adapter_led_drv_to_led_id_adp(p_led_drv);
    if (E_LED_ADAPTER_LED_ID_NUM_MAX == led_id_adp)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Toggle LED - HAL_GPIO_TogglePin works regardless of active level */
    E_MCU_GPIO_RET_STATUS_T mcu_gpio_ret_status = E_MCU_GPIO_RET_STATUS_OK;
    mcu_gpio_ret_status = mcu_gpio_toggle_pin(gs_led_adp_led_config[led_id_adp].gpio_pin);

    return (E_MCU_GPIO_RET_STATUS_OK == mcu_gpio_ret_status) ? E_LED_DRIVER_RET_STATUS_OK : E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
}

static E_LED_DRIVER_RET_STATUS_T _led_adapter_led_drv_disp_status_get(S_LED_DRIVER_T* const p_led_drv, E_LED_DRIVER_DISP_STATUS_T* const p_status)
{
    /* Check input parameter */
    if (NULL == p_led_drv || NULL == p_status)
    {
        return E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get LED ID */
    E_LED_ADAPTER_LED_ID_T led_id_adp = _led_adapter_led_drv_to_led_id_adp(p_led_drv);
    if (E_LED_ADAPTER_LED_ID_NUM_MAX == led_id_adp)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Get current GPIO pin state */
    E_MCU_GPIO_RET_STATUS_T mcu_gpio_ret_status = E_MCU_GPIO_RET_STATUS_OK;
    E_MCU_GPIO_PIN_STATE_T mcu_gpio_pin_state = E_MCU_GPIO_PIN_STATE_RESET;

    mcu_gpio_ret_status = mcu_gpio_read_pin(gs_led_adp_led_config[led_id_adp].gpio_pin, &mcu_gpio_pin_state);
    if (E_MCU_GPIO_RET_STATUS_OK != mcu_gpio_ret_status)
    {
        return E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Determine LED status based on active level */
    if (E_LED_ADAPTER_LED_ACTIVE_HIGH == gs_led_adp_led_config[led_id_adp].active_level)
    {
        *p_status = (E_MCU_GPIO_PIN_STATE_SET == mcu_gpio_pin_state) ? E_LED_DRIVER_DISP_STATUS_ON : E_LED_DRIVER_DISP_STATUS_OFF;
    }
    else
    {
        *p_status = (E_MCU_GPIO_PIN_STATE_RESET == mcu_gpio_pin_state) ? E_LED_DRIVER_DISP_STATUS_ON : E_LED_DRIVER_DISP_STATUS_OFF;
    }

    return E_LED_DRIVER_RET_STATUS_OK;
}
