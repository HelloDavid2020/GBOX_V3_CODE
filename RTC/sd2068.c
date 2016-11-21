#include "MSP430F5438A.h"
#include "sd2068.h"
#include "stdio.h"
#include "uart.h"
#include "main.h"
uchar RTC_year,RTC_mon,RTC_day,RTC_week,RTC_hour,RTC_min,RTC_sec;                                       // 定义RTC时间变量
uchar RTC_year_temp,RTC_mon_temp,RTC_day_temp,RTC_week_temp,RTC_hour_temp,RTC_min_temp,RTC_sec_temp;    // 定义RTC时间临时变量
uchar alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec;                                    // 定义报警时间变量
uchar alarm_hour_temp,alarm_min_temp,alarm_sec_temp;                                                    // 定义报警时间临时变量
uchar start_year,start_mon,start_day,start_hour,start_min,start_sec;                                    // 定义启动时间变量
uchar BTC_year,BTC_mon,BTC_day,BTC_week,BTC_hour,BTC_min,BTC_sec;                                       // 定义北京时间变量
unsigned long int RTC_time_data,start_time_data,BTC_time_data,work_time_data;                           
uchar time_correction;                                                                                  // 校时成功标志
//-----------------IIC0初始化子函数---------------------------------------------
void I2C0_Init(void)
{
    IIC0_IO_INIT;                               // 配置功能口
    UCB0CTL1 |= UCSWRST;                        // 使能软件复位
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // 本地地址7位、从设备地址7位、单主环境、主设备模式、i2c模式、同步模式
    UCB0CTL1 |= UCSSEL_2 + UCSWRST;             // USCI时钟源选择SMCLK=XT2=8M，保持软件复位
    UCB0BR0 = 73;				                // 分频因子数值
    UCB0BR1 = 0;				                // fSCL = SMCLK/UCB1BR0 = 7.3728MHz/30 = ~250kHz
    UCB0I2CSA = SD2068_ADDR;                    // EEPROM地址
    UCB0CTL0 &= ~UCSLA10;                   	// 确保7位地址模式
    UCB0CTL1 &= ~UCSWRST;                       // 清除软件复位，恢复运行
}
//----------------------IIC0写一个字节子函数------------------------------------
void I2C0_ByteWrite(uchar TXdata, uchar Addr)
{
    __delay_cycles(100000);
    while( UCB0CTL1 & UCTXSTP );                    // 确保停止条件没有产生
    UCB0CTL1 |= UCTR;                               // 写模式
    UCB0CTL1 |= UCTXSTT;                            // 发动启动位
    UCB0TXBUF = Addr;                               // 发送从字节地址
    while( !( UCB0IFG & UCTXIFG ) );                // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
    while( UCB0CTL1 & UCTXSTT );                    // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB0TXBUF = TXdata;                             // 发送数据
    while( !( UCB0IFG & UCTXIFG ) );                // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
    UCB0CTL1 |= UCTXSTP;                            // 发送停止位
    while( UCB0CTL1 & UCTXSTP );                    // 等待发送完成  
}
//-----------------IIC0读一个字节子函数-----------------------------------------
uchar I2C0_ByteRead(uchar Addr)
{
    uchar re;
    
    __delay_cycles(100000);
    while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
    UCB0CTL1 |= UCTR;                                   // 写模式
    UCB0CTL1 |= UCTXSTT;                                // 发动启动位
    UCB0TXBUF = Addr;                                   // 发送从字节地址
    while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
    while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    
    UCB0CTL1 &= ~UCTR;                                  // 读模式
    UCB0CTL1 |= UCTXSTT;                                // 发动启动位
    while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
    while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
    UCB0CTL1 |= UCTXSTP;                                // 发送停止位
    while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成  
    while( !( UCB0IFG & UCRXIFG ) );                    // 收到数据
    re = UCB0RXBUF;                                     // 读取字节数据
    return re;                                          // 返回数据
}
//---------读时间子函数---------------------------------------------------------

