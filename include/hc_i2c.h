#ifndef __HC_I2C_H__
#define __HC_I2C_H__

#include <string.h>
#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <i2c/smbus.h> // i2c-tools

#ifdef __cplusplus
}
#endif // __cplusplus


#include "hc_device.h"

namespace HiCreation
{
    class TI2CDev : public TDevFile
    {
        typedef TDevFile inherited;
    public:
        TI2CDev(uint8_t bus, uint8_t addr):
            TDevFile(NULL), FAddr(addr)
        {
            FName = new char[20];
            memset(FName, 0, sizeof(FName));
            sprintf(FName, "/dev/i2c-%d", bus);
        }

        TI2CDev(const char *bus_name, uint8_t addr):
            TDevFile(bus_name), FAddr(addr)
        {}

        virtual int Open(int flag = O_RDWR)
        {
            inherited::Open(flag);
            if (FAddr > 0x7F)
                FAddr = FAddr >> 1;
            if (FHandle <= 0)
                return -ENODEV;

            /*When Force read/write the registers even other used, set I2C_SLAVE_FORCE*/
            return ioctl(FHandle, I2C_SLAVE, FAddr); 
        }

        /*buf idx 0 is command, if count is 0, then read directly*/
        virtual ssize_t Read(unsigned char *buf, size_t count)
        {
            if (FHandle <= 0)
                return -ENODEV;
            
            if (count > 0)
                return i2c_smbus_read_i2c_block_data(FHandle, *buf, count, buf);
            else if (count == 0)
                return i2c_smbus_read_byte(FHandle);
        }

        virtual ssize_t Write(unsigned char *buf, size_t count)
        {
            if (FHandle <= 0)
                return -ENODEV;
            
            if (count > 1)
                return i2c_smbus_write_i2c_block_data(FHandle, *buf, count - 1, buf + 1);
            else if (count == 1)
                return i2c_smbus_write_byte(FHandle, *buf);
            else
                return -EINVAL;
        }

        int ReadByte() { return Read(NULL, 0); }
        int ReadByteData(uint8_t command)
            { return i2c_smbus_read_byte_data(FHandle, command); }
        int ReadWordData(uint8_t command)
            { return i2c_smbus_read_word_data(FHandle, command); }
        int ReadBlockData(uint8_t command, uint8_t *data, int count)
            { return i2c_smbus_read_i2c_block_data(FHandle, command, count, data); }

        int WriteByte(uint8_t value) 
            { return i2c_smbus_write_byte(FHandle, value);}
        int WriteByteData(uint8_t command, uint8_t value)
            { return i2c_smbus_write_byte_data(FHandle, command, value); }
        int WriteWordData(uint8_t command, uint16_t value)
            { return i2c_smbus_write_word_data(FHandle, command, value); }
        int WriteBlockData(uint8_t command, uint8_t *data, int count)
            { return i2c_smbus_write_i2c_block_data(FHandle, command, count, data); }

        uint8_t Addr() { return FAddr; }

    protected:
        uint8_t FAddr;
    };
};

#endif