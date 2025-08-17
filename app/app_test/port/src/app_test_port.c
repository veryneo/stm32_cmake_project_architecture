#include "app_test_port.h"

#include "bsp_led_adapter.h"


extern void app_test_func(void)
{
    E_LED_ADAPTER_RET_STATUS_T ret_status_led_adp = E_LED_ADAPTER_RET_STATUS_OK;

    ret_status_led_adp = led_adapter_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_BLUE, E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_ON);
    if (E_LED_ADAPTER_RET_STATUS_OK != ret_status_led_adp)
    {
        (void)ret_status_led_adp;
        return;
    }

    return; 
}

extern void app_test_func_2(void)
{
    E_LED_ADAPTER_RET_STATUS_T ret_status_led_adp = E_LED_ADAPTER_RET_STATUS_OK;

    ret_status_led_adp = led_adapter_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_GREEN, E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_ON);
    if (E_LED_ADAPTER_RET_STATUS_OK != ret_status_led_adp)
    {
        (void)ret_status_led_adp;
        return;
    }

    return;
}

extern void app_test_func_3(void)
{
    E_LED_ADAPTER_RET_STATUS_T ret_status_led_adp = E_LED_ADAPTER_RET_STATUS_OK;

    ret_status_led_adp = led_adapter_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_BLUE, E_LED_ADAPTER_DISP_PATTERN_TYPE_STEADY_OFF);
    if (E_LED_ADAPTER_RET_STATUS_OK != ret_status_led_adp)
    {
        (void)ret_status_led_adp;   
        return;
    }

    ret_status_led_adp = led_adapter_disp_ptn_preset_set(E_LED_ADAPTER_LED_ID_RED, E_LED_ADAPTER_DISP_PATTERN_TYPE_HEARTBEAT);
    if (E_LED_ADAPTER_RET_STATUS_OK != ret_status_led_adp)
    {
        (void)ret_status_led_adp;
        return;
    }

    return;
}











