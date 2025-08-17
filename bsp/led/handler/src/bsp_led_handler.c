/*==============================================================================
 * FILE INFORMATION
 *============================================================================*/
/**
 * @file    bsp_led_handler.c
 * @brief   BSP LED Handler implementation
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

#include "bsp_led_handler.h"

#include "osal.h"

#include "stdbool.h"
#include "string.h"


/*==============================================================================
 * Macro
 *============================================================================*/

 /* OS parameters */
#define D_LED_HANDLER_OS_QUEUE_SIZE                     10
#define D_LED_HANDLER_OS_QUEUE_SEND_TIMEOUT_MS          0
#define D_LED_HANDLER_OS_QUEUE_RECEIVE_TIMEOUT_MS       0
#define D_LED_HANDLER_OS_THREAD_DELAY_MS                10

/* Display pattern parameters */
#define D_LED_HANDLER_DISP_PATTERN_DURATION_INFINITE    0xFFFFFFFF
#define D_LED_HANDLER_DISP_PATTERN_EXEC_INFINITE        0xFFFFFFFF           
#define D_LED_HANDLER_DISP_PATTERN_EXEC_ONCE            1   
#define D_LED_HANDLER_DISP_PATTERN_EXEC_NO              0

#define D_LED_HANDLER_DISP_PATTERN_BLINK_SLOW_ON_MS     1000    /* Slow blink: on 1000 ms */
#define D_LED_HANDLER_DISP_PATTERN_BLINK_SLOW_OFF_MS    1000    /* Slow blink: off 1000 ms */

#define D_LED_HANDLER_DISP_PATTERN_BLINK_NORMAL_ON_MS   500     /* Normal blink: on 500 ms */
#define D_LED_HANDLER_DISP_PATTERN_BLINK_NORMAL_OFF_MS  500     /* Normal blink: off 500 ms */

#define D_LED_HANDLER_DISP_PATTERN_BLINK_FAST_ON_MS     200     /* Fast blink: on 200 ms */
#define D_LED_HANDLER_DISP_PATTERN_BLINK_FAST_OFF_MS    200     /* Fast blink: off 200 ms */

#define D_LED_HANDLER_DISP_PATTERN_PULSE_SHORT_ON_MS    100     /* Short pulse: on 100 ms */
#define D_LED_HANDLER_DISP_PATTERN_PULSE_SHORT_OFF_MS   900     /* Short pulse: off 900 ms */
#define D_LED_HANDLER_DISP_PATTERN_PULSE_LONG_ON_MS     1000    /* Long pulse: on 1000 ms */
#define D_LED_HANDLER_DISP_PATTERN_PULSE_LONG_OFF_MS    1000    /* Long pulse: off 1000 ms */

#define D_LED_HANDLER_DISP_PATTERN_SOS_DOT_MS           200     /* SOS: dot 200 ms */
#define D_LED_HANDLER_DISP_PATTERN_SOS_DASH_MS          600     /* SOS: dash 600 ms */
#define D_LED_HANDLER_DISP_PATTERN_SOS_GAP_MS           200     /* SOS: gap 200 ms */
#define D_LED_HANDLER_DISP_PATTERN_SOS_LETTER_GAP_MS    600     /* SOS: letter gap 600 ms */
#define D_LED_HANDLER_DISP_PATTERN_SOS_WORD_GAP_MS      1400    /* SOS: word gap 1400 ms */

#define D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_BEAT1_MS   150     /* Heartbeat: first beat 150 ms */
#define D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_GAP1_MS    100     /* Heartbeat: first gap 100 ms */
#define D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_BEAT2_MS   150     /* Heartbeat: second beat 150 ms */
#define D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_GAP2_MS    600     /* Heartbeat: second gap 600 ms */

/* Display pattern configuration: Once execution */
#define D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_ONCE(...) \
{ \
    .steps = { __VA_ARGS__}, \
    .step_num = sizeof((S_LED_HANDLER_DISP_PATTERN_STEP_T[]){ __VA_ARGS__ }) / sizeof(S_LED_HANDLER_DISP_PATTERN_STEP_T), \
    .exec_times = D_LED_HANDLER_DISP_PATTERN_EXEC_ONCE, \
    .exec_loop_start_idx = 0 \
}

