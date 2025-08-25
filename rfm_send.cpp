#include "main.h"
#include <Arduino.h>
#include <Wire.h>
#include <RH_RF69.h>
#include <SPI.h>

#include "rfm69.h"
#include "rfm_send.h"
#include "main.h"
#include "json.h"

extern module_data_st  me;
extern RH_RF69 *rf69p;

rfm_send_msg_st     send_msg;

//char radio_packet[RH_RF69_MAX_MESSAGE_LEN];

rfm_send_msg_st *rfm_send_get_data_ptr(void)
{
    return &send_msg;
}

void rfm_send_radiate_msg( char *radio_msg )
{
     
    if (radio_msg[0] != 0)
    {
        #ifdef DEBUG_PRINT
        Serial.println(radio_msg);
        #endif
        rf69p->waitPacketSent();
        rf69p->send((uint8_t *)radio_msg, strlen(radio_msg));      
    }
}


