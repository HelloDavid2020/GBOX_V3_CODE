#include "MSP430F5438A.h"
#include "uart.h"
#include "isr.h"
//---------------����0��ʼ���Ӻ���----------------------------------------------
void gps_port_init( void )
{
  P3SEL |= BIT4+BIT5;         // ����P34��P35ΪUART
  UCA0CTL1 |= UCSWRST;        // ���������λ
  //UCA0CTL0 |= UCPEN;          // ��żʹ�ܡ���Ч�顢LSB�ȡ�8λ����λ��1ֹͣλ��USARTģʽ���첽ģʽ
  UCA0CTL1 |= UCSSEL1;        // CLK=SMCLK�������ܴ����ַ�����λ��������ͣ�ַ�����λ����˯�ߡ���һ֡���ݡ���һ֡����ͣ
  UCA0BR0 = 0x41;             // 8M����9600����������
  UCA0BR1 = 0x03;             // 8M����9600����������
  UCA0MCTL = 0x00;            // 8M����9600����������
  UCA0CTL1 &= ~UCSWRST;       // ��ֹUSCI�����λ
  UCA0IE |= UCRXIE;           // ������0�����ж�
  //UCA0IE |= UCTXIE;           // ������0�����ж�(���Ͳ��ò�ѯ���ʲ����������ж�)
}
//---------------����2��ʼ���Ӻ���----------------------------------------------
void gsm_port_init( void )
{
  P9SEL |= BIT4+BIT5;         // ����P94��P95ΪUART
  UCA2CTL1 |= UCSWRST;        // ���������λ
  //UCA2CTL0 |= UCPEN;          // ��żʹ�ܡ���Ч�顢LSB�ȡ�8λ����λ��1ֹͣλ��USARTģʽ���첽ģʽ
  UCA2CTL1 |= UCSSEL1;        // CLK=SMCLK�������ܴ����ַ�����λ��������ͣ�ַ�����λ����˯�ߡ���һ֡���ݡ���һ֡����ͣ
  UCA2BR0 = 0x45;             // 8M����115200����������
  UCA2BR1 = 0x00;             // 8M����115200����������
  UCA2MCTL = 0x4A;            // 8M����115200����������
  UCA2CTL1 &= ~UCSWRST;       // ��ֹUSCI�����λ
  UCA2IE |= UCRXIE;           // ������0�����ж�
  //UCA2IE |= UCTXIE;           // ������0�����ж�(���Ͳ��ò�ѯ���ʲ����������ж�)
}
//---------------����3��ʼ���Ӻ���----------------------------------------------
void debug_port_init( unsigned int baud_rate )
{
  P10SEL |= BIT4+BIT5;         // ����P104��P105ΪUART
  UCA3CTL1 |= UCSWRST;        // ���������λ
  //UCA3CTL0 |= UCPEN;          // ��żʹ�ܡ���Ч�顢LSB�ȡ�8λ����λ��1ֹͣλ��USARTģʽ���첽ģʽ
  UCA3CTL1 |= UCSSEL1;        // CLK=SMCLK�������ܴ����ַ�����λ��������ͣ�ַ�����λ����˯�ߡ���һ֡���ݡ���һ֡����ͣ
  
  switch(baud_rate)
  {
    case 9600:
//      UCA3BR0 = 0x00;             // 7.3728M����115200����������
//      UCA3BR1 = 0x03;             // 7.3728M����115200����������
//      UCA3MCTL = 0x00;            // 7.3728M����115200����������  
      UCA3BR0 = 0x41;             // 7.3728M����115200����������
      UCA3BR1 = 0x03;             // 7.3728M����115200����������
      UCA3MCTL = 0x00;            // 7.3728M����115200����������        
      break;
    case 38400:
      UCA3BR1 = 0x00;             // 7.3728M����115200����������
      UCA3BR0 = 0xC0;             // 7.3728M����115200����������      
      UCA3MCTL = 0x00;            // 7.3728M����115200����������  
      break;
    case 57600:
      UCA3BR1 = 0x00;             // 7.3728M����115200����������
      UCA3BR0 = 0x80;             // 7.3728M����115200����������
      UCA3MCTL = 0x00;            // 7.3728M����115200����������  
      break;

  default :
      UCA3BR1 = 0x00;             // 7.3728M����115200����������
      UCA3BR0 = 0x45;             // 7.3728M����115200����������
      UCA3MCTL = 0x4A;            // 7.3728M����115200����������  
      break;      



  }
  

  
  
  UCA3CTL1 &= ~UCSWRST;       // ��ֹUSCI�����λhaizeiw
  UCA3IE |= UCRXIE;           // ������0�����ж�
  //UCA3IE |= UCTXIE;           // ������0�����ж�(���Ͳ��ò�ѯ���ʲ����������ж�)
}
//---------------���ڴ�ӡ�Ӻ���-------------------------------------------------
int putchar( int ch )
{  
  if (debug == 0) return 0;
  while (!(UCA3IFG&UCTXIFG));   
  UCA3TXBUF = ch; 
  return ch;
}
//---------------����2�����ַ����Ӻ���------------------------------------------
void uscia2_send( char send_data[])//����һ���ַ���
{
  static uchar send_num;
  do
  {
    while (!(UCA2IFG&UCTXIFG));     // �ȴ�UCA0TXBUFΪ��
    UCA2TXBUF = send_data[send_num];
    send_num++;
  }
  while (send_data[(send_num - 0x01)] != 0x0A);
  send_num = 0;
}
//---------------����2�����ַ����Ӻ���------------------------------------------
void send_PostBack_data(char send_data[],uchar data_num)
{
  static uchar send_data_num;
  do
  {
    while (!(UCA2IFG&UCTXIFG));     // �ȴ�UCA0TXBUFΪ��
    UCA2TXBUF = send_data[send_data_num];
    send_data_num++;
  }
  while ( send_data_num < data_num );
  send_data_num = 0;
}

void SerialPutChar(uchar c)
{
  while (!(UCA3IFG&UCTXIFG));   
  UCA3TXBUF = c; 
}

void SerialPutString(uchar *s)
{
    while (*s != '\0')
    {
        SerialPutChar(*s);
        s++;
    }
}


