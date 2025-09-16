#include "Rx_lib_SIM.h"
#include "setup_sim.h"
char com[50];
char buffer_response[BUFFER_SIZE];
char buffer_calib[BUFFER_SIZE];

bool received_flag = false;
int req_length =strlen(topic); 


//=====================================work_flow===========================================

//=====================================handle_data===========================================
void Receiver_IRQHandler(void)
{
	uint16_t data_length = 0;
	uint8_t* buffer_to_process = NULL;
    
    if (__HAL_UART_GET_FLAG(g_receiver.huart, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(g_receiver.huart);

        HAL_UART_DMAStop(g_receiver.huart);
        //SWITCH BUFFER A<->B
        /*
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
        */

        //DMA->BUFFER
        data_length = BUFFER_SIZE - __HAL_DMA_GET_COUNTER(g_receiver.huart->hdmarx); //ERROR
        HAL_UART_Receive_DMA(g_receiver.huart,g_receiver.buffer_A, BUFFER_SIZE);
        //check
        g_receiver.response_count = filter_data(g_receiver.buffer_A,g_receiver.result_data,BUFFER_SIZE);
        for (int i = 0; i <= g_receiver.resp_count; i++)
        {
            index_cmd[i] = Check_Type_Response(g_receiver.result_data[i]);
            ResponseType_e type = index_cmd[i];
            response_table[type].handler(g_receiver.result_data[i]);
        }
        clear();
        }
    }


int Filter_Data(uint8_t * input,uint8_t output_tokens[][BUFFER_SIZE], int max_tokens)
{
    //check NUll
    if (!input || !output_tokens) {
        return 0;
    }
    //
    int token_count = 0;
    const char* current_pos = input;

    while (*current_pos != '\0' && token_count < max_tokens){
        //Kiểm tra vị trí con trỏ
        while (*current_pos != '\0' && (*current_pos == '\r' || *current_pos == '\n')) {
            current_pos++;
        }
        if (*current_pos == '\0') {
            break;
        }
        //Tìm điểm kết thúc của dòng hiện tại (\r\n)
        const char* end_of_line = strstr(current_pos, "\r\n");
        const char* line_start = current_pos;
        size_t token_len = 0;
        if (end_of_line != NULL) { 
            token_len = end_of_line - line_start;
        } else {
            token_len = strlen(line_start);
        }
        if (token_len > 0) {
            size_t len_to_copy = (token_len < BUFFER_SIZE) ? token_len : BUFFER_SIZE - 1;
            
            memcpy(output_tokens[token_count], line_start, len_to_copy);
            output_tokens[token_count][len_to_copy] = '\0'; 
            
            token_count++;
            
        }
        if (end_of_line != NULL) {
            current_pos = end_of_line + 2;
        } else {
            // Đã đến cuối chuỗi
            break;
        }
    }

    return token_count;
}


ResponseType_e Check_Type_Response(const char* token)
{
    // Lấy số lượng mục trong bảng tra cứu
    int num_mappings = sizeof(response_table) / sizeof(response_table[0]);
    for (int i = 0; i < num_mappings; i++)
    {
        const char* expected_str = response_table[i].str_to_compare;

        if (response_table[i].cmp_type == CMP_EXACT)
        {
            // So sánh chính xác
            if (strcmp(token, expected_str) == 0) {
                
                return response_table[i].type; 
            }
        }
        else 
        {
            // So sánh tiền tố
            if (strncmp(token, expected_str, strlen(expected_str)) == 0) {
                return response_table[i].type; 
            }
        }
    }
    return RESPONSE_TYPE_UNKNOWN;
}

  void clear(void){
    memset(g_receiver.buffer_A, 0, sizeof(g_receiver.buffer_A));
    memset(g_receiver.result_data, 0, sizeof(g_receiver.result_data));
  }
/*===================================================================================*/
 
void SysTick_Init(volatile uint32_t set_time) {
    g_receiver.tickCount = 0;
    SysTick->LOAD = 72000 - 1; // 72 MHz / 1000 = 72,000 cycles cho 1ms
    SysTick->VAL = 0;          // Reset counter
    g_receiver.timeset_count = set_time;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // HCLK
                    SysTick_CTRL_TICKINT_Msk |   // Bật ngắt
                    SysTick_CTRL_ENABLE_Msk;     // Bật SysTick
    
}

 void SysTick_Handler(void) {
    g_receiver.tickCount++;
    if (g_receiver.response_count != 0 ) { 
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    } else if (g_receiver.tickCount >= g_receiver.timeset_count ){
        
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    
    }
}
void Wait_Response( ResponseType_e *response_correct, volatile uint32_t set_time, UART_HandleTypeDef* huart){
    SysTick_Init(set_time);
    if (g_receiver.response_count !=0){ //response not correct
        for(int i = 0; i < g_receiver.response_count;i++){
            if(g_receiver.index_cmd[i] == response_correct ){
                return;
            } else if( i == (g_receiver.response_count-1)){
                g_state = g_state -  1;  
            }
        }
    } else { //Timeout
        Lib_SIM_Setup(huart);

    }
}
 void Error_Connect(void){

 }










bool handle_Default(const char* token) {
	return true;
}
bool handle_No_data(const char* token) {
    if (strcmp(token,"> ")==0 || strcmp(token,"OK")==0 || strcmp(token,"ERROR")==0){
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