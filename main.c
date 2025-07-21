#include "lib_sim.h"
//================================================================================
char com[50];
char buffer_response[BUFFER_SIZE];
char buffer_calib[BUFFER_SIZE];
char tokens[MAX_TOKENS][BUFFER_SIZE];
bool received_flag = false;
int req_length =strlen(topic); //độ dài chuỗi topic(byte)
//================================================================================

int tokenize_at_response(const char* input, char output_tokens[][BUFFER_SIZE], int max_tokens)
{
    if (!input || !output_tokens) {
        return 0;
    }

    int token_count = 0;
    const char* current_pos = input;

    while (*current_pos != '\0' && token_count < max_tokens)
    {
        // 1. Bỏ qua các ký tự \r, \n hoặc khoảng trắng ở đầu
        while (*current_pos != '\0' && (*current_pos == '\r' || *current_pos == '\n')) {
            current_pos++;
        }

        // Nếu đã đến cuối chuỗi thì dừng lại
        if (*current_pos == '\0') {
            break;
        }

        // 2. Tìm điểm kết thúc của dòng hiện tại (\r\n)
        const char* end_of_line = strstr(current_pos, "\r\n");
        const char* line_start = current_pos;
        size_t token_len = 0;

        if (end_of_line != NULL) {
            // Nếu tìm thấy \r\n, tính độ dài của token
            token_len = end_of_line - line_start;
        } else {
            // Nếu không, lấy phần còn lại của chuỗi làm token cuối cùng
            token_len = strlen(line_start);
        }

        // 3. Sao chép token vào mảng kết quả nếu nó có nội dung
        if (token_len > 0) {
            // Đảm bảo không ghi tràn bộ đệm của token
            size_t len_to_copy = (token_len < MAX_TOKEN_LEN) ? token_len : MAX_TOKEN_LEN - 1;
            
            memcpy(output_tokens[token_count], line_start, len_to_copy);
            output_tokens[token_count][len_to_copy] = '\0'; // Đảm bảo kết thúc null
            
            token_count++;
        }
        
        // 4. Di chuyển con trỏ đến sau \r\n để chuẩn bị cho vòng lặp tiếp theo
        if (end_of_line != NULL) {
            current_pos = end_of_line + 2;
        } else {
            // Đã đến cuối chuỗi
            break;
        }
    }

    return token_count;
}


/*===================================================================================*/


ResponseType_e Parse_Response_Token(const char* token)
{
    if (token == NULL) {
        return RESPONSE_TYPE_UNKNOWN;
    }

    // Lấy số lượng mục trong bảng tra cứu
    int num_mappings = sizeof(response_map) / sizeof(response_map[0]);

    // Duyệt qua từng mục trong bảng
    for (int i = 0; i < num_mappings; i++)
    {
        const char* expected_str = response_map[i].str_to_compare;

        if (response_map[i].cmp_type == CMP_EXACT)
        {
            // So sánh chính xác
            if (strcmp(token, expected_str) == 0) {
                return response_map[i].type; // Trả về loại nếu khớp
            }
        }
        else // CMP_PREFIX
        {
            // So sánh tiền tố
            if (strncmp(token, expected_str, strlen(expected_str)) == 0) {
                return response_map[i].type; // Trả về loại nếu khớp
            }
        }
    }

    // Nếu không tìm thấy kết quả khớp nào sau khi duyệt hết bảng
    return RESPONSE_TYPE_UNKNOWN;
}

 //==========================RECEIVER======================================//
 void Lib_SIM_Setup(GenericReceiver_t* receiver, UART_HandleTypeDef* huart, Receiver_Callback_t callback)
{
	if (receiver != NULL && receiver->huart != NULL){
    	receiver->huart = huart;
    	receiver->data_ready_callback = NULL; 
    	receiver->dma_buffer_idx = 0;
    	
    	receiver->data_ready_callback = callback;
    	
    	__HAL_UART_ENABLE_IT(receiver->huart, UART_IT_IDLE);
		HAL_UART_Receive_DMA(receiver->huart, receiver->buffer_A,BUFFER_SIZE);
        connect_broker(huart);
        connect_SMS(huart);
	}
}

