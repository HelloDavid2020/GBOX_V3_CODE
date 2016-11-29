#include "MSP430F5438A.h"
#include "at24c512.h"

//-----------------IIC2初始化子函数---------------------------------------------
void I2C2_Init(void)
{
    //EEP_WP_INIT;                                // EEP_WP IO初始化
    //EEP_WP_ENABLE;                              // EEP写保护使能
    IIC2_IO_INIT;                               // 配置功能口
    UCB2CTL1 |= UCSWRST;                        // 使能软件复位
    UCB2CTL0 = UCMST + UCMODE_3 + UCSYNC;       // 本地地址7位、从设备地址7位、单主环境、主设备模式、i2c模式、同步模式
    UCB2CTL1 |= UCSSEL_2 + UCSWRST;             // USCI时钟源选择SMCLK=XT2=8M，保持软件复位
    UCB2BR0 = 80;				// 分频因子数值
    UCB2BR1 = 0;				// fSCL = SMCLK/UCB1BR0 = 8000KHz/40 = ~250kHz
    UCB2I2CSA = EEPROM_ADDRESS;                 // EEPROM地址
    UCB2CTL0 &= ~UCSLA10;                   	// 确保7位地址模式
    UCB2CTL1 &= ~UCSWRST;                       // 清除软件复位，恢复运行
}
//-----------------IIC2写一个字节子函数-----------------------------------------
void I2C2_ByteWrite(uchar TXdata, uint Addr)
{
    __delay_cycles(100000);
    //EEP_WP_DISABLE;                                 // 写保护禁止
    while( UCB2CTL1 & UCTXSTP );                    // 确保停止条件没有产生
    UCB2CTL1 |= UCTR;                               // 写模式
    UCB2CTL1 |= UCTXSTT;                            // 发动启动位
    UCB2TXBUF = (uchar)( ( Addr & 0xFF00 ) >> 8 );  // 发送从字节高地址
    while( !( UCB2IFG & UCTXIFG ) );                // 等待发送完成（UCB2TXBUF为空时UCTXIFG位被置1）
    while( UCB2CTL1 & UCTXSTT );                    // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB2TXBUF = (uchar)( Addr & 0x00FF );           // 发送从字节低地址
    while( !( UCB2IFG & UCTXIFG ) );                // 等待发送完成

    UCB2TXBUF = TXdata;                             // 发送数据
    while( !( UCB2IFG & UCTXIFG ) );                // 等待发送完成（UCB2TXBUF为空时UCTXIFG位被置1）
    UCB2CTL1 |= UCTXSTP;                            // 发送停止位
    while( UCB2CTL1 & UCTXSTP );                    // 等待发送完成  
    //EEP_WP_ENABLE;                                  // 写保护使能
}
//-----------------IIC2读一个字节子函数-----------------------------------------
uchar I2C2_ByteRead(uint Addr)
{
    uchar re;
    
    __delay_cycles(100000);
    while( UCB2CTL1 & UCTXSTP );                        // 确保停止条件没有产生
    UCB2CTL1 |= UCTR;                                   // 写模式
    UCB2CTL1 |= UCTXSTT;                                // 发动启动位
    UCB2TXBUF = (uchar)( ( Addr & 0xFF00 ) >> 8 );      // 发送从字节高地址
    while( !( UCB2IFG & UCTXIFG ) );                    // 等待发送完成（UCB2TXBUF为空时UCTXIFG位被置1）
    while( UCB2CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB2TXBUF = (uchar)( Addr & 0x00FF );               // 发送从字节低地址
    while( !( UCB2IFG & UCTXIFG ) );                    // 等待发送完成
    
    UCB2CTL1 &= ~UCTR;                                  // 读模式
    UCB2CTL1 |= UCTXSTT;                                // 发动启动位
    while( UCB2CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB2CTL1 |= UCTXSTP;                                // 发送停止位
    while( UCB2CTL1 & UCTXSTP );                        // 等待发送完成  
    while( !( UCB2IFG & UCRXIFG ) );                    // 收到数据
    re = UCB2RXBUF;                                     // 读取字节数据
    return re;                                          // 返回数据
}
//-----------IIC2写多个字节（最多128个字节）子函数------------------------------
void eeprom_PageWrite(uchar* pBuffer, uint WriteAddr, uchar NumByteToWrite)
{
    return;

    __delay_cycles(100000);
	//EEP_WP_DISABLE;                                     // 写保护禁止
	while( UCB2CTL1 & UCTXSTP );                        // 确保停止条件没有产生
    UCB2CTL1 |= UCTR;                                   // 写模式
	UCB2CTL1 |= UCTXSTT;                                // 发动启动位
    UCB2TXBUF = (uchar)( ( WriteAddr & 0xFF00 ) >> 8 ); // 发送从字节高地址
	while( !( UCB2IFG & UCTXIFG ) );                    // 等待发送完成（UCB2TXBUF为空时UCTXIFG位被置1）
    while( UCB2CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB2TXBUF = (uchar)( WriteAddr & 0x00FF );          // 发送从字节低地址
    while( !( UCB2IFG & UCTXIFG ) );                    // 等待发送完成

	while(NumByteToWrite-- )                            // 循环发送数据包 
	{
		UCB2TXBUF = *pBuffer++;       
		while( !( UCB2IFG & UCTXIFG ) );
	}
	UCB2CTL1 |= UCTXSTP;                                // 发送停止位
	while( UCB2CTL1 & UCTXSTP );                        // 等待发送完成  
    //EEP_WP_ENABLE;                                      // 写保护使能
}
//-----------IIC2读多个字节（最多128个字节）子函数------------------------------
void eeprom_PageRead(uchar* pBuffer, uint WriteAddr, uchar NumByteToRead)
{
  return;
    __delay_cycles(100000);
	while( UCB2CTL1 & UCTXSTP );                        // 确保停止条件没有产生
    UCB2CTL1 |= UCTR;                                   // 写模式
	UCB2CTL1 |= UCTXSTT;                                // 发动启动位
    UCB2TXBUF = (uchar)( ( WriteAddr & 0xFF00 ) >> 8 ); // 发送从字节高地址
    while( !( UCB2IFG & UCTXIFG ) );                    // 等待发送完成（UCB2TXBUF为空时UCTXIFG位被置1）
    while( UCB2CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB2TXBUF = (uchar)( WriteAddr & 0x00FF );          // 发送从字节低地址
    while( !( UCB2IFG & UCTXIFG ) );                    // 等待发送完成
    
    UCB2CTL1 &= ~UCTR;                                  // 读模式
    UCB2CTL1 |= UCTXSTT;                                // 发动启动位
    while( UCB2CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
	NumByteToRead -- ;
	while(NumByteToRead)
	{
        while( !( UCB2IFG & UCRXIFG ) );                // 收到数据
		*pBuffer++ = UCB2RXBUF;                         //读取字节数据
		NumByteToRead--;

	}

	UCB2CTL1 |= UCTXSTP;                                // 发送停止位
    while( UCB2CTL1 & UCTXSTP );                        // 等待发送完成  
    while( !( UCB2IFG & UCRXIFG ) );                    // 收到数据
	*pBuffer = UCB2RXBUF;                               //读取字节数据       
	 
}
