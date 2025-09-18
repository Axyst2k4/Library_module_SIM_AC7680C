
#include "setup_sim.h"
#include "Rx_lib_SIM.h"

//==========================RECEIVER======================================//

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == g_huart_processing) 
    {
        g_tx_complete = 1;
    }
}

void Connect_broker(UART_HandleTypeDef* huart){
	g_huart_processing = huart;
	switch(g_state){
		case STATE_MQTT_1:
			HAL_UART_Transmit_IT(huart, "AT+CMQTTSTART\r\n", strlen("AT+CMQTTSTART\r\n"));
			g_state = RESPONSE_MQTT_1;
			break;
		case RESPONSE_MQTT_1:
			if (g_tx_complete ==1){
				g_tx_complete =0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				Wait_Response( RESPONSE_TYPE_CMQTTSTART,10000,huart);
				g_state= STATE_MQTT_2;
				break;
			}
			break;
		case STATE_MQTT_2:
			sprintf(com ,"AT+CMQTTACCQ=%d,\"%s\",%d\r\n",client_index,clientID,server_type);
			HAL_UART_Transmit_IT(huart,com, strlen(com), 1000);
			g_state = RESPONSE_MQTT_2;
			break;
		case RESPONSE_MQTT_2:
		if (g_tx_complete ==1){
				g_tx_complete =0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				g_state = STATE_MQTT_3;
				break;
			}
			break;
		case STATE_MQTT_3:
			sprintf(com ,"AT+CMQTTCONNECT=%d,\"%s\",%d,%d\r\n",client_index,server_addr,keepalive_time,clean_session);
			HAL_UART_Transmit_IT(huart,com, strlen(com), 1000);
			g_state = RESPONSE_MQTT_3;
			break;
		case RESPONSE_MQTT_3:
		if (g_tx_complete ==1){
				g_tx_complete =0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				Wait_Response( RESPONSE_TYPE_CMQTTCONNECT,10000,huart);
				g_state = DONE_CONNECT_MQTT;
			break;
			}
			break;
		}
		
	}

void Connect_SMS(UART_HandleTypeDef* huart){
	g_state = STATE_SMS_1;
	switch(g_state){
		case STATE_SMS_1:
			sprintf(com,"AT+CMGF=1\r\n");
			HAL_UART_Transmit_IT(huart,com, strlen(com), 1000);
			g_state = RESPONSE_SMS_1;
			break;
		case RESPONSE_SMS_1:
		if (g_tx_complete ==1){
				g_tx_complete =0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				g_state = STATE_SMS_2;
				break;
			}
			break;
		case STATE_SMS_2:
			sprintf(com,"AT+CSCS=\"GSM\"\r\n");
			HAL_UART_Transmit_IT(huart,com, strlen(com), 1000);
			g_state = RESPONSE_SMS_2;
		case RESPONSE_SMS_2:
		if (g_tx_complete ==1){
				g_tx_complete =0;
				Wait_Response( RESPONSE_TYPE_OK,2000,huart);
				g_state = STATE_IDLE;
				break;
		}
	}
}

 void Lib_SIM_Setup(GenericReceiver_t* receiver, UART_HandleTypeDef* huart, Receiver_Callback_t callback)
{
	g_state_setup = STATE_INIT_UART;
	while(g_state_setup != DONE_SETUP){
	switch (g_state_setup){
		case STATE_INIT_UART:
			receiver->huart = huart;
			receiver->dma_buffer_idx = 0;
			receiver->data_ready_callback = callback;
			__HAL_UART_ENABLE_IT(receiver->huart, UART_IT_IDLE);
			HAL_UART_Receive_DMA(receiver->huart, receiver->buffer_A,BUFFER_SIZE);
			g_state_setup = STATE_CONNECT_MQTT;
			break;
		case STATE_CONNECT_MQTT:
			connect_broker(huart);
			g_state_setup = STATE_CONNECT_SMS;
			break;
		case STATE_CONNECT_SMS:
			connect_SMS(huart);
			g_state_setup = DONE_SETUP;
			break;
		case STATE_ERROR:
			Lib_SIM_Setup(huart);
			g_state = STATE_MQTT_1;
			break;
		}
	}
}

