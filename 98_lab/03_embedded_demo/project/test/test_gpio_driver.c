/**
 * @file test_gpio_driver.c
 * @brief Unit tests for GPIO driver
 */

#include "unity.h"
#include "gpio_driver.h"

/* External test helper */
extern void gpio_simulate_input(uint8_t port, uint8_t pin, gpio_state_t state);

void setUp(void) {
    /* This is run before each test */
}

void tearDown(void) {
    /* This is run after each test */
}

void test_gpio_init_with_valid_config(void) {
    gpio_config_t config = {
        .port = 0,
        .pin = 5,
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    
    TEST_ASSERT_EQUAL_INT(0, gpio_init(&config));
}

void test_gpio_init_with_null_config(void) {
    TEST_ASSERT_EQUAL_INT(-1, gpio_init(NULL));
}

void test_gpio_init_with_invalid_port(void) {
    gpio_config_t config = {
        .port = 16,  /* Invalid - max is 15 */
        .pin = 5,
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    
    TEST_ASSERT_EQUAL_INT(-1, gpio_init(&config));
}

void test_gpio_init_with_invalid_pin(void) {
    gpio_config_t config = {
        .port = 0,
        .pin = 16,  /* Invalid - max is 15 */
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    
    TEST_ASSERT_EQUAL_INT(-1, gpio_init(&config));
}

void test_gpio_write_high(void) {
    /* Initialize pin as output */
    gpio_config_t config = {
        .port = 0,
        .pin = 7,
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    gpio_init(&config);
    
    /* Write high */
    TEST_ASSERT_EQUAL_INT(0, gpio_write(0, 7, GPIO_HIGH));
    
    /* Verify by reading back */
    gpio_state_t state;
    gpio_read(0, 7, &state);
    TEST_ASSERT_EQUAL(GPIO_HIGH, state);
}

void test_gpio_write_low(void) {
    /* Initialize pin as output */
    gpio_config_t config = {
        .port = 0,
        .pin = 8,
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    gpio_init(&config);
    
    /* Write low */
    TEST_ASSERT_EQUAL_INT(0, gpio_write(0, 8, GPIO_LOW));
    
    /* Verify by reading back */
    gpio_state_t state;
    gpio_read(0, 8, &state);
    TEST_ASSERT_EQUAL(GPIO_LOW, state);
}

void test_gpio_write_to_input_pin_fails(void) {
    /* Initialize pin as input */
    gpio_config_t config = {
        .port = 1,
        .pin = 3,
        .dir = GPIO_INPUT,
        .pull_up = false
    };
    gpio_init(&config);
    
    /* Attempting to write to input should fail */
    TEST_ASSERT_EQUAL_INT(-1, gpio_write(1, 3, GPIO_HIGH));
}

void test_gpio_read_input_pin(void) {
    /* Initialize pin as input */
    gpio_config_t config = {
        .port = 2,
        .pin = 4,
        .dir = GPIO_INPUT,
        .pull_up = false
    };
    gpio_init(&config);
    
    /* Simulate external input */
    gpio_simulate_input(2, 4, GPIO_HIGH);
    
    /* Read the pin */
    gpio_state_t state;
    TEST_ASSERT_EQUAL_INT(0, gpio_read(2, 4, &state));
    TEST_ASSERT_EQUAL(GPIO_HIGH, state);
}

void test_gpio_read_with_null_state_pointer(void) {
    gpio_config_t config = {
        .port = 0,
        .pin = 1,
        .dir = GPIO_INPUT,
        .pull_up = false
    };
    gpio_init(&config);
    
    TEST_ASSERT_EQUAL_INT(-1, gpio_read(0, 1, NULL));
}

void test_gpio_toggle(void) {
    /* Initialize pin as output */
    gpio_config_t config = {
        .port = 3,
        .pin = 9,
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    gpio_init(&config);
    
    /* Initial state should be LOW */
    gpio_state_t state;
    gpio_read(3, 9, &state);
    TEST_ASSERT_EQUAL(GPIO_LOW, state);
    
    /* Toggle to HIGH */
    gpio_toggle(3, 9);
    gpio_read(3, 9, &state);
    TEST_ASSERT_EQUAL(GPIO_HIGH, state);
    
    /* Toggle back to LOW */
    gpio_toggle(3, 9);
    gpio_read(3, 9, &state);
    TEST_ASSERT_EQUAL(GPIO_LOW, state);
}

void test_gpio_input_with_pullup(void) {
    /* Initialize pin as input with pull-up */
    gpio_config_t config = {
        .port = 4,
        .pin = 10,
        .dir = GPIO_INPUT,
        .pull_up = true
    };
    gpio_init(&config);
    
    /* With pull-up, should read HIGH by default */
    gpio_state_t state;
    gpio_read(4, 10, &state);
    TEST_ASSERT_EQUAL(GPIO_HIGH, state);
}
