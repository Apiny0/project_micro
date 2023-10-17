#include "pro.h"

static const char *TAG = "ESP32";
static QueueHandle_t switch_queue = NULL;

void gpio_init() {
    gpio_config_t io_conf;
    esp_rom_gpio_pad_select_gpio(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(SWITCH_GPIO);
    gpio_set_direction(SWITCH_GPIO, GPIO_MODE_INPUT);
    io_conf.pin_bit_mask = (1ULL << SWITCH_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    esp_rom_gpio_pad_select_gpio(MOTION_SENSOR_GPIO);
    gpio_set_direction(MOTION_SENSOR_GPIO, GPIO_MODE_INPUT);

    esp_rom_gpio_pad_select_gpio(LIGHT_SENSOR_GPIO);
    gpio_set_direction(LIGHT_SENSOR_GPIO, GPIO_MODE_INPUT);

}
void switch_mode_task(void *pvParameter) {
    int switch_mode;
    while (1) {
        if (xQueueReceive(switch_queue, &switch_mode, portMAX_DELAY) == pdPASS) {
            if (switch_mode == 1) {
                ESP_LOGI(TAG, "Switch turned ON");
            } else {
                ESP_LOGI(TAG, "Switch turned OFF");
            }
        }
    }
}
void app_main() {
    nvs_flash_init();
    gpio_init();

    switch_queue = xQueueCreate(1, sizeof(int));

    xTaskCreate(switch_mode_task, "switch_mode_task", 2048, NULL, 10, NULL);

    bool last_switch_state = false;
    int current_mode = 0;

    TickType_t start_time = 0;
    bool relay_active = false;
    int light_blink_count = 0;
    bool light_on = false;

    while (1) {
        int current_switch_state = gpio_get_level(SWITCH_GPIO);

        if (current_switch_state != last_switch_state) {
            if (current_switch_state == 1) {
                current_mode = 1;
            } else {
                current_mode = 0; 
            }
            last_switch_state = current_switch_state;

            xQueueSend(switch_queue, &current_mode, portMAX_DELAY);
        }

        int motion_value = gpio_get_level(MOTION_SENSOR_GPIO);
        int light_value = gpio_get_level(LIGHT_SENSOR_GPIO);

        if (current_mode == 0) {
            ESP_LOGI(TAG, "select 0");
            if (light_value == 1) {
                ESP_LOGI(TAG, "Light 1");
                gpio_set_level(RELAY_GPIO, 1);
                relay_active = true;
                start_time = xTaskGetTickCount();
                light_on = true;
            } else {
                if (relay_active && (xTaskGetTickCount() - start_time >= 2000 / portTICK_PERIOD_MS)) {
                    gpio_set_level(RELAY_GPIO, 0);
                    relay_active = false;
                }
                if (light_on) {
                    light_blink_count++;
                    if (light_blink_count >= 5) {  
                        light_on = false;
                        light_blink_count = 0;
                    }
                }
                ESP_LOGI(TAG, "Line 0 ");
            }
        } else {
            ESP_LOGI(TAG, "select 1");
            if (motion_value == 1 && light_value == 1) {
                ESP_LOGI(TAG, "Motion and light ");
                gpio_set_level(RELAY_GPIO, 1);
                relay_active = true;
                start_time = xTaskGetTickCount();
            } else {
                if (relay_active && (xTaskGetTickCount() - start_time >= 2000 / portTICK_PERIOD_MS)) {
                    gpio_set_level(RELAY_GPIO, 0);
                    relay_active = false;
                }
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Loop");
    }
}
