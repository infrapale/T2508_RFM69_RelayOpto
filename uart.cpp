#include "main.h"
#include "atask.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"
#include "pir.h"

uart_st         uart;
atask_st uart_alarm_handle     = {"UART Alarm Task", 100,0, 0, 255, 0, 1, uart_alarm_handling_task};
//atask_st debug_print_handle        = {"Debug Print    ", 5000,0, 0, 255, 0, 1, debug_print_task};

uart_st *uart_get_data_ptr(void)
{
    return &uart;
}

void uart_initialize(void)
{
    uart.rx.msg.avail = false;
    atask_add_new(&uart_alarm_handle);
}

bool uart_read_uart(void)
{
    bool avail = false;
    if (SerialX.available())
    {
        io_led_flash(LED_INDX_BLUE,20);
        uart.rx.msg.str = SerialX.readStringUntil('\n');
        if (uart.rx.msg.str.length()> 0)
        {
            uart.rx.msg.avail = true;
            //uart.rx.str.remove(uart.rx.str.length()-1);
        }
        avail = true;
        #ifdef DEBUG_PRINT
        Serial.println("rx is available");
        #endif        
    } 
    return avail;
}

void uart_parse_rx_frame(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    // Opto:  <R1O2:L>
    //         \\\\\\______ value, message (optional)
    //          \\\\\______ action set: '=' get: '?' reply: ':'
    //           \\\\______ index
    //            \\\______ function
    //             \\______ module_addr
    //              \______ module_tag



    bool do_continue = true;
    uart.rx.msg.status = STATUS_UNDEFINED;
    uint8_t len;
    uart.rx.msg.str.trim();
    uart.rx.msg.len = uart.rx.msg.str.length();
    uart.rx.msg.function = uart.rx.msg.str.charAt(1);

    if ((uart.rx.msg.str.charAt(0) != '<') || 
        (uart.rx.msg.str.charAt(uart.rx.msg.len-1) != '>'))  do_continue = false;

    if (do_continue)
    {
        uart.rx.msg.module_tag  = uart.rx.msg.str.charAt(1);
        uart.rx.msg.module_addr = uart.rx.msg.str.charAt(2);
        uart.rx.msg.function    = uart.rx.msg.str.charAt(3);
        uart.rx.msg.index       = uart.rx.msg.str.charAt(4);
        uart.rx.msg.action      = uart.rx.msg.str.charAt(5);
        #ifdef DEBUG_PRINT
        Serial.print("Buffer frame is OK\n");
        #endif

        switch(uart.rx.msg.action)
        {
            case UART_ACTION_REPLY:
                uart.rx.msg.value = uart.rx.msg.str.charAt(6);
                if(uart.rx.msg.value == 'L' || uart.rx.msg.value == 'H') 
                {
                    uart.rx.msg.status = STATUS_OK_FOR_ME;
                }
                break;
            default:
                uart.rx.msg.status = STATUS_NOT_FOR_ME;
                break;   
        }

    }
    else uart.rx.msg.status = STATUS_INCORRECT_FRAME;
}

void uart_build_node_from_rx_str(void)
{
    uint8_t indx1;
    uint8_t indx2;
    indx1 = 0;  //uart.rx.str.indexOf(':')
    indx2 = uart.rx.msg.str.indexOf(';');
    uart.node.zone = uart.rx.msg.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.msg.str.indexOf(';',indx1+1);
    uart.node.name = uart.rx.msg.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.msg.str.indexOf(';',indx1+1);
    uart.node.value = uart.rx.msg.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.msg.str.indexOf(';',indx1+1);
    uart.node.remark = uart.rx.msg.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.msg.str.indexOf(';',indx1+1);
    
}





