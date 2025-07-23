#include "mcu_core.h"

#include "stm32wbxx_hal.h"


static E_MCU_CORE_RET_STATUS_T _mcu_core_system_clock_config(void);

extern E_MCU_CORE_RET_STATUS_T mcu_core_init(void)
{
    HAL_Init();

    return _mcu_core_system_clock_config();    
}

static E_MCU_CORE_RET_STATUS_T _mcu_core_system_clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure the main internal regulator output voltage */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**
     * Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 32;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV5;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
    if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct))
    {
        return E_MCU_CORE_RET_STATUS_ERROR;
    }

    /* Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                                |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

    if (HAL_OK != HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3))
    {
        return E_MCU_CORE_RET_STATUS_ERROR;
    }

    return E_MCU_CORE_RET_STATUS_OK;
}

extern void NMI_Handler(void)
{

}

extern void HardFault_Handler(void)
{
    while (1)
    {

    }
}

extern void MemManage_Handler(void)
{
    while (1)
    {

    }
}

// extern void SVC_Handler(void)
// {

// }

extern void DebugMon_Handler(void)
{

}

// extern void PendSV_Handler(void)
// {

// }

// extern void SysTick_Handler(void)
// {
//     HAL_IncTick();
// }