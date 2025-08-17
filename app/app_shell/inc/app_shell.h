#ifndef __APP_SHELL_H__
#define __APP_SHELL_H__

typedef enum
{
    E_APP_SHELL_RET_STATUS_OK,
    E_APP_SHELL_RET_STATUS_ERROR,
} E_APP_SHELL_RET_STATUS_T;

extern E_APP_SHELL_RET_STATUS_T app_shell_init(void);

extern void app_shell_thread(void* argument);

#endif