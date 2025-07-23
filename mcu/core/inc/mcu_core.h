
#ifndef __MCU_CORE_H__
#define __MCU_CORE_H__

typedef enum
{
    E_MCU_CORE_RET_STATUS_OK = 0,
    E_MCU_CORE_RET_STATUS_ERROR,
} E_MCU_CORE_RET_STATUS_T;

extern E_MCU_CORE_RET_STATUS_T mcu_core_init(void);

extern void NMI_Handler(void);
extern void HardFault_Handler(void);
extern void MemManage_Handler(void);
extern void SVC_Handler(void);
extern void DebugMon_Handler(void);
extern void PendSV_Handler(void);
extern void SysTick_Handler(void);

#endif /* __MCU_CORE_H__ */
