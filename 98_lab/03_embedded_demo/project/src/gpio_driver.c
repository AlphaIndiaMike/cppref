/**
 * @file gpio_driver.c
 * @brief GPIO driver implementation
 */

#include "gpio_driver.h"
#include <string.h>

#define MAX_PORTS 16
#define MAX_PINS 16

/* Hardware abstraction - in real embedded systems, 
   these would map to actual hardware registers */
typedef struct {
    uint16_t direction;  /* Direction register (0=input, 1=output) */
    uint16_t output;     /* Output data register */
    uint16_t input;      /* Input data register (simulated) */
    uint16_t pullup;     /* Pull-up enable register */
    bool initialized[MAX_PINS];  /* Track initialization per pin */
} gpio_port_regs_t;

/* Simulated hardware registers */
static gpio_port_regs_t gpio_ports[MAX_PORTS];

int gpio_init(const gpio_config_t *config) {
    /* Validate parameters */
    if (config == NULL) {
        return -1;
    }
    
    if (config->port >= MAX_PORTS || config->pin >= MAX_PINS) {
        return -1;
    }
    
    gpio_port_regs_t *port_regs = &gpio_ports[config->port];
    uint16_t pin_mask = (1 << config->pin);
    
    /* Configure direction */
    if (config->dir == GPIO_OUTPUT) {
        port_regs->direction |= pin_mask;
    } else {
        port_regs->direction &= ~pin_mask;
    }
    
    /* Configure pull-up */
    if (config->pull_up) {
        port_regs->pullup |= pin_mask;
        /* For inputs with pull-up, default to high */
        if (config->dir == GPIO_INPUT) {
            port_regs->input |= pin_mask;
        }
    } else {
        port_regs->pullup &= ~pin_mask;
    }
    
    /* Initialize output to low if it's an output pin */
    if (config->dir == GPIO_OUTPUT) {
        port_regs->output &= ~pin_mask;
    }
    
    /* Mark pin as initialized */
    port_regs->initialized[config->pin] = true;
    
    return 0;
}

int gpio_write(uint8_t port, uint8_t pin, gpio_state_t state) {
    /* Validate parameters */
    if (port >= MAX_PORTS || pin >= MAX_PINS) {
        return -1;
    }
    
    gpio_port_regs_t *port_regs = &gpio_ports[port];
    
    /* Check if pin is initialized */
    if (!port_regs->initialized[pin]) {
        return -1;
    }
    
    uint16_t pin_mask = (1 << pin);
    
    /* Check if pin is configured as output */
    if (!(port_regs->direction & pin_mask)) {
        return -1;  /* Pin is input */
    }
    
    /* Write the state */
    if (state == GPIO_HIGH) {
        port_regs->output |= pin_mask;
    } else {
        port_regs->output &= ~pin_mask;
    }
    
    return 0;
}

int gpio_read(uint8_t port, uint8_t pin, gpio_state_t *state) {
    /* Validate parameters */
    if (port >= MAX_PORTS || pin >= MAX_PINS || state == NULL) {
        return -1;
    }
    
    gpio_port_regs_t *port_regs = &gpio_ports[port];
    
    /* Check if pin is initialized */
    if (!port_regs->initialized[pin]) {
        return -1;
    }
    
    uint16_t pin_mask = (1 << pin);
    uint16_t reg_value;
    
    /* Read from input register for inputs, output register for outputs */
    if (port_regs->direction & pin_mask) {
        reg_value = port_regs->output;  /* Output pin - read back what was written */
    } else {
        reg_value = port_regs->input;   /* Input pin - read input register */
    }
    
    *state = (reg_value & pin_mask) ? GPIO_HIGH : GPIO_LOW;
    
    return 0;
}

int gpio_toggle(uint8_t port, uint8_t pin) {
    gpio_state_t current_state;
    
    /* Read current state */
    if (gpio_read(port, pin, &current_state) != 0) {
        return -1;
    }
    
    /* Write opposite state */
    gpio_state_t new_state = (current_state == GPIO_HIGH) ? GPIO_LOW : GPIO_HIGH;
    return gpio_write(port, pin, new_state);
}

/* Test helper function - simulates external input change */
void gpio_simulate_input(uint8_t port, uint8_t pin, gpio_state_t state) {
    if (port < MAX_PORTS && pin < MAX_PINS) {
        uint16_t pin_mask = (1 << pin);
        if (state == GPIO_HIGH) {
            gpio_ports[port].input |= pin_mask;
        } else {
            gpio_ports[port].input &= ~pin_mask;
        }
    }
}