/* Display pattern configuration: Infinite execution */
#define D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(...) \
{ \
    .steps = { __VA_ARGS__ }, \
    .step_num = sizeof((S_LED_HANDLER_DISP_PATTERN_STEP_T[]){ __VA_ARGS__ }) / sizeof(S_LED_HANDLER_DISP_PATTERN_STEP_T), \
    .exec_times = D_LED_HANDLER_DISP_PATTERN_EXEC_INFINITE, \
    .exec_loop_start_idx = 0 \
}

/* Display pattern configuration: Execution with times */
#define D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_TIMES(exec_times, ...) \
{ \
    .steps = { __VA_ARGS__ }, \
    .step_num = sizeof((S_LED_HANDLER_DISP_PATTERN_STEP_T[]){ __VA_ARGS__ }) / sizeof(S_LED_HANDLER_DISP_PATTERN_STEP_T), \
    .exec_times = exec_times, \
    .exec_loop_start_idx = 0 \
}

/* Display pattern configuration: Infinite execution from specified position */
#define D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE_FROM(exec_loop_start_idx, ...) \
{ \
    .steps = { __VA_ARGS__ }, \
    .step_num = sizeof((S_LED_HANDLER_DISP_PATTERN_STEP_T[]){ __VA_ARGS__ }) / sizeof(S_LED_HANDLER_DISP_PATTERN_STEP_T), \
    .exec_times = D_LED_HANDLER_DISP_PATTERN_EXEC_INFINITE, \
    .exec_loop_start_idx = exec_loop_start_idx \
}

/* Display pattern configuration: Fully customizable */
#define D_LED_HANDLER_DISP_PATTERN_CONFIG_CUSTOM(exec_times, loop_exec_start_idx, ...) \
{ \
    .steps = { __VA_ARGS__ }, \
    .step_num = sizeof((S_LED_HANDLER_DISP_PATTERN_STEP_T[]){ __VA_ARGS__ }) / sizeof(S_LED_HANDLER_DISP_PATTERN_STEP_T), \
    .exec_times = exec_times, \
    .exec_loop_start_idx = exec_loop_start_idx \
}


/*==============================================================================
 * Private Structure
 *============================================================================*/

struct S_LED_HANDLER_DISP_PATTERN_INTERFACE_T
{
    E_LED_HANDLER_RET_STATUS_T (*pf_disp_ptn_preset_set)(S_LED_HANDLER_T* const, const E_LED_HANDLER_LED_ID_T, const E_LED_HANDLER_DISP_PATTERN_TYPE_T); 
    E_LED_HANDLER_RET_STATUS_T (*pf_disp_ptn_custom_set)(S_LED_HANDLER_T* const, const E_LED_HANDLER_LED_ID_T, const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* const);
    E_LED_HANDLER_RET_STATUS_T (*pf_disp_ptn_start)(S_LED_HANDLER_T* const, const E_LED_HANDLER_LED_ID_T);
    E_LED_HANDLER_RET_STATUS_T (*pf_disp_ptn_process)(S_LED_HANDLER_T* const);
} ;

typedef struct 
{
    E_LED_HANDLER_DISP_PATTERN_TYPE_T                   disp_ptn_type;
    const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* const    p_disp_ptn_config; 
} S_LED_HANDLER_DISP_PATTERN_MAP_T;


/*==============================================================================
 * Private Function Declaration
 *============================================================================*/

static bool _led_handler_init_conf_is_valid(const S_LED_HANDLER_INIT_CONFIG_T* const);

static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_preset_set(S_LED_HANDLER_T* const, const E_LED_HANDLER_LED_ID_T, const E_LED_HANDLER_DISP_PATTERN_TYPE_T);
static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_custom_set(S_LED_HANDLER_T* const, const E_LED_HANDLER_LED_ID_T, const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* const);
static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_start(S_LED_HANDLER_T* const, const E_LED_HANDLER_LED_ID_T);
static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_process(S_LED_HANDLER_T* const);

static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* _led_handler_disp_ptn_preset_search(const E_LED_HANDLER_DISP_PATTERN_TYPE_T);


/*==============================================================================
 * Private Variable
 *============================================================================*/

