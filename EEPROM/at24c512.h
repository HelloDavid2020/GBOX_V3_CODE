#ifndef __at24c512_H
#define __at24c512_H

#define uchar unsigned char 
#define uint unsigned int

#define EEP_WP_INIT             P9DIR |= BIT0;P9DIR |= BIT3           // EEP_WP IO初始化
#define EEP_WP_ENABLE           P9OUT |= BIT0;P9OUT |= BIT3           // 写保护使能
#define EEP_WP_DISABLE          P9OUT &= ~BIT0;P9OUT &= ~BIT3          // 写保护禁止

#define EEP_POWER_ON             P9DIR |= BIT3;P9OUT |= BIT3           // EEP_WP IO初始化
#define EEP_POWER_OFF            P9DIR |= BIT3;P9OUT &= ~BIT3            // EEP_WP IO初始化




#define IIC2_IO_INIT            P9SEL |= (BIT1+BIT2)    // 配置功能口
#define EEPROM_ADDRESS          0x50                    // EEPROM地址

extern void I2C2_Init(void);                            // I2C2初始化子函数
extern void I2C2_ByteWrite(uchar TXdata, uint Addr);    // I2C2写一个字节子函数
extern uchar I2C2_ByteRead(uint Addr);                  // I2C2读一个字节子函数
extern void eeprom_PageWrite(uchar* pBuffer, uint WriteAddr, uchar NumByteToWrite); //I2C2写多个字节子函数
extern void eeprom_PageRead(uchar* pBuffer, uint WriteAddr, uchar NumByteToRead);   //I2C2读多个字节子函数

#endif