#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <twi_debug.h>
#include <twi.h>
#include <usart.h>

#define RTC_CHIP_ID        (0xD0)
#define RTC_CHIP_ADDRESS   (0x00)

typedef struct _TESTBENCH_NODE_X
{
   char *pc_desc;
   void (*pfn_function)(void *);
   
}TESTBENCH_NODE_X;

static void testbench_display_rtc_date(void *pv_data);
static void testbench_display_rtc_time(void *pv_data);
static void testbench_update_rtc_date(void *pv_data);
static void testbench_update_rtc_time(void *pv_data);
static void testbench_check_i2c(void *pv_data);

TESTBENCH_NODE_X gxa_testbench_nodes[] = {
                                          {"0: Exit",
                                           NULL},
                                          {"1: Erase EEPROM",
                                           NULL},
                                          {"2: Write EEPROM",
                                           NULL},
                                          {"3: Read EEPROM",
                                           NULL},
                                          {"4: Write eeprom page",
                                           NULL},
                                          {"5: Read eeprom page",
                                           NULL},
                                          {"6: Display RTC Date",
                                           testbench_display_rtc_date},
                                          {"7: Display RTC time",
                                           testbench_display_rtc_time},
                                          {"8: Update RTC Date",
                                           testbench_update_rtc_date},
                                          {"9: Update RTC time",
                                           testbench_update_rtc_time},
                                          {":: Check I2C",
                                           testbench_check_i2c},
                                          };

int testbench_debug(char *pc_str)
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

int init_io(void)
{
   int i_ret = -1;
   TWI_INIT_PARAMS_X x_twi_init_params;
   TWI_RET_E e_twi_ret = eTWI_FAILURE;
   USART_RET_E e_usart_ret = eUSART_FAILURE;

   USART_INIT_PARAMS_X x_usart_init_params = {eUSART_INIT_MODE_16_0000M_9600_8_N_1_1x};

   e_usart_ret = usart_init(&x_usart_init_params);
   if(eUSART_SUCCESS != e_usart_ret)
   {
      i_ret = -1;
      goto LBL_INIT_IO_RET;
   }

#if 1
   x_twi_init_params.e_mode = eTWI_INIT_MODE_16_0000M_100K_1;
   x_twi_init_params.pfn_debug = testbench_debug;
   e_twi_ret = twi_init(&x_twi_init_params);
   if(eTWI_SUCCESS != e_twi_ret)
   {
      i_ret = -1;
      goto LBL_INIT_IO_RET;
   }
#endif
   i_ret = 0;
LBL_INIT_IO_RET:
   return i_ret;
}

int main(void)
{
   int8_t c_selection = -1;
   USART_RET_E e_ret = eUSART_FAILURE;
   uint8_t uc_testbench_nodes_sz = sizeof(gxa_testbench_nodes)/sizeof(gxa_testbench_nodes[0]); 
   uint8_t uc_i = 0;
#if 1
   void *pv_data = NULL;
#endif
   
   init_io();
   testbench_debug("Hello World 7");

   while('0' != c_selection)
   {
      
      for(uc_i=0 ; uc_i < uc_testbench_nodes_sz ; uc_i++)
      {
         testbench_debug(gxa_testbench_nodes[uc_i].pc_desc);
      }

      e_ret = usart_async_recv_buffer( (uint8_t *)&c_selection, sizeof(c_selection));
      if(eUSART_SUCCESS != e_ret)
      {
         testbench_debug("Failed to receive");
         break;
      }
     
      testbench_debug("Received ");
      usart_async_send_buffer((uint8_t *)&c_selection, sizeof(c_selection));
      testbench_debug("");
      if('0' == c_selection)
      {
         testbench_debug("Exit Called!");
         break;
      }

      if(NULL != gxa_testbench_nodes[(c_selection - '0')].pfn_function)
      {
         gxa_testbench_nodes[(c_selection - '0')].pfn_function(pv_data);
      }
   }

   testbench_debug("Bye Bye World");

	return 0;
}

