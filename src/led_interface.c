#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "inttypes.h"


typedef struct
{
	uint32_t gpio_no, on_time, off_time;
}led_info_t;


void blink_task(void *pvParameter);


static TaskHandle_t g_blink_task_handle;
static led_info_t g_led_info;

void led_blink_init(uint32_t led_gpio_no, uint32_t led_on_time_ms, uint32_t led_off_time_ms)
{
	if(g_blink_task_handle == 0)
	{
		xTaskCreate(&blink_task, "blink_k", 1024, NULL, 1, &g_blink_task_handle);
	}
	g_led_info.gpio_no = led_gpio_no;
	g_led_info.on_time = led_on_time_ms;
	g_led_info.off_time = led_off_time_ms;
}

void blink_task(void *pvParameter)
{
    gpio_config_t config1 = 
    {
    	(1 << g_led_info.gpio_no), 
    	GPIO_MODE_OUTPUT, 
    	GPIO_PULLUP_ENABLE,
    	GPIO_PULLDOWN_DISABLE,
    	GPIO_INTR_DISABLE
    };

    gpio_config(&config1);
    
    while(1) 
	{		
		gpio_set_level(g_led_info.gpio_no, 1);
		vTaskDelay(g_led_info.on_time /portTICK_PERIOD_MS);
		gpio_set_level(g_led_info.gpio_no, 0);
		vTaskDelay(g_led_info.off_time/portTICK_PERIOD_MS);
    }
}