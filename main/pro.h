#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/adc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"

#include "esp_system.h"
#include "nvs_flash.h"

#define RELAY_GPIO                  CONFIG_RELAY_OUTPUT

#define LIGHT_SENSOR_GPIO           CONFIG_LIGHT_SENSOR_INPUT
#define MOTION_SENSOR_GPIO          CONFIG_MOTION_SENSOR_INPUT
#define SWITCH_GPIO                 CONFIG_SWITCH_GPIO_INPUT

