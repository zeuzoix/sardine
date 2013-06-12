#ifndef _TWI_STATES_H_
#define _TWI_STATES_H_

#define TWI_STATE_MAX_ADDR_TX_RETRY_CNT ((uint8_t)100)

typedef enum _TWI_STATE_E
{
   eTWI_STATE_TX_START = 0,
   eTWI_STATE_TX_WRITE_ADDRESS,
   eTWI_STATE_TX_READ_ADDRESS,
   eTWI_STATE_TX_REG_ADDR,
   eTWI_STATE_TX_DATA,
   eTWI_STATE_RX_DATA,
   eTWI_STATE_TX_STOP,
   eTWI_STATE_COMPLETE,
   eTWI_STATE_INVALID,
   
}TWI_STATE_E;

typedef struct _TWI_SEND_RECV_STATE_CTXT_X
{
   TWI_STATE_E e_state;
   TWI_MODE_E e_state_mode;
   TWI_STATE_E e_stop_state;
   uint8_t uc_dev_addr;
   uint8_t uc_dev_id; 
   uint8_t *puc_reg_addr;
   uint32_t ui_reg_addr_sz;
   uint32_t ui_reg_addr_cnt;
   uint8_t *puc_data;
   uint32_t ui_data_sz;
   uint32_t ui_data_cnt;
   uint8_t uc_addr_tx_retry_cnt;
   uint8_t uc_max_addr_tx_retry_cnt;

}TWI_SEND_RECV_STATE_CTXT_X;

typedef TWI_RET_E (*TWI_STATE_PFN)(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
typedef TWI_RET_E (*TWI_NEXT_STATE_PFN)(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);

typedef struct _TWI_STATE_CTXT_X
{
   TWI_STATE_E e_state;
   TWI_STATE_PFN pfn_state;
   TWI_STATE_PFN pfn_next_state;
   char *pc_state_name;
   
}TWI_STATE_CTXT_X;

TWI_RET_E twi_state_tx_start(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_start_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_write_address(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_write_address_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_read_address(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_read_address_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_reg_addr(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_reg_addr_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_data(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_data_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_stop(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_tx_stop_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_rx_data(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
TWI_RET_E twi_state_rx_data_next(TWI_SEND_RECV_STATE_CTXT_X *px_state_ctxt);
#endif
