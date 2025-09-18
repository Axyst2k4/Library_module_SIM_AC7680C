

#include "stm32f1xx_hal.h"  
#include "stdio.h"
#include <string.h>

// ==================================================GLOBAL===================================================================
void Lib_SIM_Setup(GenericReceiver_t* receiver, UART_HandleTypeDef* huart, Receiver_Callback_t callback)
void Receiver_IRQHandler(GenericReceiver_t* receiver); 
void Send_data_broker(UART_HandleTypeDef* huart,  const char* topic,const char* payload);
void Send_message_SMS(UART_HandleTypeDef* huart, char* phone, const char* message);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
// ==================================================DEFINE===================================================================


#define MY_UART_BAUDRATE    115200
#define BUFFER_SIZE      	256
#define MAX_TOKENS			100
#define pub_timeout 60
#define client_index 0













int g_tx_complete =0;
// ==================================================RRECEIVER===================================================================

typedef enum {
    STATE_IDLE,                   
	STATE_ERROR,
    //connect mqtt
    STATE_MQTT_1,       
	RESPONSE_MQTT_1,
    STATE_MQTT_2,        
	RESPONSE_MQTT_2,  
	STATE_MQTT_3,
	RESPONSE_MQTT_3,
	DONE_CONNECT_MQTT,
    // connect sms
    STATE_SMS_1,       
	RESPONSE_SMS_1,
    STATE_SMS_2,        
	RESPONSE_SMS_2,  
    DONE_CONNECT_SMS,
	//send broker
    STATE_SENDBROKER_1,       
	RESPONSE_SENDBROKER_1,
    STATE_SENDBROKER_2,
    RESPONSE_SENDBROKER_2, 
	STATE_SENDBROKER_3,  
    RESPONSE_SENDBROKER_3,
    STATE_SENDBROKER_4,  
    RESPONSE_SENDBROKER_4,
    STATE_SENDBROKER_5,  
    RESPONSE_SENDBROKER_5,
    DONE_SENDBROKER,
	// send sms
	STATE_SENDSMS_1,
	RESPONSE_SENDSMS_1,
    STATE_SENDSMS_2,
	RESPONSE_SENDSMS_2,
	STATE_SENDSMS_3,
	DONE_SENDSMS,
	
	// setup
	STATE_INIT_UART,
	STATE_CONNECT_MQTT,
	STATE_CONNECT_SMS,
	DONE_SETUP,

} SimMachineState_t;

// Khai báo biến trạng thái
SimMachineState_t g_state = STATE_IDLE;
SimMachineState_t g_state_setup = STATE_IDLE;
SimMachineState_t g_state_send_broker = STATE_IDLE;
SimMachineState_t g_state_send_SMS = STATE_IDLE;
//
typedef struct{
    int status;
    char command;
} match_table


typedef void (*Receiver_Callback_t)(uint8_t*data, uint8_t len); //dinh nghia con tro ham


// Enum định nghĩa các loại phản hồi
typedef enum {
    RESPONSE_TYPE_UNKNOWN   = 0,
    RESPONSE_TYPE_OK        = 1,
    RESPONSE_TYPE_ERROR     = 2,
    RESPONSE_TYPE_PROMPT    = 3,

    // Phản hồi mạng & trạng thái chung
    RESPONSE_TYPE_CSQ       = 4,
    RESPONSE_TYPE_CREG      = 5,

    // Phản hồi lỗi SMS
    RESPONSE_TYPE_CMS_ERROR = 6,       // Lỗi cụ thể của SMS

    // URC - Tin nhắn đến
    RESPONSE_TYPE_CMTI      = 7,       // Thông báo vị trí tin nhắn mới
    RESPONSE_TYPE_CMT       = 8,       // Thông báo nội dung tin nhắn mới

    // Phản hồi lệnh SMS
    RESPONSE_TYPE_CSMS      = 9,       // AT+CSMS
    RESPONSE_TYPE_CPMS      = 10,      // AT+CPMS
    RESPONSE_TYPE_CMGF      = 11,      // AT+CMGF
    RESPONSE_TYPE_CSCA      = 12,      // AT+CSCA
    RESPONSE_TYPE_CSCB      = 13,      // AT+CSCB
    RESPONSE_TYPE_CSMP      = 14,      // AT+CSMP
    RESPONSE_TYPE_CSDH      = 15,      // AT+CSDH
    RESPONSE_TYPE_CNMI      = 16,      // AT+CNMI
    RESPONSE_TYPE_CGSMS     = 17,      // AT+CGSMS
    RESPONSE_TYPE_CMGL      = 18,      // AT+CMGL
    RESPONSE_TYPE_CMGR      = 19,      // AT+CMGR
    RESPONSE_TYPE_CMGS      = 20,      // AT+CMGS
    RESPONSE_TYPE_CMSS      = 21,      // AT+CMSS
    RESPONSE_TYPE_CMGW      = 22,      // AT+CMGW
    RESPONSE_TYPE_CMGRD     = 23,      // AT+CMGRD
    RESPONSE_TYPE_CMGSEX    = 24,      // AT+CMGSEX
    RESPONSE_TYPE_CMSSEX    = 25 ,      // AT+CMSSEX
	// phản hồi lệnh broker
    RESPONSE_TYPE_CMQTTDISC = 26,       //AT+CMQTTDISC
    RESPONSE_TYPE_CMQTTSTART = 27 ,     //AT+CMQTTSTART
    RESPONSE_TYPE_CMQTTCONNECT = 28 ,    //AT+CMQTTCONNECT
    RESPONSE_TYPE_CMQTTPUB,
} ResponseType_e;

