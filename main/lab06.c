#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE (CONFIG_EXAMPLE_TASK_STACK_SIZE)

static const char *TAG = "UART TEST";
#define BUF_SIZE (1024)
static void echo_task(void *arg){
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    uint8_t *RX_data = (uint8_t *) malloc(BUF_SIZE);
    /*uint8_t *RX_data = (uint8_t *) malloc(BUF_SIZE);*/
    while(1){
        //fflush(stdin);
        printf("Enter data for test loopback: ");
        gets((char*)data);
        //scanf("%s\n",data);

        int len = strlen((const char *)data);
        uart_write_bytes(ECHO_UART_PORT_NUM,(const char *)data,len);
        uart_read_bytes(ECHO_UART_PORT_NUM,RX_data,len,20/portTICK_PERIOD_MS);   
        if(len){
            data[len] = '\0';
            ESP_LOGI(TAG,"Recv str: %s",(char *)data);
        }
         
    }
}
void app_main(void)
{
    xTaskCreate(echo_task, "uart_echo_task",ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
