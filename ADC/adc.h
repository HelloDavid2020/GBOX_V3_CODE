#ifndef __adc_H
#define __adc_H

#define uchar unsigned char 
#define uint unsigned int

//#define ADC_INIT    P6SEL |= BIT4
//#define ADC_ON      P6DIR |= BIT5;P6OUT &= ~BIT5
//#define ADC_OFF     P6OUT |= BIT5

#define ADC_INIT    P6SEL |= BIT4
#define ADC_ON      P6DIR |= BIT5;P6OUT |= BIT5
#define ADC_OFF     P6DIR |= BIT5;P6OUT &= ~BIT5

#define AD_collec_num_max 22

extern uint AD_voltage;

extern void AD_init( void );
extern void AD_collect( void );

#endif