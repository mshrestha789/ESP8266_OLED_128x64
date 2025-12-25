#include "oled_128x64.h"
#include "oled_cmd.h"
#include "font_basic/font8x8_basic.h"
#include "font_basic/basic_font1.h"

#include "i2c_interface.h"
#include "driver/i2c.h"
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef OLED_DEBUG
#define oled_debug(fmt, ...) printf("\n %s: " fmt , "oled", ## __VA_ARGS__)
#else
#define oled_debug(fmt, ...)
#endif

#define OLED_HEIGHT     64
#define OLED_WIDTH      128

// Optimization: Send 128 bytes (1 page) at a time to minimize I2C Start/Stop overhead
// 128 bytes fits well within standard I2C timeouts and buffers
#define OLED_I2C_CHUNK_SIZE 128 

typedef uint32_t (*oled_callback_ptr)(oled_info_t *info);
static oled_callback_ptr fire_oled_info;

static uint32_t oled_send(oled_info_type_t info_type, uint8_t  address, uint8_t register_add, uint8_t *data, uint32_t data_len);

static uint8_t g_frame_buffer[OLED_WIDTH * (OLED_HEIGHT/8)];

uint32_t oled_init(uint32_t (*oled_callback)(oled_info_t *info)) 
{
    uint32_t ret_val = 0;
    uint8_t cmd[] = 
    {     
        OLED_CMD_DISPLAY_OFF,
        OLED_CMD_SET_DISPLAY_CLK_DIV, 0x80,
        OLED_CMD_SET_MUX_RATIO, (OLED_HEIGHT - 1), 
        OLED_CMD_SET_DISPLAY_OFFSET, 0x00,
        OLED_CMD_SET_DISPLAY_START_LINE, 0x00,
        OLED_CMD_SET_CHARGE_PUMP, 0x14,
        OLED_CMD_SET_MEMORY_ADDR_MODE, 0x00,
        OLED_CMD_SET_SEGMENT_REMAP,
        OLED_CMD_SET_COM_SCAN_MODE_REMAP,
        OLED_CMD_SET_COM_PIN_MAP, 0x12,
        OLED_CMD_SET_CONTRAST, 0xCF,
        OLED_CMD_SET_PRECHARGE, 0xF1,
        OLED_CMD_SET_VCOMH_DESELCT, 0x40,
        OLED_CMD_DISPLAY_RAM,
        OLED_CMD_DISPLAY_NORMAL,
        OLED_DEACTIVATE_SCROLL, 
        OLED_CMD_DISPLAY_ON
    };
    fire_oled_info = oled_callback;
    
    if(oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, cmd, sizeof(cmd)))
    {
        oled_debug("Line 68: Oled init error; First initialize communication protocol");
        ret_val = 1;
    }    
    return ret_val;
}

static uint32_t oled_send(oled_info_type_t info_type, uint8_t  address, uint8_t register_add, uint8_t *data, uint32_t data_len)
{
    oled_info_t oled_info;
    oled_info.info_type = info_type;
    oled_info.address = address;
    oled_info.register_add = register_add;
    oled_info.data = data;
    oled_info.data_len = data_len;
    return fire_oled_info(&oled_info);
}

uint32_t oled_invert_display(uint8_t t)
{
    uint32_t ret_val = 0;
    uint8_t cmd = OLED_CMD_SET_SEGMENT_NORMAL_REMAP;    
    if(!t)
    {
        cmd = OLED_CMD_SET_SEGMENT_REMAP;
    }
    if(oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, &cmd, 1))
    {
        oled_debug("Line 93: Oled invert display error");
        ret_val = 1;
    } 
    return ret_val;
}

uint32_t oled_reverse_display_scan(uint8_t t)
{
    uint32_t ret_val = 0;
    uint8_t cmd = OLED_CMD_SET_COM_SCAN_MODE_NORMAL;
    if(!t)
    {
        cmd = OLED_CMD_SET_COM_SCAN_MODE_REMAP;
    }
    if(oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, &cmd, 1))
    {
        oled_debug("Line 93: Oled reverse display error");
        ret_val = 1;
    } 
    return ret_val;   
}

void oled_display_text(uint8_t *text, uint32_t text_len) 
{    
    uint8_t current_page = 0;
    uint8_t cmd[] = 
    {       
        OLED_CMD_SET_COLUMN_RANGE, 0x00, (OLED_WIDTH - 1),
        OLED_CMD_SET_PAGE_RANGE, current_page, ((OLED_HEIGHT/8)-1),
    };    
    
    // Initialize display area
    oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, cmd, sizeof(cmd)); 

    // Optimization: Buffer font data to send larger I2C packets
    // 64 bytes holds 8 characters (8x8 font), a reasonable chunk size
    uint8_t buffer[64]; 
    uint32_t buf_idx = 0;

    for (uint32_t i = 0; i < text_len; i++) 
    {
        if (text[i] == '\n') 
        {
            // Flush any pending data in buffer before changing page
            if (buf_idx > 0)
            {
                oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_DATA_STREAM, buffer, buf_idx);
                buf_idx = 0;
            }

            if(current_page++ == (OLED_HEIGHT/8))
            {
                current_page = 0;
            }
            // Update page command
            cmd[1] = 0x00; // Reset column start
            cmd[4] = current_page; // Update page start
            oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, cmd, sizeof(cmd)); 
        }
        else
        {
            // Copy font data to buffer
            memcpy(&buffer[buf_idx], font8x8_basic_tr[(uint8_t)text[i]], 8);
            buf_idx += 8;

            // If buffer is full, send it
            if (buf_idx >= sizeof(buffer))
            {
                oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_DATA_STREAM, buffer, buf_idx);
                buf_idx = 0;
            }
        }
    }
    
    // Flush remaining data
    if (buf_idx > 0)
    {
        oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_DATA_STREAM, buffer, buf_idx);
    }
}

