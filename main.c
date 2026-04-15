#include "multi_button.h"
#include <stdio.h>

/* ---- Mock GPIO ---- */
static uint8_t mock_gpio_value = 0;

static uint8_t mock_read_gpio(uint8_t button_id)
{
    (void)button_id;
    return mock_gpio_value;
}

static void log_press(Button* btn){
    if (btn) {
        printf("Press (id=%u)\n", btn->button_id);
    } else {
        printf("Press\n");
    }
}

static void log_release(Button* btn){
    if (btn) {
        printf("Release (id=%u)\n", btn->button_id);
    } else {
        printf("Release\n");
    }
}

static void log_single_click(Button* btn){
    if (btn) {
        printf("Single Click (id=%u)\n", btn->button_id);
    } else {
        printf("Single Click\n");
    }
}

static void log_double_click(Button* btn){
    if (btn) {
        printf("Double Click (id=%u)\n", btn->button_id);
    } else {
        printf("Double Click\n");
    }
}

static void log_triple_click(Button* btn){
    if (btn) {
        printf("Triple Click (id=%u)\n", btn->button_id);
    } else {
        printf("Triple Click\n");
    }
}

static void log_long_press_stage_1(Button* btn){
    if (btn) {
        printf("Long Press Stage 1 (id=%u)\n", btn->button_id);
    } else {
        printf("Long Press Stage 1\n");
    }
}

static void log_long_press_stage_2(Button* btn){
    if (btn) {
        printf("Long Press Stage 2 (id=%u)\n", btn->button_id);
    } else {
        printf("Long Press Stage 2\n");
    }
}

static void log_long_press_stage_3(Button* btn){
    if (btn) {
        printf("Long Press Stage 3 (id=%u)\n", btn->button_id);
    } else {
        printf("Long Press Stage 3\n");
    }
}

static Button btn;

static void Setup_Button(){
    Button_Init(&btn, 1, 1, mock_read_gpio);
    Button_Attach(&btn, BTN_EVENT_PRESS, log_press);
    Button_Attach(&btn, BTN_EVENT_RELEASE, log_release);
    Button_Attach(&btn, BTN_EVENT_SINGLE_CLICK, log_single_click);
    Button_Attach(&btn, BTN_EVENT_DOUBLE_CLICK, log_double_click);
    Button_Attach(&btn, BTN_EVENT_TRIPLE_CLICK, log_triple_click);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_1, log_long_press_stage_1);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_2, log_long_press_stage_2);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_3, log_long_press_stage_3);
    /* register and start the button */
    if (Button_Start(&btn) != 0) {
        printf("Button_Start failed\n");
    }
}

/* ---- Helper: advance N ticks ---- */
static void tick_n(int n)
{
    for (int i = 0; i < n; i++) {
        Button_Tick();
    }
}

static void teardown_button(void)
{
    Button_Stop(&btn);
    mock_gpio_value = 0;
}

/* Test 1: Single click */
static int test_single_click(void)
{
    Setup_Button();

    /* Press for ~50ms (10 ticks), then release */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 300 + 10);  /* wait for timeout */

    printf("Expect: Single Click\n");
    teardown_button();
    printf("Test 1 completed\n");
    return 0;
}

static int test_long_press(void)
{
    Setup_Button();

    /* Press for ~3.5s (700 ticks), then release */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_PRESS_STAGE_2_TICKS + 100);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 300 + 10);  /* wait for timeout */

    /* Press for ~1.5s, then release */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_PRESS_STAGE_1_TICKS + 100);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 300 + 10);  /* wait for timeout */

    /* Press for ~5.5s, then release */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_PRESS_STAGE_3_TICKS + 100);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 300 + 10);  /* wait for timeout */

    teardown_button();
    printf("Test 2 completed\n");
    return 0;
}

static int test_double_click(void)
{
    Setup_Button();

    /* Press and release twice within the double click time window */
    for (int i = 0; i < 2; i++) {
        mock_gpio_value = 1;
        tick_n(DEBOUNCE_TICKS + 10);

        mock_gpio_value = 0;
        tick_n(DEBOUNCE_TICKS + REPEAT_PRESS_TICKS - 10);  /* wait but within double click window */
    }

    tick_n(DEBOUNCE_TICKS + 300 + 10);  /* wait for timeout */

    teardown_button();
    printf("Test 3 completed\n");
    return 0;
}

int main(){
    printf("Running tests\n");
    int res = test_double_click();
    printf("Test finished (code=%d)\n", res);
    return res;
}