/**
 * @file main.c
 * @brief Demo application for GPIO and circular buffer modules
 * 
 * This simple application demonstrates:
 * - GPIO pin initialization and control
 * - Circular buffer for data storage
 * - Integration of multiple modules
 */

#include <stdio.h>
#include "gpio_driver.h"
#include "circular_buffer.h"

/* External test helper for simulating inputs */
extern void gpio_simulate_input(uint8_t port, uint8_t pin, gpio_state_t state);

int main(void) {
    printf("========================================\n");
    printf("  Embedded C Demo Application\n");
    printf("========================================\n\n");
    
    /* ===== GPIO Demo ===== */
    printf("--- GPIO Driver Demo ---\n");
    
    /* Initialize LED pin (output) */
    gpio_config_t led_config = {
        .port = 0,
        .pin = 5,
        .dir = GPIO_OUTPUT,
        .pull_up = false
    };
    
    if (gpio_init(&led_config) == 0) {
        printf("✓ LED initialized (Port 0, Pin 5)\n");
    }
    
    /* Turn LED on */
    gpio_write(0, 5, GPIO_HIGH);
    printf("  LED turned ON\n");
    
    /* Read back the state */
    gpio_state_t led_state;
    gpio_read(0, 5, &led_state);
    printf("  LED state: %s\n", led_state == GPIO_HIGH ? "HIGH" : "LOW");
    
    /* Toggle LED */
    gpio_toggle(0, 5);
    gpio_read(0, 5, &led_state);
    printf("  After toggle: %s\n\n", led_state == GPIO_HIGH ? "HIGH" : "LOW");
    
    /* Initialize button pin (input with pull-up) */
    gpio_config_t button_config = {
        .port = 1,
        .pin = 3,
        .dir = GPIO_INPUT,
        .pull_up = true
    };
    
    if (gpio_init(&button_config) == 0) {
        printf("✓ Button initialized (Port 1, Pin 3)\n");
    }
    
    /* Read button state */
    gpio_state_t button_state;
    gpio_read(1, 3, &button_state);
    printf("  Button state (with pull-up): %s\n", 
           button_state == GPIO_HIGH ? "HIGH (not pressed)" : "LOW (pressed)");
    
    /* Simulate button press */
    gpio_simulate_input(1, 3, GPIO_LOW);
    gpio_read(1, 3, &button_state);
    printf("  After simulated press: %s\n\n", 
           button_state == GPIO_HIGH ? "HIGH" : "LOW");
    
    /* ===== Circular Buffer Demo ===== */
    printf("--- Circular Buffer Demo ---\n");
    
    /* Create buffer storage */
    uint8_t uart_storage[16];
    circular_buffer_t uart_rx_buffer;
    
    if (circular_buffer_init(&uart_rx_buffer, uart_storage, sizeof(uart_storage)) == 0) {
        printf("✓ UART RX buffer initialized (16 bytes)\n");
    }
    
    /* Simulate receiving UART data */
    const char *message = "Hello!";
    printf("  Simulating UART RX: \"%s\"\n", message);
    
    for (int i = 0; message[i] != '\0'; i++) {
        circular_buffer_put(&uart_rx_buffer, (uint8_t)message[i]);
    }
    
    printf("  Buffer size: %zu/%zu bytes\n", 
           circular_buffer_size(&uart_rx_buffer),
           circular_buffer_available(&uart_rx_buffer) + circular_buffer_size(&uart_rx_buffer));
    
    /* Read back the data */
    printf("  Reading from buffer: \"");
    uint8_t data;
    while (!circular_buffer_is_empty(&uart_rx_buffer)) {
        if (circular_buffer_get(&uart_rx_buffer, &data) == 0) {
            printf("%c", data);
        }
    }
    printf("\"\n");
    
    printf("  Buffer now empty: %s\n\n", 
           circular_buffer_is_empty(&uart_rx_buffer) ? "YES" : "NO");
    
    /* ===== Wraparound Demo ===== */
    printf("--- Buffer Wraparound Demo ---\n");
    
    /* Fill buffer completely */
    printf("  Filling buffer with sequence 0-15...\n");
    for (uint8_t i = 0; i < 16; i++) {
        circular_buffer_put(&uart_rx_buffer, i);
    }
    printf("  Buffer full: %s\n", 
           circular_buffer_is_full(&uart_rx_buffer) ? "YES" : "NO");
    
    /* Remove half */
    printf("  Removing first 8 bytes...\n");
    for (int i = 0; i < 8; i++) {
        circular_buffer_get(&uart_rx_buffer, &data);
    }
    
    /* Add more (wraparound) */
    printf("  Adding bytes 100-107 (wraparound)...\n");
    for (uint8_t i = 100; i < 108; i++) {
        circular_buffer_put(&uart_rx_buffer, i);
    }
    
    /* Read all and verify order */
    printf("  Reading all: ");
    while (!circular_buffer_is_empty(&uart_rx_buffer)) {
        circular_buffer_get(&uart_rx_buffer, &data);
        printf("%d ", data);
    }
    printf("\n");
    
    printf("\n========================================\n");
    printf("  Demo Complete!\n");
    printf("========================================\n");
    
    return 0;
}
