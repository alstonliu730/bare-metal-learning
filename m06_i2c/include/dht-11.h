#ifndef __DHT_H__
#define __DHT_H__

#include <stdint.h>

#define DNT_MAX_TIMINGS             85
#define DHT_GPIO_PIN                24

#define DHT_MAX_INPUT               5
#define DHT_MAX_DATA_BITS           40
#define DHT_TIMEOUT                 255

// microsecond delay for the data
#define DHT_DATA_1                  70
#define DHT_DATA_0_LOW              24
#define DHT_DATA_0_HIGH             30

#define DHT_CHECKSUM_MASK           0xFF
/**
 * Reads the DHT11 temperature sensor
 */
int read_dht11_data(int* data);
#endif /* __DHT_H__*/