#ifndef __packdata_H
#define __packdata_H

#define uchar unsigned char 
#define uint unsigned int

#define protocol_VER    0x0A        // ͨѶЭ��汾��
#define procedure_VER   0x2C        // ����汾��

extern uchar data_type;            // ��������
extern uchar data_packet[113];      // ���ݰ�

extern void pack_data( void );      //���ݴ������

#endif