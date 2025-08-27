/**
T2508_RFM69_Modem 
HW: Arduino Mini + RFM69 Module

Send and receive data via UART

*******************************************************************************
https://github.com/infrapale/T2310_RFM69_TxRx
https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio
https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide/all
*******************************************************************************

Module = 'X'
Address = '1'

******** UART ***************** Transmit Raw ********* Radio ********************
                                  --------
 <#X1T:Hello World>\n             |      |
--------------------------------->|      | Hello World
                                  |      |-------------------------------------->
                                  |      |
<---------------------------------|      |
                                  |      |<-------------------------------------
                                  --------

******** UART ***************** Transmit Node ********* Radio ********************
                                  --------
 <#X1N:RMH1;RKOK1;T;->\n          |      |
--------------------------------->|      | {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
                                  |      |-------------------------------------->
                                  |      |
<---------------------------------|      |
                                  |      |<-------------------------------------
                                  --------

******** UART *************** Check Radio Data ********* Radio ********************
                                  --------
<#X1A:>\n                         |      |
--------------------------------->|      | 
<#X1a:0>\n                        |      |
<---------------------------------|      | {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                                  |      |<-------------------------------------
<#X1A:>\n                         |      |
--------------------------------->|      | 
<#X1a:1>\n                        |      |
<---------------------------------|      | 
                                  |      |
                                  --------

******** UART ************ Read Radio Raw Data ********* Radio ********************
                                  --------
                                  |      | {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                                  |      |<-------------------------------------
<#X1R:>\n                         |      |
--------------------------------->|      | 
<#X1r:{"Z":"OD_1","S":"Temp",     |      |
"V":23.1,"R":"-"}>                |      |
<---------------------------------|      | 
                                  |      |
                                  --------

******** UART ************ Read Radio Node Data ********* Radio ********************
                                  --------
                                  |      | {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                                  |      |<-------------------------------------
<#X1O:>\n                         |      |
--------------------------------->|      | 
<#X1o:OD_1;Temp;23.1;->\n         |      |
<---------------------------------|      | 
                                  |      |
                                  --------


*******************************************************************************

UART Commands
  UART_CMD_TRANSMIT_RAW   = 'T',
  UART_CMD_TRANSMIT_NODE  = 'N',
  UART_CMD_GET_AVAIL      = 'A',
  UART_CMD_READ_RAW       = 'R',
  UART_CMD_READ_NODE      = 'O' 

UART Replies
  UART_REPLY_AVAILABLE    = 'a',
  UART_REPLY_READ_RAW     = 'r',
  UART_REPLY_READ_NODE    = 'o' 

*******************************************************************************
Sensor Radio Message:   {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                        {"Z":"Dock","S":"T_dht22","V":"8.7","R":"-"}
Relay Radio Message     {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
Sensor Node Rx Mesage:  <#X1N:OD1;Temp;25.0;->
Relay Node Rx Mesage:   <#X1N:RMH1;RKOK1;T;->

******** UART ******************* Get @home  ********* Radio ********************
                                  -----------
                                  |         |
                                  |         |
                                  | Home=F  |<----{"Z":"TK1","S":"Home","V":"F","R":"-"}-
                                  |         |
                                  |         |<-------------------------------------
                                  -----------




******** UART ******************* Read PIR    ********* Radio ********************
                                  -----------
<----<#O11>\n---------------------|         | 
                                  |         |
-----<#O11:L>\n------------------>|         |
                                  | UART_SM |
                                  |         |----{"Z":"TK1","S":"PIR_1","V":"A","R":"-"}-->
                                  | Home==T |----{"Z":"VA","S":"Beep","V":"1","R":"-"}---->
                                  | Home==F |----{"Z":"VA","S":"Beep","V":"5","R":"-"}---->
                                  |         |
                                  |         |<-------------------------------------
                                  -----------

  UART_SM:  UART Alarm handling state machine


*******************************************************************************
Relay Mesage:      
      <#Rur=x>   u = relay unit, r= relay index, x=  0=off, 1=on, T=toggle
      <#R12=x>   x:  0=off, 1=on, T=toggle

Read Opto Input Message:
      <#Oui>   u = opto unit, i= opto index
      <=Oui:s>  s=state = H|L   High/Low
      ----> <#O12>  
      <---- <#O12:L>  



*******************************************************************************
**/

#include <Arduino.h>
#include "main.h"
#ifdef ADAFRUIT_FEATHER_M0
#include <wdt_samd21.h>
#endif
#ifdef PRO_MINI_RFM69
#include "avr_watchdog.h"
#endif
#include "secrets.h"
#include <RH_RF69.h>
#include <VillaAstridCommon.h>
#include "atask.h"
#include "json.h"
#include "rfm69.h"
#include "uart.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"
#include "pir.h"

