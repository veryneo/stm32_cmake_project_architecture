#ifndef __BSP_LED_ADAPTER_H__
#define __BSP_LED_ADAPTER_H__


#include "osal.h"

#include "stdint.h"


/*==============================================================================
 * Macro
 *============================================================================*/

 #define D_LED_ADAPTER_DISP_PATTERN_STEP_NUM_MAX          16      
 #define D_LED_ADAPTER_DISP_PATTERN_DURATION_INFINITE     0xFFFFFFFF
 #define D_LED_ADAPTER_DISP_PATTERN_EXEC_INFINITE         0xFFFFFFFF           
 #define D_LED_ADAPTER_DISP_PATTERN_EXEC_ONCE             1   
 #define D_LED_ADAPTER_DISP_PATTERN_EXEC_NO               0


 /*==============================================================================
 * Enumeration
 *============================================================================*/

typedef enum
{
    E_LED_ADAPTER_RET_STATUS_OK,
    E_LED_ADAPTER_RET_STATUS_INPUT_PARAM_ERROR,
    E_LED_ADAPTER_RET_STATUS_RESOURCE_ERROR,
} E_LED_ADAPTER_RET_STATUS_T;

typedef enum
{
    E_LED_ADAPTER_LED_ID_BLUE,
    E_LED_ADAPTER_LED_ID_GREEN,
    E_LED_ADAPTER_LED_ID_RED,
    E_LED_ADAPTER_LED_ID_NUM_MAX
} E_LED_ADAPTER_LED_ID_T;

typedef enum
{
    E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_OFF = 0,
    E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_ON,
} E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_T;

typedef enum
{
    E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_OFF = 0,      /* Pattern type: steady off */      
    E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_ON,           /* Pattern type: steady on */                
    E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_SLOW,          /* Pattern type: blink slow */        
    E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_NORMAL,        /* Pattern type: blink normal */      
    E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_FAST,          /* Pattern type: blink fast */        
    E_LED_ADAPTER_DISP_PATTERN_TYPE_PULSE_SHORT,         /* Pattern type: pulse short */       
    E_LED_ADAPTER_DISP_PATTERN_TYPE_PULSE_LONG,          /* Pattern type: pulse long */                    
    E_LED_ADAPTER_DISP_PATTERN_TYPE_HEARTBEAT,           /* Pattern type: heartbeat */         
    E_LED_ADAPTER_DISP_PATTERN_TYPE_CUSTOM,              /* Pattern type: custom */           
    E_LED_ADAPTER_DISP_PATTERN_TYPE_NUM_MAX,             /* Pattern maximum number */
} E_LED_ADAPTER_DISP_PATTERN_TYPE_T;

typedef struct
{
    E_LED_ADAPTER_DISP_PATTERN_STEP_STATE_T step_state;
    uint32_t                                dur_ms;
} S_LED_ADAPTER_DISP_PATTERN_STEP_T;

typedef struct 
{
    S_LED_ADAPTER_DISP_PATTERN_STEP_T       steps[D_LED_ADAPTER_DISP_PATTERN_STEP_NUM_MAX];    
    uint8_t                                 step_num;
    uint32_t                                exec_times;                                   
    uint8_t                                 exec_loop_start_idx;                             
} S_LED_ADAPTER_DISP_PATTERN_CONFIG_T;


extern E_LED_ADAPTER_RET_STATUS_T led_adapter_init(void);
extern void* led_adapter_thread_entry_get(void);
extern E_LED_ADAPTER_RET_STATUS_T led_adapter_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_T, E_LED_ADAPTER_DISP_PATTERN_TYPE_T);
extern E_LED_ADAPTER_RET_STATUS_T led_adapter_disp_ptn_custom_set(E_LED_ADAPTER_LED_ID_T, S_LED_ADAPTER_DISP_PATTERN_CONFIG_T*);

#endif /* __BSP_LED_ADAPTER_H__ */