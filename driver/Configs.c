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
		.cfg[0].periodsCnt = 1,
		.cfg[0].pConfig[0].hStart = 0,
		.cfg[0].pConfig[0].mStart = 0,
		.cfg[0].pConfig[0].temperature = 250,

		.cfg[1].periodsCnt = 1,
		.cfg[1].pConfig[0].hStart = 0,
		.cfg[1].pConfig[0].mStart = 0,
		.cfg[1].pConfig[0].temperature = 230,

	    .hwSettings.wifi.mode = STATION_MODE,
		.hwSettings.wifi.SSID = "voodoo",
        .hwSettings.wifi.SSID_PASS = "eminem82"};
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