#define ZONE  "OD_1"
//*********************************************************************************************
#define SERIAL_BAUD   9600
#define ENCRYPTKEY    RFM69_KEY   // defined in secret.h


RH_RF69         rf69(RFM69_CS, RFM69_INT);
RH_RF69         *rf69p;
module_data_st  me = {'X','1'};
time_type       MyTime = {2023, 11,01,1,01,55}; 

#define NBR_TEST_MSG  4
#define LEN_TEST_MSG  32
const char test_msg[NBR_TEST_MSG][LEN_TEST_MSG] =
{  //12345678901234567890123456789012
    "<#X1N:Dock;T_bmp1;9.1;->",
    "<#X1N:Dock;T_dht22;8.7;->",
    "<#X1N:Dock;T_Water;5.3;->",
    "<#X1N:Dock;ldr1;0.33;->",
};

void debug_print_task(void);
void run_100ms(void);
void send_test_data_task(void);
void rfm_receive_task(void); 


atask_st debug_print_handle        = {"Debug Print    ", 5000,0, 0, 255, 0, 1, debug_print_task};
atask_st clock_handle              = {"Tick Task      ", 100,0, 0, 255, 0, 1, run_100ms};
atask_st rfm_receive_handle        = {"Receive <- RFM ", 10000,0, 0, 255, 0, 1, rfm_receive_task};

#ifdef SEND_TEST_MSG
atask_st send_test_data_handle     = {"Send Test Task ", 10000,0, 0, 255, 0, 1, send_test_data_task};
#endif

#ifdef PRO_MINI_RFM69
//AVR_Watchdog watchdog(4);
#endif

rfm_receive_msg_st  *receive_p;
rfm_send_msg_st     *send_p;
uart_st         *uart_p;



void initialize_tasks(void)
{
  atask_initialize();
  atask_add_new(&debug_print_handle);
  atask_add_new(&clock_handle);
  atask_add_new(&rfm_receive_handle);

  #ifdef SEND_TEST_MSG
  atask_add_new(&send_test_data_handle);
  #endif
  Serial.print("Tasks initialized "); Serial.println(TASK_NBR_OF);
}


void setup() 
{
    //while (!Serial); // wait until serial console is open, remove if not tethered to computer
    delay(2000);
    Serial.begin(9600);
    Serial.print("T2508_RelayOpto"); Serial.print(" Compiled: ");
    Serial.print(__DATE__); Serial.print(" ");
    Serial.print(__TIME__); Serial.println();

    SerialX.begin(9600);
    
    uart_p = uart_get_data_ptr();
    send_p = rfm_send_get_data_ptr();

    rf69p = &rf69;
    rfm69_initialize(&rf69);
    rfm_receive_initialize();
    io_initialize();
    // Hard Reset the RFM module
    
    initialize_tasks();
    uart_initialize();
    pir_initialize();

    rfm_send_radiate_msg("T2508_RelayOpto");


    #ifdef ADAFRUIT_FEATHER_M0
    // Initialze WDT with a 2 sec. timeout
    wdt_init ( WDT_CONFIG_PER_16K );
    #endif
    #ifdef PRO_MINI_RFM69
    //watchdog.set_timeout(4);
    #endif


}



void loop() 
{
    atask_run();  
}


void rfm_receive_task(void) 
{
    // uart_read_uart();    // if available -> uart->prx.str uart->rx.avail
    // if(uart_p->rx.avail)
    // {
    //     uart_parse_rx_frame();
    //     #ifdef DEBUG_PRINT
    //     Serial.println(uart_p->rx.str);
    //     uart_print_rx_metadata();
    //     #endif
    //     if ( uart_p->rx.status == STATUS_OK_FOR_ME)
    //     {
    //         uart_exec_cmnd(uart_p->rx.cmd);
    //     }
    //     uart_p->rx.avail = false;
    // }
    rfm_receive_message();
    #ifdef ADAFRUIT_FEATHER_M0
    wdt_reset();
    #endif
    #ifdef PRO_MINI_RFM69
    // watchdog.clear();
    #endif
}


void run_100ms(void)
{
    static uint8_t ms100 = 0;
    if (++ms100 >= 10 )
    {
        ms100 = 0;
        if (++MyTime.second > 59 )
        {
          MyTime.second = 0;
          if (++MyTime.minute > 59 )
          {    
            MyTime.minute = 0;
            if (++MyTime.hour > 23)
            {
                MyTime.hour = 0;
            }
          }   
      }
    }
    io_run_100ms();
}

void debug_print_task(void)
{
  atask_print_status(true);
}

#ifdef SEND_TEST_MSG
void send_test_data_task(void)
{
    if  (send_test_data_handle.state >= NBR_TEST_MSG ) send_test_data_handle.state = 0;

    uart_p->rx.str  = test_msg[send_test_data_handle.state];
    uart_p->rx.avail = true;
    send_test_data_handle.state++;
}
#endif

