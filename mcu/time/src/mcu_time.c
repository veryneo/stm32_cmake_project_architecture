/*==============================================================================
 * Include
 *============================================================================*/

#include "mcu_time.h"

#include "stm32wbxx_hal.h"


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

extern uint32_t mcu_time_tick_get(void)
{
    return HAL_GetTick();
}

extern void mcu_time_delay_ms(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}