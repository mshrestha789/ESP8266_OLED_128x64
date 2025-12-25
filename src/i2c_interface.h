#ifndef __I2C_INTERFACE_H__
#define __I2c_INTERFACE_H__

#include "inttypes.h"

#define I2C_SCL_PIN                 GPIO_NUM_5                /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN                 GPIO_NUM_4               /*!< gpio number for I2C master data  */
#define I2C_MASTER_PORT_NUM         I2C_NUM_0        /*!< I2C port number for master dev */


uint32_t i2c_interface_init(void);
uint32_t i2c_interface_write_register(uint8_t address, uint8_t register_add, uint8_t *data, uint32_t data_len);

#endif
