#ifndef _USART_H_
#define _USART_H_

typedef enum _USART_RET_E
{
   eUSART_SUCCESS = 0,
   eUSART_FAILURE

}USART_RET_E;

typedef enum _USART_INIT_MODE_E
{
   eUSART_INIT_MODE_16_0000M_9600_8_N_1_1x = 0,
   eUSART_INIT_MODE_16_0000M_9600_8_N_1_2x = 1,
   eUSART_INIT_MODE_INVALID

}USART_INIT_MODE_E;

typedef struct _USART_INIT_PARAMS_X
{
   USART_INIT_MODE_E e_mode;

}USART_INIT_PARAMS_X;


USART_RET_E usart_init(USART_INIT_PARAMS_X *px_init_params);
USART_RET_E usart_async_send_str(char *pc_str);
USART_RET_E usart_async_recv_str(char *pc_str, uint32_t ui_str_sz);
USART_RET_E usart_async_send_buffer(uint8_t *puc_buf, uint32_t ui_buf_sz);
USART_RET_E usart_async_recv_buffer(uint8_t *puc_buf, uint32_t ui_buf_sz);
#endif
