# MultiButton

A lightweight multi-button handler library for embedded systems, supports single/double/triple click and long press detection.

## Features

- **Click Detection**: Single click, double click, triple click
- **Long Press**: Three stages with configurable duration thresholds
- **Debounce**: Built-in software debounce
- **Low Memory**: Single linked list, no dynamic allocation
- **Easy Port**: Only requires a GPIO read function

## Events

| Event                              | Description                     |
|------------------------------------|---------------------------------|
| `BTN_EVENT_PRESS`                  | Button pressed down             |
| `BTN_EVENT_RELEASE`                | Button released                 |
| `BTN_EVENT_SINGLE_CLICK`           | Single click completed          |
| `BTN_EVENT_DOUBLE_CLICK`           | Double click completed          |
| `BTN_EVENT_TRIPLE_CLICK`           | Triple click completed          |
| `BTN_EVENT_LONG_PRESS_STAGE_1`     | Long press stage 1 completed    |
| `BTN_EVENT_LONG_PRESS_STAGE_2`     | Long press stage 2 completed    |
| `BTN_EVENT_LONG_PRESS_STAGE_3`     | Long press stage 3 completed    |

## Usage

```c
#include "multi_button.h"

// Implement GPIO read function
uint8_t read_btn_gpio(uint8_t button_id) {
    return GPIO_ReadPin(BUTTON_PIN); // your implementation
}

Button myBtn;

void on_press(Button* btn) { /* handle press */ }
void on_single_click(Button* btn) { /* handle single click */ }

int main(void) {
    Button_Init(&myBtn, 0, 1, read_btn_gpio);
    Button_Attach(&myBtn, BTN_EVENT_PRESS, on_press);
    Button_Attach(&myBtn, BTN_EVENT_SINGLE_CLICK, on_single_click);
    Button_Start(&myBtn);

    while (1) {
        Button_Tick(); // call every TICKS_INTERVAL ms
    }
}
```

## Configuration

Edit `multi_button.h` to adjust timing:

```c
#define TICKS_INTERVAL            5   // ms per tick
#define DEBOUNCE_TICKS            3   // debounce duration
#define LONG_PRESS_STAGE_1_TICKS  (1000/TICKS_INTERVAL)  // 1s
#define LONG_PRESS_STAGE_2_TICKS  (3000/TICKS_INTERVAL)  // 3s
#define LONG_PRESS_STAGE_3_TICKS  (10000/TICKS_INTERVAL) // 10s
#define REPEAT_PRESS_TICKS        (200/TICKS_INTERVAL)   // double click window
```

## Build & Test

```bash
make        # build test binary
make run    # build and run
make clean  # clean artifacts
```
