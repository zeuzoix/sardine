#include <stdlib.h>
#include <avr/io.h>
#include <twi_debug.h>
#include <twi.h>
#include <twi_states.h>

typedef struct _TWI_INIT_REG_X
{
   TWI_INIT_MODE_E e_mode;
   uint8_t uc_twbr;
   uint8_t uc_twcr;
   uint8_t uc_twsr;
   uint8_t uc_twdr;
   uint8_t uc_twar;
   
}TWI_INIT_REG_X;

typedef struct _TWI_CTXT_X
{
   TWI_INIT_MODE_E e_mode;
   
}TWI_CTXT_X;

static TWI_CTXT_X gx_twi_ctxt = { eTWI_INIT_MODE_INVALID};

static TWI_INIT_REG_X gxa_init_reg[] = 
{
   {
      eTWI_INIT_MODE_16_0000M_100K_1,
      0x48,                               //0b01001000
      0x80,                               //0b10000000
      0x00,                               //0b00000000
      0x00,                               //0b00000000
      0x00                                //0b00000000
   } 
};

TWI_DEBUG_PFN gpfn_twi_debug = NULL;

static TWI_STATE_CTXT_X gxa_state_ctxt[] = 
{
   {  
      eTWI_STATE_TX_START, 
      twi_state_tx_start, 
      twi_state_tx_start_next,
      "START"
   },
   {  eTWI_STATE_TX_WRITE_ADDRESS, 
      twi_state_tx_write_address, 
      twi_state_tx_write_address_next,
      "WRITE_ADDRESS"
   },
   {  eTWI_STATE_TX_READ_ADDRESS, 
      twi_state_tx_read_address, 
      twi_state_tx_read_address_next,
      "READ_ADDRESS"
   },
   {  
      eTWI_STATE_TX_REG_ADDR,
      twi_state_tx_reg_addr,
      twi_state_tx_reg_addr_next,
      "REG_ADDR"
   },
   { 
      eTWI_STATE_TX_DATA,
      twi_state_tx_data,
      twi_state_tx_data_next,
      "TX_DATA"
   },
   { 
      eTWI_STATE_RX_DATA,
      twi_state_rx_data,
      twi_state_rx_data_next,
      "RX_DATA"
   },
   {  
      eTWI_STATE_TX_STOP,
      twi_state_tx_stop,
      twi_state_tx_stop_next,
      "STOP"
   }      
};

TWI_RET_E twi_init(TWI_INIT_PARAMS_X *px_init_params)
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   TWI_INIT_REG_X *px_init_reg = NULL;
   extern TWI_DEBUG_PFN gpfn_twi_debug;
   uint8_t uc_i = 0;
   uint8_t uc_size_arr = sizeof(gxa_init_reg)/sizeof(gxa_init_reg[0]);

   if(NULL == px_init_params)
   {
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_INIT_RET;
   }

   for(uc_i = 0 ; uc_i < uc_size_arr ; uc_i++)
   {
      if(px_init_params->e_mode == gxa_init_reg[uc_i].e_mode)
      {
         px_init_reg = &gxa_init_reg[uc_i];
         break;
      }
   }

   if(NULL == px_init_reg)
   {
      if(NULL != px_init_params->pfn_debug)
      {
         px_init_params->pfn_debug("Can't find correct mode");
      }
      
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_INIT_RET;
   }

   TWBR = px_init_reg->uc_twbr;
   TWCR = px_init_reg->uc_twcr;
   TWSR = px_init_reg->uc_twsr;
   TWDR = px_init_reg->uc_twdr;
   TWAR = px_init_reg->uc_twar;

   gpfn_twi_debug = px_init_params->pfn_debug;
   gx_twi_ctxt.e_mode = px_init_params->e_mode; 
   
   e_ret = eTWI_SUCCESS;
LBL_TWI_INIT_RET:
   return e_ret;
}

TWI_RET_E twi_send_recv_buffer(TWI_SEND_RECV_BUFFER_PARAMS_X *px_send_recv_buffer_params, 
                               TWI_MODE_E e_mode) 
{
   TWI_RET_E e_ret = eTWI_FAILURE;
   TWI_SEND_RECV_STATE_CTXT_X x_send_recv_state_ctxt = { eTWI_STATE_TX_START, 
                                                         e_mode,
                                                         eTWI_STATE_TX_STOP,
                                                         px_send_recv_buffer_params->uc_dev_addr,
                                                         px_send_recv_buffer_params->uc_dev_id,                  
                                                         px_send_recv_buffer_params->puc_reg_addr,
                                                         px_send_recv_buffer_params->ui_reg_addr_sz,
                                                         0,
                                                         px_send_recv_buffer_params->puc_data,
                                                         px_send_recv_buffer_params->ui_data_sz,
                                                         0,
                                                         0,
                                                         TWI_STATE_MAX_ADDR_TX_RETRY_CNT};
   TWI_STATE_CTXT_X *px_state_ctxt = NULL;
   uint8_t uc_i = 0;
   uint8_t uc_state_ctxt_arr_sz = sizeof(gxa_state_ctxt)/sizeof(gxa_state_ctxt[0]);

   if((NULL == px_send_recv_buffer_params)||
      (NULL == px_send_recv_buffer_params->puc_reg_addr)||
      (NULL == px_send_recv_buffer_params->puc_data)||
      (0 == px_send_recv_buffer_params->ui_reg_addr_sz)||
      (0 == px_send_recv_buffer_params->ui_data_sz))
   {
      TWI_DEBUG("px_send_buffer_params is NULL");
      e_ret = eTWI_FAILURE;
      goto LBL_TWI_SEND_BUFFER_RET;
   }

   TWI_DEBUG("in");

   while(eTWI_STATE_COMPLETE != x_send_recv_state_ctxt.e_state)
   {
      px_state_ctxt = NULL;
      for(uc_i = 0 ; uc_i < uc_state_ctxt_arr_sz ; uc_i++)
      {
         if(x_send_recv_state_ctxt.e_state == gxa_state_ctxt[uc_i].e_state)
         {
            px_state_ctxt = &gxa_state_ctxt[uc_i];
            break;
         }
      }


      if((NULL == px_state_ctxt)||
         (NULL == px_state_ctxt->pfn_state)||
         (NULL == px_state_ctxt->pfn_next_state)||
         (NULL == px_state_ctxt->pc_state_name))
      {
         TWI_DEBUG("px_state_ctxt is NULL");
         e_ret = eTWI_FAILURE;
         goto LBL_TWI_SEND_BUFFER_RET;
      }
      
      TWI_DEBUG(px_state_ctxt->pc_state_name);
      e_ret = px_state_ctxt->pfn_state(&x_send_recv_state_ctxt);
      if(eTWI_SUCCESS != e_ret)
      {
         TWI_DEBUG(px_state_ctxt->pc_state_name);
         TWI_DEBUG("pfn_state returned !eTWI_SUCCESS");
         e_ret = eTWI_FAILURE;
         goto LBL_TWI_SEND_BUFFER_RET;
      }
      
      e_ret = px_state_ctxt->pfn_next_state(&x_send_recv_state_ctxt);
      if(eTWI_SUCCESS != e_ret)
      {
         TWI_DEBUG(px_state_ctxt->pc_state_name);
         TWI_DEBUG("pfn_next_state returned !eTWI_SUCCESS");
         goto LBL_TWI_SEND_BUFFER_RET;
      }
   }

   e_ret = eTWI_SUCCESS; 
LBL_TWI_SEND_BUFFER_RET:
   return e_ret;
}

