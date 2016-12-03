#ifndef __led_H
#define __led_H

#include "MSP430F5438A.h"
#include "stdio.h"
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



#define uchar unsigned char 
#define uint unsigned int

#define ledInit()    P1DIR |= BIT5

//#define LED_INIT    P2DIR |= BIT1;  P5DIR |=  BIT4
#define LED_ON      P1DIR |= BIT5; 
#define LED_OFF     P1DIR |= BIT5; 
#define LED_RESET   P1DIR &= ~BIT5;

#define CPU_F           ((double)8000000) 
#define delay_us(x)     __delay_cycles((long)(CPU_F*(double)x/1000000.0)) 
#define delay_ms(x)     __delay_cycles((long)(CPU_F*(double)x/1000.0)) 




extern void led_start( void ); 
extern void delay100ms(void);
extern void delay_500ms(void) ;
extern void delay_1000ms(void);
extern void led_toggle(void);

#endif