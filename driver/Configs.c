//=============================================================================
#include <ets_sys.h>
#include "osapi.h"
#include "c_types.h"
#include "gpio.h"
#include "os_type.h"
#include <mem.h>
#include "driver/wifi.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/Configs.h"
#include "driver/services.h"
//==============================================================================
uint8 flashWriteBit;
uint8 periphWord = 0x00;
uint8 timeTrue = 0;
//================== SNTP ==================================================
void sntp_initialize(void)
{
	ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
	sntp_setservername(0, "us.pool.ntp.org"); // set server 0 by domain name
	sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 1 by domain name
	ipaddr_aton("210.72.145.44", addr);
	sntp_setserver(2, addr); // set server 2 by IP address
	sntp_set_timezone(+2);
	sntp_init();
	os_free(addr);
}
//================== SNTP ==================================================
uint8 timeSync(void)
{
	uint32 ts = sntp_get_current_timestamp();
			uint8 t[24];
			os_memset(t, 0, sizeof(t));
			os_sprintf(t, "%s", sntp_get_real_time(ts));
	//		int i;
	//		for(i =0; i < sizeof(t); i++)
	//			ets_uart_printf("t[%d] = %c\r\n", i, t[i]);


			ets_uart_printf("time %s \r\n", t);
			if(ts != 0)
			{
				time.sec  = (t[17]-'0')*10 + (t[18]-'0');
				time.min  = (t[14]-'0')*10 + (t[15]-'0');
				time.hour = (t[11]-'0')*10 + (t[12]-'0');

				sntp_stop();
				return 1;
			}
			return 0;
}
//=============================================================================
u_CONFIG configs = {
	    .wifi.mode = STATION_MODE,
		.wifi.SSID = "voodoo",
        .wifi.SSID_PASS = "eminem82"
};
//=============================================================================
uint16 getMinutes(uint8 aHour, uint8 aMinute)
{
	return aHour * 60 + aMinute;
}
//=============================================================================
void checkConfigs(void)
{
	int i;
	for(i = 0; i < 3; i++)
	{
		if(configs.periph[i].hStart > 23) configs.periph[i].hStart = 0;
		if(configs.periph[i].hStop  > 23) configs.periph[i].hStop = 0;

		if(configs.periph[i].mStart > 59) configs.periph[i].mStart = 0;
		if(configs.periph[i].mStop  > 59) configs.periph[i].mStop = 0;

	}

	for(i = 0; i < 2; i++)
	{
		if (configs.light[i].hour   > 23) configs.light[i].hour = 0;
		if (configs.light[i].minute > 59) configs.light[i].minute = 0;
	}
}
//=============================================================================
#define DELTA	(2)
uint8 currentLight = 0;
uint8 day_night = 0;
uint16 eatSecCounter;
//=============================================================================
void configsProcced(void)
{
	int i = 0;
	uint16 a, b, c;

	c = getMinutes(time.hour, time.min);
	//============ light ===========================
//	ets_uart_printf("%d. mStart = %d, mEnd = %d, mCur %d\r\n", i, a, b, c);
	a = getMinutes(configs.light[0].hour, configs.light[0].minute);
	b = getMinutes(configs.light[1].hour, configs.light[1].minute);
	if(eatSecCounter)
	{
		eatSecCounter--;
		currentLight = configs.light[0].light;
	}
	else if( a != b)
	{
		if(a < b)
		{
			if(c >= a && c < b)  {currentLight = configs.light[0].light; day_night = 1;}
			else 				 {currentLight = configs.light[1].light; day_night = 0;}
		}
		else
		{
			if (c >= a || c < b) {currentLight = configs.light[0].light; day_night = 1;}
			else				 {currentLight = configs.light[1].light; day_night = 0;}
		}
	}
	pwm_set_duty(currentLight * 81, 0);
	pwm_start();
	//============ peripherial =====================
	//periphWord = (periphWord & 40);
	for(i = 0; i < 2; i++)
	{
		a = getMinutes(configs.periph[i].hStart, configs.periph[i].mStart);
		b = getMinutes(configs.periph[i].hStop,  configs.periph[i].mStop);
		if( a != b)
		{
			if(a < b)
			{
				if((c >= a && c < b) && !eatSecCounter)  periphWord |=  (1 << (i + 6));
				else 				                     periphWord &= ~(1 << (i + 6));
			}
			else
			{
				if ((c >= a || c < b) && !eatSecCounter) periphWord |=  (1 << (i + 6));
				else				                     periphWord &= ~(1 << (i + 6));
			}
		}
	}
	//============= temperature ====================
	getTemperature();
	if(currentTemperature[0] <= (configs.temperature - DELTA))      periphWord |=  (1 << 5);
	else if(currentTemperature[0] >= (configs.temperature + DELTA)) periphWord &= ~(1 << 5);
	ets_uart_printf("temp = %d\r\n", currentTemperature[0]);
	//================= stepper ====================
	if((((time.hour * 60 + time.min) == (configs.periph[2].hStart * 60 + configs.periph[2].mStart))||
		   ((time.hour * 60 + time.min) == (configs.periph[2].hStop  * 60 + configs.periph[2].mStop)))
			&& time.sec == 0)
	{
		stepperGo();
		eatSecCounter = configs.eatMinutes * 60;
	}
	//=================
	hspi_send_uint8(periphWord);
	hspi_wait_ready();
}
//=============================================================================
void ICACHE_FLASH_ATTR saveConfigs(void) {
    flashWriteBit = 0;
	int result = -1;
	os_delay_us(100000);
	result = spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
	result = -1;
	os_delay_us(100000);
	result = spi_flash_write(
			(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
			(uint32 *) &configs, sizeof(u_CONFIG));

	ets_uart_printf("Write W = %d\r\n", result);
}

//=============================================================================
void ICACHE_FLASH_ATTR readConfigs(void) {
	int result = -1;
	result = spi_flash_read(
			(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
			(uint32 *) &configs, sizeof(u_CONFIG));
}
//==============================================================================


