#include "main.h"
#include "io.h"


typedef struct
{
    uint8_t  pin;
    uint16_t tick_cntr;
} led_ctrl_st;


led_ctrl_st led_ctrl[LED_NBR_OF] = 
{
    [LED_INDX_ONBOARD]   = {PIN_LED_ONBOARD, 0},
    [LED_INDX_RED]       = {PIN_LED_RED, 0,},
    [LED_INDX_GREEN]     = {PIN_LED_GREEN, 0},
    [LED_INDX_BLUE]      = {PIN_LED_BLUE, 0}
};

void io_initialize(void)
{
    pinMode(PIN_LED_ONBOARD, OUTPUT);
    digitalWrite(PIN_LED_ONBOARD, LOW);

    #ifdef  LED_INDICATION
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_BLUE, LOW);
    // digitalWrite(PIN_LED_RED, HIGH);   delay(1000);
    // digitalWrite(PIN_LED_GREEN, HIGH); delay(1000);
    // digitalWrite(PIN_LED_BLUE, HIGH);  delay(1000);    
    // digitalWrite(PIN_LED_RED, LOW); digitalWrite(PIN_LED_GREEN, LOW);  digitalWrite(PIN_LED_BLUE, LOW);
    #endif
    io_led_flash(LED_INDX_GREEN,10);

}

void io_led_flash(led_index_et led_indx, uint16_t nbr_ticks )
{
    // duration = nbr_ticks * 100ms
    uint8_t lindx = LED_INDX_ONBOARD;
    #ifdef  LED_INDICATION
    lindx = led_indx;
    #endif
    digitalWrite(led_ctrl[lindx].pin, HIGH);
    led_ctrl[lindx].tick_cntr = nbr_ticks;
}

void io_run_100ms(void)
{
    for (uint8_t lindx = 0; lindx < LED_NBR_OF; lindx++ )
    {
       if (led_ctrl[lindx].tick_cntr > 0)
       {
          led_ctrl[lindx].tick_cntr--;
          if (led_ctrl[lindx].tick_cntr == 0)
          {
            digitalWrite(led_ctrl[lindx].pin, LOW);
          }
       } 
    }
}
