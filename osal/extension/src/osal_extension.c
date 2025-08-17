#include "osal_extension.h"

#include "stdint.h"

#ifdef OSAL_EX_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

extern void osal_critical_enter(void)
{   
#ifdef OSAL_EX_FREERTOS
    taskENTER_CRITICAL();
#endif
}

extern void osal_critical_exit(void)
{
#ifdef OSAL_EX_FREERTOS
    taskEXIT_CRITICAL();
#endif
}

extern void* osal_mem_malloc(uint32_t size)
{
#ifdef OSAL_EX_FREERTOS
    return pvPortMalloc(size);
#endif
}

extern void osal_mem_free(void* ptr)
{
#ifdef OSAL_EX_FREERTOS
    vPortFree(ptr);
#endif
}