void Receiver_IRQHandler(GenericReceiver_t* receiver)
{
	uint16_t data_length = 0;
	uint8_t* buffer_to_process = NULL;
    
    if (__HAL_UART_GET_FLAG(receiver->huart, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(receiver->huart);

        HAL_UART_DMAStop(receiver->huart);

        if (receiver->dma_buffer_idx == 0)
        {
            data_length = BUFFER_SIZE - __HAL_DMA_GET_COUNTER(receiver->huart->hdmarx);
            buffer_to_process = receiver->buffer_A;
            receiver->dma_buffer_idx = 1;
            HAL_UART_Receive_DMA(receiver->huart, receiver->buffer_B, BUFFER_SIZE);
        }
        else 
        {
            
            data_length = BUFFER_SIZE - __HAL_DMA_GET_COUNTER(receiver->huart->hdmarx);
            buffer_to_process = receiver->buffer_B;
            receiver->dma_buffer_idx = 0;
            HAL_UART_Receive_DMA(receiver->huart, receiver->buffer_A, BUFFER_SIZE);
        }

        if (receiver->data_ready_callback != NULL && data_length > 0)
        {
			int count = tokenize_at_response(buffer_to_process, tokens, MAX_TOKENS);
			
       		for (int i = 0; i < count; i++)
       		{
       		size_t len = strlen(tokens[i]);
           
            if(!response_table[Parse_Response_Token(tokens[i])].handler(tokens[i])){
                receiver->data_ready_callback(tokens[i], len); //kiểm tra và lưu calib
                }
       		}
            //received_flag = true;
        }
    }
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
void connect_broker(UART_HandleTypeDef* huart){
    //1.
    HAL_UART_Transmit(huart, "AT+CMQTTSTART", strlen("AT+CMQTTSTART"), 1000);
    //2.
    sprintf(com ,"AT+CMQTTACCQ=%d,\"%s\",%d",client_index,clientID,server_type);
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    //3.
    sprintf(com ,"AT+CMQTTCONNECT=%d,\"%s\",%d,%d",client_index,clientID,keepalive_time,clean_session);
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout(&ouput_data.CMQTTCONNECT_data.state,0);
}

void Send_broker(UART_HandleTypeDef* huart,const char* payload )
{
    //-----------------------------------------------------------------------------------------------------
    //4.topic
    sprintf(com ,"AT+CMQTTTOPIC=%d,%d",client_index,req_length);
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout(huart,ouput_data.no_data.index,0);
    HAL_UART_Transmit(huart,topic,req_length, 1000);
    //5.payload
    sprintf(com ,"AT+CMQTTPAYLOAD=%d,%d",client_index,req_length);
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout(huart,ouput_data.no_data.index,0);
    HAL_UART_Transmit(huart,payload,strlen(payload), 1000);
    //6.publish
    sprintf(com ,"AT+CMQTTPUB=%d,%d,0",client_index,qos);
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout(huart,ouput_data.no_data.index,0);   
}

//=====================================send sms======================================================
void connect_SMS(UART_HandleTypeDef* huart){
    sprintf(com,"AT+CMGF=1");
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    sprintf(com,"AT+CSCS=\"GSM\"");
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout(huart,ouput_data.no_data.index,0); 
}
void Send_SMS(UART_HandleTypeDef* huart,char*phone){
    sprintf(com ,"AT+CMGS=\"%s\"",phone);
    HAL_UART_Transmit(huart,com, strlen(com), 1000);
    timeout(huart,ouput_data.no_data.index,0); 
}
//=======================================handler=================================================================
bool handle_Default(const char* token) {
	return true;
}
bool handle_No_data(const char* token) {
    if (strcmp(token,"<")==0 || strcmp(token,"OK")==0 || strcmp(token,"ERROR")==0){
    ouput_data.no_data.index = 0;
    return true;
    } 
    ouput_data.no_data.index = 1;
	return false;
}

bool handle_CMQTTDISC(const char* token) {
    sscanf(token, "+CMQTTDISC: %d,%d",&ouput_data.CMQTT_data.client_index,&ouput_data.CMQTT_data.disc_state);
	return true;
}
bool handle_CMQTTSTART(const char* token) {
    sscanf(token, "+CMQTTSTART: %d",&ouput_data.CMQTTSTART_data.index);
	return true;
}
bool handle_CMQTTCONNECT(const char* token) {
    sscanf(token, "+CMQTTCONNECT: %d,%d",&ouput_data.CMQTTCONNECT_data.index,&ouput_data.CMQTTCONNECT_data.state);
	return true;
}