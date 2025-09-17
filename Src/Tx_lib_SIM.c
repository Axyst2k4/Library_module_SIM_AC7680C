#include "Tx_lib_SIM.h"

void Tx_data(){
    while (1){
    switch (state){
        case 
    }
}
}
void Send_broker(UART_HandleTypeDef* huart, const char* comman) {
    HAL_UART_Transmit(huart, comman, strlen(comman), 1000);
    timeout(huart, &ouput_data.no_data.index, 0);
	}