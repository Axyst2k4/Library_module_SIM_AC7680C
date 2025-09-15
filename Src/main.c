#include "lib_sim.h"
//================================================================================
char com[50];
char buffer_response[BUFFER_SIZE];
char buffer_calib[BUFFER_SIZE];
char tokens[MAX_TOKENS][BUFFER_SIZE];
bool received_flag = false;
int req_length =strlen(topic); 
//================================================================================


match_table matching_command(char *token){






}
//====================================SEND_broker===============================================================
void timeout(UART_HandleTypeDef* huart,volatile int *state_ptr,int value){
    bool connected = false; 
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < 1000){
        if (*state_ptr == value) {
        connected = true;
        break; 
        }
    }
     if (!connected){
    connect_broker(huart); 
    }
}





void Send_data_broker(UART_HandleTypeDef* huart, const char* payload) {
    sprintf(com, "AT+CMQTTTOPIC=%d,%d\r\n", client_index, req_length);
    HAL_UART_Transmit(huart, com, strlen(com), 1000);
    timeout_module(huart, &ouput_data.no_data.index, 0,com);
    HAL_UART_Transmit(huart, topic, req_length, 1000);
    HAL_UART_Transmit(huart, "\r\n", 2, 1000); 
    sprintf(com, "AT+CMQTTPAYLOAD=%d,%d\r\n", client_index, strlen(payload));
    HAL_UART_Transmit(huart, com, strlen(com), 1000);
    timeout_module(huart, &ouput_data.no_data.index, 0,com);
    HAL_UART_Transmit(huart, payload, strlen(payload), 1000);
    HAL_UART_Transmit(huart, "\r\n", 2, 1000); 
    sprintf(com, "AT+CMQTTPUB=%d,%d,0\r\n", client_index, qos);
    HAL_UART_Transmit(huart, com, strlen(com), 1000);
    timeout(huart, &ouput_data.no_data.index, 0);
}

//=====================================send sms======================================================

void Send_message_SMS(UART_HandleTypeDef* huart, char* phone, const char* message) {
    sprintf(com, "AT+CMGS=\"%s\"\r\n", phone);
    HAL_UART_Transmit(huart, com, strlen(com), 1000);
    timeout_module(huart, &ouput_data.no_data.index, 0,com);
    HAL_UART_Transmit(huart, message, strlen(message), 1000);
    HAL_UART_Transmit(huart, "\x1A", 1, 1000); 
}
//=======================================handler=================================================================