static S_LED_HANDLER_T gs_led_handler = {0};

static S_LED_HANDLER_DISP_PATTERN_INTERFACE_T gs_led_handler_disp_ptn_intf = 
{
    .pf_disp_ptn_preset_set = _led_handler_disp_ptn_preset_set,
    .pf_disp_ptn_custom_set = _led_handler_disp_ptn_custom_set,
    .pf_disp_ptn_start      = _led_handler_disp_ptn_start,
    .pf_disp_ptn_process    = _led_handler_disp_ptn_process,
};

static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_steady_off = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_ONCE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_DURATION_INFINITE}
);

/* Steady on */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_steady_on = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_ONCE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON, D_LED_HANDLER_DISP_PATTERN_DURATION_INFINITE}
);

/* Blink slow */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_blink_slow = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_BLINK_SLOW_ON_MS},
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_BLINK_SLOW_OFF_MS}
);

/* Blink normal */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_blink_normal = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_BLINK_NORMAL_ON_MS},
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_BLINK_NORMAL_OFF_MS}
);

/* Blink fast */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_blink_fast = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_BLINK_FAST_ON_MS},
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_BLINK_FAST_OFF_MS}
);

/* Pulse short */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_pulse_short = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_PULSE_SHORT_ON_MS},
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_PULSE_SHORT_OFF_MS}
);

/* Pulse long */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_pulse_long = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_PULSE_LONG_ON_MS},
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_PULSE_LONG_OFF_MS}
);

/* Heartbeat */
static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T gs_led_handler_disp_ptn_config_heartbeat = D_LED_HANDLER_DISP_PATTERN_CONFIG_EXEC_INFINITE(
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_BEAT1_MS},   
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_GAP1_MS},   
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,  D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_BEAT2_MS},  
    {E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF, D_LED_HANDLER_DISP_PATTERN_HEARTBEAT_GAP2_MS}     
);

static const S_LED_HANDLER_DISP_PATTERN_MAP_T gs_led_handler_disp_ptn_preset_map[] = 
{
    /* Pattern type: steady off */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_OFF,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_steady_off
    },
    
    /* Pattern type: steady on */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_ON,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_steady_on
    },
    
    /* Pattern type: blink slow */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_SLOW,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_blink_slow
    },
    
    /* Pattern type: blink normal */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_NORMAL,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_blink_normal
    },
    
    /* Pattern type: blink fast */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_FAST,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_blink_fast
    },
    
    /* Pattern type: pulse short */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_PULSE_SHORT,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_pulse_short
    },
    
    /* Pattern type: pulse long */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_PULSE_LONG,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_pulse_long
    },
    
    /* Pattern type: heartbeat */
    {
        .disp_ptn_type = E_LED_HANDLER_DISP_PATTERN_TYPE_HEARTBEAT,
        .p_disp_ptn_config = &gs_led_handler_disp_ptn_config_heartbeat
    }
};

/*==============================================================================
 * External Function
 *============================================================================*/

