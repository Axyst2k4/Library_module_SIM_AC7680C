#ifndef _RX_LIB_SIM_H_
#define _RX_LIB_SIM_H_

#include "stm32f1xx_hal.h"
#include "string.h"  
// ==================================================RRECEIVER===================================================================
//
#define BUFFER_SIZE 256
void Wait_Response( ResponseType_e *response_correct, volatile uint32_t set_time);
typedef struct{
    int status;
    char command;
} match_table


typedef void (*Receiver_Callback_t)(uint8_t*data, uint8_t len); //dinh nghia con tro ham
typedef struct{
	UART_HandleTypeDef* huart;
	uint8_t buffer_A[BUFFER_SIZE];
	uint8_t buffer_B[BUFFER_SIZE];
	uint8_t dma_buffer_idx; //0=A,1=B
    uint8_t result_data[BUFFER_SIZE][BUFFER_SIZE];
    int response_count;
    ResponseType_e index_cmd[100]; 
    volatile uint32_t tickCount;
    volatile uint32_t timeset_count;
    
} GenericReceiver_t;

GenericReceiver_t g_receiver = {
    .huart = &huart1,       // Gán handle của UART1
    .dma_buffer_idx = 0,
    .response_count = 0,
    .tickCount =0,
    .timeset_count = 1000,
    
};
// Enum định nghĩa các loại phản hồi
typedef enum {
    RESPONSE_TYPE_UNKNOWN   ,
    RESPONSE_TYPE_OK        ,
    RESPONSE_TYPE_ERROR     ,
    RESPONSE_TYPE_PROMPT    ,

    // Phản hồi mạng & trạng thái chung
    RESPONSE_TYPE_CSQ      ,
    RESPONSE_TYPE_CREG      ,

    // Phản hồi lỗi SMS
    RESPONSE_TYPE_CMS_ERROR ,       // Lỗi cụ thể của SMS

    // URC - Tin nhắn đến
    RESPONSE_TYPE_CMTI      ,       // Thông báo vị trí tin nhắn mới
    RESPONSE_TYPE_CMT       ,       // Thông báo nội dung tin nhắn mới

    
	// phản hồi lệnh broker
    RESPONSE_TYPE_CMQTTDISC  ,     //AT+CMQTTDISC
    RESPONSE_TYPE_CMQTTSTART  ,   //AT+CMQTTSTART
    RESPONSE_TYPE_CMQTTCONNECT     //AT+CMQTTCONNECT
} ResponseType_e;



typedef void (*ResponseHandler_t)(const char* token);
typedef struct {
    const char* str_to_compare; // Chuỗi cần so sánh
    ResponseType_e    type;
    ResponseHandler_t handler
} ResponseMapping_t;


static const ResponseMapping_t response_table[] = {
    // String,         Enum Type,              Handler Function
    {"OK",          RESPONSE_TYPE_OK,            handle_No_data},
    {"ERROR",       RESPONSE_TYPE_ERROR,         handle_No_data},
    {"> ",          RESPONSE_TYPE_PROMPT,        handle_No_data},
    //broker
    {"+CMQTTDISC:",    RESPONSE_TYPE_CMQTTDISC,      handle_CMQTTDISC},
    {"+CMQTTSTART:",    RESPONSE_TYPE_CMQTTSTART,       handle_CMQTTSTART},
    {"+CMQTTCONNECT:",    RESPONSE_TYPE_CMQTTCONNECT,       handle_CMQTTCONNECT}
};



//========================================định nghĩa cho các hàm handler============================================
typedef struct {
    int i;
} Default_Data_t;

typedef struct {
    int  index ;
} No_Data_t;

typedef struct {
    int client_index;
    int disc_state ;
} CMQTTDISC_t;

typedef struct {
    int  index;
} CMQTTSTARTData_t;

typedef struct {
    int  index;
    int  state;
} CMQTTCONNECTData_t;
typedef struct {
    ResponseType_e type; // loại dữ liệu bên trong union

    union {
        DefaultData_t default_data;
        No_Data_t no_data;
        CMQTTDISC_t CMQTT_data;
        CMQTTSTARTData_t CMQTTSTART_data;
        CMQTTCONNECTData_t CMQTTCONNECT_data;
    } params;

} Mag_ResponseData_t;
typedef void (*GenericEventHandler_t)(const Mag_ResponseData_t* response);
Mag_ResponseData_t ouput_data;

#endif 