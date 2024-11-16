#include "ds18b20.h"
#ifndef CONSTANTS // Header guard to prevent multiple inclusions
#define CONSTANTS

static const char *TAG = "ESP-NOW Temperature Sender";

// Debug Clock ESP32 MAC address: 24:6f:28:24:75:1c
// True Clock ESP32 MAC address: e0:e2:e6:0d:72:14

// MAC address of the receiver
const uint8_t receiver_mac[] = {0xe0, 0xe2, 0xe6, 0x0d, 0x72, 0x14}; 



typedef struct struct_message {
    int id;
    int sec_since_last_update;
    float temperature_outside;
    float temperature_inside;
} struct_message;

const uint8_t GPIO_DS18B20_0 = 5;
const uint8_t SELECTED_DS18B20_RESOLUTION = DS18B20_RESOLUTION_12_BIT;
const uint16_t SAMPLE_PERIOD = 1000;   // milliseconds
#endif 