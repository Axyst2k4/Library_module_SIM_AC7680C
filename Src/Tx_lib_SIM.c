#include "Tx_lib_SIM.h"
//====================================SEND_broker===============================================================
void Send_data_broker(UART_HandleTypeDef* huart,  const char* topic,const char* payload) {
    g_state_send_broker = STATE_SENDBROKER_1;
    int topic_length =strlen(topic); 
    while(g_state_send_broker == DONE_SENDBROKER)
    {
	switch(g_state_send_broker){
		case STATE_SENDBROKER_1:
            sprintf(com, "AT+CMQTTTOPIC=%d,%d\r\n", client_index,topic_length);
            HAL_UART_Transmit_IT(huart, com, strlen(com));
            g_state_send_broker = RESPONSE_SENDBROKER_1;
            break;
        case RESPONSE_SENDBROKER_1:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_PROMPT,2000,huart);
				g_state_send_broker = STATE_SENDBROKER_2;
				break;
			}
			break;
        
        case STATE_SENDBROKER_2:
            HAL_UART_Transmit_IT(huart, topic, topic_length);
            HAL_UART_Transmit_IT(huart, "\r\n", 2, 1000); 
            g_state_send_broker = RESPONSE_SENDBROKER_2;
			break;
        case RESPONSE_SENDBROKER_2:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				g_state_send_broker = STATE_SENDBROKER_3;
				break;
			}
			break;
        case STATE_SENDBROKER_3:
            sprintf(com, "AT+CMQTTPAYLOAD=%d,%d\r\n", client_index, strlen(payload));
            HAL_UART_Transmit_IT(huart, com, strlen(com));
            g_state_send_broker = RESPONSE_SENDBROKER_3;
			break;
        case RESPONSE_SENDBROKER_3:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_PROMPT,2000,huart);
				g_state_send_broker = STATE_SENDBROKER_4;
				break;
			}
			break;
        
        case STATE_SENDBROKER_4:
            HAL_UART_Transmit_IT(huart, payload, strlen(payload));
            HAL_UART_Transmit_IT(huart, "\r\n", 2, 1000); 
            g_state_send_broker = RESPONSE_SENDBROKER_4;
			break;
        case RESPONSE_SENDBROKER_4:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				g_state_send_broker = STATE_SENDBROKER_5;
				break;
			}
			break;
        case STATE_SENDBROKER_5:
            sprintf(com, "AT+CMQTTPUB=%d,0,%d\r\n", client_index,pub_timeout);
            HAL_UART_Transmit_IT(huart, com, strlen(com), 1000);
            g_state_send_broker = RESPONSE_SENDBROKER_5;
			break;
        case RESPONSE_SENDBROKER_5:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
                Wait_Response( RESPONSE_TYPE_CMQTTPUB,2000,huart);
				g_state_send_broker = DONE_SENDBROKER;
				break;
			}
			break;
        }
    }
}
//=====================================send sms======================================================
void Send_message_SMS(UART_HandleTypeDef* huart, char* phone, const char* message) {

    g_state_send_SMS = STATE_SENDSMS_1;
    int topic_length =strlen(topic); 
    while(g_state_send_SMS == DONE_SENDSMS)
    {
	switch(g_state_send_SMS){
		case STATE_SENDSMS_1:
            sprintf(com, "AT+CMGS=\"%s\"\r\n", phone);
            HAL_UART_Transmit_IT(huart, com, strlen(com), 1000);
            g_state_send_SMS = RESPONSE_SENDSMS_1;
            break;
        case RESPONSE_SENDSMS_1:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_PROMPT,2000,huart);
				g_state_send_SMS = STATE_SENDSMS_2;
				break;
			}
			break;
        
        case STATE_SENDSMS_2:
            HAL_UART_Transmit_IT(huart, message, strlen(message), 1000);
             g_state_send_SMS = RESPONSE_SENDSMS_2;
            break;
        case RESPONSE_SENDSMS_2:
            if (g_tx_complete == 1){
				g_tx_complete = 0;
				Wait_Response( RESPONSE_TYPE_PROMPT,2000,huart);
				g_state_send_SMS = STATE_SENDSMS_2;
				break;
			}
			break;

        case STATE_SENDSMS_3:
            HAL_UART_Transmit_IT(huart, "\x1A", 1, 1000); 
            g_state_send_SMS = DONE_SENDSMS;
            break;
        }
    }
}
