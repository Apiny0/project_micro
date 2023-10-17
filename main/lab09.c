#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define STACK_SIZE 2048
#define TASK_PRIPRITY 10

const char *pcTextForTask1 = "Task 1 is runing\r\n";
const char *pcTextForTask2 = "Task 2 is runing\r\n";

void vTaskFunction(void *pvParameters){
    while(1){
        printf("%s",(char*)pvParameters);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
void app_main(void)
{
    xTaskCreate(vTaskFunction, "Task 1",STACK_SIZE,(void*)pcTextForTask1,TASK_PRIPRITY, NULL);
    xTaskCreate(vTaskFunction, "Task 2", STACK_SIZE, (void *)pcTextForTask2,TASK_PRIPRITY, NULL);

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