extern void led_handler_thread(void* argument)
{
    (void)argument;

    /* Check if LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK != gs_led_handler.is_inited)
    {
        /* TBD: If we use state machine, we can set the state to error */
        return;
    }

    /* Initialize return status */
    E_LED_HANDLER_RET_STATUS_T ret_status = E_LED_HANDLER_RET_STATUS_OK;

    /* Initialize event */
    S_LED_HANDLER_EVENT_T event = {0};

    /* Start LED handler loop process */
    while (1)
    {
        /* Try to receive event from queue with timeout */
        if (E_OSAL_RET_STATUS_OK == osal_queue_receive(gs_led_handler.p_os_queue_handle, &event, D_LED_HANDLER_OS_QUEUE_RECEIVE_TIMEOUT_MS))
        {
            /* Process received event */
            switch (event.event_type)
            {
                case E_LED_HANDLER_EVENT_TYPE_DISP_PATTERN_PRESET_SET:
                {
                    /* Handle preset pattern setting event */
                    ret_status = gs_led_handler.p_disp_ptn_intf->pf_disp_ptn_preset_set(&gs_led_handler, 
                                                                                        event.event_data.disp_ptn_preset.led_id,
                                                                                        event.event_data.disp_ptn_preset.ptn_type);
        
                    /* If preset pattern is set successfully, start it automatically */
                    if (E_LED_HANDLER_RET_STATUS_OK == ret_status)
                    {
                        gs_led_handler.p_disp_ptn_intf->pf_disp_ptn_start(&gs_led_handler, event.event_data.disp_ptn_preset.led_id);
                    }
                    break;
                }
                
                case E_LED_HANDLER_EVENT_TYPE_DISP_PATTERN_CUSTOM_SET:
                {
                    /* Handle custom pattern setting event */
                    ret_status = gs_led_handler.p_disp_ptn_intf->pf_disp_ptn_custom_set(&gs_led_handler, 
                                                                            event.event_data.disp_ptn_custom.led_id,
                                                                            &event.event_data.disp_ptn_custom.ptn_config);
                    
                    /* If custom pattern is set successfully, start it automatically */
                    if (E_LED_HANDLER_RET_STATUS_OK == ret_status)
                    {
                        gs_led_handler.p_disp_ptn_intf->pf_disp_ptn_start(&gs_led_handler, event.event_data.disp_ptn_custom.led_id);
                    }
                    break;
                }
                
                case E_LED_HANDLER_EVENT_TYPE_NONE:
                default:
                {
                    /* Unknown or invalid event type, ignore */
                    break;
                }
            }
            
            /* Clear event structure for next iteration */
            memset(&event, 0, sizeof(S_LED_HANDLER_EVENT_T));
        }
        
        /* Process all LED patterns (regardless of whether we received an event or not) */
        ret_status = gs_led_handler.p_disp_ptn_intf->pf_disp_ptn_process(&gs_led_handler);
        if (E_LED_HANDLER_RET_STATUS_OK != ret_status)
        {
            /* Pattern processing failed, but continue thread execution */
            /* In production code, this might be logged for debugging */
            (void)ret_status;
        }
        
        /* Small delay to prevent excessive CPU usage */
        /* This also determines the LED pattern timing resolution */
        osal_delay_ms(D_LED_HANDLER_OS_THREAD_DELAY_MS);
    }
}

