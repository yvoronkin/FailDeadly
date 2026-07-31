#ifndef FD_LOG_H
#define FD_LOG_H

#define LOG_BUFFER_LEN  64
#define LOG_SEND_MAX_WAIT 100

void FDLogTaskInit(void);
int32_t FDSendLog(const unsigned char * buf, size_t len, int omit_rtos, int noblock);
void FDLogFreeRTOSOnceTask(void);

#endif
