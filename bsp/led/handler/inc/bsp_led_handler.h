#ifndef __BSP_LED_HANDLER_H__
#define __BSP_LED_HANDLER_H__


/*==============================================================================
 * Include
 *============================================================================*/

#include "bsp_led_driver.h"


/*==============================================================================
 * Macro
 *============================================================================*/

#define D_LED_HANDLER_DISP_PATTERN_STEP_NUM_MAX          16      


/*==============================================================================
 * Enumeration
 *============================================================================*/

typedef enum
{
    E_LED_HANDLER_RET_STATUS_OK = 0,
    E_LED_HANDLER_RET_STATUS_INPUT_PARAM_ERROR,
    E_LED_HANDLER_RET_STATUS_INIT_STATUS_ERROR,
    E_LED_HANDLER_RET_STATUS_RESOURCE_ERROR,
} E_LED_HANDLER_RET_STATUS_T;

typedef enum
{
    E_LED_HANDLER_INIT_STATUS_NO = 0,
    E_LED_HANDLER_INIT_STATUS_OK,
} E_LED_HANDLER_INIT_STATUS_T;

typedef enum
{
    E_LED_HANDLER_LED_ID_0,
    E_LED_HANDLER_LED_ID_1,
    E_LED_HANDLER_LED_ID_2,
    E_LED_HANDLER_LED_ID_3,
    E_LED_HANDLER_LED_ID_NUM_MAX
} E_LED_HANDLER_LED_ID_T;

typedef enum
{
    E_LED_HANDLER_DISP_PATTERN_STEP_STATE_OFF = 0,
    E_LED_HANDLER_DISP_PATTERN_STEP_STATE_ON,
} E_LED_HANDLER_DISP_PATTERN_STEP_STATE_T;

typedef enum
{
    E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_OFF = 0,     /* Pattern type: steady off */      
    E_LED_HANDLER_DISP_PATTERN_TYPE_STEADY_ON,          /* Pattern type: steady on */                
    E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_SLOW,         /* Pattern type: blink slow */        
    E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_NORMAL,       /* Pattern type: blink normal */      
    E_LED_HANDLER_DISP_PATTERN_TYPE_BLINK_FAST,         /* Pattern type: blink fast */        
    E_LED_HANDLER_DISP_PATTERN_TYPE_PULSE_SHORT,        /* Pattern type: pulse short */       
    E_LED_HANDLER_DISP_PATTERN_TYPE_PULSE_LONG,         /* Pattern type: pulse long */                    
    E_LED_HANDLER_DISP_PATTERN_TYPE_HEARTBEAT,          /* Pattern type: heartbeat */         
    E_LED_HANDLER_DISP_PATTERN_TYPE_CUSTOM,             /* Pattern type: custom */           
    E_LED_HANDLER_DISP_PATTERN_TYPE_NUM_MAX,            /* Pattern maximum number */
} E_LED_HANDLER_DISP_PATTERN_TYPE_T;

typedef enum
{
    E_LED_HANDLER_DISP_PATTERN_STATUS_NOSTART = 0,
    E_LED_HANDLER_DISP_PATTERN_STATUS_RUNNING,
    E_LED_HANDLER_DISP_PATTERN_STATUS_FINISHED,
    E_LED_HANDLER_DISP_PATTERN_STATUS_ERROR,
} E_LED_HANDLER_DISP_PATTERN_STATUS_T;

typedef enum
{
    E_LED_HANDLER_EVENT_TYPE_NONE = 0,
    E_LED_HANDLER_EVENT_TYPE_DISP_PATTERN_PRESET_SET,
    E_LED_HANDLER_EVENT_TYPE_DISP_PATTERN_CUSTOM_SET,
    E_LED_HANDLER_EVENT_TYPE_NUM_MAX,
} E_LED_HANDLER_EVENT_TYPE_T;


/*==============================================================================
 * Structures
 *============================================================================*/

typedef struct
{
    E_LED_HANDLER_DISP_PATTERN_STEP_STATE_T step_state;
    uint32_t                                dur_ms;
} S_LED_HANDLER_DISP_PATTERN_STEP_T;

typedef struct 
{
    S_LED_HANDLER_DISP_PATTERN_STEP_T   steps[D_LED_HANDLER_DISP_PATTERN_STEP_NUM_MAX];    
    uint8_t                             step_num;
    uint32_t                            exec_times;                                   
    uint8_t                             exec_loop_start_idx;                             
} S_LED_HANDLER_DISP_PATTERN_CONFIG_T;

