//============================================================================================================================
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "driver/DS18B20_PR.h"
#include "driver/Configs.h"
#include "driver/UDP_Source.h"
#include "driver/gpio16.h"
#include "driver/services.h"
#include "driver/stepper.h"
//============================================================================================================================
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

#define LOOP_PERIOD		(1000) // in msec
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

static volatile os_timer_t loop_timer;
static void  loop(os_event_t *events);


#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_0_OUT_IO_NUM 12
#define PWM_0_OUT_IO_FUNC FUNC_GPIO12

#define DEFAULT_PWM_DUTY_STEP	(80)

uint32 io_info[][3] = {   {PWM_0_OUT_IO_MUX,PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM},
       	                  //{PWM_1_OUT_IO_MUX,PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM},
               	          //{PWM_2_OUT_IO_MUX,PWM_2_OUT_IO_FUNC,PWM_2_OUT_IO_NUM},
			};
uint32 duty[1] = {DEFAULT_PWM_DUTY_STEP*(100 - 10)};

//======================= Main code function ============================================================
void ICACHE_FLASH_ATTR loop(os_event_t *events)
{
	timeIncrement();

	if(!timeTrue &&
			configs.wifi.mode == STATION_MODE &&
			wifi_station_get_connect_status() == STATION_GOT_IP)
		timeTrue = timeSync();

	if (flashWriteBit == 1) saveConfigs();

	configsProcced();


}
//==============================================================================
void ICACHE_FLASH_ATTR setup(void)
{
	set_gpio_mode(1,GPIO_PULLUP, GPIO_OUTPUT);
	button_init();
	
	pwm_init(10000, duty, 1, io_info);
    pwm_start();


	UDP_Init_client();
	ds18b20_init();

	// Start loop timer
	os_timer_disarm(&loop_timer);
	os_timer_setfn(&loop_timer, (os_timer_func_t *) loop, NULL);
	os_timer_arm(&loop_timer, LOOP_PERIOD, true);

	stepperTimerStop();
}
//========================== Init function  =============================================================
//
void ICACHE_FLASH_ATTR user_init(void)
{

	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);
	ets_uart_printf("System init...\r\n");

	//saveConfigs();
	readConfigs();
	if(configs.wifi.mode == STATION_MODE) sntp_initialize();

	wifi_station_disconnect();
	wifi_station_set_auto_connect(0);
	//system_update_cpu_freq(160);

	hspi_init();
	hspi_send_uint8(0);
	hspi_wait_ready();

	ets_uart_printf("configs.wifi.SSID %s\r\n", configs.wifi.SSID);
	ets_uart_printf("configs.wifi.SSID_PASS %s\r\n", configs.wifi.SSID_PASS);

	if(configs.wifi.mode == STATION_MODE) setup_wifi_st_mode();
	else if(configs.wifi.mode == SOFTAP_MODE)
		setup_wifi_ap_mode();

	// Start setup timer
	os_timer_disarm(&loop_timer);
	os_timer_setfn(&loop_timer, (os_timer_func_t *) setup, NULL);
	os_timer_arm(&loop_timer, LOOP_PERIOD * 2, false);
}
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}
