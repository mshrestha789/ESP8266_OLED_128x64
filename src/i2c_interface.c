#include "i2c_interface.h"
#include "driver/i2c.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#ifdef I2C_INTERFACE_DEBUG
#define i2c_debug(fmt, ...) printf("\n %s: " fmt , "i2c_interface", ## __VA_ARGS__)
#else
#define i2c_debug(fmt, ...)
#endif



uint32_t i2c_interface_init(void)
{
    uint32_t ret_val = 0;
    uint32_t i2c_master_port = I2C_MASTER_PORT_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_PIN;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_SCL_PIN;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    if(i2c_driver_install(i2c_master_port, conf.mode))
    {
        ret_val = 1;
        i2c_debug("Line 31: driver installation error");
    }
    if(i2c_param_config(i2c_master_port, &conf))
    {
        ret_val = 1;
        i2c_debug("Line 36: param config error");
    }
    return ret_val;
}


uint32_t i2c_interface_write_register(uint8_t address, uint8_t register_add, uint8_t *data, uint32_t data_len)
{
    
    uint32_t ret_val = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if(i2c_master_start(cmd))
    {
        ret_val = 1;
        i2c_debug("Line 50: i2c start error");
    }
    if(!ret_val)
    {
        i2c_debug("Line 54: %x", address|I2C_MASTER_WRITE);
        if(i2c_master_write_byte(cmd, address|I2C_MASTER_WRITE, true))
        {
            ret_val = 1;
            i2c_debug("Line 58: send bytes write byte error");
        }
    }
    if(!ret_val)
    {
        i2c_debug("Line 63: %x", register_add);
        if(i2c_master_write_byte(cmd, register_add, true))
        {
            ret_val = 1;
            i2c_debug("Line 58: send bytes write byte error");
        }
    }
    if(!ret_val)
    {
        for(uint32_t i = 0;  i < data_len; i++)
        {
            i2c_debug("Line 65: %x", *(data + i));
            if(i2c_master_write_byte(cmd, *(data + i), true))
            {
                ret_val = 1;
                i2c_debug("Line 69: send bytes write byte error");
            }
        }
    }    
    if(!ret_val)
    {
        if(i2c_master_stop(cmd))
        {
            ret_val = 1;
            i2c_debug("Line 78: master_stop error");
        }
    }          
    if(!ret_val)
    {
        if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 200/portTICK_PERIOD_MS))
        {
            i2c_debug("Line 85: cmd begin error");
            ret_val = 1;                
        }
    }
    i2c_cmd_link_delete(cmd);
    return ret_val;
}

#if 0
uint32_t i2c_interface_send_bytes(uint8_t address, uint8_t *data, uint32_t data_len)
{
    
    uint32_t ret_val = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if(i2c_master_start(cmd))
    {
        ret_val = 1;
        i2c_debug("Line 50: i2c start error");
    }
    if(!ret_val)
    {
        i2c_debug("Line 54: %x", address|I2C_MASTER_WRITE);
        if(i2c_master_write_byte(cmd, address|I2C_MASTER_WRITE, true))
        {
            ret_val = 1;
            i2c_debug("Line 58: send bytes write byte error");
        }
    }
    if(!ret_val)
    {
        for(uint32_t i = 0;  i < data_len; i++)
        {
            i2c_debug("Line 65: %x", *(data + i));
            if(i2c_master_write_byte(cmd, *(data + i), true))
            {
                ret_val = 1;
                i2c_debug("Line 69: send bytes write byte error");
            }
        }
    }    
    if(!ret_val)
    {
        if(i2c_master_stop(cmd))
        {
            ret_val = 1;
            i2c_debug("Line 78: master_stop error");
        }
    }          
    if(!ret_val)
    {
        if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 200/portTICK_PERIOD_MS))
        {
            i2c_debug("Line 85: cmd begin error");
            ret_val = 1;                
        }
    }
    i2c_cmd_link_delete(cmd);
    return ret_val;
}



uint32_t i2c_interface_send_byte(uint8_t address, uint8_t *data)
{
    
    uint32_t ret_val = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if(i2c_master_start(cmd))
    {
        ret_val = 1;
        i2c_debug("Line 102: i2c start error");
    }
    if(!ret_val)
    {
        i2c_debug("Line 106: %x", address|I2C_MASTER_WRITE);
        if(i2c_master_write_byte(cmd, address|I2C_MASTER_WRITE, true))
        {
            ret_val = 1;
            i2c_debug("Line 110: send bytes write byte error");
        }
    }
    if(!ret_val)
    {
        i2c_debug("Line 115: %x", *data);
        if(i2c_master_write_byte(cmd, *data, true))
        {
            ret_val = 1;
            i2c_debug("Line 119: write error");
        }
    }
    if(!ret_val)
    {
        if(i2c_master_stop(cmd))
        {
            ret_val = 1;
            i2c_debug("Line 127: stop error");
        }
    }
    if(!ret_val)
    {
        if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 200/portTICK_PERIOD_MS))
        {
            i2c_debug("Line 134: cmd begin error");
            ret_val = 1;                
        }
    }    
    i2c_cmd_link_delete(cmd);
    return ret_val;
}
#endif
