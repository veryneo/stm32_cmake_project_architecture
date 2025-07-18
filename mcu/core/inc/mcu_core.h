
#ifndef __MCU_CORE_H__
#define __MCU_CORE_H__

typedef enum
{
    E_MCU_CORE_RET_STATUS_OK = 0,
    E_MCU_CORE_RET_STATUS_ERROR,
} E_MCU_CORE_RET_STATUS_T;

extern E_MCU_CORE_RET_STATUS_T mcu_core_init(void);

#endif /* __MCU_CORE_H__ */