static void testbench_check_i2c(void *pv_data)
{
   TWI_RET_E  e_ret = eTWI_FAILURE;
   TWI_SEND_RECV_BUFFER_PARAMS_X x_send_buffer_params = {0, 0, NULL, 0, NULL, 0};
   TWI_SEND_RECV_BUFFER_PARAMS_X x_recv_buffer_params = {0, 0, NULL, 0, NULL, 0};
   uint8_t uca_reg_addr[1] = {0x07};
   uint8_t uca_data[1] = {0x80};
   uint8_t uc_data = 0;

   testbench_debug("Update rtc date");
   x_send_buffer_params.uc_dev_id = RTC_CHIP_ID;
   x_send_buffer_params.uc_dev_addr = RTC_CHIP_ADDRESS;
   x_send_buffer_params.puc_reg_addr = uca_reg_addr;
   x_send_buffer_params.ui_reg_addr_sz = sizeof(uca_reg_addr);
   x_send_buffer_params.puc_data = uca_data;
   x_send_buffer_params.ui_data_sz = sizeof(uca_data);
   
   e_ret = twi_send_recv_buffer(&x_send_buffer_params, eTWI_MODE_WRITE);
   if(eTWI_SUCCESS != e_ret)
   {
      testbench_debug("Failed to send buffer");
   }

   x_recv_buffer_params.uc_dev_id = RTC_CHIP_ID;
   x_recv_buffer_params.uc_dev_addr = RTC_CHIP_ADDRESS;
   x_recv_buffer_params.puc_reg_addr = uca_reg_addr;
   x_recv_buffer_params.ui_reg_addr_sz = sizeof(uca_reg_addr);
   x_recv_buffer_params.puc_data = &uc_data;
   x_recv_buffer_params.ui_data_sz = sizeof(uc_data);
   
#if 1
   e_ret = twi_send_recv_buffer(&x_recv_buffer_params, eTWI_MODE_READ);
   if(eTWI_SUCCESS != e_ret)
   {
      testbench_debug("Failed to recv buffer");
   }
#endif

   if(uc_data == uca_data[0])
   {
      testbench_debug("Y");
   }
   else
   {
      testbench_debug("N");
   }
   
}

