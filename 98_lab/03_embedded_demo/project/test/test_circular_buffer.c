/**
 * @file test_circular_buffer.c
 * @brief Unit tests for circular buffer
 */

#include "unity.h"
#include "circular_buffer.h"

#define BUFFER_SIZE 8

static uint8_t storage[BUFFER_SIZE];
static circular_buffer_t buffer;

void setUp(void) {
    /* Initialize buffer before each test */
    circular_buffer_init(&buffer, storage, BUFFER_SIZE);
}

void tearDown(void) {
    /* Cleanup after each test */
}

void test_circular_buffer_init_success(void) {
    uint8_t temp_storage[16];
    circular_buffer_t temp_buffer;
    
    TEST_ASSERT_EQUAL_INT(0, circular_buffer_init(&temp_buffer, temp_storage, 16));
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_size(&temp_buffer));
    TEST_ASSERT_TRUE(circular_buffer_is_empty(&temp_buffer));
    TEST_ASSERT_FALSE(circular_buffer_is_full(&temp_buffer));
}

void test_circular_buffer_init_null_buffer(void) {
    uint8_t temp_storage[16];
    
    TEST_ASSERT_EQUAL_INT(-1, circular_buffer_init(NULL, temp_storage, 16));
}

void test_circular_buffer_init_null_storage(void) {
    circular_buffer_t temp_buffer;
    
    TEST_ASSERT_EQUAL_INT(-1, circular_buffer_init(&temp_buffer, NULL, 16));
}

void test_circular_buffer_init_zero_capacity(void) {
    uint8_t temp_storage[16];
    circular_buffer_t temp_buffer;
    
    TEST_ASSERT_EQUAL_INT(-1, circular_buffer_init(&temp_buffer, temp_storage, 0));
}

void test_circular_buffer_put_single_byte(void) {
    TEST_ASSERT_EQUAL_INT(0, circular_buffer_put(&buffer, 0x42));
    TEST_ASSERT_EQUAL_size_t(1, circular_buffer_size(&buffer));
    TEST_ASSERT_FALSE(circular_buffer_is_empty(&buffer));
}

void test_circular_buffer_get_single_byte(void) {
    uint8_t data;
    
    circular_buffer_put(&buffer, 0x42);
    
    TEST_ASSERT_EQUAL_INT(0, circular_buffer_get(&buffer, &data));
    TEST_ASSERT_EQUAL_HEX8(0x42, data);
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_size(&buffer));
}

void test_circular_buffer_get_from_empty_buffer(void) {
    uint8_t data;
    
    TEST_ASSERT_EQUAL_INT(-1, circular_buffer_get(&buffer, &data));
}

void test_circular_buffer_put_to_full_buffer(void) {
    /* Fill the buffer */
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT(0, circular_buffer_put(&buffer, (uint8_t)i));
    }
    
    TEST_ASSERT_TRUE(circular_buffer_is_full(&buffer));
    
    /* Attempt to add one more */
    TEST_ASSERT_EQUAL_INT(-1, circular_buffer_put(&buffer, 0xFF));
}

void test_circular_buffer_fifo_order(void) {
    /* Add sequence of bytes */
    for (uint8_t i = 0; i < 5; i++) {
        circular_buffer_put(&buffer, i);
    }
    
    /* Verify FIFO order */
    uint8_t data;
    for (uint8_t i = 0; i < 5; i++) {
        circular_buffer_get(&buffer, &data);
        TEST_ASSERT_EQUAL_HEX8(i, data);
    }
}

void test_circular_buffer_wraparound(void) {
    /* Fill buffer completely */
    for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        circular_buffer_put(&buffer, i);
    }
    
    /* Remove half */
    uint8_t data;
    for (size_t i = 0; i < BUFFER_SIZE / 2; i++) {
        circular_buffer_get(&buffer, &data);
    }
    
    /* Add more (this will wraparound) */
    for (uint8_t i = 100; i < 100 + BUFFER_SIZE / 2; i++) {
        TEST_ASSERT_EQUAL_INT(0, circular_buffer_put(&buffer, i));
    }
    
    /* Verify correct order */
    for (size_t i = BUFFER_SIZE / 2; i < BUFFER_SIZE; i++) {
        circular_buffer_get(&buffer, &data);
        TEST_ASSERT_EQUAL_HEX8((uint8_t)i, data);
    }
    for (size_t i = 100; i < 100 + BUFFER_SIZE / 2; i++) {
        circular_buffer_get(&buffer, &data);
        TEST_ASSERT_EQUAL_HEX8((uint8_t)i, data);
    }
}

void test_circular_buffer_size_tracking(void) {
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_size(&buffer));
    TEST_ASSERT_EQUAL_size_t(BUFFER_SIZE, circular_buffer_available(&buffer));
    
    circular_buffer_put(&buffer, 0x11);
    TEST_ASSERT_EQUAL_size_t(1, circular_buffer_size(&buffer));
    TEST_ASSERT_EQUAL_size_t(BUFFER_SIZE - 1, circular_buffer_available(&buffer));
    
    circular_buffer_put(&buffer, 0x22);
    TEST_ASSERT_EQUAL_size_t(2, circular_buffer_size(&buffer));
    TEST_ASSERT_EQUAL_size_t(BUFFER_SIZE - 2, circular_buffer_available(&buffer));
    
    uint8_t data;
    circular_buffer_get(&buffer, &data);
    TEST_ASSERT_EQUAL_size_t(1, circular_buffer_size(&buffer));
    TEST_ASSERT_EQUAL_size_t(BUFFER_SIZE - 1, circular_buffer_available(&buffer));
}

void test_circular_buffer_clear(void) {
    /* Add some data */
    for (uint8_t i = 0; i < 4; i++) {
        circular_buffer_put(&buffer, i);
    }
    
    TEST_ASSERT_EQUAL_size_t(4, circular_buffer_size(&buffer));
    
    /* Clear buffer */
    TEST_ASSERT_EQUAL_INT(0, circular_buffer_clear(&buffer));
    
    /* Verify it's empty */
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_size(&buffer));
    TEST_ASSERT_TRUE(circular_buffer_is_empty(&buffer));
    TEST_ASSERT_EQUAL_size_t(BUFFER_SIZE, circular_buffer_available(&buffer));
}

void test_circular_buffer_get_with_null_data(void) {
    circular_buffer_put(&buffer, 0x42);
    
    TEST_ASSERT_EQUAL_INT(-1, circular_buffer_get(&buffer, NULL));
}
