#ifndef _SETUP_LIB_SIM_H_
#define _SETUP_LIB_SIM_H_

//------------------------------------------------connect_broker------------------------------------------------------------------
#define client_index 0
#define clientID "1234"
#define server_type 0
#define server_addr "tcp://<địa chỉ>:<port>"
#define keepalive_time 600
#define clean_session 1
#define topic "abc"

#define qos 0
#define PHONE_NUMBER "0000000000"
char com[256]; 

int g_state_count;

UART_HandleTypeDef* g_huart_processing = NULL;