extern E_LED_HANDLER_RET_STATUS_T led_handler_init(const S_LED_HANDLER_INIT_CONFIG_T* const p_led_hdl_init_conf)
{
    /* Check input parameter */
    if (NULL == p_led_hdl_init_conf || false == _led_handler_init_conf_is_valid(p_led_hdl_init_conf))
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK == gs_led_handler.is_inited)
    {
        return E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Clear LED handler */
    memset(&gs_led_handler, 0, sizeof(S_LED_HANDLER_T));

    /* Initialize return status */
    E_LED_HANDLER_RET_STATUS_T ret_status = E_LED_HANDLER_RET_STATUS_OK;
    E_LED_DRIVER_RET_STATUS_T ret_status_drv = E_LED_DRIVER_RET_STATUS_OK;

    /* Initialize led driver initialized count */
    uint8_t led_drv_initialized_cnt = 0;

    /* Set LED handler interface */
    gs_led_handler.p_timebase_intf  =   p_led_hdl_init_conf->p_timebase_intf;
    gs_led_handler.p_disp_ptn_intf  =   &gs_led_handler_disp_ptn_intf;

    /* Create OS queue */
    S_OSAL_QUEUE_CONFIG_T os_queue_conf = 
    {
        .p_name = "led handler queue"
    };

    if (E_OSAL_RET_STATUS_OK != osal_queue_create(&(gs_led_handler.p_os_queue_handle), 
                                                  &os_queue_conf,
                                                  D_LED_HANDLER_OS_QUEUE_SIZE, 
                                                  sizeof(S_LED_HANDLER_EVENT_T) ) )
    {
        ret_status = E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
        goto cleanup_and_exit;
    }

    /* LED driver initialization */
    gs_led_handler.led_drv_num = p_led_hdl_init_conf->led_drv_num;

    for (uint8_t led_drv_idx = 0; led_drv_idx < gs_led_handler.led_drv_num; led_drv_idx++)
    {
        gs_led_handler.p_led_drv[led_drv_idx] = p_led_hdl_init_conf->pp_led_drv[led_drv_idx];

        /* Initialize led driver */
        ret_status_drv = led_driver_init(gs_led_handler.p_led_drv[led_drv_idx], p_led_hdl_init_conf->p_led_drv_init_conf);
        if (E_LED_DRIVER_RET_STATUS_OK != ret_status_drv)
        {
            ret_status = E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
            goto cleanup_and_exit;
        }

        /* Update led driver initialized count */
        led_drv_initialized_cnt++;

        /* Initialize pattern runtime information */
        gs_led_handler.disp_ptn_runtime[led_drv_idx].ptn_type       =   E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_OFF;
        gs_led_handler.disp_ptn_runtime[led_drv_idx].p_ptn_config   =   _led_handler_disp_ptn_preset_search(gs_led_handler.disp_ptn_runtime[led_drv_idx].ptn_type);

        if (NULL == gs_led_handler.disp_ptn_runtime[led_drv_idx].p_ptn_config)
        {
            ret_status = E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
            goto cleanup_and_exit;
        }

        gs_led_handler.disp_ptn_runtime[led_drv_idx].ptn_status         =   E_LED_HANDLER_DISP_PATTERN_STATUS_NOSTART;
        gs_led_handler.disp_ptn_runtime[led_drv_idx].ptn_start_time_ms  =   0;
        gs_led_handler.disp_ptn_runtime[led_drv_idx].exec_count         =   0;
        gs_led_handler.disp_ptn_runtime[led_drv_idx].step_start_time_ms =   0;
        gs_led_handler.disp_ptn_runtime[led_drv_idx].step_idx           =   0;
    
        /* Initialize custom pattern configuration area */
        memset(&gs_led_handler.disp_ptn_config_custom[led_drv_idx], 0, sizeof(S_LED_HANDLER_DISP_PATTERN_CONFIG_T));
    }

    /* Set LED handler status to initialized */
    gs_led_handler.is_inited = E_LED_HANDLER_INIT_STATUS_OK;

    return E_LED_HANDLER_RET_STATUS_OK;

cleanup_and_exit:
    /* LED driver deinitialization */
    for (uint8_t led_drv_idx = 0; led_drv_idx < led_drv_initialized_cnt; led_drv_idx++)
    {
        led_driver_deinit(gs_led_handler.p_led_drv[led_drv_idx]);
    }

    /* Delete OS queue */
    if (NULL != gs_led_handler.p_os_queue_handle)
    {
        osal_queue_delete(gs_led_handler.p_os_queue_handle);
    }

    /* Clear LED handler */
    memset(&gs_led_handler, 0, sizeof(S_LED_HANDLER_T));

    return ret_status;
}

extern E_LED_HANDLER_RET_STATUS_T led_handler_disp_ptn_set(const S_LED_HANDLER_EVENT_T* const p_event)
{
    /* Check input parameter */
    if (NULL == p_event)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (E_LED_HANDLER_EVENT_TYPE_NONE > p_event->event_type ||
        E_LED_HANDLER_EVENT_TYPE_NUM_MAX <= p_event->event_type)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK != gs_led_handler.is_inited)
    {
        return E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Send event to queue */
    if (E_OSAL_RET_STATUS_OK != osal_queue_send(gs_led_handler.p_os_queue_handle, p_event, D_LED_HANDLER_OS_QUEUE_SEND_TIMEOUT_MS))
    {
        return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
    }

    return E_LED_HANDLER_RET_STATUS_OK;
}


/*==============================================================================
 * Private Functions
 *============================================================================*/

static bool _led_handler_init_conf_is_valid(const S_LED_HANDLER_INIT_CONFIG_T* const p_led_hdl_init_conf)
{
    if (NULL == p_led_hdl_init_conf)
    {
        return false;
    }

    if (0 == p_led_hdl_init_conf->led_drv_num || E_LED_HANDLER_LED_ID_NUM_MAX < p_led_hdl_init_conf->led_drv_num )
    {
        return false;
    }

    for (uint8_t led_drv_idx = 0; led_drv_idx < p_led_hdl_init_conf->led_drv_num; led_drv_idx++)
    {
        if (NULL == p_led_hdl_init_conf->pp_led_drv[led_drv_idx])
        {
            return false;
        }

        if (NULL == p_led_hdl_init_conf->p_led_drv_init_conf)
        {
            return false;
        }
    }

    if (NULL == p_led_hdl_init_conf->p_timebase_intf ||
        NULL == p_led_hdl_init_conf->p_timebase_intf->pf_time_ms_get)
    {
        return false;
    }

    return true;
}

