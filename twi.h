#ifndef _TWI_H_
#define _TWI_H_

typedef enum _TWI_RET_E
{
   eTWI_SUCCESS = 0,
   eTWI_FAILURE_TX_START,
   eTWI_FAILURE_TX_WRITE_ADDR_MAX_RETRY,
   eTWI_FAILURE_TX_READ_ADDR,
   eTWI_FAILURE_TX_ADDR_MT_ABR_LOST,
   eTWI_FAILURE_TX_REG_ADDR_MT_ABR_LOST,
   eTWI_FAILURE_TX_DATA_MT_ABR_LOST,
   eTWI_FAILURE_RX_DATA_MT_ABR_LOST,
   eTWI_FAILURE_TX_REG_ADDR,
   eTWI_FAILURE_TX_DATA,
   eTWI_FAILURE_RX_DATA,
   eTWI_FAILURE 

}TWI_RET_E;

typedef enum _TWI_INIT_MODE_E
{
   eTWI_INIT_MODE_16_0000M_100K_1 = 0,
   eTWI_INIT_MODE_16_0000M_400K_1,
   eTWI_INIT_MODE_16_0000M_100K_4,
   eTWI_INIT_MODE_16_0000M_400K_4,
   eTWI_INIT_MODE_INVALID

}TWI_INIT_MODE_E;

typedef enum _TWI_STATE_MODE_E
{
   eTWI_MODE_READ = 0,
   eTWI_MODE_WRITE,
   eTWI_MODE_INVALID

}TWI_MODE_E;

typedef struct _TWI_INIT_PARAMS_X
{
   TWI_INIT_MODE_E e_mode;
   TWI_DEBUG_PFN pfn_debug;

}TWI_INIT_PARAMS_X;

typedef struct _TWI_SEND_RECV_BUFFER_PARAMS_X
{
   uint8_t uc_dev_addr;
   uint8_t uc_dev_id; 
   uint8_t *puc_reg_addr;
   uint32_t ui_reg_addr_sz;
   uint8_t *puc_data;
   uint32_t ui_data_sz;
   
}TWI_SEND_RECV_BUFFER_PARAMS_X;


TWI_RET_E twi_init(TWI_INIT_PARAMS_X *px_init_params);
TWI_RET_E twi_send_recv_buffer(TWI_SEND_RECV_BUFFER_PARAMS_X *px_send_buffer_params,
                               TWI_MODE_E e_mode); 

#endif
