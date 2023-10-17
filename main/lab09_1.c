#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

uint32_t ulVar = 0;
QueueHandle_t queue;

void taskProduser(void *pvParameters){
    while(1){
        ulVar++;
        xQueueSend(queue,(void*)&ulVar,(TickType_t)10);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
void TaskConsumer(void *pvParameters){
    uint32_t data;
    while(1){
        if(xQueueReceive(queue,&data,portMAX_DELAY)){
            printf("[QUEUE] Recv, val: %ld\r\n",data);
        }
    }
}
void app_main(void)
{
    queue = xQueueCreate(10,sizeof(uint32_t));
    xTaskCreate(taskProduser, "Produser",2048,NULL,10, NULL);
    xTaskCreate(TaskConsumer, "Consumer",2048,NULL,10, NULL);

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}