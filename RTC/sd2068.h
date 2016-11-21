#ifndef __sd2068_H
#define __sd2068_H

#define uchar unsigned char 
#define uint unsigned int

#define PULL_UP_ON     P2DIR |= BIT6;P2OUT |= BIT6 
#define PULL_UP_OFF    P2DIR |= BIT6;P2OUT &= ~BIT3 


#define IIC0_IO_INIT            P3SEL |= (BIT1+BIT2)    // 配置功能口
#define SD2068_ADDR		        0x32	                // SD2068地址

#define ADDR_SEC	            0x00	                // 秒寄存器地址
#define ADDR_MIN			    0x01	                // 分寄存器地址
#define ADDR_HOUR			    0x02	                // 时寄存器地址
#define ADDR_WEEK			    0x03	                // 周寄存器地址
#define ADDR_DAY			    0x04	                // 日寄存器地址
#define ADDR_MON			    0x05	                // 月寄存器地址
#define ADDR_YEAR			    0x06	                // 年寄存器地址

#define ADDR_ALARM_SEC		    0x07	                // 秒报警寄存器地址
#define ADDR_ALARM_MIN		    0x08	                // 分报警寄存器地址
#define ADDR_ALARM_HOUR		    0x09	                // 时报警寄存器地址
#define ADDR_ALARM_WEEK		    0x0A	                // 周报警寄存器地址
#define ADDR_ALARM_DAY		    0x0B	                // 日报警寄存器地址
#define ADDR_ALARM_MON          0x0C                    // 月报警寄存器地址
#define ADDR_ALARM_YEAR	        0x0D                    // 年报警寄存器地址

#define ADDR_ALARM_ENABLE       0x0E                    // 报警使能寄存器地址

#define ADDR_CTR1   		    0x0F	                // 控制状态寄存器地址1
#define ADDR_CTR2   		    0x10	                // 控制状态寄存器地址2
#define ADDR_CTR3   		    0x11	                // 控制状态寄存器地址3

//#define RTC_INIT_ON             P2DIR &= ~BIT7;P2IFG &= ~BIT7;P2IES |= BIT7;P2IE |= BIT7;
#define RTC_INIT_ON             P2DIR &= ~BIT7;P2REN |= BIT7;P2OUT |= BIT7;P2IFG &= ~BIT7;P2IES |= BIT7;P2IE |= BIT7;
#define RTC_INIT_OFF            P2IE &= ~BIT7
extern uchar RTC_year,RTC_mon,RTC_day,RTC_week,RTC_hour,RTC_min,RTC_sec;
extern uchar RTC_year_temp,RTC_mon_temp,RTC_day_temp,RTC_week_temp,RTC_hour_temp,RTC_min_temp,RTC_sec_temp;
extern uchar BTC_year,BTC_mon,BTC_day,BTC_week,BTC_hour,BTC_min,BTC_sec; 
extern uchar start_year,start_mon,start_day,start_hour,start_min,start_sec; // 启动时间变量
extern uchar time_correction;

extern void I2C0_Init(void);                                                                            // I2C0初始化子函数
extern void I2C0_ByteWrite(uchar TXdata, uchar Addr);                                                   // I2C0写一个字节子函数
extern uchar I2C0_ByteRead(uchar Addr);                                                                 // I2C0读一个字节子函数
extern void Read_time( void );                                                                          // 读时间子函数
extern void Write_time( void );                                                                         // 写时间子函数
extern void Write_alarm( unsigned long int alarm_time );                                                // 写报警子函数
extern void Write_alarm_sec( unsigned long int alarm_time );
extern void Write_timer( unsigned long int timer_time );
extern void Write_takt( unsigned long int takt_time );
extern void Correction_Time(uchar year , uchar mon ,uchar day , uchar hour , uchar min , uchar sec );   // 校时子程序
extern void write_register_ENABLE( void );                                                              // 写寄存器使能子函数
extern void write_register_DISABLE( void );                                                             // 写寄存器禁止子函数
extern void ban_alarm( void );                                                                          // 禁止时钟报警
extern uchar HEX2DEC(uchar bcd);                                                                        // HEX转DEC子函数
extern uchar DEC2HEX(uchar dec);                                                                        // DEC转HEX子函数

#endif