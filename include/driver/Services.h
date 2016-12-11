/*
 * Services.h
 *
 *  Created on: 16 трав. 2016
 *      Author: Voodoo
 */

#ifndef INCLUDE_DRIVER_SERVICES_H_
#define INCLUDE_DRIVER_SERVICES_H_
//==============================================================================

void ICACHE_FLASH_ATTR service_timer_start (void);
void ICACHE_FLASH_ATTR service_timer_stop (void);
void ICACHE_FLASH_ATTR button_intr_callback(unsigned pin, unsigned level);
void ICACHE_FLASH_ATTR button_init(void);
void ICACHE_FLASH_ATTR getTemperature(void);
//==============================================================================
#define MODE_NORMAL		(0)
#define MODE_SW_RESET	(1)
#define MODE_BTN_RESET	(2)
#define MODE_REMOTE_CONTROL	(3)
#define MODE_FLASH_WRITE	(4)
//==============================================================================
extern uint8	serviceMode;
extern sint16 currentTemperature[2];
extern int cntr;
//==============================================================================
typedef struct __packed
{
  union
  {
    uint8 byte[3];
    struct
    {
    	uint8 sec;
    	uint8 min;
    	uint8 hour;
    };
  };
}s_DATE_TIME;
extern s_DATE_TIME time;
//==============================================================================
#endif /* INCLUDE_DRIVER_SERVICES_H_ */
