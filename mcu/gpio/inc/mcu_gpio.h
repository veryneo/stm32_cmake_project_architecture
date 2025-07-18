#ifndef __MCU_GPIO_H__
#define __MCU_GPIO_H__


/*==============================================================================
 * Include
 *============================================================================*/

#include "stdint.h"


/*==============================================================================
 * Enum
 *============================================================================*/

 /**
  * @brief GPIO return status enum
  */
 typedef enum
 {
    E_MCU_GPIO_RET_STATUS_OK = 0,
    E_MCU_GPIO_RET_STATUS_INPUT_PARAM_ERR, 
 } E_MCU_GPIO_RET_STATUS_T;

 /**
  * @brief GPIO pin state enum
  */
 typedef enum
 {
    E_MCU_GPIO_PIN_STATE_RESET = 0,
    E_MCU_GPIO_PIN_STATE_SET,
 } E_MCU_GPIO_PIN_STATE_T;

 /**
  * @brief GPIO pin enum
  */
typedef enum
{
    E_MCU_GPIO_PIN_LED_0 = 0,
    E_MCU_GPIO_PIN_LED_1,
    E_MCU_GPIO_PIN_LED_2,
    E_MCU_GPIO_PIN_NUM_MAX
} E_MCU_GPIO_PIN_T;


/*==============================================================================
 * External Function
 *============================================================================*/

extern void mcu_gpio_init(void);
extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_write_pin(const E_MCU_GPIO_PIN_T gpio_pin, const E_MCU_GPIO_PIN_STATE_T pin_state);
extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_read_pin(const E_MCU_GPIO_PIN_T gpio_pin, E_MCU_GPIO_PIN_STATE_T* const pin_state);
extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_toggle_pin(const E_MCU_GPIO_PIN_T gpio_pin); 


#endif /* __MCU_GPIO_H__ */