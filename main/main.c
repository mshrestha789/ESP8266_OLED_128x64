#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "i2c_interface.h"
#include "oled_128x64.h"

#include "my_logo.XBM"
#include "data/font_terminus_14x28_iso8859_1.h" 

// --- Constants & Config ---
#define LED_GPIO        15
#define TEST_DELAY_MS   3000

// --- Prototypes ---
extern void led_blink_init(uint32_t led_gpio_no, uint32_t led_on_time_ms, uint32_t led_off_time_ms);
uint32_t oled_listener(oled_info_t *info);

// --- Global Font Pointer ---
const font_info_t *font_large = &_fonts_terminus_14x28_iso8859_1_info;

void app_main()
{   
    printf("\n--- System Starting ---\n");

    // 1. Initialize Status LED
    led_blink_init(LED_GPIO, 300, 1000);

    // 2. Initialize I2C
    if(i2c_interface_init() != 0) {
        printf("\n[Error] I2C Init Failed!\n");
        return;
    }
    printf("\n[Info] I2C Initialized");

    // 3. Initialize OLED
    // We pass the listener callback to the driver here
    int retry = 0;
    while(oled_init(&oled_listener) != 0)
    {
        printf("\n[Error] OLED Init Failed, Retrying...");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if(retry++ > 5) {
            printf("\n[Fatal] OLED Init Gave Up.");
            return;
        }
    }
    printf("\n[Info] OLED Initialized");
    
    oled_clear_display();

    // 4. Main Test Loop
    uint8_t demo_text[] = "Hello User!\n\nSystem: Online\nMode: Testing\nStatus: OK";
    
    while(1)
    {
        // --- Test 1: Basic Text Streaming ---
        printf("\n[Test] Streaming Text Mode");
        oled_clear_display();
        oled_display_text(demo_text, sizeof(demo_text));
        vTaskDelay(TEST_DELAY_MS / portTICK_PERIOD_MS);

        // --- Test 2: XBM Bitmap Graphics ---
        printf("\n[Test] XBM Bitmap Mode");
        oled_clear_display();
        // image_bits comes from my_logo.XBM
        oled_load_xbm(image_bits); 
        vTaskDelay(TEST_DELAY_MS / portTICK_PERIOD_MS);

        // --- Test 3: Frame Buffer & Custom Fonts ---
        printf("\n[Test] Frame Buffer & Custom Font");
        oled_clear_display();
        
        // Draw "Hello" centered roughly at top
        oled_prepare_string_frame(font_large, 30, 0, "Hello", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        // Draw "World" centered roughly below it
        oled_prepare_string_frame(font_large, 30, 32, "World", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        
        // Push the composed frame to display
        oled_display_prepared_frame();
        vTaskDelay(TEST_DELAY_MS / portTICK_PERIOD_MS);

        // --- Test 4: Display Inversion ---
        printf("\n[Test] Display Inversion");
        oled_invert_display(1); // Invert colors
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        oled_invert_display(0); // Normal colors
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // --- Test 5: Scan Direction (Screen Flip) ---
        printf("\n[Test] Screen Flip/Rotation");
        oled_reverse_display_scan(1); // Flip 180 (depends on hardware wiring)
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        oled_reverse_display_scan(0); // Restore normal
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// --- Driver Callback Implementation ---
uint32_t oled_listener(oled_info_t *info)
{
    uint32_t ret_val = 0;
    switch(info->info_type)
    {
        case OLED_SEND_DATA:
            // Pass the data buffer directly to the I2C interface
            ret_val = i2c_interface_write_register(info->address, info->register_add, info->data, info->data_len);
        break;

        case OLED_FUNCTION_YEILD:
            // Yield to FreeRTOS scheduler to prevent Watchdog timeouts during long frame updates
            vTaskDelay(0); // yield
            ret_val = 0;
        break;      

        default:
            printf("\n[Warning] Unsupported OLED Listener Command: %d", info->info_type);
            ret_val = 1;
        break;
    }
    return ret_val;
}