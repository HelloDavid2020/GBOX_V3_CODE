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
    ADC12CTL0 |= ADC12SHT0_8 + ADC12ON; // 设置采样保持时间、内部参考电压打开、ADC12_A打开
    ADC12CTL1 |= ADC12SHP;
    ADC12CTL2 |= ADC12RES_2;                            // 12位AD模式
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
    printf("\r\n电池电压：%d.%dV\r\n",AD_voltage / 1000,AD_voltage % 1000);
    
    //-----------------------------------------------------------
    // added by Alex 2015-3-22 20:22:39
    ADC12CTL0 &=~ ADC12ENC;                      //停止转换
    ADC12CTL0 &= ~ADC12ON;  //ADC12ON复位时关闭ADC12的内核，置位时打开ADC12的内核
    //-----------------------------------------------------------

    __delay_cycles(10000);
}