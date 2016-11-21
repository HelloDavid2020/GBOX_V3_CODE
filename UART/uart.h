#ifndef __uart_H
#define __uart_H

#define uchar unsigned char 
#define uint unsigned int

extern void gps_port_init( void );     // 串口0初始化子函数
extern void gsm_port_init( void );     // 串口2初始化子函数
extern void debug_port_init( unsigned int baud_rate );     // 串口3初始化子函数
extern int putchar( int ch );       // 串口打印子函数
extern void uscia2_send( char send_data[]);
extern void send_PostBack_data(char send_data[],uchar data_num);

extern void SerialPutString(uchar *s);
extern void SerialPutChar(uchar c);


#endif