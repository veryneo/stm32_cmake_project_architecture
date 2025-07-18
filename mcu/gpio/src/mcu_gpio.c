/*==============================================================================
 * Include
 *============================================================================*/

#include "mcu_gpio.h"

#include "stm32wbxx_hal.h"


/*==============================================================================
 * Macro
 *============================================================================*/

 #define D_MCU_GPIO_PIN_CONF_LIST \
    X(LED_0, GPIOB, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, __HAL_RCC_GPIOB_CLK_ENABLE, __HAL_RCC_GPIOB_CLK_DISABLE) \
    X(LED_1, GPIOB, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, __HAL_RCC_GPIOB_CLK_ENABLE, __HAL_RCC_GPIOB_CLK_DISABLE) \
    X(LED_2, GPIOB, GPIO_PIN_1, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, __HAL_RCC_GPIOB_CLK_ENABLE, __HAL_RCC_GPIOB_CLK_DISABLE)

/* Count the number of items in LED GPIO configuration list */
#define D_MCU_GPIO_PIN_CONF_COUNT(...)  (+1) 
#define X(...)  D_MCU_GPIO_PIN_CONF_COUNT(__VA_ARGS__),  /* Don't remove the comma */
#define D_MCU_GPIO_PIN_CONF_NUM_ACTUAL (sizeof((uint8_t[]){ D_MCU_GPIO_PIN_CONF_LIST }) / sizeof(uint8_t))
#undef  X

/* Compile-time check: Ensure the number of items in LED GPIO configuration list is equal to LED GPIO configuration maximum value */
/* Each enumeration value of E_MCU_GPIO_PIN_T must have a corresponding GPIO PIN configuration */
_Static_assert(E_MCU_GPIO_PIN_NUM_MAX == D_MCU_GPIO_PIN_CONF_NUM_ACTUAL,
    "LED GPIO configuration count mismatch! Check D_MCU_GPIO_PIN_CONF_LIST and E_MCU_GPIO_PIN_NUM_MAX"
);

/* Compile-time check: Ensure the pin state enum is equal to HAL pin state enum */
_Static_assert(E_MCU_GPIO_PIN_STATE_RESET == GPIO_PIN_RESET, "Pin state RESET mismatch!");
_Static_assert(E_MCU_GPIO_PIN_STATE_SET == GPIO_PIN_SET, "Pin state SET mismatch!");


 /*==============================================================================
 * Structure
 *============================================================================*/

typedef struct
{
    const char* name;
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t mode;
    uint32_t pull;
    uint32_t speed;
    void (*pf_clk_enable)(void);
    void (*pf_clk_disable)(void);
} S_MCU_GPIO_PIN_CONFIG_T;

static const S_MCU_GPIO_PIN_CONFIG_T gs_mcu_gpio_pin_config[D_MCU_GPIO_PIN_CONF_NUM_MAX] = 
{
#define X(name, port, pin, mode, pull, speed, clk_en, clk_dis) \
    [E_MCU_GPIO_PIN_##name] = { \
        .name = name, \
        .port = port, \
        .pin = pin, \
        .mode = mode, \
        .pull = pull, \
        .speed = speed, \
        .pf_clk_enable = clk_en, \
        .pf_clk_disable = clk_dis \
    },
    D_MCU_GPIO_PIN_CONF_LIST
#undef X
};


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

extern void mcu_gpio_init(void)
{
    for (uint32_t i = 0; i < E_MCU_GPIO_PIN_NUM_MAX; i++)
    {
        gs_mcu_gpio_pin_config[i].pf_clk_enable();

        GPIO_InitTypeDef gpio_initStruct = {0};
        gpio_initStruct.Pin    =   gs_mcu_gpio_pin_config[i].pin;
        gpio_initStruct.Mode   =   gs_mcu_gpio_pin_config[i].mode;
        gpio_initStruct.Pull   =   gs_mcu_gpio_pin_config[i].pull;
        gpio_initStruct.Speed  =   gs_mcu_gpio_pin_config[i].speed;
        HAL_GPIO_Init(gs_mcu_gpio_pin_config[i].port, &gpio_initStruct);
    }
}

extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_write_pin(const E_MCU_GPIO_PIN_T gpio_pin, const E_MCU_GPIO_PIN_STATE_T pin_state)
{
    if (0 > gpio_pin || E_MCU_GPIO_PIN_NUM_MAX <= gpio_pin)
    {
        return E_MCU_GPIO_RET_STATUS_INPUT_PARAM_ERR;
    }

    HAL_GPIO_WritePin(gs_mcu_gpio_pin_config[gpio_pin].port, gs_mcu_gpio_pin_config[gpio_pin].pin, (GPIO_PinState)pin_state);

    return E_MCU_GPIO_RET_STATUS_OK;
}

extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_read_pin(const E_MCU_GPIO_PIN_T gpio_pin, E_MCU_GPIO_PIN_STATE_T* const pin_state)
{
    if (0 > gpio_pin || E_MCU_GPIO_PIN_NUM_MAX <= gpio_pin)
    {
        return E_MCU_GPIO_RET_STATUS_INPUT_PARAM_ERR;
    }

    *pin_state = (E_MCU_GPIO_PIN_STATE_T)HAL_GPIO_ReadPin(gs_mcu_gpio_pin_config[gpio_pin].port, gs_mcu_gpio_pin_config[gpio_pin].pin);

    return E_MCU_GPIO_RET_STATUS_OK;
}

extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_toggle_pin(const E_MCU_GPIO_PIN_T gpio_pin)
{
    if (0 > gpio_pin || E_MCU_GPIO_PIN_NUM_MAX <= gpio_pin)
    {
        return E_MCU_GPIO_RET_STATUS_INPUT_PARAM_ERR;
    }

    HAL_GPIO_TogglePin(gs_mcu_gpio_pin_config[gpio_pin].port, gs_mcu_gpio_pin_config[gpio_pin].pin);

    return E_MCU_GPIO_RET_STATUS_OK;
}