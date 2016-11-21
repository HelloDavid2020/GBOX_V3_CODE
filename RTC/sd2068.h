#ifndef __sd2068_H
#define __sd2068_H

#define uchar unsigned char 
#define uint unsigned int

#define PULL_UP_ON     P2DIR |= BIT6;P2OUT |= BIT6 
#define PULL_UP_OFF    P2DIR |= BIT6;P2OUT &= ~BIT3 


#define IIC0_IO_INIT            P3SEL |= (BIT1+BIT2)    // ���ù��ܿ�
#define SD2068_ADDR		        0x32	                // SD2068��ַ

#define ADDR_SEC	            0x00	                // ��Ĵ�����ַ
#define ADDR_MIN			    0x01	                // �ּĴ�����ַ
#define ADDR_HOUR			    0x02	                // ʱ�Ĵ�����ַ
#define ADDR_WEEK			    0x03	                // �ܼĴ�����ַ
#define ADDR_DAY			    0x04	                // �ռĴ�����ַ
#define ADDR_MON			    0x05	                // �¼Ĵ�����ַ
#define ADDR_YEAR			    0x06	                // ��Ĵ�����ַ

#define ADDR_ALARM_SEC		    0x07	                // �뱨���Ĵ�����ַ
#define ADDR_ALARM_MIN		    0x08	                // �ֱ����Ĵ�����ַ
#define ADDR_ALARM_HOUR		    0x09	                // ʱ�����Ĵ�����ַ
#define ADDR_ALARM_WEEK		    0x0A	                // �ܱ����Ĵ�����ַ
#define ADDR_ALARM_DAY		    0x0B	                // �ձ����Ĵ�����ַ
#define ADDR_ALARM_MON          0x0C                    // �±����Ĵ�����ַ
#define ADDR_ALARM_YEAR	        0x0D                    // �걨���Ĵ�����ַ

#define ADDR_ALARM_ENABLE       0x0E                    // ����ʹ�ܼĴ�����ַ

#define ADDR_CTR1   		    0x0F	                // ����״̬�Ĵ�����ַ1
#define ADDR_CTR2   		    0x10	                // ����״̬�Ĵ�����ַ2
#define ADDR_CTR3   		    0x11	                // ����״̬�Ĵ�����ַ3

//#define RTC_INIT_ON             P2DIR &= ~BIT7;P2IFG &= ~BIT7;P2IES |= BIT7;P2IE |= BIT7;
#define RTC_INIT_ON             P2DIR &= ~BIT7;P2REN |= BIT7;P2OUT |= BIT7;P2IFG &= ~BIT7;P2IES |= BIT7;P2IE |= BIT7;
#define RTC_INIT_OFF            P2IE &= ~BIT7
extern uchar RTC_year,RTC_mon,RTC_day,RTC_week,RTC_hour,RTC_min,RTC_sec;
extern uchar RTC_year_temp,RTC_mon_temp,RTC_day_temp,RTC_week_temp,RTC_hour_temp,RTC_min_temp,RTC_sec_temp;
extern uchar BTC_year,BTC_mon,BTC_day,BTC_week,BTC_hour,BTC_min,BTC_sec; 
extern uchar start_year,start_mon,start_day,start_hour,start_min,start_sec; // ����ʱ�����
extern uchar time_correction;

extern void I2C0_Init(void);                                                                            // I2C0��ʼ���Ӻ���
extern void I2C0_ByteWrite(uchar TXdata, uchar Addr);                                                   // I2C0дһ���ֽ��Ӻ���
extern uchar I2C0_ByteRead(uchar Addr);                                                                 // I2C0��һ���ֽ��Ӻ���
extern void Read_time( void );                                                                          // ��ʱ���Ӻ���
extern void Write_time( void );                                                                         // дʱ���Ӻ���
extern void Write_alarm( unsigned long int alarm_time );                                                // д�����Ӻ���
extern void Write_alarm_sec( unsigned long int alarm_time );
extern void Write_timer( unsigned long int timer_time );
extern void Write_takt( unsigned long int takt_time );
extern void Correction_Time(uchar year , uchar mon ,uchar day , uchar hour , uchar min , uchar sec );   // Уʱ�ӳ���
extern void write_register_ENABLE( void );                                                              // д�Ĵ���ʹ���Ӻ���
extern void write_register_DISABLE( void );                                                             // д�Ĵ�����ֹ�Ӻ���
extern void ban_alarm( void );                                                                          // ��ֹʱ�ӱ���
extern uchar HEX2DEC(uchar bcd);                                                                        // HEXתDEC�Ӻ���
extern uchar DEC2HEX(uchar dec);                                                                        // DECתHEX�Ӻ���

#endif