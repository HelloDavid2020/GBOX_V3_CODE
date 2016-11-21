#ifndef __packdata_H
#define __packdata_H

#define uchar unsigned char 
#define uint unsigned int

#define protocol_VER    0x0A        // 通讯协议版本号
#define procedure_VER   0x2C        // 程序版本号

extern uchar data_type;            // 数据类型
extern uchar data_packet[113];      // 数据包

extern void pack_data( void );      //数据打包程序

#endif