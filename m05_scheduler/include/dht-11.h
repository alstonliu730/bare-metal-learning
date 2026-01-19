#ifndef __DHT_H__
#define __DHT_H__

#include <stdint.h>

#define MAX_TIMINGS             85
#define DHT_GPIO_PIN            24

#define MAX_DHT_INPUT           5
#define MAX_DATA_BITS           40
#define DHT_TIMEOUT             255

// microsecond delay for the data
#define DHT_DATA_1              70
#define DHT_DATA_0_LOW          24
#define DHT_DATA_0_HIGH         30

#define CHECKSUM_MASK           0xFF
/**
 * Reads the DHT11 temperature sensor
 */
void read_dht11_data(int* data);
#endif /* __DHT_H__*/