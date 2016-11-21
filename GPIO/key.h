#ifndef __KEY_H
#define __KEY_H

#include "MSP430F5438A.h"
#include "stdio.h"
#include <stdbool.h>
#include "uart.h"
#include "main.h"
#include "clock.h"
#include "at24c512.h"
#include "led.h"
#include "ublox.h"
#include "sd2068.h"
#include "time.h"
#include "mg323b.h"
#include "adc.h"
#include "packdata.h"
#include "system.h"
#include "config.h"
#include "string.h"
//#include "isr.h"
#include "device.h"

extern bool key_value;
extern bool ray_value;
//#define KEY (P2IN & BIT3)
#define KEY ((P6IN & BIT1) >>1)
#define RAY ((P1IN & BIT5) >> 5)

extern void check_key(void);
extern void gpio_init(void);


#endif