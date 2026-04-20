#include "multi_button.h"
#include <stdio.h>
#include <assert.h>

/* ---- Mock GPIO ---- */
static uint8_t mock_gpio_value = 0;

static uint8_t mock_read_gpio(uint8_t button_id)
{
    (void)button_id;
    return mock_gpio_value;
}

/* ---- Event counter for validation ---- */
static struct {
    int press;
    int release;
    int single_click;
    int double_click;
    int triple_click;
    int lp1_start;
    int lp2_start;
    int lp3_start;
    int lp1;
    int lp2;
    int lp3;
} evt_cnt;

static void reset_cnt(void)
{
    evt_cnt.press = 0;
    evt_cnt.release = 0;
    evt_cnt.single_click = 0;
    evt_cnt.double_click = 0;
    evt_cnt.triple_click = 0;
    evt_cnt.lp1_start = 0;
    evt_cnt.lp2_start = 0;
    evt_cnt.lp3_start = 0;
    evt_cnt.lp1 = 0;
    evt_cnt.lp2 = 0;
    evt_cnt.lp3 = 0;
}

#define CHECK_CNT(exp_press, exp_release, exp_single, exp_double, exp_triple, \
                  exp_lp1_start, exp_lp2_start, exp_lp3_start, \
                  exp_lp1, exp_lp2, exp_lp3) \
    do { \
        int ok = 1; \
        if (evt_cnt.press != (exp_press)) { printf("FAIL: press expected %d, got %d\n", exp_press, evt_cnt.press); ok = 0; } \
        if (evt_cnt.release != (exp_release)) { printf("FAIL: release expected %d, got %d\n", exp_release, evt_cnt.release); ok = 0; } \
        if (evt_cnt.single_click != (exp_single)) { printf("FAIL: single_click expected %d, got %d\n", exp_single, evt_cnt.single_click); ok = 0; } \
        if (evt_cnt.double_click != (exp_double)) { printf("FAIL: double_click expected %d, got %d\n", exp_double, evt_cnt.double_click); ok = 0; } \
        if (evt_cnt.triple_click != (exp_triple)) { printf("FAIL: triple_click expected %d, got %d\n", exp_triple, evt_cnt.triple_click); ok = 0; } \
        if (evt_cnt.lp1_start != (exp_lp1_start)) { printf("FAIL: lp1_start expected %d, got %d\n", exp_lp1_start, evt_cnt.lp1_start); ok = 0; } \
        if (evt_cnt.lp2_start != (exp_lp2_start)) { printf("FAIL: lp2_start expected %d, got %d\n", exp_lp2_start, evt_cnt.lp2_start); ok = 0; } \
        if (evt_cnt.lp3_start != (exp_lp3_start)) { printf("FAIL: lp3_start expected %d, got %d\n", exp_lp3_start, evt_cnt.lp3_start); ok = 0; } \
        if (evt_cnt.lp1 != (exp_lp1)) { printf("FAIL: lp1 expected %d, got %d\n", exp_lp1, evt_cnt.lp1); ok = 0; } \
        if (evt_cnt.lp2 != (exp_lp2)) { printf("FAIL: lp2 expected %d, got %d\n", exp_lp2, evt_cnt.lp2); ok = 0; } \
        if (evt_cnt.lp3 != (exp_lp3)) { printf("FAIL: lp3 expected %d, got %d\n", exp_lp3, evt_cnt.lp3); ok = 0; } \
        if (ok) printf("PASS\n"); \
        pass = ok; \
    } while(0)

/* ---- Callbacks ---- */
static void cb_press(Button* btn) { (void)btn; evt_cnt.press++; }
static void cb_release(Button* btn) { (void)btn; evt_cnt.release++; }
static void cb_single(Button* btn) { (void)btn; evt_cnt.single_click++; }
static void cb_double(Button* btn) { (void)btn; evt_cnt.double_click++; }
static void cb_triple(Button* btn) { (void)btn; evt_cnt.triple_click++; }
static void cb_lp1_start(Button* btn) { (void)btn; evt_cnt.lp1_start++; }
static void cb_lp2_start(Button* btn) { (void)btn; evt_cnt.lp2_start++; }
static void cb_lp3_start(Button* btn) { (void)btn; evt_cnt.lp3_start++; }
static void cb_lp1(Button* btn) { (void)btn; evt_cnt.lp1++; }
static void cb_lp2(Button* btn) { (void)btn; evt_cnt.lp2++; }
static void cb_lp3(Button* btn) { (void)btn; evt_cnt.lp3++; }

/* ---- Button instance ---- */
static Button btn;

static void setup_button(void)
{
    Button_Init(&btn, 1, 1, mock_read_gpio);
    Button_Attach(&btn, BTN_EVENT_PRESS, cb_press);
    Button_Attach(&btn, BTN_EVENT_RELEASE, cb_release);
    Button_Attach(&btn, BTN_EVENT_SINGLE_CLICK, cb_single);
    Button_Attach(&btn, BTN_EVENT_DOUBLE_CLICK, cb_double);
    Button_Attach(&btn, BTN_EVENT_TRIPLE_CLICK, cb_triple);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_1_START, cb_lp1_start);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_2_START, cb_lp2_start);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_3_START, cb_lp3_start);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_1, cb_lp1);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_2, cb_lp2);
    Button_Attach(&btn, BTN_EVENT_LONG_PRESS_STAGE_3, cb_lp3);
    reset_cnt();
    Button_Start(&btn);
}

