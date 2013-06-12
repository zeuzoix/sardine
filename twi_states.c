#include <stdlib.h>
#include <avr/io.h>
#include <compat/twi.h> 
#include <twi_debug.h>
#include <twi.h>
#include <twi_states.h>

#if 0
void debug_byte(uint8_t uc_byte)
{
   uint8_t uc_i = 0;
   uint8_t uc_bit = 0;
   char ca_byte[9] = {0};

   for(uc_i = 0 ; uc_i < 8 ; uc_i++)
   {
      uc_bit = 1 << (7 - uc_i);
      if(0 == (uc_byte & uc_bit))
      {
         ca_byte[uc_i] = '0';
      }
      else
      {
         ca_byte[uc_i] = '1';
      }
   }

   ca_byte[8] = '\0';

   TWI_DEBUG(ca_byte);
   
}
#endif

TWI_RET_E twi_state_tx_start(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twcr = 0;
   
   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_START_RET;
   }

   uc_twcr = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN);
   TWCR = uc_twcr;

   do
   {
      uc_twcr = TWCR & (1 << TWINT);

   }while(0 == uc_twcr);
   
   e_ret = eTWI_SUCCESS;
LBL_TWI_STATE_TX_START_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_start_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   uint8_t uc_twsr = 0;
   TWI_RET_E e_ret = eTWI_FAILURE;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_START_NEXT_RET;
   }

   uc_twsr = TWSR & 0xF8;
   switch(uc_twsr)
   {
      case TW_START: 
      {
         //TWI_DEBUG("Sent START successfully");
         px_state_ctxt->e_state = eTWI_STATE_TX_WRITE_ADDRESS;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_REP_START: 
      {
         if(0 != px_state_ctxt->uc_addr_tx_retry_cnt)
         {
            //TWI_DEBUG("Sent ReSTART successfully");
            px_state_ctxt->e_state = eTWI_STATE_TX_WRITE_ADDRESS;
            e_ret = eTWI_SUCCESS;
         }
         else
         {  
            switch(px_state_ctxt->e_state_mode)
            {
               case eTWI_MODE_WRITE:
               {
                  px_state_ctxt->e_state = eTWI_STATE_TX_DATA;
                  e_ret = eTWI_SUCCESS;
               }
               break;
               case eTWI_MODE_READ:
               {
                  px_state_ctxt->e_state = eTWI_STATE_TX_READ_ADDRESS;
                  e_ret = eTWI_SUCCESS;
               }
               break;
               default:
               {
                  e_ret = eTWI_FAILURE_TX_START;
               }
               break;
            }
         
         }
      }
      break;
      default:
      {
         //TWI_DEBUG("Failed to send start");
         e_ret = eTWI_FAILURE_TX_START;
      }
      break;
   }

LBL_TWI_STATE_TX_START_NEXT_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_write_address(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twdr = 0;
   uint8_t uc_twcr = 0;
   
   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_ADDRESS_RET;
   }

   uc_twdr = (px_state_ctxt->uc_dev_id & 0xF0);
   uc_twdr |= ((px_state_ctxt->uc_dev_addr << 1) & 0x0E);
   uc_twdr |= TW_WRITE;

#if 0
   debug_byte(uc_twdr);
#endif

   TWDR = uc_twdr;

   uc_twcr = (1 << TWINT)|(1 << TWEN);
   TWCR = uc_twcr;

   do
   {
      uc_twcr = TWCR & (1 << TWINT);

   }while(0 == uc_twcr);
   
   e_ret = eTWI_SUCCESS;
LBL_TWI_STATE_TX_ADDRESS_RET:
   return e_ret; 
}

