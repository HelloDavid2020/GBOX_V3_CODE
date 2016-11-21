#include "MSP430F5438A.h"
#include "led.h"
#include "key.h"
void delay100ms(void)   //Îó²î -0.000000000007us
{
    unsigned char a,b,c;
    for(c=67;c>0;c--)
        for(b=141;b>0;b--)
            for(a=18;a>0;a--);
}

void delay_500ms(void)   //Îó²î -0.000000000007us
{
    unsigned char i=5;
    while(i--)
    {
      delay100ms();
    }
}

void led_start( void )
{
  ledInit();
  LED_ON;
  delay100ms();delay100ms();
  LED_OFF;
  delay100ms();delay100ms();
  LED_ON;
  delay100ms();delay100ms();
  LED_OFF;
  delay100ms();delay100ms();
  LED_ON;;
  delay100ms();delay100ms();
  LED_OFF;
  delay100ms();delay100ms();
  LED_RESET;
}

void led_toggle(void)
{
  
  static unsigned int count =0;
#ifdef DEBUG

  count++;
  if(count >= 30000)
  {
    count = 0;
    //LED_OFF;

    P2OUT ^= BIT1; // Toggle P1.0
    P5OUT ^= BIT4; // Toggle P1.0  
    ray_value = RAY;
    printf( "> ray_in: %d\r\n", ray_value);   
    printf( "> GSM_status: 0x%02X,  data_send_flag: %d, blind_data_size:% d\r\n", GSM_status,data_send_flag,blind_data_size);
  }
#endif
  
}
