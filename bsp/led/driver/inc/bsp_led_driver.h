#ifndef __BSP_LED_DRIVER_H__
#define __BSP_LED_DRIVER_H__


/*==============================================================================
 * INCLUDES
 *============================================================================*/

#include "stdint.h"      


/*==============================================================================
 * ENUMERATIONS
 *============================================================================*/

typedef enum
{
    E_LED_DRIVER_RET_STATUS_OK = 0,
    E_LED_DRIVER_RET_STATUS_INPUT_PARAM_ERROR,
    E_LED_DRIVER_RET_STATUS_INIT_STATUS_ERROR,
    E_LED_DRIVER_RET_STATUS_RESOURCE_ERROR,
} E_LED_DRIVER_RET_STATUS_T;

typedef enum
{
    E_LED_DRIVER_INIT_STATUS_NO = 0,
    E_LED_DRIVER_INIT_STATUS_OK,
} E_LED_DRIVER_INIT_STATUS_T;

/**
 * @brief LED driver display status
 *  
 * @note 
 * 1. E_LED_DRIVER_DISP_STATUS_OFF: LED display is off
 * 2. E_LED_DRIVER_DISP_STATUS_ON: LED display is on
 */
typedef enum
{
    E_LED_DRIVER_DISP_STATUS_OFF = 0,   // LED灭
    E_LED_DRIVER_DISP_STATUS_ON,        // LED亮
} E_LED_DRIVER_DISP_STATUS_T;


/*==============================================================================
 * STRUCTURES
 *============================================================================*/

typedef struct S_LED_DRIVER_T S_LED_DRIVER_T; /* Forward declaration */

typedef struct
{   
    E_LED_DRIVER_RET_STATUS_T (*pf_disp_on)(S_LED_DRIVER_T* const);
    E_LED_DRIVER_RET_STATUS_T (*pf_disp_off)(S_LED_DRIVER_T* const);
    E_LED_DRIVER_RET_STATUS_T (*pf_disp_toggle)(S_LED_DRIVER_T* const);
    E_LED_DRIVER_RET_STATUS_T (*pf_disp_status_get)(S_LED_DRIVER_T* const, E_LED_DRIVER_DISP_STATUS_T* const);
} S_LED_DRIVER_DISP_OPERATION_INTERFACE_T;

typedef struct
{
    S_LED_DRIVER_DISP_OPERATION_INTERFACE_T*    p_disp_op_intf;    
} S_LED_DRIVER_INIT_CONFIG_T;

// 修改这里：使用有名字的结构体
struct S_LED_DRIVER_T
{
    E_LED_DRIVER_INIT_STATUS_T                  is_inited;
    
    S_LED_DRIVER_DISP_OPERATION_INTERFACE_T*    p_disp_op_intf;          
};


/*==============================================================================
 * External functions declaration
 *============================================================================*/

extern E_LED_DRIVER_RET_STATUS_T led_driver_init(S_LED_DRIVER_T* const, const S_LED_DRIVER_INIT_CONFIG_T* const);
extern E_LED_DRIVER_RET_STATUS_T led_driver_deinit(S_LED_DRIVER_T* const);
extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_on(S_LED_DRIVER_T* const);
extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_off(S_LED_DRIVER_T* const);
extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_toggle(S_LED_DRIVER_T* const);
extern E_LED_DRIVER_RET_STATUS_T led_driver_disp_status_get(S_LED_DRIVER_T* const, E_LED_DRIVER_DISP_STATUS_T* const);


#endif /* __BSP_LED_DRIVER_H__ */