#include <dht-11.h>
#include <common.h>
#include <uart.h>
#include <gpio.h>
#include <timer.h>
#include <irq.h>

void read_dht11_data(int* data) {
    // uart_printf("Attempting to read dht11 data...\n");
    // check if data is not null
    if (data == NULL) {
        uart_printf("read_dht11_data: invalid data array input address.\n");
        return;
    }

    uint8_t laststate = 1;
    uint8_t counter   = 0;
    uint8_t i, bits = 0;

    // clean data output
    for(int k = 0; k < MAX_DHT_INPUT; k++) {
        data[k] = 0;
    }

    // make this uninterruptible
    irq_disable();
    // send start signal to DHT
    gpio_function(DHT_GPIO_PIN, GPIO_FUNCTION_OUT);
    gpio_clear(DHT_GPIO_PIN, GPIO_ENABLE);
    wait_ms(18);
    gpio_set(DHT_GPIO_PIN, GPIO_ENABLE);
    wait_us(40);

    // set DHT to input
    gpio_function(DHT_GPIO_PIN, GPIO_FUNCTION_IN);
    gpio_pull(DHT_GPIO_PIN, PULL_UP);

    for(i = 0; i < MAX_TIMINGS; i++) {
        // reset counter 
        counter = 0;
        uint64_t now = get_timer64();
        // keep count of how many microseconds occurred
        while (gpio_read(DHT_GPIO_PIN) == laststate) {
            counter = get_timer64() - now;
            if (counter >= DHT_TIMEOUT) {
                break;
            }
        }

        // update the last state
        laststate = gpio_read(DHT_GPIO_PIN);

        // if the counter has been running long
        if (counter >= DHT_TIMEOUT) {
            break;
        } 
        
        // skip to the data transmission
        if ((i >= 4) && (i % 2 == 0)) {
            data[bits / 8] <<= 1;
            if (counter > DHT_DATA_0_HIGH)  {
                data[bits / 8] |= 1;
            }
            bits++;
        }
    }

    // Check if the temperature returned the right amount of data
    if ((bits < 40) || (data[4] != ((data[3] + data[2] + data[1] + data[0]) & CHECKSUM_MASK))) 
    {
        irq_enable();
        uart_printf("Invalid data, clearing data array.\n");
        uart_printf("[0] = %d, [1] = %d, [2] = %d, [3] = %d, [4] = %d\n", 
                        data[0], data[1], data[2], data[3], data[4]);
        // memset(data, 0, sizeof(int) * MAX_DHT_INPUT); // clean data
        return;
    } 
    irq_enable();

    // uart_printf("Data Received\n");
    return;
}
