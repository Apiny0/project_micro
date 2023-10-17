#include <stdio.h>
#include <inttypes.h>
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define GPIO_OUTPUT_IO_0 CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1 CONFIG_GPIO_OUTPUT_1

#define GPIO_OUTPUT_PIN_SET ((1ULL<<GPIO_OUTPUT_IO_0)|(1ULL<<GPIO_OUTPUT_IO_1))

#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_PIN_POLL_SEL (1ULL<<GPIO_INPUT_IO_0)
#define GPIO_INPUT_PIN_INTR_SEL 0

#define ESP_INTR_FLAG_DEFAULT 0
static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    uint32_t led_state = 0;
    int button_pressed = 0;
    
    for (;;)
    {
        //  vTaskDelay(100 / portTICK_PERIOD_MS);
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            printf("GPIO[%d] poll, val: %d\n", GPIO_INPUT_IO_0, gpio_get_level(GPIO_INPUT_IO_0));
            
            if (gpio_get_level(GPIO_INPUT_IO_0) == 1) {
                // vTaskDelay(100 / portTICK_PERIOD_MS);
                button_pressed = 1;
            } else if (button_pressed == 1) {
                printf("Button pressed!\n\n");
                button_pressed = 0; 
            }
            led_state = !led_state; 
            gpio_set_level(GPIO_OUTPUT_IO_1, led_state); 
        }
        
    }
    
}

void app_main(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SET;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_POLL_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*)GPIO_INPUT_IO_0);
    
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    int cnt = 0;
    for (;;)
    {
        printf("cnt : %d\n", cnt++);
        gpio_set_level(GPIO_OUTPUT_IO_0 ,cnt & 0x01);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
