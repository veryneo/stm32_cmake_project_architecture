/*==============================================================================
 * Include
 *============================================================================*/

#include "mcu_gpio.h"

#include "stm32wbxx_hal.h"

#include "assert.h"

/*==============================================================================
 * Static Function Declaration
 *============================================================================*/

 static void _mcu_gpio_port_b_clk_enable(void);


/*==============================================================================
 * Macro
 *============================================================================*/

 #define D_MCU_GPIO_PIN_CONF_LIST \
    X(LED_0, GPIOB, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, _mcu_gpio_port_b_clk_enable) \
    X(LED_1, GPIOB, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, _mcu_gpio_port_b_clk_enable) \
    X(LED_2, GPIOB, GPIO_PIN_1, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, _mcu_gpio_port_b_clk_enable)


/*==============================================================================
 * Enum
 *============================================================================*/

typedef enum
{
#define X(gpio_name, ...) E_MCU_GPIO_PIN_ACTUAL_##gpio_name,
    D_MCU_GPIO_PIN_CONF_LIST
#undef X
    E_MCU_GPIO_PIN_ACTUAL_NUM_MAX
} E_MCU_GPIO_PIN_ACTUAL_T;

/* Compile-time check: Ensure the number of items in LED GPIO configuration list is equal to LED GPIO configuration maximum value */
/* Each enumeration value of E_MCU_GPIO_PIN_T must have a corresponding GPIO PIN configuration */
static_assert((uint8_t)E_MCU_GPIO_PIN_NUM_MAX == (uint8_t)E_MCU_GPIO_PIN_ACTUAL_NUM_MAX,
    "LED GPIO configuration count mismatch! Check D_MCU_GPIO_PIN_CONF_LIST and E_MCU_GPIO_PIN_NUM_MAX"
);

/* Compile-time check: Ensure the pin state enum is equal to HAL pin state enum */
static_assert((uint8_t)E_MCU_GPIO_PIN_STATE_RESET == (uint8_t)GPIO_PIN_RESET, "Pin state RESET mismatch!");
static_assert((uint8_t)E_MCU_GPIO_PIN_STATE_SET == (uint8_t)GPIO_PIN_SET, "Pin state SET mismatch!");


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
} S_MCU_GPIO_PIN_CONFIG_T;

static const S_MCU_GPIO_PIN_CONFIG_T gs_mcu_gpio_pin_config[E_MCU_GPIO_PIN_NUM_MAX] = 
{
#define X(gpio_name, gpio_port, gpio_pin, gpio_mode, gpio_pull, gpio_speed, gpio_clk_enable) \
    [E_MCU_GPIO_PIN_##gpio_name] =  \
    {                               \
        .name   =   #gpio_name,     \
        .port   =   gpio_port,      \
        .pin    =   gpio_pin,       \
        .mode   =   gpio_mode,      \
        .pull   =   gpio_pull,      \
        .speed  =   gpio_speed,     \
        .pf_clk_enable = gpio_clk_enable, \
    },
    D_MCU_GPIO_PIN_CONF_LIST
#undef X
};


/*==============================================================================
 * External Function Implementation
 *============================================================================*/

extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_init(void)
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

    return E_MCU_GPIO_RET_STATUS_OK;
}

extern E_MCU_GPIO_RET_STATUS_T mcu_gpio_deinit(void)
{
    for (uint32_t i = 0; i < E_MCU_GPIO_PIN_NUM_MAX; i++)
    {
        HAL_GPIO_DeInit(gs_mcu_gpio_pin_config[i].port, gs_mcu_gpio_pin_config[i].pin);
    }

    return E_MCU_GPIO_RET_STATUS_OK;
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

/*==============================================================================
 * Static Function Implementation
 *============================================================================*/

static void _mcu_gpio_port_b_clk_enable(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

