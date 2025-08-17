#include "app_test.h"
#include "app_test_port.h"

#include "osal.h"

E_APP_TEST_RET_STATUS_T app_test_init(void)
{
    return E_APP_TEST_RET_STATUS_OK;
}

extern void app_test_thread(void* argument)
{

    (void)argument;

    app_test_func();

    osal_delay_ms(5000);

    app_test_func_2();

    osal_delay_ms(5000);

    app_test_func_3();

    while (1)
    {

    }
}