static void oled_load_frame(uint8_t buf[])
{
    size_t len = OLED_WIDTH * (OLED_HEIGHT / 8);
    uint8_t cmd[] = 
    {       
        OLED_CMD_SET_COLUMN_RANGE, 0x00, (OLED_WIDTH - 1),
        OLED_CMD_SET_PAGE_RANGE, 0x00, ((OLED_HEIGHT / 8) - 1), 
    };    

    oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, cmd, sizeof(cmd));    
    
    // Optimization: Use a static zero-buffer for clearing logic to avoid stack overflow
    // and sending data in larger chunks (OLED_I2C_CHUNK_SIZE)
    static const uint8_t zero_buf[OLED_I2C_CHUNK_SIZE] = {0}; 

    for (uint32_t i = 0; i < len; i += OLED_I2C_CHUNK_SIZE) 
    {
        // Calculate remaining bytes to handle potential uneven sizes (though 1024 is divisible by 128)
        uint32_t current_chunk_size = (len - i) < OLED_I2C_CHUNK_SIZE ? (len - i) : OLED_I2C_CHUNK_SIZE;
        
        oled_send(OLED_SEND_DATA, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_DATA_STREAM, 
                  buf ? &buf[i] : (uint8_t*)zero_buf, current_chunk_size);
    }
    
    // Yield only once per frame if needed, rather than per chunk
    oled_send(OLED_FUNCTION_YEILD, (OLED_I2C_ADDRESS << 1), OLED_CONTROL_BYTE_CMD_STREAM, cmd, sizeof(cmd));
}

void oled_clear_display()
{   
    memset(g_frame_buffer, 0, (OLED_WIDTH * (OLED_HEIGHT/8)));   
    oled_load_frame(g_frame_buffer); 
}

void oled_load_xbm(uint8_t *xbm)
{
    uint8_t bit = 0;
    int32_t row = 0;
    int32_t column = 0;
    
    // Optimization: Simplified pointer arithmetic logic could go here, 
    // but the main bottleneck is I2C, which is fixed in oled_load_frame.
    for (row = 0; row < OLED_HEIGHT; row++) 
    {
        for (column = 0; column < OLED_WIDTH / 8; column++) 
        {
            uint16_t xbm_offset = row * 16 + column;
            for (bit = 0; bit < 8; bit++) 
            {
                if (*(xbm + xbm_offset) & (1 << bit)) 
                {
                    *(g_frame_buffer + OLED_WIDTH * (row / 8) + column * 8 + bit) |= (1 << (row % 8));
                }
            }
        }
    }

    oled_load_frame(g_frame_buffer);
}

// Optimization: Marked static inline as it is a helper
static inline int32_t oled_draw_pixel(uint8_t *fb, int8_t x, int8_t y, oled_color_t color)
{
    if ((x >= OLED_WIDTH) || (x < 0) || (y >= OLED_HEIGHT) || (y < 0))
    {
        return -1;
    }

    uint16_t index = x + (y / 8) * OLED_WIDTH;
    
    if (color == OLED_COLOR_WHITE) {
        fb[index] |= (1 << (y & 7));
    } else {
        fb[index] &= ~(1 << (y & 7));
    }
    return 0;
}

int32_t oled_prepare_string_frame(const font_info_t *font, uint8_t x, uint8_t y, const char *str, oled_color_t foreground, oled_color_t background)
{
    uint8_t t = x;
    int err;

    if (font == NULL || str == NULL)
    {
        return 0;
    }

    while (*str) 
    {
        // Helper function to draw individual characters (internal)
        // Note: We replicate the draw logic here or wrap the original draw_char
        // For simplicity, we keep the original logic flow calling a helper
        const font_char_desc_t *d = font_get_char_desc(font, *str);
        if (d != NULL)
        {
             const uint8_t *bitmap = font->bitmap + d->offset;
             for (uint8_t j = 0; j < font->height; ++j) 
             {
                 for (uint8_t i = 0; i < d->width; ++i) 
                 {
                     uint8_t line = 0;
                     if (i % 8 == 0) 
                     {
                         line = bitmap[(d->width + 7) / 8 * j + i / 8];
                     }
                     
                     // Optimization: bit check directly
                     if (line & (0x80 >> (i % 8))) 
                     {
                         oled_draw_pixel(g_frame_buffer, x + i, y + j, foreground);
                     }
                     else 
                     {
                         oled_draw_pixel(g_frame_buffer, x + i, y + j, background);
                     }
                 }
             }
             x += d->width;
        }
        
        ++str;
        if (*str)
        {
            x += font->c;
        }
    }
    
    return x - t;
}

void oled_display_prepared_frame()
{
    oled_load_frame(g_frame_buffer);
}