void uart_build_node_tx_str(void)
{
    rfm_receive_msg_st *receive_p = rfm_receive_get_data_ptr();
    // <#X1N:RMH1;RKOK1;T;->\n   -> {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
    uart.rx.msg.str = (char*) receive_p->radio_msg;  
    uart.tx.msg.str = "<#X1a:";
    json_pick_data_from_rx(&uart);
    #ifdef DEBUG_PRINT
    Serial.print("radio_msg: ");
    Serial.println(uart.rx.msg.str);  
    Serial.println(uart.node.zone);
    Serial.println(uart.node.name);
    Serial.println(uart.node.value);
    Serial.println(uart.node.remark);
    #endif
    uart.tx.msg.str += uart.node.zone;
    uart.tx.msg.str += ';';
    uart.tx.msg.str += uart.node.name;
    uart.tx.msg.str += ';';
    uart.tx.msg.str += uart.node.value;
    uart.tx.msg.str += ';';
    uart.tx.msg.str += uart.node.remark;
    uart.tx.msg.str += '>';
}

void uart_build_raw_tx_str(void)
{
    rfm_receive_msg_st *receive_p = rfm_receive_get_data_ptr();
    uart.tx.msg.str = "<#X1r:";
    uart.tx.msg.str += (char*) receive_p->radio_msg;
    uart.tx.msg.str += '>';
}

void uart_rx_send_rfm_from_raw(void)
{
    String payload = uart.rx.msg.str.substring(6,uart.rx.msg.len - 1);
    payload.toCharArray(uart.rx.msg.radio_msg, MAX_MESSAGE_LEN);
    rfm_send_radiate_msg(uart.rx.msg.radio_msg);
}

void uart_rx_send_rfm_from_node(void)
{
    uart.rx.msg.str = uart.rx.msg.str.substring(6,uart.rx.msg.len - 1);
    uart_build_node_from_rx_str();
    rfm_send_msg_st *send_p = rfm_send_get_data_ptr();
    json_convert_uart_node_to_json(send_p->radio_msg, &uart);
    rfm_send_radiate_msg(send_p->radio_msg);
}

void uart_exec_cmnd(uart_cmd_et ucmd)
{
    String str = "<#Xux:0>";
    //str[3] = me.addr;
    switch(ucmd)
    {
        case UART_CMD_TRANSMIT_RAW:
            io_led_flash(LED_INDX_RED, 10);
            uart_rx_send_rfm_from_raw();
            break;
        case UART_CMD_TRANSMIT_NODE:
            io_led_flash(LED_INDX_RED, 20);
            uart_rx_send_rfm_from_node();
            break;
        case UART_CMD_GET_AVAIL:
            str[4] = UART_REPLY_AVAILABLE;
            if(rfm_receive_message_is_avail()) str[6] = '1';
            SerialX.println(str);
            break;
        case UART_CMD_READ_RAW:
            rfm_receive_clr_message_flag();
            uart_build_raw_tx_str();
            SerialX.println(uart.tx.msg.str);          
            break;
        case UART_CMD_READ_OPTO:
            rfm_receive_clr_message_flag();
            uart_build_node_tx_str();
            SerialX.println(uart.tx.msg.str);          
          break;

    }
}


void uart_print_rx_metadata(void)
{
    Serial.print("Length      "); Serial.println(uart.rx.msg.len);
    Serial.print("Avail       "); Serial.println(uart.rx.msg.avail);
    Serial.print("Status      "); Serial.println(uart.rx.msg.status);
    Serial.print("Module Tag  "); Serial.println(uart.rx.msg.module_tag);
    Serial.print("Module Addr "); Serial.println(uart.rx.msg.module_addr);
    Serial.print("Function    "); Serial.println(uart.rx.msg.function);
    Serial.print("Index       "); Serial.println(uart.rx.msg.index);
    Serial.print("Action      "); Serial.println((char)uart.rx.msg.action);
    Serial.print("Value       "); Serial.println(uart.rx.msg.value);
}    

