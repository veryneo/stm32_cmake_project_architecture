#ifndef __OSAL_EXTENSION_H__
#define __OSAL_EXTENSION_H__


#include "stdint.h"


extern void osal_critical_enter(void);
extern void osal_critical_exit(void);

extern void* osal_mem_malloc(uint32_t);
extern void osal_mem_free(void*);

#endif /* __OSAL_EXTENSION_H__ */