TWI_RET_E twi_state_tx_write_address_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twsr = 0;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_ADDRESS_NEXT_RET;
   }

   uc_twsr = TWSR & 0xF8;
   switch(uc_twsr)
   {
      case TW_MT_SLA_NACK:
      {
         if(px_state_ctxt->uc_addr_tx_retry_cnt >= px_state_ctxt->uc_max_addr_tx_retry_cnt) 
         {
            //TWI_DEBUG("Max addr tx retry cnt breached");
            px_state_ctxt->e_stop_state = eTWI_STATE_TX_WRITE_ADDRESS;
            px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         }
         else
         {
            //TWI_DEBUG("NACK after addr tx retrying start");
            px_state_ctxt->uc_addr_tx_retry_cnt++;
            px_state_ctxt->e_state = eTWI_STATE_TX_START;
         }
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MT_SLA_ACK:
      {
         //TWI_DEBUG("ACK after addr tx");
         px_state_ctxt->uc_addr_tx_retry_cnt = 0;
         px_state_ctxt->e_state = eTWI_STATE_TX_REG_ADDR;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MT_ARB_LOST:
      {
         //TWI_DEBUG("ARB LOST after addr tx");
         e_ret = eTWI_FAILURE_TX_ADDR_MT_ABR_LOST;
      }
      break;
      default:
      {
         //TWI_DEBUG("Unknown status after addr tx");
         e_ret = eTWI_FAILURE;
      }
      break;
   }
   
LBL_TWI_STATE_TX_ADDRESS_NEXT_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_read_address(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twdr = 0;
   uint8_t uc_twcr = 0;
   
   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_ADDRESS_RET;
   }

   uc_twdr = (px_state_ctxt->uc_dev_id & 0xF0);
   uc_twdr |= ((px_state_ctxt->uc_dev_addr << 1) & 0x0E);
   uc_twdr |= TW_READ;
   
   TWDR = uc_twdr;

   uc_twcr = (1 << TWINT)|(1 << TWEN);
   TWCR = uc_twcr;

   do
   {
      uc_twcr = TWCR & (1 << TWINT);

   }while(0 == uc_twcr);
   
   e_ret = eTWI_SUCCESS;
LBL_TWI_STATE_TX_ADDRESS_RET:
   return e_ret; 
}

TWI_RET_E twi_state_tx_read_address_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twsr = 0;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_ADDRESS_NEXT_RET;
   }

   uc_twsr = TWSR & 0xF8;
   switch(uc_twsr)
   {
      case TW_MR_SLA_NACK:
      {
         //TWI_DEBUG("Max addr tx retry cnt breached");
         px_state_ctxt->e_stop_state = eTWI_STATE_TX_READ_ADDRESS;
         px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MR_SLA_ACK:
      {
         //TWI_DEBUG("ACK after addr tx");
         px_state_ctxt->e_state = eTWI_STATE_RX_DATA;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MR_ARB_LOST:
      {
         //TWI_DEBUG("ARB LOST after addr tx");
         e_ret = eTWI_FAILURE_TX_ADDR_MT_ABR_LOST;
      }
      break;
      default:
      {
         //TWI_DEBUG("Unknown status after addr tx");
         e_ret = eTWI_FAILURE;
      }
      break;
   }
   
LBL_TWI_STATE_TX_ADDRESS_NEXT_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_reg_addr(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twcr;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_REG_ADDR_RET;
   }

   if(px_state_ctxt->ui_reg_addr_cnt < px_state_ctxt->ui_reg_addr_sz)
   {
#if 0
      debug_byte(px_state_ctxt->puc_reg_addr[px_state_ctxt->ui_reg_addr_cnt]);
#endif
      TWDR = px_state_ctxt->puc_reg_addr[px_state_ctxt->ui_reg_addr_cnt];

      uc_twcr = (1 << TWINT)|(1 << TWEN);
      TWCR = uc_twcr;

      do
      {
         uc_twcr = TWCR & (1 << TWINT);

      }while(0 == uc_twcr);
   }

   e_ret = eTWI_SUCCESS;
LBL_TWI_STATE_TX_REG_ADDR_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_reg_addr_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twsr = 0;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_REG_ADDR_NEXT_RET;
   }

   uc_twsr = TWSR & 0xF8;
   switch(uc_twsr)
   {
      case TW_MT_DATA_NACK:
      {
         //TWI_DEBUG("NACK received after reg addr tx");
         px_state_ctxt->e_stop_state = eTWI_STATE_TX_REG_ADDR;
         px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MT_DATA_ACK:
      {
         px_state_ctxt->ui_reg_addr_cnt++;
         if(px_state_ctxt->ui_reg_addr_cnt < px_state_ctxt->ui_reg_addr_sz)
         {
            px_state_ctxt->e_state = eTWI_STATE_TX_REG_ADDR;
         }
         else
         {
            switch(px_state_ctxt->e_state_mode)
            {
               case eTWI_MODE_WRITE:
               {
                  px_state_ctxt->e_state = eTWI_STATE_TX_DATA;
                  e_ret = eTWI_SUCCESS;
               }
               break;
               case eTWI_MODE_READ:
               {
                  px_state_ctxt->e_state = eTWI_STATE_TX_START;
                  e_ret = eTWI_SUCCESS;
               }
               break;
               default:
               {
                  e_ret = eTWI_FAILURE;
               }
               break;
            }
         }
      }
      break;
      case TW_MT_ARB_LOST:
      {
         //TWI_DEBUG("ARB LOST after reg addr tx");
         e_ret = eTWI_FAILURE_TX_REG_ADDR_MT_ABR_LOST;
      }
      break;
      default:
      {
         //TWI_DEBUG("Unknown status after reg addr tx");
         e_ret = eTWI_FAILURE;
      }
      break;
   }
LBL_TWI_STATE_TX_REG_ADDR_NEXT_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_data(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twcr;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_DATA_RET;
   }

   if(px_state_ctxt->ui_data_cnt < px_state_ctxt->ui_data_sz)
   {
#if 0
      debug_byte(px_state_ctxt->puc_data[px_state_ctxt->ui_data_cnt]);
#endif
      TWDR = px_state_ctxt->puc_data[px_state_ctxt->ui_data_cnt]; 

      uc_twcr = (1 << TWINT)|(1 << TWEN);
      TWCR = uc_twcr;

      do
      {
         uc_twcr = TWCR & (1 << TWINT);

      }while(0 == uc_twcr);

      e_ret = eTWI_SUCCESS;
   }
   else
   {
      e_ret = eTWI_FAILURE;
   }


LBL_TWI_STATE_TX_DATA_RET:
   return e_ret; 
}

TWI_RET_E twi_state_tx_data_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twsr = 0;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_DATA_NEXT_RET;
   }

   uc_twsr = TWSR & 0xF8;
   switch(uc_twsr)
   {
      case TW_MT_DATA_NACK:
      {
         //TWI_DEBUG("NACK received after data tx");
         px_state_ctxt->e_stop_state = eTWI_STATE_TX_DATA;
         px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MT_DATA_ACK:
      {
         //TWI_DEBUG("ACK after data tx");
         px_state_ctxt->ui_data_cnt++;
         if(px_state_ctxt->ui_data_cnt < px_state_ctxt->ui_data_sz)
         {
            px_state_ctxt->e_state = eTWI_STATE_TX_DATA;
         }
         else
         {
            px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         }
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MT_ARB_LOST:
      {
         //TWI_DEBUG("ARB LOST after data tx");
         e_ret = eTWI_FAILURE_TX_DATA_MT_ABR_LOST;
      }
      break;
      default:
      {
         //TWI_DEBUG("Unknown status after data tx");
         e_ret = eTWI_FAILURE;
      }
      break;
   }

LBL_TWI_STATE_TX_DATA_NEXT_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_stop(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twcr = 0;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_STOP_RET;
   }

   uc_twcr = (1 << TWINT)|(1 << TWSTO)|(1 << TWEN);
   TWCR = uc_twcr;

   e_ret = eTWI_SUCCESS;
LBL_TWI_STATE_TX_STOP_RET:
   return e_ret;
}

TWI_RET_E twi_state_tx_stop_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_TX_STOP_NEXT_RET;
   }

   px_state_ctxt->e_state = eTWI_STATE_COMPLETE;

   switch(px_state_ctxt->e_stop_state)
   {
      case eTWI_STATE_TX_DATA:
      {
         //TWI_DEBUG("Stop invoked after data NACK");
         e_ret = eTWI_FAILURE_TX_DATA;
      }
      break;
      case eTWI_STATE_RX_DATA:
      {
         //TWI_DEBUG("Stop invoked after data NACK");
         e_ret = eTWI_FAILURE_RX_DATA;
      }
      break;
      case eTWI_STATE_TX_REG_ADDR:
      {
         //TWI_DEBUG("Stop invoked after reg addr NACK");
         e_ret = eTWI_FAILURE_TX_REG_ADDR;
      }
      break;
      case eTWI_STATE_TX_WRITE_ADDRESS:
      {
         //TWI_DEBUG("Stop invoked after address NACK");
         e_ret = eTWI_FAILURE_TX_WRITE_ADDR_MAX_RETRY;
      }
      break;
      case eTWI_STATE_TX_READ_ADDRESS:
      {
         //TWI_DEBUG("Stop invoked after address NACK");
         e_ret = eTWI_FAILURE_TX_READ_ADDR;
      }
      break;
      case eTWI_STATE_TX_STOP:
      {
         //TWI_DEBUG("Stop invoked after completion");
         e_ret = eTWI_SUCCESS;
      }
      break;
      default:
      {
         //TWI_DEBUG("Stop invoked after unknown state");
         e_ret = eTWI_FAILURE;
      }
      break;
   }   
