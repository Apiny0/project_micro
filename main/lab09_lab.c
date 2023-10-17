#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define BUTTON_GPIO 14 

uint32_t button_count = 0;
QueueHandle_t button_queue;

void taskProducer(void *pvParameters){
    while(1){
        vTaskDelay(500 / portTICK_PERIOD_MS); 
        if (gpio_get_level(BUTTON_GPIO) == 0) {
            button_count++;
            xQueueSend(button_queue, &button_count, portMAX_DELAY);
            printf("Producer: BUTTON: %ld \n",button_count);
        }else{
             button_count = 0;
             printf("Producer: BUTTON: %ld \n",button_count);
        }
    }
}

void taskConsumer(void *pvParameters){
    uint32_t data;
    while(1){
        if (xQueueReceive(button_queue, &data, portMAX_DELAY)){
            printf("Consumer: Button pressed: %lu\n\n", data);
        }
    }
}

void app_main(void)
{
    button_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    xTaskCreate(taskProducer, "Producer", 2048, NULL, 10, NULL);
    xTaskCreate(taskConsumer, "Consumer", 2048, NULL, 10, NULL);

    while(1){
        // printf("\nBUTTON: %ld \n",button_count);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}
