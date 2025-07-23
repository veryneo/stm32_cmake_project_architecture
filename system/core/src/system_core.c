#include "system_core.h"

#include "app_test.h"

#include "bsp_led_adapter.h"

#include "osal.h"

#include "mcu.h"

#include "stddef.h"

#define D_SYSTEM_CORE_THREAD_STACK_SIZE_BSP_LED (2048)
#define D_SYSTEM_CORE_THREAD_STACK_SIZE_APP_TEST (2048)


/*==============================================================================
 * Static Variable
 *============================================================================*/

 /**
  * @brief System thread configuration
  */
static S_OSAL_THREAD_CONFIG_T gs_system_thread_configs[] = {
    /* BSP LED */
    {
        .p_name     =   "BSP LED",
        .p_entry    =   NULL,
        .p_arg      =   NULL,
        .stack_size =   D_SYSTEM_CORE_THREAD_STACK_SIZE_BSP_LED,
        .priority   =   E_OSAL_THREAD_PRIORITY_HARD_REALTIME,
    },
    /* APP Test */
    {
        .p_name     =   "APP Test",
        .p_entry    =   app_test_thread,
        .p_arg      =   NULL,
        .stack_size =   D_SYSTEM_CORE_THREAD_STACK_SIZE_APP_TEST,
        .priority   =   E_OSAL_THREAD_PRIORITY_SOFT_REALTIME,
    }
};


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

/**
 * @brief Initialize OS kernel, get thread entry and create thread
 * @return E_SYSTEM_CORE_RET_STATUS_T
 */extern E_SYSTEM_CORE_RET_STATUS_T system_core_init(void)
{
    /* Initialize kernel */
    if (E_OSAL_RET_STATUS_OK != osal_kernel_init())
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    /* Initialize BSP layer */
    if (E_LED_ADAPTER_RET_STATUS_OK != led_adapter_init() )
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    /* Get thread entry */
    gs_system_thread_configs[0].p_entry = led_adapter_thread_entry_get();

    /* Initialize thread handle */
    void* p_thread_handle_bsp_led = NULL;
    void* p_thread_handle_app_test = NULL;

    /* Create thread */
    if (E_OSAL_RET_STATUS_OK != osal_thread_create(&p_thread_handle_bsp_led, &gs_system_thread_configs[0]) )
    {
       return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    if (E_OSAL_RET_STATUS_OK != osal_thread_create(&p_thread_handle_app_test, &gs_system_thread_configs[1]) )
    {
       return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    return E_SYSTEM_CORE_RET_STATUS_OK;
}

/**
 * @brief Start OS kernel
 * @return E_SYSTEM_CORE_RET_STATUS_T
 */
extern E_SYSTEM_CORE_RET_STATUS_T system_core_start(void)
{
    /* Start kernel */
    if (E_OSAL_RET_STATUS_OK != osal_kernel_start() )
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    return E_SYSTEM_CORE_RET_STATUS_OK;
}









