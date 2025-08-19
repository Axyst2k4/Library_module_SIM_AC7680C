#include "Rx_lib_SIM"

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