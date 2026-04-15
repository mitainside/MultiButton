#ifndef MULTI_BUTTON_H
#define MULTI_BUTTON_H

#include <stdint.h>

#define TICKS_INTERVAL 5                                    // ms per tick, adjust as needed
#define DEBOUNCE_TICKS 3                                    // (0~7) Number of ticks for debouncing (e.g., 3 ticks = 15ms)
#define LONG_PRESS_STAGE_1_TICKS (1000/TICKS_INTERVAL)      // Time threshold for long press stage 1 (e.g., 1 second)
#define LONG_PRESS_STAGE_2_TICKS (3000/TICKS_INTERVAL)      // Time threshold for long press stage 2 (e.g., 3 seconds)
#define LONG_PRESS_STAGE_3_TICKS (5000/TICKS_INTERVAL)      // Time threshold for long press stage 3 (e.g., 5 seconds)
#define REPEAT_PRESS_TICKS (300/TICKS_INTERVAL)             // Time threshold to distinguish between single/double click
#define IDLE_TICKS (1000/TICKS_INTERVAL)                    // Time threshold to reset to idle state
#define REPEAT_PRESS_MAX_NUM 15                             // Maximum number of presses to count for repeat press events

typedef struct _Button Button;

typedef void (*BtnCallback)(Button* handle);

typedef enum {
    BTN_STATE_IDLE = 0,
    BTN_STATE_PRESS,
    BTN_STATE_LONG_PRESS,
    BTN_STATE_RELEASE
} ButtonState;

typedef enum {
    BTN_EVENT_PRESS = 0,                    // button is pressed down
    BTN_EVENT_RELEASE,                      // button is released
    BTN_EVENT_SINGLE_CLICK,                 // single click completed
    BTN_EVENT_DOUBLE_CLICK,                 // double click completed
    BTN_EVENT_TRIPLE_CLICK,                 // triple click completed
    BTN_EVENT_LONG_PRESS_STAGE_1,           // long press stage 1 completed
    BTN_EVENT_LONG_PRESS_STAGE_2,           // long press stage 2 completed
    BTN_EVENT_LONG_PRESS_STAGE_3,           // long press stage 3 completed
    BTN_EVENT_COUNT,                        // total number of events
    BTN_EVENT_NONE                          // no event, button idle
} ButtonEvent;

struct _Button {
    uint8_t button_id;                              // ID of the button, can be used to identify different buttons
    uint8_t active_level;                           // GPIO level that indicates the button is pressed
    uint8_t (*read_level)(uint8_t GPIO_PIN);        // Function pointer to read the button level
    BtnCallback cb[BTN_EVENT_COUNT];                // Callback function for button events
    uint8_t current_level;                          // Current button level
    uint16_t ticks;                                 // Counter for timing button events
    ButtonEvent event;                              // Current button event
    uint8_t press_counter;                          // Counter for button presses
    uint8_t debounce_counter;                       // Counter for debouncing
    ButtonState state;                              // Current state of the button, state machine
    Button *next;                                   // Pointer to the next button in the linked list
};

void Button_Init(Button* handle, uint8_t button_id, uint8_t active_level, uint8_t (*read_level)(uint8_t GPIO_PIN));
void Button_Attach(Button* handle, ButtonEvent bv, BtnCallback cb);
void Button_Detach(Button* handle, ButtonEvent bv);
int Button_Start(Button* handle);
void Button_Stop(Button* handle);
void Button_Tick();

#endif // MULTI_BUTTON_H