static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_preset_set(S_LED_HANDLER_T* const p_led_hdl, 
                                                                  const E_LED_HANDLER_LED_ID_T led_id, 
                                                                  const E_LED_HANDLER_DISP_PATTERN_TYPE_T disp_ptn_type)
{
    /* Check input parameter */
    if (NULL == p_led_hdl)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (p_led_hdl->led_drv_num <= led_id)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (0 > disp_ptn_type || E_LED_HANDLER_DISP_PATTERN_TYPE_CUSTOM <= disp_ptn_type)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK != p_led_hdl->is_inited)
    {
        return E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Update pattern runtime information */
    p_led_hdl->disp_ptn_runtime[led_id].p_ptn_config = _led_handler_disp_ptn_preset_search(disp_ptn_type);
    if (NULL == p_led_hdl->disp_ptn_runtime[led_id].p_ptn_config)
    {
        return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
    }

    p_led_hdl->disp_ptn_runtime[led_id].ptn_type             =   disp_ptn_type;
    p_led_hdl->disp_ptn_runtime[led_id].ptn_status           =   E_LED_HANDLER_DISP_PATTERN_STATUS_NOSTART;
    p_led_hdl->disp_ptn_runtime[led_id].ptn_start_time_ms    =   0;
    p_led_hdl->disp_ptn_runtime[led_id].exec_count           =   0;
    p_led_hdl->disp_ptn_runtime[led_id].step_start_time_ms   =   0;
    p_led_hdl->disp_ptn_runtime[led_id].step_idx             =   0;

    return E_LED_HANDLER_RET_STATUS_OK;
}

static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_custom_set(S_LED_HANDLER_T* const p_led_hdl, 
                                                                  const E_LED_HANDLER_LED_ID_T led_id, 
                                                                  const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* const p_disp_ptn_conf)
{
    /* Check input parameter */
    if (NULL == p_led_hdl)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (0 > led_id || p_led_hdl->led_drv_num <= led_id)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (NULL == p_disp_ptn_conf)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (0 == p_disp_ptn_conf->step_num || D_LED_HANDLER_DISP_PATTERN_STEP_NUM_MAX < p_disp_ptn_conf->step_num)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    for (uint8_t i = 0; i < p_disp_ptn_conf->step_num; i++)
    {
        if (E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON != p_disp_ptn_conf->steps[i].step_state &&
            E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF != p_disp_ptn_conf->steps[i].step_state)
        {
            return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
        }
        
        if (0 == p_disp_ptn_conf->steps[i].dur_ms)
        {
            return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
        }
    }

    if (D_LED_HANDLER_DISP_PATTERN_EXEC_NO == p_disp_ptn_conf->exec_times)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (p_disp_ptn_conf->step_num <= p_disp_ptn_conf->exec_loop_start_idx)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK != p_led_hdl->is_inited)
    {
        return E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Copy custom pattern configuration to LED handler */
    memcpy(&p_led_hdl->disp_ptn_config_custom[led_id], p_disp_ptn_conf, sizeof(S_LED_HANDLER_DISP_PATTERN_CONFIG_T));

    /* Update pattern runtime information */
    p_led_hdl->disp_ptn_runtime[led_id].ptn_type             =   E_LED_HANDLER_DISP_PATTERN_TYPE_CUSTOM;
    p_led_hdl->disp_ptn_runtime[led_id].p_ptn_config         =   &p_led_hdl->disp_ptn_config_custom[led_id];

    p_led_hdl->disp_ptn_runtime[led_id].ptn_status           =   E_LED_HANDLER_DISP_PATTERN_STATUS_NOSTART;
    p_led_hdl->disp_ptn_runtime[led_id].ptn_start_time_ms    =   0;
    p_led_hdl->disp_ptn_runtime[led_id].exec_count           =   0;
    p_led_hdl->disp_ptn_runtime[led_id].step_start_time_ms   =   0;
    p_led_hdl->disp_ptn_runtime[led_id].step_idx             =   0;

    return E_LED_HANDLER_RET_STATUS_OK;
}

