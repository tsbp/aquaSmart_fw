#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "at_custom.h"
#include "mem.h"
#include "driver/UDP_Source.h"
#include "driver/DS18B20_PR.h"
#include "driver/services.h"
#include "driver/configs.h"
//=========================================================================================
//extern u_CONFIG configs;
struct espconn *UDP_PC;


uint8 channelFree = 1;

sint16 plotData[2][24];
//============================================================================================================================
void ICACHE_FLASH_ATTR UDP_Init_client()
{

	UDP_PC = (struct espconn *) os_zalloc(
			sizeof(struct espconn));
	UDP_PC->proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));
	UDP_PC->state = ESPCONN_NONE;
	UDP_PC->type = ESPCONN_UDP;

	UDP_PC->proto.udp->local_port = 7373; //The port on which we want the esp to serve
	UDP_PC->proto.udp->remote_port = 7373; //The port on which we want the esp to serve

	//Set The call back functions
	espconn_regist_recvcb(UDP_PC, UDP_Recieved);
	espconn_create(UDP_PC);
}
//============================================================================================================================
void ICACHE_FLASH_ATTR addValueToArray(char * tPtr, sint16 *arPtr, char aRot)
{
	int i,j;
	if(aRot == ROTATE)
	{
		for(i = 0; i < POINTS_CNT-1; i++)
			arPtr[i]= arPtr[i+1];
	}

    sint16 e = (tPtr[1]- '0') * 100 + (tPtr[2]- '0') * 10 + (tPtr[3] - '0');

    if(tPtr[0] == '-')  e *= (-1);

		arPtr[POINTS_CNT-1] = e;
}
//=========================================================================================
uint8 crcCalc(uint8 *aBuf, uint8 aLng)
    {
        uint16 sum = 0, i;
        for(i = 0; i < aLng; i++)
            sum += aBuf[i];
        return  (((sum >> 8) & 0xff) + (sum & 0xff));
    }
//=========================================================================================
void UDP_Recieved(void *arg, char *pusrdata, unsigned short length)
{
	int a, i;
	ets_uart_printf("recv udp data: ");
	for (a = 0; a < length; a++)
		ets_uart_printf("%02x ", pusrdata[a]);
	ets_uart_printf("\r\n");

	struct espconn *pesp_conn = arg;
	flashWriteBit = 0;
	remot_info *premot = NULL;



	uint8 crc = crcCalc(pusrdata, pusrdata[0] + 1);
	//ets_uart_printf("crc = %02x, in_crc = %02x\r\n", crc, pusrdata[length - 1]);
	if (espconn_get_connection_info(pesp_conn, &premot, 0) == ESPCONN_OK &&
			crc == pusrdata[length - 1])
	{
		pesp_conn->proto.udp->remote_port = 7373;
		pesp_conn->proto.udp->remote_ip[0] = premot->remote_ip[0];
		pesp_conn->proto.udp->remote_ip[1] = premot->remote_ip[1];
		pesp_conn->proto.udp->remote_ip[2] = premot->remote_ip[2];
		pesp_conn->proto.udp->remote_ip[3] = premot->remote_ip[3];

		switch(pusrdata[1])
		{
			case CMD_GET_STATE:
				{
					uint8 cBuf[8];
					cBuf[0] = (uint8)(sizeof(cBuf) - 1);
					cBuf[1] = (uint8)(CMD_GET_STATE_ANS);
					cBuf[2] = time.hour;
					cBuf[3] = time.min;
					cBuf[4] = time.sec;
					cBuf[5] = (uint8)(currentTemperature[0] & 0xff);
					cBuf[6] = (uint8)(currentTemperature[0] >> 8);
					cBuf[sizeof(cBuf) - 1] = crcCalc(cBuf, sizeof(cBuf) - 1);
	//				ets_uart_printf("CMD_GET_STATE_ANS: ");
	//				for (a = 0; a < sizeof(cBuf); a++)
	//						ets_uart_printf("%02x ", cBuf[a]);
	//					ets_uart_printf("\r\n");
					espconn_sent(pesp_conn, cBuf, sizeof(cBuf));
				}
				break;

			case CMD_GET_CFG:
				{
					uint8 cnt = sizeof(u_CONFIG) - sizeof(s_WIFI_CFG) + 2;
					uint8 cBuf[cnt];
					cBuf[0] = cnt - 1;
					cBuf[1] =  CMD_GET_CFG_ANS;
					for(i = 0; i < (sizeof(cBuf) - 2); i++)
						cBuf[i + 2] = configs.byte[i];
					cBuf[sizeof(cBuf) - 1] = crcCalc(cBuf, sizeof(cBuf) - 1);

									ets_uart_printf("CMD_GET_SGF_ANS: ");
										for (a = 0; a < sizeof(cBuf); a++)
												ets_uart_printf("%02x ", cBuf[a]);
											ets_uart_printf("\r\n");

					espconn_sent(pesp_conn, cBuf, sizeof(cBuf));
				}
				break;

			case CMD_SET_CFG:
				{

					for(i = 0; i < (sizeof(u_CONFIG) - sizeof(s_WIFI_CFG)); i++ )
						configs.byte[i] = pusrdata[i + 2];

					ets_uart_printf("temperature: %d\r\n", configs.temperature);

					for(i = 0; i < 2; i++)
						ets_uart_printf("light[%d]: %d:%d  %d\r\n",
								i,
								configs.light[i].hour,
								configs.light[i].minute,
								configs.light[i].light);
					for(i = 0; i < 3; i++)
						ets_uart_printf("periph[%d]: %d:%d  %d:%d\r\n",
								i,
								configs.periph[i].hStart, configs.periph[i].mStart,
								configs.periph[i].hStop,  configs.periph[i].mStop);
					flashWriteBit = 1;
				}
				break;
//
//			case CONF_GET:
//				break;
//
//			case CONF_SET:
//				break;
//
//			case WIFI_GET:
//				break;
//
//			case WIFI_SET:
//				break;


		}

		//if (flashWriteBit == 1) saveConfigs();
	}
}



