#ifndef VCP_COMM_H
#define VCP_COMM_H

#define VCP_DRV_BUFF_LEN 1024
#define VCP_DRV_SEND_MAX_WAIT 1000


void VCPTransmitTask(void * arg);
int32_t vcpSend(const char * buf, uint16_t len);


#endif
