#include "system_core.h"

#include "mcu.h"

int main(void)
{
    /* Hardware */
    mcu_core_init();
    mcu_gpio_init();
    mcu_uart_init();
    
    /* Software */
    system_core_init();
    system_core_start();
}
