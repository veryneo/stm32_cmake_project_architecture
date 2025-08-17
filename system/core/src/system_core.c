#include "system_core.h"

#include "app_test.h"
#include "app_shell.h"

#include "bsp_led_adapter.h"
#include "bsp_serialport_adapter.h"

#include "osal.h"

#include "mcu.h"

#include "stddef.h"


#define D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_BSP_LED         (2048)
#define D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_BSP_SERIALPORT  (2048)
#define D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_APP_TEST        (2048)
#define D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_APP_SHELL       (2048)


typedef enum
{
    E_SYSTEM_CORE_OS_THREAD_ID_BSP_LED,
    E_SYSTEM_CORE_OS_THREAD_ID_BSP_SERIALPORT,
    E_SYSTEM_CORE_OS_THREAD_ID_APP_TEST,
    E_SYSTEM_CORE_OS_THREAD_ID_APP_SHELL,
    E_SYSTEM_CORE_OS_THREAD_ID_NUM_MAX,
} E_SYSTEM_CORE_OS_THREAD_ID_T;


/*==============================================================================
 * Static Variable
 *============================================================================*/

 /**
  * @brief System thread configuration
  */
static S_OSAL_THREAD_CONFIG_T gs_system_os_thread_conf[] = {
    /* BSP LED */
    [E_SYSTEM_CORE_OS_THREAD_ID_BSP_LED] = {
        .p_name     =   "BSP LED",
        .p_entry    =   NULL,
        .p_arg      =   NULL,
        .stack_size =   D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_BSP_LED,
        .priority   =   E_OSAL_THREAD_PRIORITY_HARD_REALTIME,
    },
    /* BSP Serialport */
    [E_SYSTEM_CORE_OS_THREAD_ID_BSP_SERIALPORT] = {
        .p_name     =   "BSP Serialport",
        .p_entry    =   NULL,
        .p_arg      =   NULL,
        .stack_size =   D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_BSP_SERIALPORT,
        .priority   =   E_OSAL_THREAD_PRIORITY_HARD_REALTIME,
    },
    /* APP Test */
    [E_SYSTEM_CORE_OS_THREAD_ID_APP_TEST] = {
        .p_name     =   "APP Test",
        .p_entry    =   app_test_thread,
        .p_arg      =   NULL,
        .stack_size =   D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_APP_TEST,
        .priority   =   E_OSAL_THREAD_PRIORITY_SOFT_REALTIME,
    },
    /* APP Shell */
    [E_SYSTEM_CORE_OS_THREAD_ID_APP_SHELL] = {
        .p_name     =   "APP Shell",
        .p_entry    =   app_shell_thread,
        .p_arg      =   NULL,
        .stack_size =   D_SYSTEM_CORE_OS_THREAD_STACK_SIZE_APP_SHELL,
    }
};


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

/**
 * @brief   OS kernel, APP and BSP initialization
 * @return  E_SYSTEM_CORE_RET_STATUS_T
 */extern E_SYSTEM_CORE_RET_STATUS_T system_core_init(void)
{
    /* 1. Initialize kernel */
    if (E_OSAL_RET_STATUS_OK != osal_kernel_init())
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    /* 2. Initialize BSP layer */

    /* 2.1. Initialize BSP LED and get thread entry */
    if (E_LED_ADAPTER_RET_STATUS_OK != led_adapter_init() )
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_BSP_LED].p_entry = led_adapter_thread_entry_get();
    if (NULL == gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_BSP_LED].p_entry)
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    /* 2.2 Initialize BSP Serialport and get thread entry */
    if (E_SERIALPORT_ADAPTER_RET_STATUS_OK != serialport_adapter_init() )
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_BSP_SERIALPORT].p_entry = serialport_adapter_thread_entry_get();
    if (NULL == gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_BSP_SERIALPORT].p_entry)
    {
        return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    /* 3. Initialize APP layer */

    /* 3.1 Initialize APP Test */
    app_test_init();

    /* 3.2. Initialize APP Shell */
    app_shell_init();

    /* 4. Create os thread */
    void* p_thread_handle_bsp_led           = NULL;
    void* p_thread_handle_bsp_serialport    = NULL;
    void* p_thread_handle_app_test          = NULL;
    void* p_thread_handle_app_shell         = NULL;

    if (E_OSAL_RET_STATUS_OK != osal_thread_create(&p_thread_handle_bsp_led, &gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_BSP_LED]) )
    {
       return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    if (E_OSAL_RET_STATUS_OK != osal_thread_create(&p_thread_handle_bsp_serialport, &gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_BSP_SERIALPORT]) )
    {
       return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    if (E_OSAL_RET_STATUS_OK != osal_thread_create(&p_thread_handle_app_test, &gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_APP_TEST]) )
    {
       return E_SYSTEM_CORE_RET_STATUS_ERROR;
    }

    if (E_OSAL_RET_STATUS_OK != osal_thread_create(&p_thread_handle_app_shell, &gs_system_os_thread_conf[E_SYSTEM_CORE_OS_THREAD_ID_APP_SHELL]) )
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









