#ifndef __uart_H
#define __uart_H

#define uchar unsigned char 
#define uint unsigned int

extern void gps_port_init( void );     // ����0��ʼ���Ӻ���
extern void gsm_port_init( void );     // ����2��ʼ���Ӻ���
extern void debug_port_init( unsigned int baud_rate );     // ����3��ʼ���Ӻ���
extern int putchar( int ch );       // ���ڴ�ӡ�Ӻ���
extern void uscia2_send( char send_data[]);
extern void send_PostBack_data(char send_data[],uchar data_num);

extern void SerialPutString(uchar *s);
extern void SerialPutChar(uchar c);


#endif