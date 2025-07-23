/*==============================================================================
 * Include
 *============================================================================*/

#include "mcu_time.h"

#include "stm32wbxx_hal.h"


/*==============================================================================
 * Macro
 *============================================================================*/

 #define D_MCU_TIME_TIMEBASE_INSTANCE         TIM17
 #define D_MCU_TIME_TIMEBASE_CLK_ENABLE()     __HAL_RCC_TIM17_CLK_ENABLE()
 #define D_MCU_TIME_TIMEBASE_CLK_DISABLE()    __HAL_RCC_TIM17_CLK_DISABLE()
 #define D_MCU_TIME_TIMEBASE_IRQ_NUM          TIM1_TRG_COM_TIM17_IRQn


/*==============================================================================
 * Variable
 *============================================================================*/

static TIM_HandleTypeDef gs_mcu_time_timbase_handle;


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

extern HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              uwTimclock = 0;
    uint32_t              uwPrescalerValue = 0;
    uint32_t              pFLatency;

    /* Configure the TIM17 IRQ priority */
    HAL_NVIC_SetPriority(D_MCU_TIME_TIMEBASE_IRQ_NUM, TickPriority ,0);
    /* Enable the TIM17 global Interrupt */
    HAL_NVIC_EnableIRQ(D_MCU_TIME_TIMEBASE_IRQ_NUM);

    /* Enable TIM17 clock */
    D_MCU_TIME_TIMEBASE_CLK_ENABLE();
    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
    /* Compute TIM17 clock */
    uwTimclock = HAL_RCC_GetPCLK2Freq();

    /* Compute the prescaler value to have TIM17 counter clock equal to 1MHz */
    uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

    /* Initialize TIM17 */
    gs_mcu_time_timbase_handle.Instance = D_MCU_TIME_TIMEBASE_INSTANCE;

    /* Initialize TIMx peripheral as follow:

    + Period = [(TIM17CLK/1000) - 1]. to have a (1/1000) s time base.
    + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
    + ClockDivision = 0
    + Counter direction = Up
    */
    gs_mcu_time_timbase_handle.Init.Period          =   (1000000U / 1000U) - 1U;
    gs_mcu_time_timbase_handle.Init.Prescaler       =   uwPrescalerValue;
    gs_mcu_time_timbase_handle.Init.ClockDivision   =   0;
    gs_mcu_time_timbase_handle.Init.CounterMode     =   TIM_COUNTERMODE_UP;

    if (HAL_TIM_Base_Init(&gs_mcu_time_timbase_handle) == HAL_OK)
    {
        /* Start the TIM time Base generation in interrupt mode */
        return HAL_TIM_Base_Start_IT(&gs_mcu_time_timbase_handle);
    }

    /* Return function status */
    return HAL_ERROR;
}

/**
 * @brief  Suspend Tick increment.
 * @note   Disable the tick increment by disabling TIM17 update interrupt.
 * @param  None
 * @retval None
 */
extern void HAL_SuspendTick(void)
{
    /* Disable TIM17 update Interrupt */
    __HAL_TIM_DISABLE_IT(&gs_mcu_time_timbase_handle, TIM_IT_UPDATE);
}

/**
 * @brief  Resume Tick increment.
 * @note   Enable the tick increment by Enabling TIM17 update interrupt.
 * @param  None
 * @retval None
 */
extern void HAL_ResumeTick(void)
{
    /* Enable TIM17 Update interrupt */
    __HAL_TIM_ENABLE_IT(&gs_mcu_time_timbase_handle, TIM_IT_UPDATE);
}

extern void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&gs_mcu_time_timbase_handle);
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM17 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
extern void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == gs_mcu_time_timbase_handle.Instance)
    {
        HAL_IncTick();
    }
}
