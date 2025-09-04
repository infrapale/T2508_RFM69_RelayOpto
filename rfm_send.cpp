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

void rfm_send_initialize(void)
{
    send_msg.send_timeout = 0;
}

rfm_send_msg_st *rfm_send_get_data_ptr(void)
{
    return &send_msg;
}
bool rfm_send_ready(void)
{
    return (millis() > send_msg.send_timeout);
}

void rfm_send_radiate_msg( char *radio_msg )
{
     
    if ((radio_msg[0] != 0) && rfm_send_ready())
    {
        #ifdef DEBUG_PRINT
        Serial.println(radio_msg);
        #endif
        rf69p->waitPacketSent();
        rf69p->send((uint8_t *)radio_msg, strlen(radio_msg));      
        send_msg.send_timeout = millis() + 2000;
    }
}


