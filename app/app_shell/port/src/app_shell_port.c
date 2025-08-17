#include "app_shell_port.h"

#include "bsp_serialport_adapter.h"

#include "osal.h"


extern short app_shell_port_write(char *data, unsigned short len)
{
    E_SERIALPORT_ADAPTER_RET_STATUS_T ret_status_serialport = E_SERIALPORT_ADAPTER_RET_STATUS_OK;
    
    uint16_t write_len = len;
    ret_status_serialport = serialport_adapter_transmit( (uint8_t*)data, write_len);
    if (E_SERIALPORT_ADAPTER_RET_STATUS_OK != ret_status_serialport)
    {
        return 0;
    }

    return write_len;
}

extern short app_shell_port_read(char *data, unsigned short len)
{
    E_SERIALPORT_ADAPTER_RET_STATUS_T ret_status_serialport = E_SERIALPORT_ADAPTER_RET_STATUS_OK;

    uint16_t read_len = len;
    ret_status_serialport = serialport_adapter_receive( (uint8_t*)data, (uint16_t*)&read_len);
    if (E_SERIALPORT_ADAPTER_RET_STATUS_OK != ret_status_serialport)
    {
        return 0;
    }

    return read_len;
}









