/**
 * @file gpio_driver.h
 * @brief Simple GPIO driver for embedded systems
 * @author Embedded Team
 * @date 2026-01-25
 * 
 * This module provides a basic GPIO interface for controlling
 * digital pins on embedded microcontrollers.
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief GPIO pin states
 */
typedef enum {
    GPIO_LOW = 0,   /**< Pin is logic low (0V) */
    GPIO_HIGH = 1   /**< Pin is logic high (VCC) */
} gpio_state_t;

/**
 * @brief GPIO pin directions
 */
typedef enum {
    GPIO_INPUT = 0,  /**< Pin configured as input */
    GPIO_OUTPUT = 1  /**< Pin configured as output */
} gpio_direction_t;

/**
 * @brief GPIO pin configuration structure
 */
typedef struct {
    uint8_t port;           /**< GPIO port number (0-15) */
    uint8_t pin;            /**< Pin number within port (0-15) */
    gpio_direction_t dir;   /**< Pin direction */
    bool pull_up;           /**< Enable internal pull-up resistor */
} gpio_config_t;

/**
 * @brief Initialize a GPIO pin
 * 
 * Configures a GPIO pin according to the provided configuration.
 * This function must be called before any other GPIO operations.
 * 
 * @param config Pointer to GPIO configuration structure
 * @return 0 on success, -1 on error
 * 
 * @pre config must not be NULL
 * @pre port must be in range 0-15
 * @pre pin must be in range 0-15
 * 
 * @code
 * gpio_config_t led_config = {
 *     .port = 0,
 *     .pin = 5,
 *     .dir = GPIO_OUTPUT,
 *     .pull_up = false
 * };
 * gpio_init(&led_config);
 * @endcode
 */
int gpio_init(const gpio_config_t *config);

/**
 * @brief Write a value to a GPIO pin
 * 
 * Sets the output state of a GPIO pin. The pin must be
 * configured as an output before calling this function.
 * 
 * @param port GPIO port number (0-15)
 * @param pin Pin number within port (0-15)
 * @param state Desired pin state (GPIO_LOW or GPIO_HIGH)
 * @return 0 on success, -1 on error
 * 
 * @pre Pin must be initialized as output
 * @pre port must be in range 0-15
 * @pre pin must be in range 0-15
 */
int gpio_write(uint8_t port, uint8_t pin, gpio_state_t state);

/**
 * @brief Read the current state of a GPIO pin
 * 
 * Reads the current logic level of a GPIO pin.
 * 
 * @param port GPIO port number (0-15)
 * @param pin Pin number within port (0-15)
 * @param state Pointer to store the pin state
 * @return 0 on success, -1 on error
 * 
 * @pre state must not be NULL
 * @pre port must be in range 0-15
 * @pre pin must be in range 0-15
 */
int gpio_read(uint8_t port, uint8_t pin, gpio_state_t *state);

/**
 * @brief Toggle a GPIO pin
 * 
 * Inverts the current state of a GPIO pin.
 * Pin must be configured as an output.
 * 
 * @param port GPIO port number (0-15)
 * @param pin Pin number within port (0-15)
 * @return 0 on success, -1 on error
 * 
 * @pre Pin must be initialized as output
 * @pre port must be in range 0-15
 * @pre pin must be in range 0-15
 */
int gpio_toggle(uint8_t port, uint8_t pin);

#endif /* GPIO_DRIVER_H */
