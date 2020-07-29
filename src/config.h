#include "local.h"

#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

#define BL_EEPROM_SIZE (sizeof(int)*2)
#define BL_PWM_FREQ 500
#define BL_PWM_RES 15

#ifndef BL_BLYNK_AUTH_KEY
    #error "Please define BLYNK_AUTH_KEY in your local.h"
#endif

#ifndef BL_DEVICE_NAME
    #define BL_DEVICE_NAME "Bright Light"
#endif

#ifndef BL_LED_PIN
    #define BL_LED_PIN LED_BUILTIN
#endif

#ifndef BL_TOUCH_PIN
    #define BL_TOUCH_PIN 4
#endif

#ifndef BL_TOUCH_THR
    #define BL_TOUCH_THR 20
#endif

#ifndef BL_TOUCH_TIMEOUT
    #define BL_TOUCH_TIMEOUT 10
#endif

#ifndef BL_TOUCH_CHANGE_TIMEOUT
    #define BL_TOUCH_CHANGE_TIMEOUT 1000
#endif

#ifndef BL_TOUCH_IN_ROW
    #define BL_TOUCH_IN_ROW 30
#endif

#ifndef BL_PWM_CH
    #define BL_PWM_CH 0
#endif
