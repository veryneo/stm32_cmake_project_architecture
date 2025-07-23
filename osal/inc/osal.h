#ifndef __OSAL_H__
#define __OSAL_H__


/*==============================================================================
 * Include
 *============================================================================*/

#include "stdint.h"


/*==============================================================================
 * Enum
 *============================================================================*/

typedef enum
{
    E_OSAL_RET_STATUS_OK,
    E_OSAL_RET_STATUS_INPUT_PARAM_ERROR,
    E_OSAL_RET_STATUS_RESOURCE_ERROR,
} E_OSAL_RET_STATUS_T;

typedef enum
{
    E_OSAL_THREAD_PRIORITY_EMERGENCY,     /* Emergency */
    E_OSAL_THREAD_PRIORITY_HARD_REALTIME, /* Hard realtime (Such as sensor sampling) */
    E_OSAL_THREAD_PRIORITY_SOFT_REALTIME, /* Soft realtime (Such as protocol parsing) */
    E_OSAL_THREAD_PRIORITY_NORMAL,        /* Normal */
    E_OSAL_THREAD_PRIORITY_BACKGROUND,    /* Background (Such as log upload) */
    E_OSAL_THREAD_PRIORITY_NUM_MAX,
} E_OSAL_THREAD_PRIORITY_T;

typedef struct 
{
    const char*                 p_name;
    void                       (*p_entry)(void*); 
    void*                       p_arg;
    uint32_t                    stack_size;
    E_OSAL_THREAD_PRIORITY_T    priority;
} S_OSAL_THREAD_CONFIG_T;


/*==============================================================================
 * External Function Declaration
 *============================================================================*/

 extern E_OSAL_RET_STATUS_T osal_kernel_init(void);
 extern E_OSAL_RET_STATUS_T osal_kernel_start(void);

extern E_OSAL_RET_STATUS_T osal_thread_create(void** const pp_thread_handle, 
                                              const S_OSAL_THREAD_CONFIG_T* const p_thread_config);

extern E_OSAL_RET_STATUS_T osal_delay_ms(const uint32_t delay_ms);

extern E_OSAL_RET_STATUS_T osal_mutex_create(void** const pp_mutex_handle);
extern E_OSAL_RET_STATUS_T osal_mutex_lock(void* const p_mutex_handle);
extern E_OSAL_RET_STATUS_T osal_mutex_unlock(void* const p_mutex_handle);
extern E_OSAL_RET_STATUS_T osal_mutex_delete(void* const p_mutex_handle);

extern E_OSAL_RET_STATUS_T osal_queue_create(uint32_t const item_num, uint32_t const item_size, void ** const pp_queue_handle);
extern E_OSAL_RET_STATUS_T osal_queue_send(void* const p_queue_handle, const void* const p_item, const uint32_t timeout_ms);
extern E_OSAL_RET_STATUS_T osal_queue_receive(void* const p_queue_handle, void* const p_item, const uint32_t timeout_ms);
extern E_OSAL_RET_STATUS_T osal_queue_delete(void* const p_queue_handle);
extern E_OSAL_RET_STATUS_T osal_queue_space_get(void* const p_queue_handle, uint32_t* const p_space);


#endif /* __OSAL_H__ */