#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GPTimer";

#define LED_PIN 32
static int led_state = 0;

typedef struct {
    uint64_t event_count;
} example_queue_element_t;

static bool IRAM_ATTR example_timer_on_alarm_cd(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    QueueHandle_t queue = (QueueHandle_t)user_data;
    example_queue_element_t ele = {
        .event_count = edata->count_value};
        if(led_state == 0)
        {
            led_state = 1;
            gpio_set_level(LED_PIN, 1);
        }
        else{
            led_state =0;
            gpio_set_level(LED_PIN, 0);
        }
    xQueueSendFromISR(queue, &ele, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}
void app_main(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    
    example_queue_element_t ele;
    QueueHandle_t queue = xQueueCreate(10, sizeof(example_queue_element_t));
    if(!queue){
        ESP_LOGE(TAG, "create queue failed");
        return;
    }

    ESP_LOGI(TAG, "create timer handle");
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = 
    {
        .on_alarm = example_timer_on_alarm_cd,
    };
    
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer,&cbs,queue));
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_LOGI(TAG, "Start timer ,auto-reload at alar, event");
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = 250000,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    int record = 0;
    while(1){
        if (xQueueReceive(queue , &ele ,pdMS_TO_TICKS(2000)))
        {
            ESP_LOGI(TAG, "Timer reloaded ,count=%llu", ele.event_count);
        }
    }
    ESP_LOGI(TAG, "Stop timer");
    ESP_ERROR_CHECK(gptimer_stop(gptimer));
    vQueueDelete(queue);
    }