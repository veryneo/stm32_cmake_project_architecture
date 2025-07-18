#ifndef __SYSTEM_H__
#define __SYSTEM_H__


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
    E_SYSTEM_RET_STATUS_OK = 0,
    E_SYSTEM_RET_STATUS_ERROR,
} E_SYSTEM_RET_STATUS_T;


/*==============================================================================
 * External Function Declaration  
 *============================================================================*/

extern E_SYSTEM_RET_STATUS_T system_init(void);
extern E_SYSTEM_RET_STATUS_T system_start(void);


#endif /* __SYSTEM_H__ */