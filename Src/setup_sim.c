
#include "setup_sim.h"
#include "Rx_lib_SIM.h"
typedef enum {
    STATE_IDLE,                     // 0: Trạng thái nghỉ
	
    //connect mqtt
    STATE_MQTT_1,       
	RESPONSE_MQTT_1,
    STATE_MQTT_2,        
	RESPONSE_MQTT_2,  
	STATE_MQTT_3,
	RESPONSE_MQTT_3,

    // connect sms
    STATE_SMS_1,       
	RESPONSE_SMS_1,
    STATE_SMS_2,        
	RESPONSE_SMS_2,  
	
	// send sms



} SimMachineState_t;

// Khai báo biến trạng thái
SimMachineState_t g_state = STATE_IDLE;
int g_state_count;
//==========================RECEIVER======================================//

void Connect_broker(UART_HandleTypeDef* huart){
	g_state = STATE_MQTT_1;
	g_state_count = 0;
	for(g_state_count;g_state_count < 6;g_state_count ++)
	{
	switch(g_state){
		case STATE_MQTT_1:
			HAL_UART_Transmit(huart, "AT+CMQTTSTART\r\n", strlen("AT+CMQTTSTART\r\n"), 1000);
			g_state = RESPONSE_MQTT_1;
			break;
		case RESPONSE_MQTT_1:
			Wait_Response( RESPONSE_TYPE_OK,2000);
			Wait_Response( RESPONSE_TYPE_CMQTTSTART,10000);
			g_state = STATE_MQTT_2;
			break;
		case STATE_MQTT_2:
			sprintf(com ,"AT+CMQTTACCQ=%d,\"%s\",%d\r\n",client_index,clientID,server_type);
			HAL_UART_Transmit(huart,com, strlen(com), 1000);
			g_state = RESPONSE_MQTT_2;
			break;
		case RESPONSE_MQTT_2:
			Wait_Response( RESPONSE_TYPE_OK,2000);
			g_state = STATE_MQTT_3;
			break;
		case STATE_MQTT_3:
			sprintf(com ,"AT+CMQTTCONNECT=%d,\"%s\",%d,%d\r\n",client_index,server_addr,keepalive_time,clean_session);
			HAL_UART_Transmit(huart,com, strlen(com), 1000);
			g_state = RESPONSE_MQTT_3;
			break;
		case RESPONSE_MQTT_3:
			Wait_Response( RESPONSE_TYPE_OK,2000);
			Wait_Response( RESPONSE_TYPE_CMQTTCONNECT,10000);
			break;
		}
		
	}
}	

void Connect_SMS(UART_HandleTypeDef* huart){
	g_state = STATE_SMS_1;
	while(1)
	{
	switch(g_state){
		case STATE_SMS_1:
			sprintf(com,"AT+CMGF=1\r\n");
			HAL_UART_Transmit(huart,com, strlen(com), 1000);
			g_state = RESPONSE_SMS_1;
		case RESPONSE_SMS_1:
			Wait_Response( RESPONSE_TYPE_OK,2000);
			g_state = STATE_SMS_2;
		case STATE_SMS_2:
			sprintf(com,"AT+CSCS=\"GSM\"\r\n");
			HAL_UART_Transmit(huart,com, strlen(com), 1000);
			g_state = RESPONSE_SMS_2;
		case RESPONSE_SMS_2:
			Wait_Response( RESPONSE_TYPE_OK,2000);
			return;
		}
	}
}	
    
    

void Send_broker(UART_HandleTypeDef* huart, const char* comman) {
    HAL_UART_Transmit(huart, comman, strlen(comman), 1000);
    timeout(huart, &ouput_data.no_data.index, 0);
	}

 void Lib_SIM_Setup(GenericReceiver_t* receiver, UART_HandleTypeDef* huart, Receiver_Callback_t callback)
{
	if (receiver != NULL && huart != NULL){
    	receiver->huart = huart;
    	receiver->dma_buffer_idx = 0;
    	receiver->data_ready_callback = callback;
    	
    	__HAL_UART_ENABLE_IT(receiver->huart, UART_IT_IDLE);
		HAL_UART_Receive_DMA(receiver->huart, receiver->buffer_A,BUFFER_SIZE);
        connect_broker(huart);
        connect_SMS(huart);
	}

}