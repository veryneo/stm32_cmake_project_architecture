#include "app_test_port.h"

#include "bsp_led_adapter.h"


extern void app_test_func(void)
{
    E_LED_ADAPTER_RET_STATUS_T ret_status_led_adp = E_LED_ADAPTER_RET_STATUS_OK;

    ret_status_led_adp = led_adapter_led_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_BLUE, E_LED_ADAPTER_DISP_PATTERN_TYPE_BLINK_SLOW);
    if (E_LED_ADAPTER_RET_STATUS_OK != ret_status_led_adp)
    {
        (void)ret_status_led_adp;
        return;
    }

    return; 
}









