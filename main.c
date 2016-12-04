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
#include "led.h"
#include "isr.h"
#include "device.h"
#include "key.h"


void main( void )
{
    WDTCTL = WDTPW + WDTHOLD;   // 关闭看门狗
    //Clock_init();
    dco_int();
    gpio_init();

    EEP_POWER_ON; // EEPROM
    PULL_UP_ON;   // RTC
    GPS_POWER_INIT;
    GPS_POWER_ON;
    __delay_cycles(100000);
    __delay_cycles(100000);
    __delay_cycles(100000);
    __delay_cycles(100000);

//    GPS_POWER_INIT2;
//    GPS_POWER_OFF2;
    GSM_POWER_INIT;
    GSM_POWER_OFF;
//    GSM_POWER_ON;
    GSM_PWR_KEY_OFF;
    I2C2_Init();
    I2C0_Init();
    
    gps_port_init();
    gsm_port_init();
    debug_port_init(1);
    
    TimeA0_init();
//    #ifdef WDG
//    WDTCTL = WDT_ARST_250;
//    #endif
    AD_init();
    
    _BIS_SR(GIE);   // 开启总中断
    device_start(); // EEPROM RTC KEY

    while(1)    
    {
//        system_start();
//        #ifdef WDG
//        WDTCTL = WDT_ARST_250;
//        #endif
//
//        //work_mode=2;
//        printf("work mode : %d\r\n",work_mode);
//        printf("work time : %d mins\r\n",work_time);
//        printf("sleep time: %d mins\r\n",sleep_time);
      work_mode=2;
      //  work_mode_handler(work_mode);
    }
}
