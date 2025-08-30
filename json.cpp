#include "main.h"
#include "uart.h"
#include "json.h"

uint16_t json_convert_uart_node_to_json(char *buff, uart_st *uart_p)
{
    byte i;
    unsigned int json_len;
    //Serial.println("ConvertFloatSensorToJson");
    String JsonString; 
    JsonString = "{\"Z\":\"";
    JsonString += uart_p->node.zone;
    JsonString += "\",";
    JsonString += "\"S\":\"";
    JsonString += uart_p->node.name;  
    JsonString += "\",";
    JsonString += "\"V\":\"";
    JsonString += uart_p->node.value;
    JsonString += "\",";
    JsonString += "\"R\":\"";
    JsonString += uart_p->node.remark;
    JsonString += "\"}";
    
    //Serial.println(JsonString);
    json_len = JsonString.length();
    if (json_len <= MAX_MESSAGE_LEN){
        //for (i=0;i<MAX_MESSAGE_LEN;i++) radio_packet[i]=0;
        JsonString.toCharArray(buff, MAX_MESSAGE_LEN);
        #ifdef DEBUG_PRINT
        Serial.println(json_len);
        #endif       
        return( json_len );
    }
    else {
      Serial.print(F("JSON string was too long for the radio packet: ")); 
      Serial.println(json_len);
      return(0);
    }
}


uint16_t json_convert_sensor_float_to_json(
    char *buff, 
    const char *zone, 
    const char *sensor, 
    float value, 
    const char *remark )
{
    byte i;
    unsigned int json_len;
    //Serial.println("ConvertFloatSensorToJson");
    String JsonString; 
    JsonString = "{\"Z\":\"";
    JsonString += zone;
    JsonString += "\",";
    JsonString += "\"S\":\"";
    JsonString += sensor;  
    JsonString += "\",";
    JsonString += "\"V\":";
    JsonString += value;
    JsonString += ",";
    JsonString += "\"R\":\"";
    JsonString += remark;
    JsonString += "\"}";
    
    //Serial.println(JsonString);
    json_len = JsonString.length();
    if (json_len <= MAX_MESSAGE_LEN){
       //for (i=0;i<MAX_MESSAGE_LEN;i++) radio_packet[i]=0;
       JsonString.toCharArray(buff, MAX_MESSAGE_LEN);
       Serial.println(json_len);
       return( json_len );
    }
    else {
      Serial.print(F("JSON string was too long for the radio packet: ")); 
      Serial.println(json_len);
      return(0);
    }
}



String json_parse_tag(String json_str, const char *tag){
   int pos1;
   int pos2;
   //Serial.println(fromStr); Serial.println(tag);
   pos1 = json_str.indexOf(tag);
   if (pos1 >= 0){
       if (tag == ",\"V"){
          pos1 = json_str.indexOf(":",pos1) +1;
          if (json_str.charAt(pos1) == '\"') pos1++;
          pos2 = json_str.indexOf(",",pos1);
          if (json_str.charAt(pos2-1) == '\"') pos2--;
       }
       else{
          pos1 = json_str.indexOf(":\"",pos1) +2;
          pos2 = json_str.indexOf("\"",pos1);
       }
       return(json_str.substring(pos1,pos2)); 
   }
   else {
      return("");
   }
    
}


void json_pick_data_from_rx(uart_st *uart_p)
{
    uart_p->node.zone   = json_parse_tag(uart_p->rx.msg.str, "{\"Z");
    uart_p->node.name   = json_parse_tag(uart_p->rx.msg.str, ",\"S");
    uart_p->node.value  = json_parse_tag(uart_p->rx.msg.str, ",\"V");
    uart_p->node.remark = json_parse_tag(uart_p->rx.msg.str, ",\"R");
}
