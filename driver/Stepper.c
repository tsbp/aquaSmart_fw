/*
 * Stepper.c
 *
 *  Created on: 19 дек. 2016 г.
 *      Author: Voodoo
 */
//=====================================================================
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "driver/stepper.h"
#include "driver/Configs.h"
//============= stepper motor code function ===========================
#define SMOTOR_PERIOD		(4) // in msec
static volatile os_timer_t sMotor_timer;
static void  sMotor_event(os_event_t *events);
const uint8 step_fw[] = {0x12, 0x06, 0x0c, 0x18};
const uint8 step_bw[] = {0x18, 0x0c, 0x06, 0x12};
u_STEPPER stepper = {.stepCounter = STEP_CNT};
//=====================================================================
void ICACHE_FLASH_ATTR stepperTimerStop(void)
{
	os_timer_disarm(&sMotor_timer);
}
//=====================================================================
void ICACHE_FLASH_ATTR stepperTimerStart(void)
{
	os_timer_setfn(&sMotor_timer, (os_timer_func_t *) sMotor_event, NULL);
	os_timer_arm(&sMotor_timer, SMOTOR_PERIOD, true);
}
//============= stepper motor code function ===========================
void stepperGo(void)
{
	if(!stepper.enabled)
	{
		stepper.enabled = 1;
		stepper.direction = 0;
		stepper.stepCounter = STEP_CNT;
		stepperTimerStart();
	}
}
//============= stepper motor code function ===========================
void ICACHE_FLASH_ATTR sMotor_event(os_event_t *events)
{
	static uint8 subStepCntr = 0;

	subStepCntr++;

	if(subStepCntr >= sizeof(step_fw))
	{
		subStepCntr = 0;
		if(stepper.stepCounter) stepper.stepCounter--;
	}

	if(stepper.stepCounter)
	{
		if(stepper.direction) stepper.stepperWord = step_fw[subStepCntr];
		else 		  		  stepper.stepperWord = step_bw[subStepCntr];
	}
	else
	{
		stepper.stepperWord = 0;
		if(!stepper.direction)
		{
			stepper.direction = 1;
			stepper.stepCounter = STEP_CNT;
		}
		else
		{
			stepper.enabled = 0;
			stepperTimerStop();
		}
	}

	hspi_send_uint8(periphWord | stepper.stepperWord);
	hspi_wait_ready();
}


