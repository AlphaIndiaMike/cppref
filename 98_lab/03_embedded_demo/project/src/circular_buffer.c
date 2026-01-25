/**
 * @file circular_buffer.c
 * @brief Circular buffer implementation
 */

#include "circular_buffer.h"
#include <string.h>

int circular_buffer_init(circular_buffer_t *cb, uint8_t *buffer, size_t capacity) {
    if (cb == NULL || buffer == NULL || capacity == 0) {
        return -1;
    }
    
    cb->buffer = buffer;
    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    
    /* Clear the buffer */
    memset(buffer, 0, capacity);
    
    return 0;
}

int circular_buffer_put(circular_buffer_t *cb, uint8_t data) {
    if (cb == NULL) {
        return -1;
    }
    
    /* Check if buffer is full */
    if (cb->count >= cb->capacity) {
        return -1;
    }
    
    /* Write data */
    cb->buffer[cb->head] = data;
    
    /* Update head position (wrap around) */
    cb->head = (cb->head + 1) % cb->capacity;
    
    /* Update count */
    cb->count++;
    
    return 0;
}

int circular_buffer_get(circular_buffer_t *cb, uint8_t *data) {
    if (cb == NULL || data == NULL) {
        return -1;
    }
    
    /* Check if buffer is empty */
    if (cb->count == 0) {
        return -1;
    }
    
    /* Read data */
    *data = cb->buffer[cb->tail];
    
    /* Update tail position (wrap around) */
    cb->tail = (cb->tail + 1) % cb->capacity;
    
    /* Update count */
    cb->count--;
    
    return 0;
}

bool circular_buffer_is_empty(const circular_buffer_t *cb) {
    if (cb == NULL) {
        return true;
    }
    
    return (cb->count == 0);
}

bool circular_buffer_is_full(const circular_buffer_t *cb) {
    if (cb == NULL) {
        return false;
    }
    
    return (cb->count >= cb->capacity);
}

size_t circular_buffer_size(const circular_buffer_t *cb) {
    if (cb == NULL) {
        return 0;
    }
    
    return cb->count;
}

size_t circular_buffer_available(const circular_buffer_t *cb) {
    if (cb == NULL) {
        return 0;
    }
    
    return (cb->capacity - cb->count);
}

int circular_buffer_clear(circular_buffer_t *cb) {
    if (cb == NULL) {
        return -1;
    }
    
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    
    return 0;
}
