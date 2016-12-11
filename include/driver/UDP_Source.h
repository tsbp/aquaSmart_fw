
//================== Commands ====================================================
#define OK_ANS							(0xAA)
#define BAD_ANS							(0xEE)

#define CMD_GET_STATE				(0x10)
#define CMD_GET_STATE_ANS			(0x11)




void ICACHE_FLASH_ATTR UDP_Init();
void ICACHE_FLASH_ATTR UDP_Init_client();
void UDP_Recieved(void *arg, char *pusrdata, unsigned short length);
void   mergeAnswerWith(char tPtr[2][24][4]);
void ICACHE_FLASH_ATTR sendUDPbroadcast(uint8* abuf, uint16 aLen);
void ICACHE_FLASH_ATTR addValueToArray(char * tPtr, sint16 *arPtr, char aRot);


extern uint8 channelFree;