void uart_handle_rx_data(void)
{
    uint8_t pir_indx;
    uint8_t pir_value = PIR_STATUS_UNDEFINED;

    if(uart.rx.msg.function == 'O')
    {
        pir_indx = 
            (uart.rx.msg.module_addr - '1') * 2 +
            (uart.rx.msg.index - '1');
        if ((pir_indx < NBR_OF_PIR) && (uart.rx.msg.action == UART_ACTION_REPLY));
        {
            switch(uart.rx.msg.value)
            {
                case UART_VALUE_LOW:
                    pir_value = PIR_STATUS_INACTIVE;
                    break;
                case UART_VALUE_HIGH:
                    pir_value = PIR_STATUS_ACTIVE;
                    break;
            }
        }
        if(pir_value != PIR_STATUS_UNDEFINED) 
        {
            pir_set_status(pir_indx, pir_value);
            Serial.print("PIR Status [");Serial.print(pir_indx);
            Serial.print("] = "); Serial.println(pir_value);
        } else Serial.println(F("PIR_STATUS_UNDEFINED"));
    }
}

void uart_build_tx_msg(uart_tx_st *txp)
{
    txp->msg.str = '<'; 
    txp->msg.str.concat(txp->msg.module_tag);
    txp->msg.str.concat(txp->msg.module_addr);
    txp->msg.str.concat(txp->msg.function);
    txp->msg.str.concat(txp->msg.index);
    txp->msg.str.concat((char)txp->msg.action);
    txp->msg.str.concat(txp->msg.value);
    txp->msg.str.concat('>');
    //txp->msg.str = "<XCCE>";
    
}

void uart_alarm_handling_task(void)
{
    switch(uart_alarm_handle.state)
    {
        case 0:
            uart.tx.msg.module_tag = 'R';
            uart.tx.msg.function = 'O';
            uart.tx.msg.action = '?';
            uart.tx.msg.value = '-';
            uart_alarm_handle.state = 10;
            uart.tx.msg.module_addr = '1';
            uart.tx.msg.index = '1';
            break;
        case 10:      
            uart_build_tx_msg(&uart.tx);
            // Serial.print("build_tx_msg: "); 
            //Serial.println(uart.tx.msg.str);
            uart.rx.timeout = millis() + 10000;
            uart_alarm_handle.state = 20;
            break;
        case 20:
            if (uart_read_uart())
            {
                Serial.print("uart.rx.msg.str: ");
                Serial.println(uart.rx.msg.str);
                uart_parse_rx_frame();
                //uart_print_rx_metadata();
                uart_handle_rx_data();
                uart_alarm_handle.state = 10;

            }
            else if (millis() > uart.rx.timeout)
            {
                //Serial.println("UART rx timeout");
                uart_alarm_handle.state = 10;
            }
            break;
        case 25:
            uart.tx.msg.index++;
            if(uart.tx.msg.index > '2')
            {
                uart.tx.msg.index = '1';  
                uart.tx.msg.module_addr++;
                if (uart.tx.msg.module_addr > '2') uart.tx.msg.module_addr= '1';
            }
            uart.rx.timeout = millis() + 10000;
            uart_alarm_handle.state = 26;
            break;
        case 26:
            if (millis() > uart.rx.timeout)
            {
                uart_alarm_handle.state = 10;
            }
            break;



        case 39:
            uart.tx.msg.index = '2';
            uart_build_tx_msg(&uart.tx);
            //Serial.print("build_tx_msg: "); Serial.println(uart.tx.msg.str);
            uart.rx.timeout = millis() + 10000;
            uart_alarm_handle.state = 40;
            break;
        case 40:
            if (uart_read_uart())
            {
                //Serial.println(uart.rx.msg.str);
                uart_parse_rx_frame();
                //uart_print_rx_metadata();
                uart_handle_rx_data();
                uart_alarm_handle.state = 50;

            }
            else if (millis() > uart.rx.timeout)
            {
                //Serial.println("UART rx timeout");
                uart_alarm_handle.state = 50;
            }
            break;
        case 50:
            uart.rx.timeout = millis() + 10000;
            uart_alarm_handle.state = 60;
        case 60:
            if (millis() > uart.rx.timeout) uart_alarm_handle.state = 0;
            break;
    }
}
