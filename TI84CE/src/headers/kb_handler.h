/**
 * Keyboard Handler for TI-84 CE
 * 
 * A lightweight system for keyboard event handling with callbacks.
 */

 #ifndef KEYBOARD_HANDLER_H
 #define KEYBOARD_HANDLER_H
 
 #include <stdbool.h>
 #include <keypadc.h>
 #include "ti84ce_keys.h"
 
 // Maximum number of key callbacks that can be registered
 #define KB_MAX_CALLBACKS 32
 
 // Callback types
 typedef void (*KBPressCallback)(void);
 typedef void (*KBReleaseCallback)(void);
 typedef void (*KBHoldCallback)(int hold_time);
 
 // Callback ID type
 typedef int KBCallbackID;
 
 /**
 * Enum representing the type of callback.
 */
typedef enum {
    CB_PRESS,    /**< Callback for key press events. */
    CB_RELEASE,  /**< Callback for key release events. */
    CB_HOLD      /**< Callback for key hold events. */
} CallbackType;

#include "kb_handler_public.h"

#endif // LOG_H
