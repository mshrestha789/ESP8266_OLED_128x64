# ESP8266 OLED 128x64 Driver (FreeRTOS)

A robust, efficient, and feature-rich I2C OLED driver implementation for the ESP8266 using the ESP8266 RTOS SDK (FreeRTOS). This project demonstrates how to interface with a 128x64 SSD1306-based OLED display using a custom lightweight graphics library.

## 🚀 Features

* **Optimized I2C Communication**: Uses page-based chunking (128-byte transactions) to minimize I2C overhead and maximize frame rate.
* **Dual Rendering Modes**:
    * **Direct Streaming**: Stream text directly to the display with automatic page wrapping.
    * **Frame Buffering**: Compose complex scenes (text + graphics) in an internal buffer before pushing to the display.
* **Graphics Support**: Native support for loading XBM bitmaps.
* **Custom Fonts**: Includes a font engine supporting variable-width fonts (e.g., Terminus, Roboto, Bitocra).
* **Hardware Control**: Support for display inversion, 180° rotation (scan direction flip), and contrast control.
* **Clean Architecture**: Separation of concerns between the hardware interface (`i2c_interface`), the driver logic (`oled_128x64`), and the application layer.

## 🛠️ Hardware Requirements

* **MCU**: ESP8266 (e.g., NodeMCU, ESP-01, Wemos D1 Mini).
* **Display**: 0.96" OLED Display (128x64 pixels, I2C interface, SSD1306 controller).
* **Connections**:
    * **VCC**: 3.3V
    * **GND**: Ground
    * **SCL**: GPIO 5 (Default)
    * **SDA**: GPIO 4 (Default)

## 📂 Project Structure

```text
esp8266_oled_lib_test/
├── main/
│   ├── main.c           # Application entry point & Test loop
│   └── component.mk
├── src/
│   ├── oled_128x64.c    # Core OLED driver implementation
│   ├── oled_128x64.h    # OLED driver API
│   ├── i2c_interface.c  # Hardware abstraction layer for I2C
│   ├── fonts/           # Custom font definitions (Terminus, Roboto, etc.)
│   └── my_logo.XBM      # Sample bitmap file
└── README.md
```

## ⚙️ Configuration

The I2C pins are defined in `src/i2c_interface.h`. You can modify them to suit your hardware setup:

```c
#define I2C_SCL_PIN         GPIO_NUM_5  // D1 on NodeMCU
#define I2C_SDA_PIN         GPIO_NUM_4  // D2 on NodeMCU
#define I2C_MASTER_PORT_NUM I2C_NUM_0
```

## 🔨 Build & Flash

This project is built using the **ESP8266 RTOS SDK**.

1.  **Setup Environment**: Ensure you have the toolchain and `IDF_PATH` set up.
2.  **Configure**:
    ```bash
    make menuconfig
    ```
3.  **Compile**:
    ```bash
    make all
    ```
4.  **Flash**:
    ```bash
    make flash monitor
    ```

## 📖 API Usage Examples

### 1. Initialization
```c
#include "oled_128x64.h"

// Initialize the driver with the hardware listener
if (oled_init(&oled_listener) == 0) {
    printf("OLED Initialized successfully\n");
}
```

### 2. Display Text
```c
uint8_t text[] = "Hello World!";
oled_clear_display();
oled_display_text(text, sizeof(text));
```

### 3. Draw Graphics (XBM)
```c
#include "my_image.XBM"
oled_clear_display();
oled_load_xbm(image_bits);
```

### 4. Advanced Frame Composition
```c
// Clear buffer
oled_clear_display();

// Draw text at specific coordinates (X, Y)
oled_prepare_string_frame(font_large, 10, 5, "Temperature:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
oled_prepare_string_frame(font_large, 10, 35, "24.5 C", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

// Push buffer to screen
oled_display_prepared_frame();
```

## 🧩 Debugging

* **I2C Errors**: If you see `I2C Init Failed`, check your wiring and pull-up resistors on SDA/SCL.
* **Garbage Pixels**: Ensure the display reset logic (if used) is correct, though this driver relies on software reset commands.

## ✍️ Author

**Manish Shrestha**

## 📝 License

This project is open-source. Feel free to modify and distribute.