#ifndef __SYSTEM_CORE_H__
#define __SYSTEM_CORE_H__


/*==============================================================================
 * Include
 *============================================================================*/

#include <stdint.h>


/*==============================================================================
 * Enumeration
 *============================================================================*/

/* 系统管理器返回状态 */
typedef enum
{
    E_SYSTEM_CORE_RET_STATUS_OK = 0,
    E_SYSTEM_CORE_RET_STATUS_ERROR,
} E_SYSTEM_CORE_RET_STATUS_T;


/*==============================================================================
 * External Function Declaration  
 *============================================================================*/

extern E_SYSTEM_CORE_RET_STATUS_T system_core_init(void);
extern E_SYSTEM_CORE_RET_STATUS_T system_core_start(void);


#endif /* __SYSTEM_CORE_H__ */