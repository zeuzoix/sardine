#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <usart.h>

#define MAX_TRIES 50

#define EEPROM_ID    0xA0        // I2C 24AA128 EEPROM Device Identifier
#define EEPROM_ADDR  0x00        // I2C 24AA128 EEPROM Device Address

#define I2C_START 0
#define I2C_DATA  1
#define I2C_STOP  2

static int testbench_debug(char *pc_str)
{
   int i_ret = -1;

   if(NULL == pc_str)
   {
      i_ret = -1;
   }
   else
   {
      usart_async_send_str(pc_str);
      usart_async_send_str("\r\n");
      i_ret = 0;
   }

   return i_ret;
}

unsigned char i2c_transmit(unsigned char type) {
   switch(type) {
      case I2C_START:    // Send Start Condition
         TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
         break;
      case I2C_DATA:     // Send Data
         TWCR = (1 << TWINT) | (1 << TWEN);
         break;
      case I2C_STOP:     // Send Stop Condition
         TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
         return 0;
   }

   // Wait for TWINT flag set in TWCR Register
   while (!(TWCR & (1 << TWINT)));

   // Return TWI Status Register, mask the prescaler bits (TWPS1,TWPS0)
   return (TWSR & 0xF8);
}

int i2c_writebyte(unsigned int i2c_address, unsigned int dev_id,
      unsigned int dev_addr,char data) {
   unsigned char n = 0;
   unsigned char twi_status;
   char r_val = -1;

i2c_retry:
   if (n++ >= MAX_TRIES) 
   {
      testbench_debug("1");
      return r_val;
   }

   // Transmit Start Condition
   twi_status=i2c_transmit(I2C_START);

   // Check the TWI Status
   if (twi_status == TW_MT_ARB_LOST)
   {
      testbench_debug("2");
      goto i2c_retry;
   }
   if ((twi_status != TW_START) && (twi_status != TW_REP_START))
   {
      testbench_debug("3");
      goto i2c_quit;
   }

   // Send slave address (SLA_W)
   TWDR = (dev_id & 0xF0) | ((dev_addr & 0x07) << 1) | TW_WRITE;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if ((twi_status == TW_MT_SLA_NACK) || (twi_status == TW_MT_ARB_LOST))
   {
      testbench_debug("4");
      goto i2c_retry;
   }
   if (twi_status != TW_MT_SLA_ACK)
   {
      testbench_debug("5");
      goto i2c_quit;
   }
#if 0
   // Send the High 8-bit of I2C Address
   TWDR = i2c_address >> 8;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if (twi_status != TW_MT_DATA_ACK)
   {
      testbench_debug("6");
      goto i2c_quit;
   }
#endif

   // Send the Low 8-bit of I2C Address
   TWDR = i2c_address;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if (twi_status != TW_MT_DATA_ACK)
   {
      testbench_debug("7");
      goto i2c_quit;
   }

   // Put data into data register and start transmission
   TWDR = data;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if (twi_status != TW_MT_DATA_ACK)
   {
      testbench_debug("8");
      goto i2c_quit;
   }
   // TWI Transmit Ok
   r_val=1;

i2c_quit:
   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_STOP);
   return r_val;
}

int i2c_readbyte(unsigned int i2c_address, unsigned int dev_id,
                 unsigned int dev_addr, char *data)
{
   unsigned char n = 0;
   unsigned char twi_status;
   char r_val = -1;

i2c_retry:
   if (n++ >= MAX_TRIES) return r_val;

   // Transmit Start Condition
   twi_status=i2c_transmit(I2C_START);

   // Check the TWSR status
   if (twi_status == TW_MT_ARB_LOST) goto i2c_retry;
   if ((twi_status != TW_START) && (twi_status != TW_REP_START)) goto i2c_quit;

   // Send slave address (SLA_W) 0xa0
   TWDR = (dev_id & 0xF0) | ((dev_addr << 1) & 0x0E) | TW_WRITE;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if ((twi_status == TW_MT_SLA_NACK) || (twi_status == TW_MT_ARB_LOST)) goto i2c_retry;
   if (twi_status != TW_MT_SLA_ACK) goto i2c_quit;

#if 0
   // Send the High 8-bit of I2C Address
   TWDR = i2c_address >> 8;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if (twi_status != TW_MT_DATA_ACK) goto i2c_quit;
#endif

   // Send the Low 8-bit of I2C Address
   TWDR = i2c_address;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);

   // Check the TWSR status
   if (twi_status != TW_MT_DATA_ACK) goto i2c_quit;  

   // Send start Condition
   twi_status=i2c_transmit(I2C_START);

   // Check the TWSR status
   if (twi_status == TW_MT_ARB_LOST) goto i2c_retry;
   if ((twi_status != TW_START) && (twi_status != TW_REP_START)) goto i2c_quit;

   // Send slave address (SLA_R)
   TWDR = (dev_id & 0xF0) | ((dev_addr << 1) & 0x0E) | TW_READ;

   // Transmit I2C Data
   twi_status=i2c_transmit(I2C_DATA);  

   // Check the TWSR status
   if ((twi_status == TW_MR_SLA_NACK) || (twi_status == TW_MR_ARB_LOST)) goto i2c_retry;
   if (twi_status != TW_MR_SLA_ACK) goto i2c_quit;

   // Read I2C Data
   twi_status=i2c_transmit(I2C_DATA);
   if (twi_status != TW_MR_DATA_NACK) goto i2c_quit;

   // Get the Data
   *data=TWDR;
   r_val=1;

i2c_quit:
   // Send Stop Condition
   twi_status=i2c_transmit(I2C_STOP);
   return r_val;
}

int main(void)
{
   int i_ret = -1; 
   uint8_t uc_byte = 0;
   USART_RET_E e_usart_ret = eUSART_FAILURE;
   USART_INIT_PARAMS_X x_usart_init_params = {eUSART_INIT_MODE_16_0000M_9600_8_N_1_1x};

   e_usart_ret = usart_init(&x_usart_init_params);
   if(eUSART_SUCCESS != e_usart_ret)
   {
      i_ret = -1;
      goto CLEAN_RETURN;
   }

   /* Initial TWI Peripheral */
   TWSR = 0x00;   // Select Prescaler of 1

   // SCL frequency = 16000000 / (16 + 2 * 72 * 1) = 100 khz
   TWBR = 0x48;   // 72 Decimal

   i_ret = i2c_writebyte(0x00000007, 0x000000D0, 0x00000000, 0x80);
   if(1 == i_ret)
   {
      testbench_debug("Success");
   }
   else
   {
      testbench_debug("Failure");
   }

   i_ret = i2c_readbyte(0x00000007, 0x000000D0, 0x00000000,&uc_byte);
   if(1 == i_ret)
   {
      testbench_debug("Success");
      if(0x80 == uc_byte)
      {
         testbench_debug("Success");
      }
      else
      {
         testbench_debug("Failure");
      }
   }
   else
   {
      testbench_debug("Failure");
   }

   CLEAN_RETURN:
   return i_ret;
   }
