#ifndef __RFM_RECEIVE_H__
#define __RFM_RECEIVE_H__

#include "rfm69.h"
#include "io.h"

typedef struct
{
    char            radio_msg[MAX_MESSAGE_LEN];
    uint8_t         len;
    bool            avail;
    int8_t          rssi;
} rfm_receive_msg_st;


/// @brief  Initialize module, available = false
/// @param  -
/// @return -
void rfm_receive_initialize(void);

/// @brief  Get pointer to module data
/// @note
/// @param
/// @return pointer
rfm_receive_msg_st *rfm_receive_get_data_ptr(void);

/// @brief  Receive message when available
/// @note   save message in radio_msg
/// @param  -
/// @return -
void rfm_receive_message(void);

/// @brief  Check if a radio message is available
/// @note
/// @param
/// @return
bool rfm_receive_message_is_avail(void);

void rfm_receive_clr_message_flag(void);


#endif

