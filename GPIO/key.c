#include "key.h"
bool key_value = 0;
bool ray_value = 0;

void gpio_init(void)
{
  // KEY_IN
  P6DIR &= ~BIT1;  // P6.1 ����Ϊ��������ģʽ  
  P6REN |=  BIT1;  /* Port 6 Resistor Enable */
  P6OUT &= ~BIT1; 

  // �������
//  P1DIR &= ~BIT5;  // P6.1 ����Ϊ��������ģʽ  
  
  // ADC
  P6DIR |= BIT5;
  P6OUT |= BIT5;

}


void check_key(void)
{
  //P2DIR &= ~BIT3;             // P23����Ϊ����ģʽ
  //P6DIR &= ~BIT1;  // P6.1 ����Ϊ��������ģʽ  
  //P6REN |=  BIT1;  /* Port 6 Resistor Enable */
  //P6OUT &= ~BIT1;  
  //__delay_cycles(10000);
  //key_value = KEY;
  //printf("\r\n key_in;: %d\r\n",key_value);
  //return;
  if(KEY == 1)   // ��������ģʽ
  {
    __delay_cycles(10000);
    if(KEY == 1)
    {
      printf("\r\n�豸��������ģʽ\r\n");
      UCA3IE |= UCRXIE;           // ������3�����ж�
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
                  printf("\r\n�豸�˳�����ģʽ\r\n");
                  break;
              }
          }
          
      }
     }
  }
}
