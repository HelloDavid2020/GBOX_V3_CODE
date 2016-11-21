#ifndef __at24c512_H
#define __at24c512_H

#define uchar unsigned char 
#define uint unsigned int

#define EEP_WP_INIT             P9DIR |= BIT0;P9DIR |= BIT3           // EEP_WP IO��ʼ��
#define EEP_WP_ENABLE           P9OUT |= BIT0;P9OUT |= BIT3           // д����ʹ��
#define EEP_WP_DISABLE          P9OUT &= ~BIT0;P9OUT &= ~BIT3          // д������ֹ

#define EEP_POWER_ON             P9DIR |= BIT3;P9OUT |= BIT3           // EEP_WP IO��ʼ��
#define EEP_POWER_OFF            P9DIR |= BIT3;P9OUT &= ~BIT3            // EEP_WP IO��ʼ��




#define IIC2_IO_INIT            P9SEL |= (BIT1+BIT2)    // ���ù��ܿ�
#define EEPROM_ADDRESS          0x50                    // EEPROM��ַ

extern void I2C2_Init(void);                            // I2C2��ʼ���Ӻ���
extern void I2C2_ByteWrite(uchar TXdata, uint Addr);    // I2C2дһ���ֽ��Ӻ���
extern uchar I2C2_ByteRead(uint Addr);                  // I2C2��һ���ֽ��Ӻ���
extern void eeprom_PageWrite(uchar* pBuffer, uint WriteAddr, uchar NumByteToWrite); //I2C2д����ֽ��Ӻ���
extern void eeprom_PageRead(uchar* pBuffer, uint WriteAddr, uchar NumByteToRead);   //I2C2������ֽ��Ӻ���

#endif