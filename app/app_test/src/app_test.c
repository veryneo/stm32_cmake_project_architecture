#include "app_test.h"

#include "osal.h"

extern void app_test_thread(void* argument)
{
    osal_delay_ms(1000);
    
    while (1)
    {
        app_test_func();
        osal_delay_ms(1000);
    }
}