static void teardown(void)
{
    Button_Stop(&btn);
    mock_gpio_value = 0;
}

static void tick_n(int n)
{
    for (int i = 0; i < n; i++) Button_Tick();
}

/* ---- Test: Single Click ---- */
static int test_single_click(void)
{
    printf("Test: Single Click... ");
    setup_button();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);  /* press ~50ms */

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + REPEAT_PRESS_TICKS + 10); /* wait for click detection */

    int pass = evt_cnt.press == 1 && evt_cnt.release == 1 && evt_cnt.single_click == 1;
    CHECK_CNT(1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0);

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Double Click ---- */
static int test_double_click(void)
{
    printf("Test: Double Click... ");
    setup_button();

    /* first click */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + REPEAT_PRESS_TICKS - 5);

    /* second click */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 10);
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + REPEAT_PRESS_TICKS + 10); /* wait for double-click detection */

    int pass = evt_cnt.press == 2 && evt_cnt.release == 2 && evt_cnt.double_click == 1;
    CHECK_CNT(2, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0);

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Triple Click ---- */
static int test_triple_click(void)
{
    printf("Test: Triple Click... ");
    setup_button();

    for (int i = 0; i < 3; i++) {
        mock_gpio_value = 1;
        tick_n(DEBOUNCE_TICKS + 10);
        mock_gpio_value = 0;
        tick_n(DEBOUNCE_TICKS + REPEAT_PRESS_TICKS - 5);
    }
    tick_n(REPEAT_PRESS_TICKS + 10); /* wait for triple-click detection */

    int pass = evt_cnt.press == 3 && evt_cnt.release == 3 && evt_cnt.triple_click == 1;
    CHECK_CNT(3, 3, 0, 0, 1, 0, 0, 0, 0, 0, 0);

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Long Press Stage 1 ---- */
static int test_long_press_1(void)
{
    printf("Test: Long Press Stage 1... ");
    setup_button();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_PRESS_STAGE_1_TICKS + 50);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 10);

    int pass = evt_cnt.press == 1 && evt_cnt.lp1_start == 1 && evt_cnt.lp1 == 1;
    CHECK_CNT(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0);

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Long Press Stage 2 ---- */
static int test_long_press_2(void)
{
    printf("Test: Long Press Stage 2... ");
    setup_button();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_PRESS_STAGE_2_TICKS + 50);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 10);

    int pass = evt_cnt.press == 1 &&
               evt_cnt.lp1_start == 1 && evt_cnt.lp2_start == 1 &&
               evt_cnt.lp2 == 1;
    CHECK_CNT(1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0);

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Long Press Stage 3 ---- */
static int test_long_press_3(void)
{
    printf("Test: Long Press Stage 3... ");
    setup_button();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + LONG_PRESS_STAGE_3_TICKS + 50);

    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 10);

    int pass = evt_cnt.press == 1 &&
               evt_cnt.lp1_start == 1 && evt_cnt.lp2_start == 1 && evt_cnt.lp3_start == 1 &&
               evt_cnt.lp3 == 1;
    CHECK_CNT(1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1);

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Button Start/Stop ---- */
static int test_start_stop(void)
{
    printf("Test: Button Start/Stop... ");
    setup_button();

    /* button should respond when started */
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 5);
    int before_stop = evt_cnt.press;

    Button_Stop(&btn);

    /* after stop, events should not fire */
    mock_gpio_value = 0;
    tick_n(DEBOUNCE_TICKS + 5);
    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 5);

    int pass = before_stop == 1 && evt_cnt.press == 1; /* no new press after stop */
    printf("%s\n", pass ? "PASS" : "FAIL");

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Detach callback ---- */
static int test_detach(void)
{
    printf("Test: Detach callback... ");
    setup_button();

    Button_Detach(&btn, BTN_EVENT_PRESS);
    reset_cnt();

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 5);

    int pass = evt_cnt.press == 0;
    printf("%s\n", pass ? "PASS" : "FAIL");

    teardown();
    return pass ? 0 : 1;
}

/* ---- Test: Multiple buttons ---- */
static Button btn2;

static void cb_press_btn2(Button* btn) { (void)btn; evt_cnt.press++; }

static int test_multiple_buttons(void)
{
    printf("Test: Multiple Buttons... ");
    setup_button();

    Button_Init(&btn2, 2, 1, mock_read_gpio);
    Button_Attach(&btn2, BTN_EVENT_PRESS, cb_press_btn2);
    Button_Start(&btn2);

    mock_gpio_value = 1;
    tick_n(DEBOUNCE_TICKS + 5);

    int pass = evt_cnt.press == 2; /* both buttons detect press */
    printf("%s\n", pass ? "PASS" : "FAIL");

    Button_Stop(&btn2);
    teardown();
    return pass ? 0 : 1;
}

int main(void)
{
    printf("=== MultiButton Test Suite ===\n\n");

    int failures = 0;
    failures += test_single_click();
    failures += test_double_click();
    failures += test_triple_click();
    failures += test_long_press_1();
    failures += test_long_press_2();
    failures += test_long_press_3();
    failures += test_start_stop();
    failures += test_detach();
    failures += test_multiple_buttons();

    printf("\n=== Results: %d/%d passed ===\n", 9 - failures, 9);
    return failures;
}
