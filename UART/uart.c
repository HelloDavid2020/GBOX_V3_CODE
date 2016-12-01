#include "MSP430F5438A.h"
#include "uart.h"
#include "isr.h"
//---------------串口0初始化子函数----------------------------------------------
void gps_port_init( void )
{
  P3SEL |= BIT4+BIT5;         // 设置P34、P35为UART
  UCA0CTL1 |= UCSWRST;        // 允许软件复位
  //UCA0CTL0 |= UCPEN;          // 奇偶使能、奇效验、LSB先、8位数据位、1停止位、USART模式、异步模式
  UCA0CTL1 |= UCSSEL1;        // CLK=SMCLK、不接受错误字符不置位、接受暂停字符不置位、非睡眠、下一帧数据、下一帧非暂停
  UCA0BR0 = 0x41;             // 8M晶振9600波特率设置
  UCA0BR1 = 0x03;             // 8M晶振9600波特率设置
  UCA0MCTL = 0x00;            // 8M晶振9600波特率设置
  UCA0CTL1 &= ~UCSWRST;       // 禁止USCI软件复位
  UCA0IE |= UCRXIE;           // 开串口0接受中断
  //UCA0IE |= UCTXIE;           // 开串口0发送中断(发送采用查询法故不开启发送中断)
}
//---------------串口2初始化子函数----------------------------------------------
void gsm_port_init( void )
{
  P9SEL |= BIT4+BIT5;         // 设置P94、P95为UART
  UCA2CTL1 |= UCSWRST;        // 允许软件复位
  //UCA2CTL0 |= UCPEN;          // 奇偶使能、奇效验、LSB先、8位数据位、1停止位、USART模式、异步模式
  UCA2CTL1 |= UCSSEL1;        // CLK=SMCLK、不接受错误字符不置位、接受暂停字符不置位、非睡眠、下一帧数据、下一帧非暂停
  UCA2BR0 = 0x45;             // 8M晶振115200波特率设置
  UCA2BR1 = 0x00;             // 8M晶振115200波特率设置
  UCA2MCTL = 0x4A;            // 8M晶振115200波特率设置
  UCA2CTL1 &= ~UCSWRST;       // 禁止USCI软件复位
  UCA2IE |= UCRXIE;           // 开串口0接受中断
  //UCA2IE |= UCTXIE;           // 开串口0发送中断(发送采用查询法故不开启发送中断)
}
//---------------串口3初始化子函数----------------------------------------------
void debug_port_init( unsigned int baud_rate )
{
  P10SEL |= BIT4+BIT5;         // 设置P104、P105为UART
  UCA3CTL1 |= UCSWRST;        // 允许软件复位
  //UCA3CTL0 |= UCPEN;          // 奇偶使能、奇效验、LSB先、8位数据位、1停止位、USART模式、异步模式
  UCA3CTL1 |= UCSSEL1;        // CLK=SMCLK、不接受错误字符不置位、接受暂停字符不置位、非睡眠、下一帧数据、下一帧非暂停
  
  switch(baud_rate)
  {
    case 9600:
//      UCA3BR0 = 0x00;             // 7.3728M晶振115200波特率设置
//      UCA3BR1 = 0x03;             // 7.3728M晶振115200波特率设置
//      UCA3MCTL = 0x00;            // 7.3728M晶振115200波特率设置  
      UCA3BR0 = 0x41;             // 7.3728M晶振115200波特率设置
      UCA3BR1 = 0x03;             // 7.3728M晶振115200波特率设置
      UCA3MCTL = 0x00;            // 7.3728M晶振115200波特率设置        
      break;
    case 38400:
      UCA3BR1 = 0x00;             // 7.3728M晶振115200波特率设置
      UCA3BR0 = 0xC0;             // 7.3728M晶振115200波特率设置      
      UCA3MCTL = 0x00;            // 7.3728M晶振115200波特率设置  
      break;
    case 57600:
      UCA3BR1 = 0x00;             // 7.3728M晶振115200波特率设置
      UCA3BR0 = 0x80;             // 7.3728M晶振115200波特率设置
      UCA3MCTL = 0x00;            // 7.3728M晶振115200波特率设置  
      break;

  default :
      UCA3BR1 = 0x00;             // 7.3728M晶振115200波特率设置
      UCA3BR0 = 0x45;             // 7.3728M晶振115200波特率设置
      UCA3MCTL = 0x4A;            // 7.3728M晶振115200波特率设置  
      break;      



  }
  

  
  
  UCA3CTL1 &= ~UCSWRST;       // 禁止USCI软件复位haizeiw
  UCA3IE |= UCRXIE;           // 开串口0接受中断
  //UCA3IE |= UCTXIE;           // 开串口0发送中断(发送采用查询法故不开启发送中断)
}
//---------------串口打印子函数-------------------------------------------------
int putchar( int ch )
{  
  if (debug == 0) return 0;
  while (!(UCA3IFG&UCTXIFG));   
  UCA3TXBUF = ch; 
  return ch;
}
//---------------串口2发送字符串子函数------------------------------------------
void uscia2_send( char send_data[])//发送一个字符串
{
  static uchar send_num;
  do
  {
    while (!(UCA2IFG&UCTXIFG));     // 等待UCA0TXBUF为空
    UCA2TXBUF = send_data[send_num];
    send_num++;
  }
  while (send_data[(send_num - 0x01)] != 0x0A);
  send_num = 0;
}
//---------------串口2发送字符串子函数------------------------------------------
void send_PostBack_data(char send_data[],uchar data_num)
{
  static uchar send_data_num;
  do
  {
    while (!(UCA2IFG&UCTXIFG));     // 等待UCA0TXBUF为空
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


