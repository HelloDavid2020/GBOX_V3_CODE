#ifndef __config_H
#define __config_H

#define uchar unsigned char 
#define uint unsigned int

extern uchar config_num;
extern uchar uart_flag; 

extern char config_buf[100];
extern char command_rec_enable;

extern uchar debug;
extern void scan_command(void);
extern void uscia_config( void );



#endif