static void testbench_display_rtc_date(void *pv_data)
{
   TWI_RET_E  e_ret = eTWI_FAILURE;
   TWI_SEND_RECV_BUFFER_PARAMS_X x_recv_buffer_params = {0, 0, NULL, 0, NULL, 0};
   uint8_t uc_reg_addr = 0;
   uint8_t uc_date_data[4] = {0};
   char ca_date[3] = {0};
   char ca_month[3] = {0};
   char ca_year[3] = {0};
   int i_failure = -1;

   x_recv_buffer_params.uc_dev_id = RTC_CHIP_ID;
   x_recv_buffer_params.uc_dev_addr = RTC_CHIP_ADDRESS;
   x_recv_buffer_params.puc_data = uc_date_data;
   x_recv_buffer_params.ui_data_sz = sizeof(uc_date_data);
   x_recv_buffer_params.puc_reg_addr = &uc_reg_addr;
   x_recv_buffer_params.ui_reg_addr_sz = sizeof(uc_reg_addr);
//Get Day
   uc_reg_addr = 0x03;
   e_ret = twi_send_recv_buffer(&x_recv_buffer_params, eTWI_MODE_READ);
   if(eTWI_SUCCESS != e_ret)
   {
      testbench_debug("Failed to recv buffer");
      goto LBL_TESTBENCH_DISPLAY_RTC_DATE_RET;
   }

   switch(uc_date_data[0])
   {
      case 0x01:  testbench_debug("Sun"); break;
      case 0x02:  testbench_debug("Mon"); break;
      case 0x03:  testbench_debug("Tue"); break;
      case 0x04:  testbench_debug("Wed"); break;
      case 0x05:  testbench_debug("Thu"); break;
      case 0x06:  testbench_debug("Fri"); break;
      case 0x07:  testbench_debug("Sat"); break;
      default: e_ret = i_failure = -1;  goto LBL_TESTBENCH_DISPLAY_RTC_DATE_RET; break;
   }

//Get Date
   ca_date[0] = '0' + (0x03 & (uc_date_data[1]>>4));
   ca_date[1] = '0' + (0x0F & uc_date_data[1]);
   ca_date[2] = '\0';

   if(((ca_date[0] == '0')&&(ca_date[1] == '0'))||
      (ca_date[1] > '9')||
      (ca_date[1] < '0'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_DISPLAY_RTC_DATE_RET;
   }

   testbench_debug(ca_date);

//Get Month
   ca_month[0] = '0' + (0x01 & (uc_date_data[2]>>4));
   ca_month[1] = '0' + (0x0F & uc_date_data[2]);
   ca_month[2] = '\0';

   if(((ca_month[0] == '0')&&(ca_month[1] == '0'))||
      ((ca_month[0] == '1')&&(ca_month[1] > '2'))||
      (ca_month[1] > '9')||
      (ca_month[1] < '0'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_DISPLAY_RTC_DATE_RET;
   }

   testbench_debug(ca_month);

//Get Year
   ca_year[0] = '0' + (0x0F & (uc_date_data[3]>>4));
   ca_year[1] = '0' + (0x0F & uc_date_data[3]);
   ca_year[2] = '\0';
   if((ca_year[0] < '0')||(ca_year[0] > '9')||
      (ca_year[1] < '0')||(ca_year[1] > '9'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_DISPLAY_RTC_DATE_RET;
   }

   testbench_debug(ca_year);

   i_failure = 0;
LBL_TESTBENCH_DISPLAY_RTC_DATE_RET:
   if(i_failure != 0)
   {
      testbench_debug("Date unknown");
   }

   return;
}

static void testbench_display_rtc_time(void *pv_data)
{
   TWI_RET_E  e_ret = eTWI_FAILURE;
   TWI_SEND_RECV_BUFFER_PARAMS_X x_recv_buffer_params = {0, 0, NULL, 0, NULL, 0};
   uint8_t uc_reg_addr = 0;
   uint8_t uca_time_data[3] = {0};
   char ca_secs[3] = {0};
   char ca_mins[3] = {0};
   char ca_hour[3] = {0};
   char ca_am_pm[3] = {0};
   uint8_t uc_12_24 = 0;
   uint8_t uc_am_pm = 0;
   int i_failure = -1;

   x_recv_buffer_params.uc_dev_id = RTC_CHIP_ID;
   x_recv_buffer_params.uc_dev_addr = RTC_CHIP_ADDRESS;
   x_recv_buffer_params.puc_data = uca_time_data;
   x_recv_buffer_params.ui_data_sz = sizeof(uca_time_data);
   x_recv_buffer_params.puc_reg_addr = &uc_reg_addr;
   x_recv_buffer_params.ui_reg_addr_sz = sizeof(uc_reg_addr);
   uc_reg_addr = 0x00;
   e_ret = twi_send_recv_buffer(&x_recv_buffer_params, eTWI_MODE_READ);
   if(eTWI_SUCCESS != e_ret)
   {
      testbench_debug("Failed to recv buffer");
      goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
   }

//Get seconds
   ca_secs[0] = '0' + (0x07 & (uca_time_data[0]>>4));
   ca_secs[1] = '0' + (0x0F & uca_time_data[0]);
   ca_secs[2] = '\0';

   if((ca_secs[1] > '9')||
      (ca_secs[1] < '0')||
      (ca_secs[0] > '5')||
      (ca_secs[0] < '0'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
   }

   testbench_debug(ca_secs);

//Get minutes
   ca_mins[0] = '0' + (0x07 & (uca_time_data[1]>>4));
   ca_mins[1] = '0' + (0x0F & uca_time_data[1]);
   ca_mins[2] = '\0';

   if((ca_mins[1] > '9')||
      (ca_mins[1] < '0')||
      (ca_mins[0] > '5')||
      (ca_mins[0] < '0'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
   }

   testbench_debug(ca_mins);

//Get hour
   uc_12_24 = 0x01 & (uca_time_data[2]>>6);
   switch (uc_12_24)
   {
      case 0x00:
      {
         ca_hour[0] = '0' + (0x03 & (uca_time_data[2]>>4));
         if((ca_hour[0] > '2')||
            (ca_hour[0] < '0'))
         {
            i_failure = -1;
            goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
         }
         ca_am_pm[0] = '\0';
         ca_am_pm[1] = '\0';
         ca_am_pm[2] = '\0';
      }
      break;
      case 0x01:
      {
         uc_am_pm = 0x01 & (uca_time_data[2]>>5);
         switch (uc_am_pm)
         {
            case 0x00:
            {
               ca_am_pm[0] = 'A';
            }
            break;
            case 0x01:
            {
               ca_am_pm[0] = 'P';
            }
            break;
            default:
            {
               i_failure = -1;
               goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
            }
            break;
         } 
         ca_am_pm[1] = 'M';
         ca_am_pm[2] = '\0';
         ca_hour[0] = '0' + (0x01 & (uca_time_data[2]>>4));
      }
      break;
      default:
      {
         i_failure = -1;
         goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
         
      }
      break;
   }
   ca_hour[1] = '0' + (0x0F & uca_time_data[2]);
   ca_hour[2] = '\0';
   if((ca_hour[1] < '0')||
      (ca_hour[1] > '9'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_DISPLAY_RTC_TIME_RET;
   }

   testbench_debug(ca_hour);
   testbench_debug(ca_am_pm);

   i_failure = 0;
LBL_TESTBENCH_DISPLAY_RTC_TIME_RET:
   if(i_failure != 0)
   {
      testbench_debug("Time unknown");
   }

   return;
}

static void testbench_update_rtc_date(void *pv_data)
{
}

static void testbench_update_rtc_time(void *pv_data)
{
   TWI_RET_E  e_ret = eTWI_FAILURE;
   TWI_SEND_RECV_BUFFER_PARAMS_X x_recv_buffer_params = {0, 0, NULL, 0, NULL, 0};
   USART_RET_E e_usart_ret = eUSART_FAILURE;
   uint8_t uc_reg_addr = 0;
   uint8_t uca_time_data[3] = {0};
   char ca_secs[3] = {0};
   char ca_mins[3] = {0};
   char ca_hour[3] = {0};
   char ca_am_pm[3] = {0};
   int i_failure = -1;

   testbench_debug("Enter");
   testbench_debug("Sec");
   e_usart_ret = usart_async_recv_buffer( (uint8_t *)&ca_secs, sizeof(ca_secs) - 1);
   if(eUSART_SUCCESS != e_usart_ret)
   {
      i_failure = -2;
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }
   ca_secs[2] = '\0';
   if((ca_secs[1] > '9')||
      (ca_secs[1] < '0')||
      (ca_secs[0] > '5')||
      (ca_secs[0] < '0'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }

   testbench_debug("Min");
   e_usart_ret = usart_async_recv_buffer( (uint8_t *)&ca_mins, sizeof(ca_mins) - 1);
   if(eUSART_SUCCESS != e_usart_ret)
   {
      i_failure = -2;
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }
   ca_mins[2] = '\0';
   if((ca_mins[1] > '9')||
      (ca_mins[1] < '0')||
      (ca_mins[0] > '5')||
      (ca_mins[0] < '0'))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }

   testbench_debug("Hour");
   e_usart_ret = usart_async_recv_buffer( (uint8_t *)&ca_hour, sizeof(ca_hour) - 1);
   if(eUSART_SUCCESS != e_usart_ret)
   {
      i_failure = -2;
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }
   ca_hour[2] = '\0';
   if((ca_hour[1] > '9')||
      (ca_hour[1] < '0')||
      (ca_hour[0] > '1')||
      (ca_hour[0] < '0')||
      ((ca_hour[0] == '0')&&(ca_hour[1] == '0'))||
      ((ca_hour[0] == '1')&&(ca_hour[1] > '2')))
   {
      i_failure = -2;
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }

   testbench_debug("AM/PM");
   e_usart_ret = usart_async_recv_buffer( (uint8_t *)&ca_am_pm, sizeof(ca_am_pm) - 1);
   if(eUSART_SUCCESS != e_usart_ret)
   {
      i_failure = -2;
      testbench_debug("1");
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }
   ca_am_pm[2] = '\0';
   if(((ca_am_pm[0] != 'A')&&
      (ca_am_pm[0] != 'P'))||
      (ca_am_pm[1] != 'M'))
   {
      i_failure = -2;
      testbench_debug("2");
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }

   uca_time_data[0] = (ca_secs[0] - '0')<<4;
   uca_time_data[0] |= ((ca_secs[1] - '0') & 0x0F);
   uca_time_data[1] = (ca_mins[0] - '0')<<4;
   uca_time_data[1] |= ((ca_mins[1] - '0') & 0x0F);
   uca_time_data[2] = (ca_hour[0] - '0')<<4;  
   uca_time_data[2] |= ((ca_hour[1] - '0') & 0x0F);
   if('A' == ca_am_pm[0])
   {
      uca_time_data[2] |= 0x40;
   }
   else if('P' == ca_am_pm[0]) 
   {
      uca_time_data[2] |= 0x60;
   }
   else
   {
      i_failure = -2;
      testbench_debug("3");
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }
   //Start Clock
   uca_time_data[0] &= ~(0x80);

   x_recv_buffer_params.uc_dev_id = RTC_CHIP_ID;
   x_recv_buffer_params.uc_dev_addr = RTC_CHIP_ADDRESS;
   x_recv_buffer_params.puc_data = uca_time_data;
   x_recv_buffer_params.ui_data_sz = sizeof(uca_time_data);
   x_recv_buffer_params.puc_reg_addr = &uc_reg_addr;
   x_recv_buffer_params.ui_reg_addr_sz = sizeof(uc_reg_addr);
   uc_reg_addr = 0x00;
   e_ret = twi_send_recv_buffer(&x_recv_buffer_params, eTWI_MODE_WRITE);
   if(eTWI_SUCCESS != e_ret)
   {
      testbench_debug("Failed to write buffer");
      testbench_debug("4");
      goto LBL_TESTBENCH_UPDATE_RTC_TIME_RET;
   }

   i_failure = 0;
LBL_TESTBENCH_UPDATE_RTC_TIME_RET:
   if(i_failure != 0)
   {
      testbench_debug("Unset");
   }

   return;
}