LBL_TWI_STATE_TX_STOP_NEXT_RET:
   return e_ret;
}

TWI_RET_E twi_state_rx_data(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twcr = 0;

   if((NULL == px_state_ctxt)||
      (px_state_ctxt->ui_data_cnt >= px_state_ctxt->ui_data_sz))
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_RX_DATA_RET;
   }

   TWI_DEBUG("ix");
   if(px_state_ctxt->ui_data_cnt < (px_state_ctxt->ui_data_sz - 1))
   {
      uc_twcr = (1 << TWINT)|(1 << TWEA)|(1 << TWEN);
   }
   
   if(px_state_ctxt->ui_data_cnt == (px_state_ctxt->ui_data_sz - 1))
   {
      uc_twcr = (1 << TWINT)|(1 << TWEN);
   }

   TWCR = uc_twcr;
   do
   {
      uc_twcr = TWCR & (1 << TWINT);

   }while(0 == uc_twcr);

   px_state_ctxt->puc_data[px_state_ctxt->ui_data_cnt] = TWDR;

   e_ret = eTWI_SUCCESS;
LBL_TWI_STATE_RX_DATA_RET:
   return e_ret;
}

TWI_RET_E twi_state_rx_data_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   uint8_t uc_twsr = 0;

   if(NULL == px_state_ctxt)
   {
      //TWI_DEBUG("Invalid arguments");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_STATE_RX_DATA_NEXT_RET;
   }

   uc_twsr = TWSR & 0xF8;
   switch(uc_twsr)
   {
      case TW_MR_DATA_NACK:
      {
         //TWI_DEBUG("NACK received after data tx");
         px_state_ctxt->ui_data_cnt++;
         px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MR_DATA_ACK:
      {
         //TWI_DEBUG("ACK after data tx");
         px_state_ctxt->ui_data_cnt++;
         if(px_state_ctxt->ui_data_cnt < px_state_ctxt->ui_data_sz)
         {
            TWI_DEBUG("ic");
            px_state_ctxt->e_state = eTWI_STATE_RX_DATA;
         }
         else
         {
            TWI_DEBUG("ia");
            px_state_ctxt->e_stop_state = eTWI_STATE_RX_DATA;
            px_state_ctxt->e_state = eTWI_STATE_TX_STOP;
         }
         e_ret = eTWI_SUCCESS;
      }
      break;
      case TW_MR_ARB_LOST:
      {
         //TWI_DEBUG("ARB LOST after data tx");
         e_ret = eTWI_FAILURE_RX_DATA_MT_ABR_LOST;
      }
      break;
      default:
      {
         //TWI_DEBUG("Unknown status after data tx");
         e_ret = eTWI_FAILURE;
      }
      break;
   }
   
LBL_TWI_STATE_RX_DATA_NEXT_RET:
   return e_ret;
}
