#ifndef __RFM_SEND_H__
#define __RFM_SEND_H__
#include  "uart.h"

typedef struct
{
    char            radio_msg[MAX_MESSAGE_LEN];
    uint32_t        send_timeout;
} rfm_send_msg_st;


void rfm_send_initialize(void);

bool rfm_send_ready(void);

/// @brief  Get pointer to module data 
/// @note   
/// @return pointer to data
rfm_send_msg_st *rfm_send_get_data_ptr(void);

/// @brief  Send message
/// @param  message to send
/// @return
void rfm_send_radiate_msg( char *radio_msg );

#endif