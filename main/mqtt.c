#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "Lab11_lab.h"

#include "esp_log.h"
#include "mqtt_client.h"


#define LED 2
#define BUTTON 3

static const char *TAG = "MQTT";

static void log_error_if_nonzero(const char *message, int error_code)
{
  if(error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch((esp_mqtt_event_id_t)event_id)
  {
    case MQTT_EVENT_CONNECTED: 
      esp_mqtt_client_publish(client, "/topic/64023195/64021418/LED", "on", 0, 1, 0); //test ข้อมูลที่ส่ง
      // gpio_set_level(LED, 1);
      esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
      esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
      esp_mqtt_client_unsubscribe(client, "/topic/qos1");
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      esp_mqtt_client_publish(client, "/topic/qos0", "data for qos0", 0, 0, 0);
      break;

    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;

    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;

    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("DATA=%.*s\r\n", event->data_len, event->data);
      break;

    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
      if(event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
      {
        log_error_if_nonzero("reported from esp_tls", event->error_handle->esp_tls_last_esp_err);
        log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
        log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
        ESP_LOGI(TAG, "Last error string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

      }
      break;

    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
}

void mqtt_app_start(void)
{
  gpio_set_pull_mode(BUTTON,GPIO_PULLUP_ONLY);
  esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = CONFIG_BROKER_URL,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);
  esp_rom_gpio_pad_select_gpio(BUTTON);
  esp_rom_gpio_pad_select_gpio(LED);

  gpio_set_direction(BUTTON,GPIO_MODE_INPUT);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);

  
  while(1){
    esp_mqtt_client_publish(client, "/topic/64023195/64021418/LED", "on", 1, 1, 1);
        if (gpio_get_level(BUTTON) == 0){  
          esp_mqtt_client_publish(client, "/topic/64023195/64021418/LED", "ON", 0, 1, 0);
          gpio_set_level(LED, 1);
          printf("LED: ON\n");      
        }else{
          esp_mqtt_client_publish(client, "/topic/64023195/64021418/LED", "OFF", 0, 1, 0);
          gpio_set_level(LED, 0);
          printf("LED: OFF\n");        
        }
        vTaskDelay(100);
    }

  
}