void  Read_time( void )
{
    __delay_cycles(100000);
    do
    {
        RTC_year_temp = 0x00;
        RTC_mon_temp = 0x00;
        RTC_day_temp = 0x00;
        RTC_week_temp = 0x00;
        RTC_hour_temp = 0x00;
        RTC_min_temp = 0x00;
        RTC_sec_temp = 0x00;
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x00;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_sec = HEX2DEC( UCB0RXBUF & 0x7F);               // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_min = HEX2DEC( UCB0RXBUF & 0x7F);               // 读取分数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_hour = HEX2DEC( UCB0RXBUF & 0x3F);              // 读取时数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_week = HEX2DEC( UCB0RXBUF & 0x07);              // 读取周数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_day = HEX2DEC( UCB0RXBUF & 0x3F);               // 读取日数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_mon = HEX2DEC( UCB0RXBUF & 0x1F);               // 读取月数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_year = HEX2DEC( UCB0RXBUF );                    // 读取年数据
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x00;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // 读取分数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_hour_temp = HEX2DEC( UCB0RXBUF & 0x3F);         // 读取时数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_week_temp = HEX2DEC( UCB0RXBUF & 0x07);         // 读取周数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_day_temp = HEX2DEC( UCB0RXBUF & 0x3F);          // 读取日数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_mon_temp = HEX2DEC( UCB0RXBUF & 0x1F);          // 读取月数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_year_temp = HEX2DEC( UCB0RXBUF );               // 读取年数据
    }
    while(RTC_year != RTC_year_temp || RTC_mon != RTC_mon_temp || RTC_day != RTC_day_temp || RTC_hour != RTC_hour_temp || RTC_min != RTC_min_temp);
}
//-----------写时间子函数-------------------------------------------------------
void Write_time( void )
{
    __delay_cycles(100000);
    do
    {
        RTC_year_temp = 0x00;
        RTC_mon_temp = 0x00;
        RTC_day_temp = 0x00;
        RTC_week_temp = 0x00;
        RTC_hour_temp = 0x00;
        RTC_min_temp = 0x00;
        RTC_sec_temp = 0x00;

        write_register_ENABLE();                            // 写寄存器使能
        __delay_cycles(10000);
        if(RTC_sec >= 59)
            RTC_sec--;
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x00;                                   // 发送从字节地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0TXBUF = DEC2HEX(RTC_sec);                       // 发送秒数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(RTC_min);                       // 发送分数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = (DEC2HEX(RTC_hour) | 0x80);             // 发送时数据（24小时值|0x80）
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(RTC_week);                      // 发送周数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(RTC_day);                       // 发送日数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(RTC_mon);                       // 发送月数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(RTC_year);                      // 发送年数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成
        write_register_DISABLE();                           // 写寄存器禁止
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x00;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // 读取分数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_hour_temp = HEX2DEC( UCB0RXBUF & 0x3F);         // 读取时数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_week_temp = HEX2DEC( UCB0RXBUF & 0x07);         // 读取周数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_day_temp = HEX2DEC( UCB0RXBUF & 0x3F);          // 读取日数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_mon_temp = HEX2DEC( UCB0RXBUF & 0x1F);          // 读取月数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        RTC_year_temp = HEX2DEC( UCB0RXBUF );               // 读取年数据
    }
    while(RTC_year != RTC_year_temp || RTC_mon != RTC_mon_temp || RTC_day != RTC_day_temp || RTC_hour != RTC_hour_temp || RTC_min != RTC_min_temp);
    
}
//-----------写报警时间（分）---------------------------------------------------
void Write_alarm( unsigned long int alarm_time )
{
    __delay_cycles(100000);
    unsigned long int alarm_time_temp;

    alarm_year = start_year;
    alarm_mon = start_mon;
    alarm_day = start_day;
    
    alarm_time_temp = start_hour;
    alarm_time_temp = alarm_time_temp * 3600;
    alarm_time_temp = alarm_time_temp + start_min * 60 + start_sec;
    
    alarm_time = alarm_time * 60 + alarm_time_temp;
    
    alarm_hour = alarm_time / 3600;
    alarm_min = ( alarm_time /60 ) % 60;
    alarm_sec = alarm_time % 60;
    if(alarm_sec >= 60)
    {
        alarm_sec = alarm_sec - 60;
        alarm_min++;
    }
    if(alarm_min >= 60)
    {
        alarm_min = alarm_min - 60;
        alarm_hour++;
    }
    if(alarm_hour >= 24)
    {
        alarm_hour = alarm_hour - 24;
        alarm_day++;
    }
    if( alarm_mon == 1 || alarm_mon == 3 || alarm_mon == 5 || alarm_mon == 7 || alarm_mon == 8 || alarm_mon == 10 || alarm_mon == 12 )
    {
        if( alarm_day > 31 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 4 || alarm_mon == 6 || alarm_mon == 9 || alarm_mon == 11 )
    {
        if( alarm_day > 30 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 2 )
    {
        if( alarm_year%4 == 0 )
        {
            if( alarm_day > 29)
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
        else 
        {
            if( alarm_day > 28 )
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
    }
    if(alarm_mon > 12)
    {
        alarm_mon = 1;
        alarm_year++;
    }

    do
    {
        alarm_hour_temp = 0x00;
        alarm_min_temp = 0x00;
        alarm_sec_temp = 0x00;
        write_register_ENABLE();                            // 写寄存器使能
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_min);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成
        write_register_DISABLE();                           // 写寄存器禁止
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取分数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // 读取时数据
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // 写寄存器使能
    I2C0_ByteWrite(0x07 , 0x0E);                        // 写报警允许寄存器（时分秒）
    I2C0_ByteWrite(0x92 , 0x10);                        // 写控制寄存器CTR2 INT脚报警中断输出 开报警中断
    write_register_DISABLE();                           // 写寄存器禁止
    if(work_type == 0)
    {
        printf("\r\n休眠时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
    else if(work_type == 1)
    {
        printf("\r\n唤醒时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
}
//-----------写报警时间（秒）---------------------------------------------------
void Write_alarm_sec( unsigned long int alarm_time )
{
    unsigned long int alarm_time_temp;

    __delay_cycles(100000);
    alarm_year = start_year;
    alarm_mon = start_mon;
    alarm_day = start_day;
    
    alarm_time_temp = start_hour;
    alarm_time_temp = alarm_time_temp * 3600;
    alarm_time_temp = alarm_time_temp + start_min * 60 + start_sec;
    
    alarm_time = alarm_time + alarm_time_temp;
    
    alarm_hour = alarm_time / 3600;
    alarm_min = ( alarm_time /60 ) % 60;
    alarm_sec = alarm_time % 60;
    if(alarm_sec >= 60)
    {
        alarm_sec = alarm_sec - 60;
        alarm_min++;
    }
    if(alarm_min >= 60)
    {
        alarm_min = alarm_min - 60;
        alarm_hour++;
    }
    if(alarm_hour >= 24)
    {
        alarm_hour = alarm_hour - 24;
        alarm_day++;
    }
    if( alarm_mon == 1 || alarm_mon == 3 || alarm_mon == 5 || alarm_mon == 7 || alarm_mon == 8 || alarm_mon == 10 || alarm_mon == 12 )
    {
        if( alarm_day > 31 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 4 || alarm_mon == 6 || alarm_mon == 9 || alarm_mon == 11 )
    {
        if( alarm_day > 30 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 2 )
    {
        if( alarm_year%4 == 0 )
        {
            if( alarm_day > 29)
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
        else 
        {
            if( alarm_day > 28 )
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
    }
    if(alarm_mon > 12)
    {
        alarm_mon = 1;
        alarm_year++;
    }

    do
    {
        alarm_hour_temp = 0x00;
        alarm_min_temp = 0x00;
        alarm_sec_temp = 0x00;
        write_register_ENABLE();                            // 写寄存器使能
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_min);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成
        write_register_DISABLE();                           // 写寄存器禁止
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取分数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // 读取时数据
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // 写寄存器使能
    I2C0_ByteWrite(0x07 , 0x0E);                        // 写报警允许寄存器（时分秒）
    I2C0_ByteWrite(0x92 , 0x10);                        // 写控制寄存器CTR2 INT脚报警中断输出 开报警中断
    write_register_DISABLE();                           // 写寄存器禁止
    if(work_type == 0)
    {
        printf("\r\n休眠时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
    else if(work_type == 1)
    {
        printf("\r\n唤醒时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
}
//----------------------写定时启动----------------------------------------------
void Write_timer( unsigned long int timer_time )
{
    unsigned long int timer_time_temp;

    __delay_cycles(100000);
    Read_time();
    
    alarm_year = RTC_year;
    alarm_mon = RTC_mon;
    alarm_day = RTC_day;
    
    timer_time_temp = RTC_hour;
    timer_time_temp = timer_time_temp * 3600;
    timer_time_temp = timer_time_temp + RTC_min * 60 + RTC_sec;
    
    timer_time = timer_time * 60;
    
    if(timer_time < timer_time_temp)
    {
        alarm_day++;
    }
    else
    {
        if((timer_time - timer_time_temp) < 30)
        {
            timer_time = timer_time + 60;
        }
    }
    alarm_hour = timer_time / 3600;
    alarm_min = ( timer_time /60 ) % 60;
    alarm_sec = timer_time % 60;
    if(alarm_sec >= 60)
    {
        alarm_sec = alarm_sec - 60;
        alarm_min++;
    }
    if(alarm_min >= 60)
    {
        alarm_min = alarm_min - 60;
        alarm_hour++;
    }
    if(alarm_hour >= 24)
    {
        alarm_hour = alarm_hour - 24;
        alarm_day++;
    }
    if( alarm_mon == 1 || alarm_mon == 3 || alarm_mon == 5 || alarm_mon == 7 || alarm_mon == 8 || alarm_mon == 10 || alarm_mon == 12 )
    {
        if( alarm_day > 31 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 4 || alarm_mon == 6 || alarm_mon == 9 || alarm_mon == 11 )
    {
        if( alarm_day > 30 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 2 )
    {
        if( alarm_year%4 == 0 )
        {
            if( alarm_day > 29)
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
        else 
        {
            if( alarm_day > 28 )
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
    }
    if(alarm_mon > 12)
    {
        alarm_mon = 1;
        alarm_year++;
    }

    do
    {
        alarm_hour_temp = 0x00;
        alarm_min_temp = 0x00;
        alarm_sec_temp = 0x00;
        write_register_ENABLE();                            // 写寄存器使能
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_min);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成
        write_register_DISABLE();                           // 写寄存器禁止
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取分数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // 读取时数据
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // 写寄存器使能
    I2C0_ByteWrite(0x07 , 0x0E);                        // 写报警允许寄存器（时分秒）
    I2C0_ByteWrite(0x92 , 0x10);                        // 写控制寄存器CTR2 INT脚报警中断输出 开报警中断
    write_register_DISABLE();                           // 写寄存器禁止

    printf("\r\n定时启动时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    
}
//----------------------写间隔时间----------------------------------------------
void Write_takt( unsigned long int takt_time )
{
    unsigned long int takt_time_temp;

    __delay_cycles(100000);
    Read_time();
    printf("\r\n当前RTC时间为：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
    
    alarm_year = RTC_year;
    alarm_mon = RTC_mon;
    alarm_day = RTC_day;
    
    takt_time_temp = RTC_hour;
    takt_time_temp = takt_time_temp * 3600;
    takt_time_temp = takt_time_temp + RTC_min * 60 + RTC_sec;
    
    takt_time = takt_time_temp + takt_time;
    
    alarm_hour = takt_time / 3600;
    alarm_min = ( takt_time /60 ) % 60;
    alarm_sec = takt_time % 60;
    if(alarm_sec >= 60)
    {
        alarm_sec = alarm_sec - 60;
        alarm_min++;
    }
    if(alarm_min >= 60)
    {
        alarm_min = alarm_min - 60;
        alarm_hour++;
    }
    if(alarm_hour >= 24)
    {
        alarm_hour = alarm_hour - 24;
        alarm_day++;
    }
    if( alarm_mon == 1 || alarm_mon == 3 || alarm_mon == 5 || alarm_mon == 7 || alarm_mon == 8 || alarm_mon == 10 || alarm_mon == 12 )
    {
        if( alarm_day > 31 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 4 || alarm_mon == 6 || alarm_mon == 9 || alarm_mon == 11 )
    {
        if( alarm_day > 30 )
        {
            alarm_day = 1;
            alarm_mon++;
        }
    }
    else if( alarm_mon == 2 )
    {
        if( alarm_year%4 == 0 )
        {
            if( alarm_day > 29)
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
        else 
        {
            if( alarm_day > 28 )
            {
                alarm_day = 1;
                alarm_mon++;
            }
        }
    }
    if(alarm_mon > 12)
    {
        alarm_mon = 1;
        alarm_year++;
    }

    do
    {
        alarm_hour_temp = 0x00;
        alarm_min_temp = 0x00;
        alarm_sec_temp = 0x00;
        write_register_ENABLE();                            // 写寄存器使能
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_min);                     // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // 发送数据
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成
        write_register_DISABLE();                           // 写寄存器禁止
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // 确保停止条件没有产生
        UCB0CTL1 |= UCTR;                                   // 写模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        UCB0TXBUF = 0x07;                                   // 发送从字节高地址
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成（UCB0TXBUF为空时UCTXIFG位被置1）
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        UCB0CTL1 &= ~UCTR;                                  // 读模式
        UCB0CTL1 |= UCTXSTT;                                // 发动启动位
        while( !( UCB0IFG & UCTXIFG ) );                    // 等待发送完成
        while( UCB0CTL1 & UCTXSTT );                        // 等待从设备应答（收到从设备应答时UCTXSTT被清零）
        
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取秒数据
        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // 读取分数据
        
        UCB0CTL1 |= UCTXSTP;                                // 发送停止位
        while( UCB0CTL1 & UCTXSTP );                        // 等待发送完成   

        while( !( UCB0IFG & UCRXIFG ) );                    // 等待接受完成（UCB0RXBUF接收到完成数据时UCRXIFG位被置1）
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // 读取时数据
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // 写寄存器使能
    I2C0_ByteWrite(0x07 , 0x0E);                        // 写报警允许寄存器（时分秒）
    I2C0_ByteWrite(0x92 , 0x10);                        // 写控制寄存器CTR2 INT脚报警中断输出 开报警中断 周期性报警
    write_register_DISABLE();                           // 写寄存器禁止

    printf("\r\n下次上传时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    
}
//----------------------校时子程序----------------------------------------------
void Correction_Time(uchar year , uchar mon ,uchar day , uchar hour , uchar min , uchar sec )
{
    //******************入口参数检验开始****************************************
    if( 14 > year || year > 99 )
    {
        printf("\r\n校时子程序入口参数非法，退出校时\r\n");
        return;
    }
    if( 1 > mon || mon > 12 )
    {
        printf("\r\n校时子程序入口参数非法，退出校时\r\n");
        return;
    }
    if( mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12 )
    {
        if( 1 > day || day > 31 )
        {
            printf("\r\n校时子程序入口参数非法，退出校时\r\n");
            return;
        }
    }
    else if( mon == 4 || mon == 6 || mon == 9 || mon == 11 )
    {
        if( 1 > day || day > 30 )
        {
            printf("\r\n校时子程序入口参数非法，退出校时\r\n");
            return;
        }
    }
    else if( mon == 2 )
    {
        if( year%4 == 0 )
        {
            if( 1 > day || day > 29)
            {
                printf("\r\n校时子程序入口参数非法，退出校时\r\n");
                return;
            }
        }
        else 
        {
            if( 1 > day || day > 28 )
            {
                printf("\r\n校时子程序入口参数非法，退出校时\r\n");
                return;
            }
        }
    }
    //******************入口参数检验完成****************************************
    
    printf("\r\n启动RTC时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",start_year,start_mon,start_day,start_hour,start_min,start_sec);
    start_time_data = start_hour;
    start_time_data = start_time_data * 3600;
    start_time_data = start_time_data + start_min * 60 + start_sec;
    
    Read_time();                //取当前时间
    printf("\r\n当前RTC时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
    RTC_time_data = RTC_hour;
    RTC_time_data = RTC_time_data * 3600;
    RTC_time_data = RTC_time_data + RTC_min * 60 + RTC_sec;
    
    if(start_time_data > RTC_time_data)
            RTC_time_data += 86400;
    work_time_data = RTC_time_data - start_time_data;
    
    printf("\r\n当前北京时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",year,mon,day,hour,min,sec);
    BTC_time_data = hour;
    BTC_time_data = BTC_time_data * 3600;
    BTC_time_data = BTC_time_data + min * 60 + sec;
    start_day = day;
    start_mon = mon;
    start_year = year;
    if(work_time_data > BTC_time_data)
    {
        BTC_time_data += 86400;
        start_day--;
        if(start_day <= 0)
        {
            start_mon--;
        }
        if(start_mon <= 0)
        {
            start_mon = 12;
            start_year--;
        }
        if( start_mon == 1 || start_mon == 3 || start_mon == 5 || start_mon == 7 || start_mon == 8 || start_mon == 10 || start_mon == 12 )
        {
            if(start_day <= 0)
            {
                start_day = 31;
            }
        }
        else if( start_mon == 4 || start_mon == 6 || start_mon == 9 || start_mon == 11 )
        {
            if(start_day <= 0)
            {
                start_day = 30;
            }
        }
        else if( start_mon == 2 )
        {
            if( start_year%4 == 0 )
            {
                if( start_day <= 0)
                {
                    start_day = 29;
                }
            }
            else 
            {
                if( start_day <= 0 )
                {
                    start_day = 28;
                }
            }
        }
    }
    start_time_data = BTC_time_data - work_time_data;
    start_hour = start_time_data / 3600;
    start_min = (start_time_data / 60) % 60;
    start_sec = start_time_data % 60;
    printf("\r\n启动时间修改为：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",start_year,start_mon,start_day,start_hour,start_min,start_sec);
    RTC_year = year;
    RTC_mon = mon;
    RTC_day = day;
    RTC_hour = hour;
    RTC_min = min;
    RTC_sec = sec;
    Write_time();
    Read_time();
    printf("\r\n校时完成，当前RTC时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
    if(work_mode != 3 || takt_time >= 180)
    {
        printf("\r\nRTC时间被修正，重新设置休眠时间\r\n");
        Write_alarm(work_time);                //重新设置休眠时间
    }
    else
    {
        printf("\r\nRTC时间被修正，重新设置下次上传时间\r\n");
        Write_takt(takt_time);                //重新设置休眠时间
    }
}
//-----------写寄存器使能-------------------------------------------------------
void write_register_ENABLE( void )
{
    __delay_cycles(100000);
    I2C0_ByteWrite(0x80,0x10);
    __delay_cycles(100000);
    I2C0_ByteWrite(0x84,0x0F);
    
}
//-----------写寄存器禁止-------------------------------------------------------
void write_register_DISABLE( void )
{
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x0F);
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x10);
    
}
//-----------禁止报警子函数-----------------------------------------------------
void ban_alarm( void )
{
    write_register_ENABLE(); // 写寄存器允许
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x0E); // 禁止报警
    write_register_DISABLE(); // 写寄存器禁止
    
}
//-----------清报警标志位子函数-------------------------------------------------
/*
void clear_alarm_flag(void)
{
    __delay_cycles(100000);
    I2C0_ByteWrite(0x80,0x10);
    __delay_cycles(100000);
    I2C0_ByteWrite(0x84,0x0F);

    __delay_cycles(100000);
    I2C0_ByteWrite(0x84,0x0F);

    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x0F);
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x10);
}
*/
//-----------HEX TO DEC---------------------------------------------------------
uchar HEX2DEC(uchar bcd)
{
	uchar out;
	out=((bcd>>4)* 10)+(bcd & 0xf);
	return out;
}
//----------DEC TO HEX----------------------------------------------------------
uchar DEC2HEX(uchar dec)
{
	uchar out;
	out=((dec/10)<<4)+(dec%10);
	return out;
}