typedef struct
{
    E_LED_HANDLER_DISP_PATTERN_TYPE_T            ptn_type;       
    const S_LED_HANDLER_DISP_PATTERN_CONFIG_T*   p_ptn_config;
    E_LED_HANDLER_DISP_PATTERN_STATUS_T          ptn_status;
    
    uint32_t    ptn_start_time_ms;             
    uint32_t    exec_count;                    
    uint32_t    step_start_time_ms;            
    uint8_t     step_idx;                          
} S_LED_HANDLER_DISP_PATTERN_RUNTIME_T;

typedef struct
{
    E_LED_HANDLER_LED_ID_T              led_id;
    E_LED_HANDLER_DISP_PATTERN_TYPE_T   ptn_type;
} S_LED_HANDLER_EVENT_DATA_DISP_PATTERN_PRESET_T;

typedef struct
{
    E_LED_HANDLER_LED_ID_T              led_id;
    S_LED_HANDLER_DISP_PATTERN_CONFIG_T ptn_config;
} S_LED_HANDLER_EVENT_DATA_DISP_PATTERN_CUSTOM_T;

typedef struct 
{
    E_LED_HANDLER_EVENT_TYPE_T event_type;
    union
    {
        S_LED_HANDLER_EVENT_DATA_DISP_PATTERN_PRESET_T  event_data_disp_ptn_preset;
        S_LED_HANDLER_EVENT_DATA_DISP_PATTERN_CUSTOM_T  event_data_disp_ptn_custom;
    } event_data;
} S_LED_HANDLER_EVENT_T;

typedef struct
{
    uint32_t (*pf_time_ms_get)(void);
} S_LED_HANDLER_TIMEBASE_INTERFACE_T;

typedef struct 
{
    E_LED_HANDLER_RET_STATUS_T (*pf_os_delay_ms)(const uint32_t delay_ms);
    
    E_LED_HANDLER_RET_STATUS_T (*pf_os_queue_create)(uint32_t const item_num, uint32_t const item_size, void ** const pp_queue_handle);
    E_LED_HANDLER_RET_STATUS_T (*pf_os_queue_send)(void* const p_queue_handle, void* const p_item, const uint32_t timeout_ms);
    E_LED_HANDLER_RET_STATUS_T (*pf_os_queue_receive)(void* const p_queue_handle, void* const p_item, const uint32_t timeout_ms);
    E_LED_HANDLER_RET_STATUS_T (*pf_os_queue_delete)(void* const p_queue_handle);
    E_LED_HANDLER_RET_STATUS_T (*pf_os_queue_space_get)(void* const p_queue_handle, uint32_t* const p_space);
} S_LED_HANDLER_OS_INTERFACE_T;

typedef struct 
{
    uint8_t                             led_drv_num;
    S_LED_DRIVER_T**                    pp_led_drv;
    S_LED_DRIVER_INIT_CONFIG_T*         p_led_drv_init_conf;

    S_LED_HANDLER_TIMEBASE_INTERFACE_T* p_timebase_intf;
    S_LED_HANDLER_OS_INTERFACE_T*       p_os_intf;
} S_LED_HANDLER_INIT_CONFIG_T;

typedef struct S_LED_HANDLER_DISP_PATTERN_INTERFACE S_LED_HANDLER_DISP_PATTERN_INTERFACE_T; /* Forward declaration */

typedef struct
{
    E_LED_HANDLER_INIT_STATUS_T             is_inited;

    uint8_t                                 led_drv_num;
    S_LED_DRIVER_T*                         p_led_drv[E_LED_HANDLER_LED_ID_NUM_MAX];

    S_LED_HANDLER_DISP_PATTERN_RUNTIME_T    disp_ptn_runtime[E_LED_HANDLER_LED_ID_NUM_MAX];
    S_LED_HANDLER_DISP_PATTERN_CONFIG_T     disp_ptn_config_custom[E_LED_HANDLER_LED_ID_NUM_MAX];

    void*                                   p_os_queue_handle;

    S_LED_HANDLER_DISP_PATTERN_INTERFACE_T* p_disp_ptn_intf;    /* Internal implementation */
    S_LED_HANDLER_TIMEBASE_INTERFACE_T*     p_timebase_intf;    /* External implementation */
    S_LED_HANDLER_OS_INTERFACE_T*           p_os_intf;          /* External implementation */
} S_LED_HANDLER_T;


/*==============================================================================
 * External Functions
 *============================================================================*/

extern void led_handler_thread(void*);
extern E_LED_HANDLER_RET_STATUS_T led_handler_init(const S_LED_HANDLER_INIT_CONFIG_T* const);
extern E_LED_HANDLER_RET_STATUS_T led_handler_disp_ptn_set(const S_LED_HANDLER_EVENT_T* const);


#endif /* __BSP_LED_HANDLER_H__ */