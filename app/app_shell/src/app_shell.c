#include "app_shell.h"
#include "app_shell_port.h"

#include "shell.h"
#include "log.h"

#include "osal.h"

#include "stddef.h"


#define D_APP_SHELL_PARSER_BUFFER_SIZE  (256)

typedef struct 
{
    Shell       shell_handle;
    Log         shell_log_handle;
    void*       shell_tx_os_mutex;
    uint8_t*    p_shell_rx_parser_buf;
} APP_SHELL_T;

static uint8_t gs_app_shell_rx_parser_buf[D_APP_SHELL_PARSER_BUFFER_SIZE];

static APP_SHELL_T gs_app_shell_handle = {0};


static int _app_shell_lock(Shell*);
static int _app_shell_unlock(Shell*);
static void _app_shell_log_write(char*, short);

extern E_APP_SHELL_RET_STATUS_T app_shell_init(void)
{
    /* Letter shell initialization*/
    gs_app_shell_handle.shell_handle.write  = app_shell_port_write;
    gs_app_shell_handle.shell_handle.read   = app_shell_port_read;
    gs_app_shell_handle.shell_handle.lock   = _app_shell_lock;
    gs_app_shell_handle.shell_handle.unlock = _app_shell_unlock;

    shellInit(&gs_app_shell_handle.shell_handle, (char*)gs_app_shell_rx_parser_buf, D_APP_SHELL_PARSER_BUFFER_SIZE);

    /* Log of letter shell initialization */
    gs_app_shell_handle.shell_log_handle.active =   1;
    gs_app_shell_handle.shell_log_handle.level  =   LOG_DEBUG;
    gs_app_shell_handle.shell_log_handle.write  =   _app_shell_log_write;
    
    logRegister(&gs_app_shell_handle.shell_log_handle, &gs_app_shell_handle.shell_handle);

    /* Create TX mutex*/
    S_OSAL_MUTEX_CONFIG_T app_shell_tx_mutex_conf = 
    {
        .p_name = "App shell TX mutex"
    };

    E_OSAL_RET_STATUS_T ret_status_osal = osal_mutex_create(&gs_app_shell_handle.shell_tx_os_mutex, &app_shell_tx_mutex_conf);
    if (E_OSAL_RET_STATUS_OK != ret_status_osal)
    {
        return E_APP_SHELL_RET_STATUS_ERROR;
    }

    return E_APP_SHELL_RET_STATUS_OK;
}

extern void app_shell_thread(void* argument)
{
    uint8_t data;
    while (NULL != gs_app_shell_handle.shell_handle.read && 1 == gs_app_shell_handle.shell_handle.read( (char*)&data, 1))
    {
        shellHandler(&gs_app_shell_handle.shell_handle, data);
    }
}

static int _app_shell_lock(Shell *shell)
{
    (void)shell;
    
    E_OSAL_RET_STATUS_T ret_status_osal = E_OSAL_RET_STATUS_OK;

    ret_status_osal = osal_mutex_lock(gs_app_shell_handle.shell_tx_os_mutex);
    if (E_OSAL_RET_STATUS_OK != ret_status_osal)
    {
        return -1;
    }

    return 0;
}

static int _app_shell_unlock(Shell *shell)
{
    (void)shell;

    E_OSAL_RET_STATUS_T ret_status_osal = E_OSAL_RET_STATUS_OK;
    
    ret_status_osal = osal_mutex_unlock(gs_app_shell_handle.shell_tx_os_mutex);
    if (E_OSAL_RET_STATUS_OK != ret_status_osal)
    {
        return -1;
    }

    return 0;
}

static void _app_shell_log_write(char* data, short data_size)
{
    if (NULL != gs_app_shell_handle.shell_log_handle.shell)
    {
        shellWriteEndLine(gs_app_shell_handle.shell_log_handle.shell, data, data_size);
    }
}