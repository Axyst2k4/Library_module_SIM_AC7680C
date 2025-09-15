<<<<<<< HEAD
#include "setup_sim.h"
#include "Rx_lib_SIM.h"
typedef enum {
    STATE_IDLE,                     // 0: Trạng thái nghỉ
	
    // Chuỗi kết nối
    STATE_CMD_1,       
	RESPONSE_CMD_1,
	
    STATE_CMD_2,        
	RESPONSE_CMD_2,  

	STATE_CMD_3,
	RESPONSE_CMD_3,
    // Chuỗi gửi dữ liệu
    STATE_SEND_DATA_CMD_1,          // 8: Gửi lệnh bắt đầu gửi data
    STATE_WAIT_DATA_RESPONSE_1,     // 9: Chờ phản hồi data

    // Trạng thái lỗi
    STATE_ERROR,                    // 10: Xử lý lỗi chung
    STATE_TIMEOUT,                  // 11: Xử lý khi hết giờ

} SimMachineState_t;

// Khai báo biến trạng thái
SimMachineState_t g_state = STATE_IDLE;
//==========================RECEIVER======================================//

void connect_broker(UART_HandleTypeDef* huart){
	while(1)
	{
	switch(g_state){
		case STATE_CMD_1:
			HAL_UART_Transmit(huart, "AT+CMQTTSTART\r\n", strlen("AT+CMQTTSTART\r\n"), 1000);
			g_state = RESPONSE_CMD_1;
		case RESPONSE_CMD_1:
			Wait_Response( RESPONSE_TYPE_OK,2000);
			Wait_Response( RESPONSE_TYPE_OK,2000);
			g_state = STATE_CMD_2;
		case STATE_CMD_2:
			sprintf(com ,"AT+CMQTTACCQ=%d,\"%s\",%d\r\n",client_index,clientID,server_type);
			HAL_UART_Transmit(huart,com, strlen(com), 1000);
			g_state = RESPONSE_CMD_2;
		case RESPONSE_CMD_2:
			Wait_Response();
			g_state = STATE_CMD_3;
		case STATE_CMD_3:
			sprintf(com ,"AT+CMQTTCONNECT=%d,\"%s\",%d,%d\r\n",client_index,clientID,keepalive_time,clean_session);
			HAL_UART_Transmit(huart,com, strlen(com), 1000);
			g_state = RESPONSE_CM3;
		case RESPONSE_CMD_3:
			Wait_Response();
			break;
	}

	}
}	

void connect_SMS(UART_HandleTypeDef* huart){
    sprintf(com,"AT+CMGF=1\r\n");
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    sprintf(com,"AT+CSCS=\"GSM\"\r\n");
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout_module(huart, &ouput_data.no_data.index, 0,com);
}

void Send_cm_broker(UART_HandleTypeDef* huart, const char* comman) {
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
=======
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
>>>>>>> 693b0e8 (Auto-sync: file changes)
}