typedef enum {
    CMP_EXACT,
    CMP_PREFIX
} CmpType_e;

typedef void (*ResponseHandler_t)(const char* token);

typedef struct {
    const char* str_to_compare; 
    ResponseType_e    type;
    CmpType_e         cmp_type;
    ResponseHandler_t handler
} ResponseMapping_t;

static const ResponseMapping_t response_table[] = {
    // String,         Enum Type,                 Cmp Type,   Handler Function
    {"OK",          RESPONSE_TYPE_OK,          CMP_EXACT,  handle_No_data},
    {"ERROR",       RESPONSE_TYPE_ERROR,       CMP_EXACT,  handle_No_data},
    {"> ",          RESPONSE_TYPE_PROMPT,      CMP_EXACT,  handle_No_data},
    {"+CSQ:",       RESPONSE_TYPE_CSQ,         CMP_PREFIX, handle_Default},
    {"+CREG:",      RESPONSE_TYPE_CREG,        CMP_PREFIX, handle_Default},
    {"+CMS ERROR:", RESPONSE_TYPE_CMS_ERROR,   CMP_PREFIX, handle_Default},
    {"+CMTI:",      RESPONSE_TYPE_CMTI,        CMP_PREFIX, handle_Default},
    {"+CMT:",       RESPONSE_TYPE_CMT,         CMP_PREFIX, handle_Default},
    {"+CSMS:",      RESPONSE_TYPE_CSMS,        CMP_PREFIX, handle_Default},
    {"+CPMS:",      RESPONSE_TYPE_CPMS,        CMP_PREFIX, handle_Default},
    {"+CMGF:",      RESPONSE_TYPE_CMGF,        CMP_PREFIX, handle_Default},
    {"+CSCA:",      RESPONSE_TYPE_CSCA,        CMP_PREFIX, handle_Default},
    {"+CSCB:",      RESPONSE_TYPE_CSCB,        CMP_PREFIX, handle_Default},
    {"+CSMP:",      RESPONSE_TYPE_CSMP,        CMP_PREFIX, handle_Default},
    {"+CSDH:",      RESPONSE_TYPE_CSDH,        CMP_PREFIX, handle_Default},
    {"+CNMI:",      RESPONSE_TYPE_CNMI,        CMP_PREFIX, handle_Default},
    {"+CGSMS:",     RESPONSE_TYPE_CGSMS,       CMP_PREFIX, handle_Default},
    {"+CMGL:",      RESPONSE_TYPE_CMGL,        CMP_PREFIX, handle_Default},
    {"+CMGR:",      RESPONSE_TYPE_CMGR,        CMP_PREFIX, handle_Default},
    {"+CMGS:",      RESPONSE_TYPE_CMGS,        CMP_PREFIX, handle_Default},
    {"+CMSS:",      RESPONSE_TYPE_CMSS,        CMP_PREFIX, handle_Default},
    {"+CMGW:",      RESPONSE_TYPE_CMGW,        CMP_PREFIX, handle_Default},
    {"+CMGRD:",     RESPONSE_TYPE_CMGRD,       CMP_PREFIX, handle_Default},
    {"+CMGSEX:",    RESPONSE_TYPE_CMGSEX,      CMP_PREFIX, handle_Default},
    {"+CMSSEX:",    RESPONSE_TYPE_CMSSEX,      CMP_PREFIX, handle_Default},
    //broker
    {"+CMQTTDISC:",    RESPONSE_TYPE_CMQTTDISC,      CMP_PREFIX, handle_CMQTTDISC},
    {"+CMQTTSTART:",    RESPONSE_TYPE_CMQTTSTART,      CMP_PREFIX, handle_CMQTTSTART},
    {"+CMQTTCONNECT:",    RESPONSE_TYPE_CMQTTCONNECT,      CMP_PREFIX, handle_CMQTTCONNECT},
    {"+CMQTTPUB:",    RESPONSE_TYPE_CMQTTPUB,      CMP_PREFIX, handle_No_data}
};

//response_handler_table[response_type](current_token); cách dùng trong maim

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