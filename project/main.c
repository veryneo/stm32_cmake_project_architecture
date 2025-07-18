#include "system.h"

#include "mcu.h"

int main(void)
{
    mcu_core_init();
    mcu_gpio_init();
    
    system_init();
    system_start();
}