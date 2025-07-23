#include "app_test.h"
#include "app_test_port.h"

#include "osal.h"

extern void app_test_thread(void* argument)
{

    (void)argument;

    osal_delay_ms(1000);

    app_test_func();

    osal_delay_ms(10000);

    app_test_func_2();

    while (1)
    {

    }
}
