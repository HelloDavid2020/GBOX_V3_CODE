#include "MSP430F5438A.h"
#include "adc.h"
#include "stdio.h"

uint AD_voltage;

void AD_init( void )
{
    REFCTL0 |=  REFON;
    ADC_INIT;
    ADC_ON;

    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 |= ADC12SHT0_8 + ADC12ON; // ���ò�������ʱ�䡢�ڲ��ο���ѹ�򿪡�ADC12_A��
    ADC12CTL1 |= ADC12SHP;
    ADC12CTL2 |= ADC12RES_2;                            // 12λADģʽ
    ADC12MCTL0 |= ADC12SREF_1 + ADC12INCH_4;            // 
    //ADC12CTL0 |= ADC12ENC; 
}

void AD_collect( void )
{
    uchar i;
    uint AD_voltage_min = 0xFFFF;
    uint AD_voltage_max = 0x0000;
    uint AD_voltage_temp = 0;
    unsigned long int AD_sum = 0;
    
    ADC12CTL0 |= ADC12ENC;
    for(i=0;i<AD_collec_num_max;i++)
    {
        ADC12CTL0 |= ADC12SC;
        while (!(ADC12IFG & BIT0));
        AD_voltage_temp = ADC12MEM0;
        if(AD_voltage_temp > AD_voltage_max)
            AD_voltage_max = AD_voltage_temp;
        if(AD_voltage_temp < AD_voltage_min)
            AD_voltage_min = AD_voltage_temp;
        AD_sum += AD_voltage_temp;
        //printf("\r\nAD_voltage:%d\r\n",AD_voltage_temp);
        //printf("\r\nAD_voltage_sum_temp:%ld\r\n",AD_sum);
        __delay_cycles(10000);
    }
    AD_sum = AD_sum - AD_voltage_max - AD_voltage_min;
    //printf("\r\nAD_voltage_sum:%ld\r\n",AD_sum);
    AD_voltage = (AD_sum / (AD_collec_num_max - 2) * 1500 / 4095) * 3;
    printf("\r\n��ص�ѹ��%d.%dV\r\n",AD_voltage / 1000,AD_voltage % 1000);
    
    //-----------------------------------------------------------
    // added by Alex 2015-3-22 20:22:39
    ADC12CTL0 &=~ ADC12ENC;                      //ֹͣת��
    ADC12CTL0 &= ~ADC12ON;  //ADC12ON��λʱ�ر�ADC12���ںˣ���λʱ��ADC12���ں�
    //-----------------------------------------------------------

    __delay_cycles(10000);
}