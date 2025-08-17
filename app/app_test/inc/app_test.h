#ifndef __APP_TEST_H__
#define __APP_TEST_H__

typedef enum
{
    E_APP_TEST_RET_STATUS_OK,
    E_APP_TEST_RET_STATUS_ERROR,
} E_APP_TEST_RET_STATUS_T;

extern E_APP_TEST_RET_STATUS_T app_test_init(void);

extern void app_test_thread(void* argument);


#endif 