static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_start(S_LED_HANDLER_T* const p_led_hdl, const E_LED_HANDLER_LED_ID_T led_id)
{
    /* Check input parameter */
    if (NULL == p_led_hdl)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (p_led_hdl->led_drv_num <= led_id)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK != p_led_hdl->is_inited)
    {
        return E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Check pattern status, only NOSTART status pattern can be started */
    if (E_LED_HANDLER_DISP_PATTERN_STATUS_NOSTART != p_led_hdl->disp_ptn_runtime[led_id].ptn_status)
    {
        return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Check pattern configuration is valid */
    const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* p_disp_ptn_config = p_led_hdl->disp_ptn_runtime[led_id].p_ptn_config;
    
    if (NULL == p_disp_ptn_config)
    {
        p_led_hdl->disp_ptn_runtime[led_id].ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_ERROR;
        return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
    }

    if (0 == p_disp_ptn_config->step_num || D_LED_HANDLER_DISP_PATTERN_STEP_NUM_MAX < p_disp_ptn_config->step_num)
    {
        p_led_hdl->disp_ptn_runtime[led_id].ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_ERROR;
        return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
    }

    for (uint8_t i = 0; i < p_disp_ptn_config->step_num; i++)
    {
        if (E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON != p_disp_ptn_config->steps[i].step_state &&
            E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF != p_disp_ptn_config->steps[i].step_state)
        {
            p_led_hdl->disp_ptn_runtime[led_id].ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_ERROR;
            return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
        }

        if (0 == p_disp_ptn_config->steps[i].dur_ms)
        {
            p_led_hdl->disp_ptn_runtime[led_id].ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_ERROR;
            return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
        }
    }

    if (p_disp_ptn_config->step_num <= p_disp_ptn_config->exec_loop_start_idx)
    {
        p_led_hdl->disp_ptn_runtime[led_id].ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_ERROR;
        return E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR;
    }

    /* Get current time */
    uint32_t current_time_ms = p_led_hdl->p_timebase_intf->pf_time_ms_get();

    /* Reset pattern runtime information */
    p_led_hdl->disp_ptn_runtime[led_id].step_idx             =   0;
    p_led_hdl->disp_ptn_runtime[led_id].exec_count           =   0;
    p_led_hdl->disp_ptn_runtime[led_id].ptn_start_time_ms    =   current_time_ms;
    p_led_hdl->disp_ptn_runtime[led_id].step_start_time_ms   =   current_time_ms;
    
    /* Set pattern status to running - LED state will be handled by _led_handler_ptn_process */
    p_led_hdl->disp_ptn_runtime[led_id].ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_RUNNING;

    return E_LED_HANDLER_RET_STATUS_OK;
}

static E_LED_HANDLER_RET_STATUS_T _led_handler_disp_ptn_process(S_LED_HANDLER_T* const p_led_hdl)
{
    /* Check input parameter */
    if (NULL == p_led_hdl)
    {
        return E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Check if LED handler is initialized */
    if (E_LED_HANDLER_INIT_STATUS_OK != p_led_hdl->is_inited)
    {
        return E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR;
    }

    /* Get current time */
    uint32_t current_time_ms = p_led_hdl->p_timebase_intf->pf_time_ms_get();

    /* Process each LED */
    for (uint8_t led_drv_idx = 0; led_drv_idx < p_led_hdl->led_drv_num; led_drv_idx++)
    {
        S_LED_HANDLER_DISP_PATTERN_RUNTIME_T* p_disp_ptn_runtime = &p_led_hdl->disp_ptn_runtime[led_drv_idx];
        
        /* Only process running pattern, skip other status */
        if (E_LED_HANDLER_DISP_PATTERN_STATUS_RUNNING != p_disp_ptn_runtime->ptn_status)
        {
            continue;
        }

        /* Get pattern configuration */
        const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* p_disp_ptn_config = p_disp_ptn_runtime->p_ptn_config;

        /* Check if step index is valid */
        if (p_disp_ptn_config->step_num <= p_disp_ptn_runtime->step_idx)
        {
            /* Step index is invalid, reset to 0 */
            p_disp_ptn_runtime->step_idx = 0;
            p_disp_ptn_runtime->step_start_time_ms = current_time_ms;
        }

        /* Get step duration */
        uint32_t step_duration_ms = p_disp_ptn_config->steps[p_disp_ptn_runtime->step_idx].dur_ms;

        /* Step duration is infinite, keep current state */
        if (D_LED_HANDLER_DISP_PATTERN_DURATION_INFINITE == step_duration_ms)
        {
            /* Set LED state */
            if (E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON == p_disp_ptn_config->steps[p_disp_ptn_runtime->step_idx].step_state)
            {
                led_driver_disp_on(p_led_hdl->p_led_drv[led_drv_idx]);
            }
            else
            {
                led_driver_disp_off(p_led_hdl->p_led_drv[led_drv_idx]);
            }
            continue;
        }

        /* Step duration is normal duration, check if current step is completed */
        if ( (uint32_t)(current_time_ms - p_disp_ptn_runtime->step_start_time_ms) >= step_duration_ms)
        {
            /* Current step is completed, get next step index */
            uint8_t next_step_idx = p_disp_ptn_runtime->step_idx + 1;
            
            /* Check if one execution is completed */
            if (next_step_idx >= p_disp_ptn_config->step_num)
            {
                /* For infinite execution mode, do not increase count */
                if (D_LED_HANDLER_DISP_PATTERN_EXEC_INFINITE != p_disp_ptn_config->exec_times)
                {
                    /* Only increase count for finite execution mode */
                    p_disp_ptn_runtime->exec_count++;
                    
                    /* Check if execution count reaches limit */
                    if (p_disp_ptn_config->exec_times <= p_disp_ptn_runtime->exec_count)
                    {
                        /* All executions are completed, keep last state and mark as finished. */
                        p_disp_ptn_runtime->ptn_status = E_LED_HANDLER_DISP_PATTERN_STATUS_FINISHED;
                        
                        /* Set last step LED state */
                        if (p_disp_ptn_config->step_num > 0)
                        {
                            uint8_t last_step = p_disp_ptn_config->step_num - 1;
                            if (E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON == p_disp_ptn_config->steps[last_step].step_state)
                            {
                                led_driver_disp_on(p_led_hdl->p_led_drv[led_drv_idx]); /* Ignore return status - best-effort execution */
                            }
                            else
                            {
                                led_driver_disp_off(p_led_hdl->p_led_drv[led_drv_idx]); /* Ignore return status - best-effort execution */
                            }
                        }
                        continue;
                    }
                } /* For finite execution mode */
                
                /* Restart execution */
                next_step_idx = p_disp_ptn_config->exec_loop_start_idx;
            } /* Check if one execution is completed */

            /* Update pattern runtime information */
            p_disp_ptn_runtime->step_idx = next_step_idx;
            p_disp_ptn_runtime->step_start_time_ms = current_time_ms;
        }

        /* Set LED state */
        if (E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON == p_disp_ptn_config->steps[p_disp_ptn_runtime->step_idx].step_state)
        {
            led_driver_disp_on(p_led_hdl->p_led_drv[led_drv_idx]); /* Ignore return status - best-effort execution */
        }
        else
        {
            led_driver_disp_off(p_led_hdl->p_led_drv[led_drv_idx]); /* Ignore return status - best-effort execution */
        }
    }

    return E_LED_HANDLER_RET_STATUS_OK;
}

static const S_LED_HANDLER_DISP_PATTERN_CONFIG_T* _led_handler_disp_ptn_preset_search(const E_LED_HANDLER_DISP_PATTERN_TYPE_T disp_ptn_type)
{
    const uint8_t map_size = sizeof(gs_led_handler_disp_ptn_preset_map) / sizeof(gs_led_handler_disp_ptn_preset_map[0]);
    
    for (uint8_t i = 0; i < map_size; i++)
    {
        if (gs_led_handler_disp_ptn_preset_map[i].disp_ptn_type == disp_ptn_type)
        {
            return gs_led_handler_disp_ptn_preset_map[i].p_disp_ptn_config;
        }
    }

    return NULL;
}