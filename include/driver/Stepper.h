/*
 * Stepper.h
 *
 *  Created on: 19 дек. 2016 г.
 *      Author: Voodoo
 */

#ifndef INCLUDE_DRIVER_STEPPER_H_
#define INCLUDE_DRIVER_STEPPER_H_

#define STEP_CNT 500
typedef union __packed
{
	uint8 byte[6];
	struct __packed
	{
		uint8 enabled;
		uint8 direction;
		uint8 stepperWord;
		uint16 stepCounter;
	};
}u_STEPPER;

void stepperGo(void);
void ICACHE_FLASH_ATTR stepperTimerStop(void);

#endif /* INCLUDE_DRIVER_STEPPER_H_ */
