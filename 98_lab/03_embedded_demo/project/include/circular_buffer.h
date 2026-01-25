/**
 * @file circular_buffer.h
 * @brief Thread-safe circular buffer implementation
 * @author Embedded Team
 * @date 2026-01-25
 * 
 * Provides a fixed-size circular buffer for byte data,
 * commonly used in UART, SPI, and other communication drivers.
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Circular buffer structure
 * 
 * Internal structure for managing circular buffer state.
 * Users should not access fields directly.
 */
typedef struct {
    uint8_t *buffer;      /**< Pointer to data buffer */
    size_t capacity;      /**< Maximum number of bytes */
    size_t head;          /**< Write position */
    size_t tail;          /**< Read position */
    size_t count;         /**< Current number of bytes stored */
} circular_buffer_t;

/**
 * @brief Initialize a circular buffer
 * 
 * Prepares a circular buffer for use with the provided storage.
 * The buffer must remain valid for the lifetime of the circular buffer.
 * 
 * @param cb Pointer to circular buffer structure
 * @param buffer Pointer to data storage array
 * @param capacity Size of the data storage array
 * @return 0 on success, -1 on error
 * 
 * @pre cb must not be NULL
 * @pre buffer must not be NULL
 * @pre capacity must be > 0
 * 
 * @code
 * uint8_t storage[256];
 * circular_buffer_t rx_buffer;
 * circular_buffer_init(&rx_buffer, storage, sizeof(storage));
 * @endcode
 */
int circular_buffer_init(circular_buffer_t *cb, uint8_t *buffer, size_t capacity);

/**
 * @brief Write a byte to the circular buffer
 * 
 * Adds a byte to the buffer if space is available.
 * 
 * @param cb Pointer to circular buffer
 * @param data Byte to write
 * @return 0 on success, -1 if buffer is full
 * 
 * @pre cb must be initialized
 */
int circular_buffer_put(circular_buffer_t *cb, uint8_t data);

/**
 * @brief Read a byte from the circular buffer
 * 
 * Removes and returns a byte from the buffer.
 * 
 * @param cb Pointer to circular buffer
 * @param data Pointer to store the read byte
 * @return 0 on success, -1 if buffer is empty
 * 
 * @pre cb must be initialized
 * @pre data must not be NULL
 */
int circular_buffer_get(circular_buffer_t *cb, uint8_t *data);

/**
 * @brief Check if buffer is empty
 * 
 * @param cb Pointer to circular buffer
 * @return true if buffer is empty, false otherwise
 */
bool circular_buffer_is_empty(const circular_buffer_t *cb);

/**
 * @brief Check if buffer is full
 * 
 * @param cb Pointer to circular buffer
 * @return true if buffer is full, false otherwise
 */
bool circular_buffer_is_full(const circular_buffer_t *cb);

/**
 * @brief Get number of bytes in buffer
 * 
 * @param cb Pointer to circular buffer
 * @return Number of bytes currently stored
 */
size_t circular_buffer_size(const circular_buffer_t *cb);

/**
 * @brief Get available space in buffer
 * 
 * @param cb Pointer to circular buffer
 * @return Number of free bytes
 */
size_t circular_buffer_available(const circular_buffer_t *cb);

/**
 * @brief Clear all data from buffer
 * 
 * Resets the buffer to empty state without freeing memory.
 * 
 * @param cb Pointer to circular buffer
 * @return 0 on success, -1 on error
 */
int circular_buffer_clear(circular_buffer_t *cb);

#endif /* CIRCULAR_BUFFER_H */
