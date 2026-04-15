#include "multi_button.h"
#include <stddef.h>

Button* header = NULL;

/**
 * @brief Initialize a button instance
 * 
 * @param handle Pointer to the button instance
 * @param button_id ID of the button
 * @param active_level Active level of the button (0 or 1)
 * @param read_level Function pointer to read the GPIO level of the button
 */
void Button_Init(Button* handle, uint8_t button_id, uint8_t active_level, uint8_t (*read_level)(uint8_t GPIO_PIN)) {
    if (handle == NULL) {
        return;
    }
    handle->button_id = button_id;
    handle->active_level = active_level;
    handle->read_level = read_level;
    handle->current_level = 1 - active_level; // Initialize to the opposite of active level
    handle->ticks = 0;
    handle->press_counter = 0;
    handle->debounce_counter = 0;
    handle->state = BTN_STATE_IDLE;
    handle->next = NULL;

    // Initialize callback functions to NULL
    for (int i = 0; i < BTN_EVENT_COUNT; i++) {
        handle->cb[i] = NULL;
    }
}

/**
 * @brief Attach a callback function to a button event
 * 
 * @param handle Pointer to the button instance
 * @param bv Button event to attach the callback to
 * @param cb Callback function to be called when the event occurs
 */
void Button_Attach(Button* handle, ButtonEvent bv, BtnCallback cb) {
    if (handle == NULL) return;
    handle->cb[bv] = cb;
}

/**
 * @brief Detach a callback function from a button event
 * 
 * @param handle Pointer to the button instance
 * @param bv Button event to detach the callback from
 */
void Button_Detach(Button* handle, ButtonEvent bv) {
    if (handle == NULL) return;
    handle->cb[bv] = NULL;
}

/**
 * @brief Read the GPIO level of a button
 * 
 * @param handle Pointer to the button instance
 * @return uint8_t GPIO level of the button
 */
static inline uint8_t Read_GPIO_Level(Button* handle) {
    return handle->read_level(handle->button_id);
}

/**
 * @brief Set the event for a button and trigger the corresponding callback
 * 
 * @param handle Pointer to the button instance
 * @param event Button event to trigger
 */
static inline void SetEvent_TriggerCallback(Button* handle, ButtonEvent event) {
    handle->event = event;
    if (event != BTN_EVENT_NONE && handle->cb[event]) {
        handle->cb[event](handle);
    }
}

/**
 * @brief Get the current event of a button
 * 
 * @param handle Pointer to the button instance
 * @return ButtonEvent Current button event
 */
ButtonEvent GetButton_Event(Button* handle) {
    if (handle == NULL) return BTN_EVENT_NONE;
    return handle->event;
}

/**
 * @brief Handle the button state machine
 * 
 * @param handle Pointer to the button instance
 */
static void Button_Handler(Button* handle) {
    if (handle == NULL) return;

    if (handle->state > BTN_STATE_IDLE && handle->ticks < UINT16_MAX) {
        handle->ticks++;
    }

    // Read GPIO
    uint8_t gpio_level_readed = Read_GPIO_Level(handle);

    // Debounce and button level control
    if (gpio_level_readed != handle->current_level) {
        if (++(handle->debounce_counter) > DEBOUNCE_TICKS) {
            handle->current_level = gpio_level_readed;
            handle->debounce_counter = 0;
        }
    } else {
        handle->debounce_counter = 0;
    }

    switch (handle->state) {
    case BTN_STATE_IDLE:
    // Check for button press
        if (handle->current_level == handle->active_level) {
            handle->ticks = 0;
            handle->press_counter = 1;
            SetEvent_TriggerCallback(handle, BTN_EVENT_PRESS);
            handle->state = BTN_STATE_PRESS;
        } else if (handle->ticks > IDLE_TICKS) {
            SetEvent_TriggerCallback(handle, BTN_EVENT_NONE);
        }
        break;
    case BTN_STATE_PRESS:
    // Check for button release or transition to long press state
        if (handle->current_level != handle->active_level) {
            handle->ticks = 0;
            SetEvent_TriggerCallback(handle, BTN_EVENT_RELEASE);
            handle->state = BTN_STATE_RELEASE;
        } else {
            if (handle->ticks >= LONG_PRESS_STAGE_1_TICKS) {
                handle->state = BTN_STATE_LONG_PRESS;
            }
        }
        break;
    case BTN_STATE_RELEASE:
    // Check for repeat press events
        if (handle->ticks > REPEAT_PRESS_TICKS) {
            if (handle->press_counter == 1) {
                SetEvent_TriggerCallback(handle, BTN_EVENT_SINGLE_CLICK);
            } else if (handle->press_counter == 2) {
                SetEvent_TriggerCallback(handle, BTN_EVENT_DOUBLE_CLICK);
            } else if (handle->press_counter == 3) {
                SetEvent_TriggerCallback(handle, BTN_EVENT_TRIPLE_CLICK);
            }
            handle->state = BTN_STATE_IDLE;

    // If the button is pressed again within the repeat press time window, count it as another press
        } else if (handle->current_level == handle->active_level) {
            if (handle->press_counter < REPEAT_PRESS_MAX_NUM) {
                handle->press_counter++;
            }
            SetEvent_TriggerCallback(handle, BTN_EVENT_PRESS);
            handle->state = BTN_STATE_PRESS;
        }
        break;
    case BTN_STATE_LONG_PRESS:
    // Trigger long press events based on the duration of the press
        if (handle->current_level != handle->active_level) {
            if (handle->ticks >= LONG_PRESS_STAGE_3_TICKS) {
                SetEvent_TriggerCallback(handle, BTN_EVENT_LONG_PRESS_STAGE_3);
            } else if (handle->ticks >= LONG_PRESS_STAGE_2_TICKS) {
                SetEvent_TriggerCallback(handle, BTN_EVENT_LONG_PRESS_STAGE_2);
            } else {
                SetEvent_TriggerCallback(handle, BTN_EVENT_LONG_PRESS_STAGE_1);
            }
            handle->state = BTN_STATE_IDLE;
        }
        break;
    default:
        handle->state = BTN_STATE_IDLE;
        break;
    }
}

/**
 * @brief Start a button instance and add it to the button list
 * 
 * @param handle Pointer to the button instance
 * @return int 0 if successful, -1 if the button is already started, -2 if the handle is NULL
 */
int Button_Start(Button* handle) {
    if (handle == NULL) return -2;

    Button* curr = header;
    while (curr) {
        if (curr == handle) {
            return -1;
        }
        curr = curr->next;
    }
    handle->next = header;
    header = handle;
    return 0;
}

/**
 * @brief Stop a button instance and remove it from the button list
 * 
 * @param handle Pointer to the button instance
 */
void Button_Stop(Button* handle) {
    if (handle == NULL) return;

    Button** curr = &header;
    while(*curr) {
        if (*curr == handle) {
            *curr = (*curr)->next;
            handle->next = NULL;
            return;
        }
        curr = &((*curr)->next);
    }
}

/**
 * @brief Update all button instances by calling their handler functions
 */
void Button_Tick() {
    Button* target = header;
    while(target) {
        Button_Handler(target);
        target = target->next;
    }
}