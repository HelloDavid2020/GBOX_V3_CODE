#include "MSP430F5438A.h"
#include "sd2068.h"
#include "stdio.h"
#include "uart.h"
#include "main.h"
uchar RTC_year,RTC_mon,RTC_day,RTC_week,RTC_hour,RTC_min,RTC_sec;                                       // ����RTCʱ�����
uchar RTC_year_temp,RTC_mon_temp,RTC_day_temp,RTC_week_temp,RTC_hour_temp,RTC_min_temp,RTC_sec_temp;    // ����RTCʱ����ʱ����
uchar alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec;                                    // ���屨��ʱ�����
uchar alarm_hour_temp,alarm_min_temp,alarm_sec_temp;                                                    // ���屨��ʱ����ʱ����
uchar start_year,start_mon,start_day,start_hour,start_min,start_sec;                                    // ��������ʱ�����
uchar BTC_year,BTC_mon,BTC_day,BTC_week,BTC_hour,BTC_min,BTC_sec;                                       // ���山��ʱ�����
unsigned long int RTC_time_data,start_time_data,BTC_time_data,work_time_data;                           
uchar time_correction;                                                                                  // Уʱ�ɹ���־
//-----------------IIC0��ʼ���Ӻ���---------------------------------------------
void I2C0_Init(void)
{
    IIC0_IO_INIT;                               // ���ù��ܿ�
    UCB0CTL1 |= UCSWRST;                        // ʹ�������λ
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // ���ص�ַ7λ�����豸��ַ7λ���������������豸ģʽ��i2cģʽ��ͬ��ģʽ
    UCB0CTL1 |= UCSSEL_2 + UCSWRST;             // USCIʱ��Դѡ��SMCLK=XT2=8M�����������λ
    UCB0BR0 = 73;				                // ��Ƶ������ֵ
    UCB0BR1 = 0;				                // fSCL = SMCLK/UCB1BR0 = 7.3728MHz/30 = ~250kHz
    UCB0I2CSA = SD2068_ADDR;                    // EEPROM��ַ
    UCB0CTL0 &= ~UCSLA10;                   	// ȷ��7λ��ַģʽ
    UCB0CTL1 &= ~UCSWRST;                       // ��������λ���ָ�����
}
//----------------------IIC0дһ���ֽ��Ӻ���------------------------------------
void I2C0_ByteWrite(uchar TXdata, uchar Addr)
{
    __delay_cycles(100000);
    while( UCB0CTL1 & UCTXSTP );                    // ȷ��ֹͣ����û�в���
    UCB0CTL1 |= UCTR;                               // дģʽ
    UCB0CTL1 |= UCTXSTT;                            // ��������λ
    UCB0TXBUF = Addr;                               // ���ʹ��ֽڵ�ַ
    while( !( UCB0IFG & UCTXIFG ) );                // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
    while( UCB0CTL1 & UCTXSTT );                    // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB0TXBUF = TXdata;                             // ��������
    while( !( UCB0IFG & UCTXIFG ) );                // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
    UCB0CTL1 |= UCTXSTP;                            // ����ֹͣλ
    while( UCB0CTL1 & UCTXSTP );                    // �ȴ��������  
}
//-----------------IIC0��һ���ֽ��Ӻ���-----------------------------------------
uchar I2C0_ByteRead(uchar Addr)
{
    uchar re;
    
    __delay_cycles(100000);
    while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
    UCB0CTL1 |= UCTR;                                   // дģʽ
    UCB0CTL1 |= UCTXSTT;                                // ��������λ
    UCB0TXBUF = Addr;                                   // ���ʹ��ֽڵ�ַ
    while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
    while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    
    UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
    UCB0CTL1 |= UCTXSTT;                                // ��������λ
    while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
    while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
    while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������  
    while( !( UCB0IFG & UCRXIFG ) );                    // �յ�����
    re = UCB0RXBUF;                                     // ��ȡ�ֽ�����
    return re;                                          // ��������
}
//---------��ʱ���Ӻ���---------------------------------------------------------

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
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x00;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_sec = HEX2DEC( UCB0RXBUF & 0x7F);               // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_min = HEX2DEC( UCB0RXBUF & 0x7F);               // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_hour = HEX2DEC( UCB0RXBUF & 0x3F);              // ��ȡʱ����
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_week = HEX2DEC( UCB0RXBUF & 0x07);              // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_day = HEX2DEC( UCB0RXBUF & 0x3F);               // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_mon = HEX2DEC( UCB0RXBUF & 0x1F);               // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_year = HEX2DEC( UCB0RXBUF );                    // ��ȡ������
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x00;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_hour_temp = HEX2DEC( UCB0RXBUF & 0x3F);         // ��ȡʱ����
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_week_temp = HEX2DEC( UCB0RXBUF & 0x07);         // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_day_temp = HEX2DEC( UCB0RXBUF & 0x3F);          // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_mon_temp = HEX2DEC( UCB0RXBUF & 0x1F);          // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_year_temp = HEX2DEC( UCB0RXBUF );               // ��ȡ������
    }
    while(RTC_year != RTC_year_temp || RTC_mon != RTC_mon_temp || RTC_day != RTC_day_temp || RTC_hour != RTC_hour_temp || RTC_min != RTC_min_temp);
}
//-----------дʱ���Ӻ���-------------------------------------------------------
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

        write_register_ENABLE();                            // д�Ĵ���ʹ��
        __delay_cycles(10000);
        if(RTC_sec >= 59)
            RTC_sec--;
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x00;                                   // ���ʹ��ֽڵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0TXBUF = DEC2HEX(RTC_sec);                       // ����������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(RTC_min);                       // ���ͷ�����
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = (DEC2HEX(RTC_hour) | 0x80);             // ����ʱ���ݣ�24Сʱֵ|0x80��
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(RTC_week);                      // ����������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(RTC_day);                       // ����������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(RTC_mon);                       // ����������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(RTC_year);                      // ����������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������
        write_register_DISABLE();                           // д�Ĵ�����ֹ
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x00;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);          // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_hour_temp = HEX2DEC( UCB0RXBUF & 0x3F);         // ��ȡʱ����
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_week_temp = HEX2DEC( UCB0RXBUF & 0x07);         // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_day_temp = HEX2DEC( UCB0RXBUF & 0x3F);          // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_mon_temp = HEX2DEC( UCB0RXBUF & 0x1F);          // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        RTC_year_temp = HEX2DEC( UCB0RXBUF );               // ��ȡ������
    }
    while(RTC_year != RTC_year_temp || RTC_mon != RTC_mon_temp || RTC_day != RTC_day_temp || RTC_hour != RTC_hour_temp || RTC_min != RTC_min_temp);
    
}
//-----------д����ʱ�䣨�֣�---------------------------------------------------
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
        write_register_ENABLE();                            // д�Ĵ���ʹ��
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_min);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������
        write_register_DISABLE();                           // д�Ĵ�����ֹ
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // ��ȡʱ����
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // д�Ĵ���ʹ��
    I2C0_ByteWrite(0x07 , 0x0E);                        // д��������Ĵ�����ʱ���룩
    I2C0_ByteWrite(0x92 , 0x10);                        // д���ƼĴ���CTR2 INT�ű����ж���� �������ж�
    write_register_DISABLE();                           // д�Ĵ�����ֹ
    if(work_type == 0)
    {
        printf("\r\n����ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
    else if(work_type == 1)
    {
        printf("\r\n����ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
}
//-----------д����ʱ�䣨�룩---------------------------------------------------
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
        write_register_ENABLE();                            // д�Ĵ���ʹ��
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_min);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������
        write_register_DISABLE();                           // д�Ĵ�����ֹ
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // ��ȡʱ����
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // д�Ĵ���ʹ��
    I2C0_ByteWrite(0x07 , 0x0E);                        // д��������Ĵ�����ʱ���룩
    I2C0_ByteWrite(0x92 , 0x10);                        // д���ƼĴ���CTR2 INT�ű����ж���� �������ж�
    write_register_DISABLE();                           // д�Ĵ�����ֹ
    if(work_type == 0)
    {
        printf("\r\n����ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
    else if(work_type == 1)
    {
        printf("\r\n����ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    }
}
//----------------------д��ʱ����----------------------------------------------
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
        write_register_ENABLE();                            // д�Ĵ���ʹ��
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_min);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������
        write_register_DISABLE();                           // д�Ĵ�����ֹ
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // ��ȡʱ����
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // д�Ĵ���ʹ��
    I2C0_ByteWrite(0x07 , 0x0E);                        // д��������Ĵ�����ʱ���룩
    I2C0_ByteWrite(0x92 , 0x10);                        // д���ƼĴ���CTR2 INT�ű����ж���� �������ж�
    write_register_DISABLE();                           // д�Ĵ�����ֹ

    printf("\r\n��ʱ����ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    
}
//----------------------д���ʱ��----------------------------------------------
void Write_takt( unsigned long int takt_time )
{
    unsigned long int takt_time_temp;

    __delay_cycles(100000);
    Read_time();
    printf("\r\n��ǰRTCʱ��Ϊ��20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
    
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
        write_register_ENABLE();                            // д�Ĵ���ʹ��
        
        //__delay_cycles(100000);
        //I2C0_ByteWrite(0x84,0x0F);
        
        __delay_cycles(10000);
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0TXBUF = DEC2HEX(alarm_sec);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_min);                     // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        UCB0TXBUF = DEC2HEX(alarm_hour);                    // ��������
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������
        write_register_DISABLE();                           // д�Ĵ�����ֹ
        
        __delay_cycles(10000);
        
        while( UCB0CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
        UCB0CTL1 |= UCTR;                                   // дģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        UCB0TXBUF = 0x07;                                   // ���ʹ��ֽڸߵ�ַ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB0TXBUFΪ��ʱUCTXIFGλ����1��
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        UCB0CTL1 &= ~UCTR;                                  // ��ģʽ
        UCB0CTL1 |= UCTXSTT;                                // ��������λ
        while( !( UCB0IFG & UCTXIFG ) );                    // �ȴ��������
        while( UCB0CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
        
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_sec_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_min_temp = HEX2DEC( UCB0RXBUF & 0x7F);        // ��ȡ������
        
        UCB0CTL1 |= UCTXSTP;                                // ����ֹͣλ
        while( UCB0CTL1 & UCTXSTP );                        // �ȴ��������   

        while( !( UCB0IFG & UCRXIFG ) );                    // �ȴ�������ɣ�UCB0RXBUF���յ��������ʱUCRXIFGλ����1��
        alarm_hour_temp = HEX2DEC( UCB0RXBUF );             // ��ȡʱ����
    }
    while(alarm_hour != alarm_hour_temp || alarm_min != alarm_min_temp || alarm_sec != alarm_sec_temp);
    
    __delay_cycles(10000);
    
    write_register_ENABLE();                            // д�Ĵ���ʹ��
    I2C0_ByteWrite(0x07 , 0x0E);                        // д��������Ĵ�����ʱ���룩
    I2C0_ByteWrite(0x92 , 0x10);                        // д���ƼĴ���CTR2 INT�ű����ж���� �������ж� �����Ա���
    write_register_DISABLE();                           // д�Ĵ�����ֹ

    printf("\r\n�´��ϴ�ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",alarm_year,alarm_mon,alarm_day,alarm_hour,alarm_min,alarm_sec);
    
}
//----------------------Уʱ�ӳ���----------------------------------------------
void Correction_Time(uchar year , uchar mon ,uchar day , uchar hour , uchar min , uchar sec )
{
    //******************��ڲ������鿪ʼ****************************************
    if( 14 > year || year > 99 )
    {
        printf("\r\nУʱ�ӳ�����ڲ����Ƿ����˳�Уʱ\r\n");
        return;
    }
    if( 1 > mon || mon > 12 )
    {
        printf("\r\nУʱ�ӳ�����ڲ����Ƿ����˳�Уʱ\r\n");
        return;
    }
    if( mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12 )
    {
        if( 1 > day || day > 31 )
        {
            printf("\r\nУʱ�ӳ�����ڲ����Ƿ����˳�Уʱ\r\n");
            return;
        }
    }
    else if( mon == 4 || mon == 6 || mon == 9 || mon == 11 )
    {
        if( 1 > day || day > 30 )
        {
            printf("\r\nУʱ�ӳ�����ڲ����Ƿ����˳�Уʱ\r\n");
            return;
        }
    }
    else if( mon == 2 )
    {
        if( year%4 == 0 )
        {
            if( 1 > day || day > 29)
            {
                printf("\r\nУʱ�ӳ�����ڲ����Ƿ����˳�Уʱ\r\n");
                return;
            }
        }
        else 
        {
            if( 1 > day || day > 28 )
            {
                printf("\r\nУʱ�ӳ�����ڲ����Ƿ����˳�Уʱ\r\n");
                return;
            }
        }
    }
    //******************��ڲ����������****************************************
    
    printf("\r\n����RTCʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",start_year,start_mon,start_day,start_hour,start_min,start_sec);
    start_time_data = start_hour;
    start_time_data = start_time_data * 3600;
    start_time_data = start_time_data + start_min * 60 + start_sec;
    
    Read_time();                //ȡ��ǰʱ��
    printf("\r\n��ǰRTCʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
    RTC_time_data = RTC_hour;
    RTC_time_data = RTC_time_data * 3600;
    RTC_time_data = RTC_time_data + RTC_min * 60 + RTC_sec;
    
    if(start_time_data > RTC_time_data)
            RTC_time_data += 86400;
    work_time_data = RTC_time_data - start_time_data;
    
    printf("\r\n��ǰ����ʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",year,mon,day,hour,min,sec);
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
    printf("\r\n����ʱ���޸�Ϊ��20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",start_year,start_mon,start_day,start_hour,start_min,start_sec);
    RTC_year = year;
    RTC_mon = mon;
    RTC_day = day;
    RTC_hour = hour;
    RTC_min = min;
    RTC_sec = sec;
    Write_time();
    Read_time();
    printf("\r\nУʱ��ɣ���ǰRTCʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
    if(work_mode != 3 || takt_time >= 180)
    {
        printf("\r\nRTCʱ�䱻������������������ʱ��\r\n");
        Write_alarm(work_time);                //������������ʱ��
    }
    else
    {
        printf("\r\nRTCʱ�䱻���������������´��ϴ�ʱ��\r\n");
        Write_takt(takt_time);                //������������ʱ��
    }
}
//-----------д�Ĵ���ʹ��-------------------------------------------------------
void write_register_ENABLE( void )
{
    __delay_cycles(100000);
    I2C0_ByteWrite(0x80,0x10);
    __delay_cycles(100000);
    I2C0_ByteWrite(0x84,0x0F);
    
}
//-----------д�Ĵ�����ֹ-------------------------------------------------------
void write_register_DISABLE( void )
{
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x0F);
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x10);
    
}
//-----------��ֹ�����Ӻ���-----------------------------------------------------
void ban_alarm( void )
{
    write_register_ENABLE(); // д�Ĵ�������
    __delay_cycles(100000);
    I2C0_ByteWrite(0x00,0x0E); // ��ֹ����
    write_register_DISABLE(); // д�Ĵ�����ֹ
    
}
//-----------�屨����־λ�Ӻ���-------------------------------------------------
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
