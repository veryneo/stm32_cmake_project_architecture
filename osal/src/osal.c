/*==============================================================================
 * Include
 *============================================================================*/

#include "osal.h"

#include "cmsis_os2.h"

#include "stdint.h"


/*==============================================================================
 * Static Function Definition
 *============================================================================*/

static inline uint32_t _osal_ms_to_os_tick(const uint32_t delay_ms);
static inline E_OSAL_RET_STATUS_T _osal_priority_osal_to_cmsis(const E_OSAL_THREAD_PRIORITY_T osal_priority, osPriority_t* const p_cmsis_priority);


/*==============================================================================
 * External Function
 *============================================================================*/

extern E_OSAL_RET_STATUS_T osal_kernel_init(void)
{
    osStatus_t status = osKernelInitialize();
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_kernel_start(void)
{
    osStatus_t status = osKernelStart();
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_thread_create(void** const pp_thread_handle, 
                                              const S_OSAL_THREAD_CONFIG_T* const p_thread_config)
{
    /* Check input parameters */
    if (NULL == pp_thread_handle || NULL == p_thread_config)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Convert OSAL priority to CMSIS priority */
    osPriority_t cmsis_priority;
    E_OSAL_RET_STATUS_T ret_status = _osal_priority_osal_to_cmsis(p_thread_config->priority, &cmsis_priority);
    if (E_OSAL_RET_STATUS_OK != ret_status)
    {
        return ret_status;
    }

    /* Define cmsis thread attributes */
    osThreadAttr_t cmsis_thread_attr = 
    {
        .name = p_thread_config->p_name,            /* Thread name */
        .attr_bits = osThreadDetached,              /* Attribute bits */
        .cb_mem = NULL,                             /* Control block memory (use dynamic allocation) */
        .cb_size = 0,                               /* Control block size */
        .stack_mem = NULL,                          /* Stack memory (use dynamic allocation) */
        .stack_size = p_thread_config->stack_size,  /* Stack size (bytes) */
        .priority = cmsis_priority,                 /* Mapped priority */
        .tz_module = 0,                             /* TrustZone module ID */
        .reserved = 0                               /* Reserved field */
    };

    /* Create thread */
    osThreadId_t cmsis_thread_id = osThreadNew( (osThreadFunc_t)p_thread_config->p_entry, p_thread_config->p_arg, &cmsis_thread_attr);
    if (NULL == cmsis_thread_id)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    /* Return thread handle */
    *pp_thread_handle = (void*)cmsis_thread_id;

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_delay_ms(const uint32_t delay_ms)
 {
    osDelay(_osal_ms_to_os_tick(delay_ms) );

    return E_OSAL_RET_STATUS_OK;    
}

extern E_OSAL_RET_STATUS_T osal_mutex_create(void** const pp_mutex_handle)
{
    /* Check input parameter */
    if (NULL == pp_mutex_handle)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Create mutex */
    *pp_mutex_handle = osMutexNew(NULL);
    if (NULL == *pp_mutex_handle)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_mutex_lock(void* const p_mutex_handle)
{
    /* Check input parameter */
    if (NULL == p_mutex_handle)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Lock mutex */
    osStatus_t status = osMutexAcquire( (osMutexId_t)p_mutex_handle, osWaitForever);
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_mutex_unlock(void* const p_mutex_handle)
{
    /* Check input parameter */
    if (NULL == p_mutex_handle)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Unlock mutex */
    osStatus_t status = osMutexRelease( (osMutexId_t)p_mutex_handle);
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_mutex_delete(void* const p_mutex_handle)
{
    /* Check input parameter */
    if (NULL == p_mutex_handle)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Delete mutex */
    osStatus_t status = osMutexDelete( (osMutexId_t)p_mutex_handle);
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_queue_create(const uint32_t item_num, 
                                                            const uint32_t item_size, 
                                                            void ** const pp_queue_handle)
{
    /* Check input parameters */
    if (NULL == pp_queue_handle || 0 == item_num || 0 == item_size)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Create queue */    
    *pp_queue_handle = osMessageQueueNew(item_num, item_size, NULL);
    if (NULL == *pp_queue_handle)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_queue_send(void* const p_queue_handle, const void* const p_item, const uint32_t timeout_ms)
{
    /* Check input parameters */
    if (NULL == p_queue_handle || NULL == p_item)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Send item to queue */
    uint32_t timeout_ticks = _osal_ms_to_os_tick(timeout_ms);
    osStatus_t status = osMessageQueuePut( (osMessageQueueId_t)p_queue_handle, p_item, 0, timeout_ticks);
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_queue_receive(void* const p_queue_handle, void* const p_item, const uint32_t timeout_ms)
{
    /* Check input parameters */
    if (NULL == p_queue_handle || NULL == p_item)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Receive item from queue */
    uint32_t timeout_ticks = _osal_ms_to_os_tick(timeout_ms);
    osStatus_t status = osMessageQueueGet( (osMessageQueueId_t)p_queue_handle, p_item, NULL, timeout_ticks);
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_queue_delete(void* const p_queue_handle)
{
    /* Check input parameter */
    if (NULL == p_queue_handle)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Delete queue */
    osStatus_t status = osMessageQueueDelete( (osMessageQueueId_t)p_queue_handle);
    if (osOK != status)
    {
        return E_OSAL_RET_STATUS_RESOURCE_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}

extern E_OSAL_RET_STATUS_T osal_queue_space_get(void* const p_queue_handle, uint32_t* const p_space)
{
    /* Check input parameters */
    if (NULL == p_queue_handle || NULL == p_space)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Get queue space */
    uint32_t space = osMessageQueueGetSpace( (osMessageQueueId_t)p_queue_handle);

    /* Return the space */
    *p_space = space;

    return E_OSAL_RET_STATUS_OK;
}

/*==============================================================================
 * Static Function Implementation
 *============================================================================*/

static inline uint32_t _osal_ms_to_os_tick(const uint32_t delay_ms)
{
    /* Convert milliseconds to OS ticks */
    return (delay_ms * osKernelGetTickFreq()) / 1000;
}

static inline E_OSAL_RET_STATUS_T _osal_priority_osal_to_cmsis(const E_OSAL_THREAD_PRIORITY_T osal_priority, osPriority_t* const p_cmsis_priority)
{
    /* Check input parameters */
    if (NULL == p_cmsis_priority)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    if (0 > osal_priority || E_OSAL_THREAD_PRIORITY_NUM_MAX <= osal_priority)
    {
        return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    /* Convert OSAL priority to CMSIS priority */
    switch (osal_priority)
    {
        case E_OSAL_THREAD_PRIORITY_EMERGENCY:
            *p_cmsis_priority = osPriorityRealtime7;
            break;

        case E_OSAL_THREAD_PRIORITY_HARD_REALTIME:
            *p_cmsis_priority = osPriorityRealtime4;
            break;

        case E_OSAL_THREAD_PRIORITY_SOFT_REALTIME:
            *p_cmsis_priority = osPriorityRealtime1;
            break;

        case E_OSAL_THREAD_PRIORITY_NORMAL:
            *p_cmsis_priority = osPriorityNormal;
            break;

        case E_OSAL_THREAD_PRIORITY_BACKGROUND:
            *p_cmsis_priority = osPriorityLow;
            break;

        default:
            return E_OSAL_RET_STATUS_INPUT_PARAM_ERROR;
    }

    return E_OSAL_RET_STATUS_OK;
}
 