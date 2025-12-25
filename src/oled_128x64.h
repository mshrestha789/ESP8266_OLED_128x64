#ifndef __OLED_128X64_H__
#define __OLED_128X64_H__

#include <stdint.h>
#include "fonts/fonts.h"

/**
 * @brief OLED callback command types
 */
typedef enum {
    OLED_SEND_DATA = 0x0,
    OLED_FUNCTION_YEILD = 0x1 // Kept spelling as per implementation
} oled_info_type_t;

/**
 * @brief OLED pixel colors
 */
typedef enum {
    OLED_COLOR_BLACK = 0,        //!< Black (pixel off)
    OLED_COLOR_WHITE = 1,        //!< White (pixel on)    
} oled_color_t;

/**
 * @brief Structure passed to the hardware interface callback
 */
typedef struct {
    oled_info_type_t info_type;
    uint8_t address;
    uint8_t register_add;
    uint8_t *data;
    uint32_t data_len;
} oled_info_t;

/* Public API */

/**
 * @brief Initialize the OLED driver
 * @param oled_callback Function pointer to the hardware/I2C handler
 * @return 0 on success, non-zero on failure
 */
uint32_t oled_init(uint32_t (*oled_callback)(oled_info_t *info));

/**
 * @brief Clear the display buffer and update the screen (turn all pixels off)
 */
void oled_clear_display();

/**
 * @brief Display a string on the OLED
 * @param text Pointer to the string/data
 * @param text_len Length of the data
 */
void oled_display_text(uint8_t *text, uint32_t text_len);

/**
 * @brief Render a string into the internal frame buffer
 * * @param font Pointer to the font definition
 * @param x Starting X coordinate
 * @param y Starting Y coordinate
 * @param str Null-terminated string to draw
 * @param foreground Color of the text
 * @param background Color of the background
 * @return int32_t Total width of the rendered string in pixels
 */
int32_t oled_prepare_string_frame(const font_info_t *font, uint8_t x, uint8_t y, const char *str, 
                                  oled_color_t foreground, oled_color_t background);

/**
 * @brief Push the prepared frame buffer to the physical display
 */
void oled_display_prepared_frame();

/**
 * @brief Load a raw XBM image into the frame buffer and display it
 * @param xbm Pointer to XBM data
 */
void oled_load_xbm(uint8_t *xbm);

/**
 * @brief Invert the display colors
 * @param t 1 to invert, 0 for normal
 * @return 0 on success
 */
uint32_t oled_invert_display(uint8_t t);

/**
 * @brief Flip the display scan direction (rotate 180 degrees effectively if combined with segment remap)
 * @param t 1 to reverse, 0 for normal
 * @return 0 on success
 */
uint32_t oled_reverse_display_scan(uint8_t t);

#endif // __OLED_128X64_H__