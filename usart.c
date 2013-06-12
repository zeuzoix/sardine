#include <stdlib.h>
#include <avr/io.h>
#include <usart.h>

typedef struct _USART_INIT_REG_X
{
   USART_INIT_MODE_E e_mode;
   uint8_t uc_ubrrh;
   uint8_t uc_ubrrl;
   uint8_t uc_ucsra; 
   uint8_t uc_ucsrb;
   uint8_t uc_ucsrc;

}USART_INIT_REG_X;

static USART_INIT_REG_X gxa_init_reg[] =
{
   {  
      eUSART_INIT_MODE_16_0000M_9600_8_N_1_1x, 
      0x00, 
      0x67, 
      0x00, 
      0x18, 
      0x86  
   }
};

USART_RET_E usart_init(USART_INIT_PARAMS_X *px_init_params)
{
   USART_RET_E e_ret = eUSART_FAILURE;
   USART_INIT_REG_X *px_init_reg = NULL;
   uint8_t uc_i = 0;
   uint8_t uc_size_arr = sizeof(gxa_init_reg)/sizeof(gxa_init_reg[0]);
   
   if(NULL == px_init_params)
   {
      e_ret = eUSART_FAILURE;
      goto LBL_USART_INIT_RET;
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
      e_ret = eUSART_FAILURE;
      goto LBL_USART_INIT_RET;
   }
   
   UBRRH = px_init_reg->uc_ubrrh;
   UBRRL = px_init_reg->uc_ubrrl;
   UCSRA = px_init_reg->uc_ucsra;
   UCSRB = px_init_reg->uc_ucsrb;
   UCSRC = px_init_reg->uc_ucsrc;

   e_ret = eUSART_SUCCESS;
LBL_USART_INIT_RET:
   return e_ret;
}

USART_RET_E usart_async_send_str(char *pc_str)
{
   USART_RET_E e_ret = eUSART_FAILURE;
   char *pc_str_tmp = NULL;

   if(NULL == pc_str)
   {
      e_ret = eUSART_FAILURE;
      goto LBL_USART_ASYNC_SEND_STR_RET;
   }

   pc_str_tmp = pc_str;
   while('\0' != *pc_str_tmp)
   {
      while(!(UCSRA & (1 << UDRE)))
      {
         continue;
      }

      UDR = *pc_str_tmp;
      pc_str_tmp += 1;
   }
   
   e_ret = eUSART_SUCCESS;
LBL_USART_ASYNC_SEND_STR_RET:
   return e_ret;
}

USART_RET_E usart_async_recv_str(char *pc_str, uint32_t ui_str_sz)
{
   USART_RET_E e_ret = eUSART_FAILURE;
   char *pc_str_tmp = NULL;

   if((NULL == pc_str)||
      (1 > ui_str_sz)) 
   {
      e_ret = eUSART_FAILURE;
      goto LBL_USART_ASYNC_RECV_STR_RET;
   }
   pc_str_tmp = pc_str;

   while(ui_str_sz > 1)
   {
      while(!(UCSRA & (1 << RXC)))
      {
         continue;
      }

      *pc_str_tmp = UDR;
      if('\0' == *pc_str_tmp)
      {
         break;
      }

      pc_str_tmp += 1;
      ui_str_sz -= 1;
   }
   *pc_str_tmp = '\0';

   e_ret = eUSART_SUCCESS;
LBL_USART_ASYNC_RECV_STR_RET:
   return e_ret;
}

USART_RET_E usart_async_send_buffer(uint8_t *puc_buf, uint32_t ui_buf_sz)
{
   USART_RET_E e_ret = eUSART_FAILURE;
   uint32_t ui_i = 0;

   if((NULL == puc_buf)||
      (0 == ui_buf_sz))
   {
      e_ret = eUSART_FAILURE; 
      goto LBL_USART_ASYNC_SEND_BUFFER_RET;
   }

   for(ui_i = 0 ; ui_i < ui_buf_sz ; ui_i++)
   {
      while(!(UCSRA & (1 << UDRE)))
      {
         continue;
      }

      UDR = *(puc_buf + ui_i);
   }

   e_ret = eUSART_SUCCESS;
LBL_USART_ASYNC_SEND_BUFFER_RET:
   return e_ret;
}

USART_RET_E usart_async_recv_buffer(uint8_t *puc_buf, uint32_t ui_buf_sz)
{
   USART_RET_E e_ret = eUSART_FAILURE;
   uint32_t ui_i = 0;

   if((NULL == puc_buf)||
      (0 == ui_buf_sz))
   {
      e_ret = eUSART_FAILURE;
      goto LBL_USART_ASYNC_RECV_BUFFER_RET;
   }

   for(ui_i = 0 ; ui_i < ui_buf_sz ; ui_i++)
   { 
      while(!(UCSRA & (1 << RXC)))
      {
         continue;
      }
      *(puc_buf + ui_i) = UDR;
   }
   
   e_ret = eUSART_SUCCESS;
LBL_USART_ASYNC_RECV_BUFFER_RET:
   return e_ret;
}
