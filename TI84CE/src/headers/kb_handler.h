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
  * Initialize the keyboard handler
  * Call this once at program startup
  */
 void kb_init(void);
 
 /**
  * Register a callback for key press events
  * 
  * @param key The key to monitor
  * @param callback Function to call when the key is pressed
  * @return Callback ID that can be used for removal, or -1 if registration failed
  */
 KBCallbackID kb_register_press(CombinedKey key, KBPressCallback callback);
 
 /**
  * Register a callback for key release events
  * 
  * @param key The key to monitor
  * @param callback Function to call when the key is released
  * @return Callback ID that can be used for removal, or -1 if registration failed
  */
 KBCallbackID kb_register_release(CombinedKey key, KBReleaseCallback callback);
 
 /**
  * Register a callback for key hold events
  * 
  * @param key The key to monitor
  * @param callback Function to call when the key is held down
  * @param trigger_time_ms Time in milliseconds before triggering the hold event
  * @param repeat Whether the hold event should repeat while key is held
  * @param repeat_interval_ms Time between repeat triggers if repeat is true
  * @return Callback ID that can be used for removal, or -1 if registration failed
  */
 KBCallbackID kb_register_hold(CombinedKey key, KBHoldCallback callback, 
                               int trigger_time_ms, bool repeat, int repeat_interval_ms);
 
 /**
  * Unregister a specific callback by ID
  * 
  * @param callback_id The ID returned when the callback was registered
  * @return true if the callback was found and removed, false otherwise
  */
 bool kb_unregister_id(KBCallbackID callback_id);
 
 /**
  * Unregister all callbacks for a specific key
  * 
  * @param key The key whose callbacks should be removed
  * @return Number of callbacks that were removed
  */
 int kb_unregister(CombinedKey key);
 
 /**
  * Clear all registered callbacks
  */
 void kb_clear(void);
 
 /**
  * Process keyboard events and trigger callbacks
  * Call this once per main loop iteration
  */
 void kb_process(void);
 
 #include "kb_handler_public.h"

#endif // LOG_H
