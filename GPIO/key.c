#include "key.h"
bool key_value = 0;
bool ray_value = 0;

void gpio_init(void)
{
  // KEY_IN
  P6DIR &= ~BIT1;  // P6.1 设置为下拉输入模式  
  P6REN |=  BIT1;  /* Port 6 Resistor Enable */
  P6OUT &= ~BIT1; 

  // 光敏检测
//  P1DIR &= ~BIT5;  // P6.1 设置为下拉输入模式  
  
  // ADC
  P6DIR |= BIT5;
  P6OUT |= BIT5;

}


void check_key(void)
{
  //P2DIR &= ~BIT3;             // P23设置为输入模式
  //P6DIR &= ~BIT1;  // P6.1 设置为下拉输入模式  
  //P6REN |=  BIT1;  /* Port 6 Resistor Enable */
  //P6OUT &= ~BIT1;  
  //__delay_cycles(10000);
  //key_value = KEY;
  //printf("\r\n key_in;: %d\r\n",key_value);
  //return;
  if(KEY == 1)   // 进入配置模式
  {
    __delay_cycles(10000);
    if(KEY == 1)
    {
      printf("\r\n设备进入配置模式\r\n");
      UCA3IE |= UCRXIE;           // 开串口3接受中断
      while(1)
      {
          #ifdef WDG
          WDTCTL = WDT_ARST_250;
          #endif
          //uscia_config();
          if( KEY == 0)
          {
              __delay_cycles(10000);
              if( KEY == 0)
              {
                  printf("\r\n设备退出配置模式\r\n");
                  break;
              }
          }
          
      }
     }
  }
}
