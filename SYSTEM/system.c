#include "MSP430F5438A.h"
#include "system.h"
#include "stdio.h"
#include "uart.h"
#include "main.h"
#include "clock.h"
#include "at24c512.h"
#include "led.h"
#include "ublox.h"
#include "sd2068.h"
#include "time.h"
#include "mg323b.h"
#include "adc.h"
#include "packdata.h"
#include "string.h"
#include "key.h"
//---------------ϵͳ����ǰ���ݴ���---------------------------------------------
void system_start(void)
{
    uchar i = 1;
    uchar err_count = 0;
    uchar j = 29;
    work_type_flag = 0;
    //dco_int();
    Clock_init();

    gpio_init();
    ledInit();
    LED_OFF;

    EEP_POWER_ON;
    PULL_UP_ON;
    I2C2_Init();
    I2C0_Init();
    gps_port_init();
    gsm_port_init();
    debug_port_init(1);
    TimeA0_init();

    AD_init();
    AD_collect();


    #ifdef WDG
    WDTCTL = WDT_ARST_250;
    #endif
    if(activate_status == 0x04 || activate_status == 0x01)
    {
      ALARM_OFF;                // �رչ����ж�
    }
    else
    {
      ALARM_ON;                 // ���������ж�
    }
    
    printf("\r\nϵͳ�����ɹ������ڶ�ȡϵͳ��������\r\n"); 
 
    //******************RTCʱ�Ӽ��鿪ʼ****************************************

    Read_time();
    if( 14 > RTC_year || RTC_year > 99 )
      err_count++;
  
    if( 1 > RTC_mon || RTC_mon > 12 )
    {
      err_count++;
    }
    if( RTC_mon == 1 || RTC_mon == 3 || RTC_mon == 5 || RTC_mon == 7 || RTC_mon == 8 || RTC_mon == 10 || RTC_mon == 12 )
    {
        if( 1 > RTC_day || RTC_day > 31 )
        {
          err_count++;
        }
    }
    else if( RTC_mon == 4 || RTC_mon == 6 || RTC_mon == 9 || RTC_mon == 11 )
    {
        if( 1 > RTC_day || RTC_day > 30 )
        {
          err_count++;
        }
    }
    else if( RTC_mon == 2 )
    {
        if( RTC_year%4 == 0 )
        {
            if( 1 > RTC_day || RTC_day > 29)
            {
              err_count++;
            }
        }
        else 
        {
          if( 1 > RTC_day || RTC_day > 28 )
          {
            err_count++;
          }
        }
    }
    //******************RTCʱ�Ӽ������****************************************
    if(err_count>0)
    {
      printf("\r\nRTCʱ��ʱ��Ƿ�����ʼ��ʼ��ʱ��\r\n");
      RTC_year = 0x0E;
      RTC_mon = 0x01;
      RTC_day = 0x01;
      RTC_hour = 0x00;
      RTC_min = 0x00;
      RTC_sec = 0x00;
      Write_time();    
    }
    start_year = RTC_year;
    start_mon = RTC_mon;
    start_day = RTC_day;
    start_hour = RTC_hour;
    start_min = RTC_min;
    start_sec = RTC_sec;
    
    /*
    printf("\r\nRTCʱ�䣺20%.2d-%.2d-%.2d  %.2d:%.2d:%.2d\r\n",start_year,start_mon,start_day,start_hour,start_min,start_sec);
    vendor_number = I2C2_ByteRead(0x0000);
    printf("\r\n�豸���ұ�ţ�%.2d\r\n",vendor_number);
    
    eeprom_PageRead(device_number, 0x0001, 8); 
    printf("\r\n�豸�ն˱�ţ�%.*s\r\n", 8, device_number);
    
    printf("\r\n�豸����汾�ţ�V%d.%d\r\n",procedure_VER / 10,procedure_VER % 10);
    
    hardware = I2C2_ByteRead(0x0009);
    if(hardware == 0)
    {
        printf("\r\n�豸Ӳ���汾�ţ�δд��\r\n");
    }
    else
    {
        printf("\r\n�豸Ӳ���汾�ţ�V%d.%d\r\n",hardware / 10,hardware % 10);
    }
    */
    if(alarm == 0x03)   // ����豸����
    {
        I2C2_ByteWrite(0x03, 0x000A);
        alarm = 0x00;
    }
    else if(alarm == 0x04)  // ����豸���
    {
        I2C2_ByteWrite(0x04, 0x000A);
        alarm = 0x00;
    }
    __delay_cycles(10000);
    activate_status = I2C2_ByteRead(0x000A);
    if(activate_status == 0x01)
    {
        printf("\r\n�豸����״̬������\r\n");        
    }
    else if(activate_status == 0x02)
    {
        printf("\r\n�豸����״̬��������\r\n");        
    }
    else if(activate_status == 0x03)
    {
        printf("\r\n�豸����״̬���Ѽ���\r\n");        
    }
    else if(activate_status == 0x04)
    {
        printf("\r\n�豸����״̬���Ѳ��\r\n");        
    }
    
    activation_max_send_num = I2C2_ByteRead(0x007D);
    printf("\r\n�豸����ʱ�跢�͵���������%d��\r\n",activation_max_send_num);
    activation_send_num = I2C2_ByteRead(0x007C);
    printf("\r\n�豸�ѷ���%d����������\r\n",activation_send_num);
    if(activation_send_num >= activation_max_send_num)
    {
        printf("\r\n�豸��ִ����ɼ������\r\n");
    }
    else
    {
        printf("\r\n�豸δִ����ɼ������\r\n");
    }
    work_mode = I2C2_ByteRead(0x000B);
    if(work_mode == 0x01)
    {
        printf("\r\n�豸����ģʽ����׼ģʽ\r\n");       
    }
    else if(work_mode == 0x02)
    {
        printf("\r\n�豸����ģʽ����׼ģʽ\r\n");        
    }
    else if(work_mode == 0x03)
    {
        printf("\r\n�豸����ģʽ��׷��ģʽ\r\n");        
    }
    
    work_time = I2C2_ByteRead(0x000C);
    printf("\r\n����ʱ�䣺%dСʱ%d����\r\n",work_time / 60,work_time % 60);
    
    sleep_time = I2C2_ByteRead(0x000D);
    sleep_time = ( sleep_time << 8 ) + I2C2_ByteRead(0x000E);
    printf("\r\n����ʱ�䣺%dСʱ%d����\r\n",sleep_time / 60,sleep_time % 60);
    
    takt_time = I2C2_ByteRead(0x000F);
    takt_time = (takt_time << 8) + I2C2_ByteRead(0x0010);
    printf("\r\n���ʱ�䣺%dСʱ%d����%d��\r\n",takt_time / 3600,takt_time / 60 % 60,takt_time % 60);
    
    //timing_time = I2C2_ByteRead(0x0011);
    //timing_time = ( timing_time << 8 ) + I2C2_ByteRead(0x0012);
    if(timing_time == 0xFFFF)
    {
        printf("\r\n��ʱʱ�䣺δ����\r\n");
    }
    else
    {
        printf("\r\n��ʱʱ�䣺%dʱ%d��\r\n",timing_time / 60,timing_time % 60);
    }
    
    labor_time = I2C2_ByteRead(0x0013);
    labor_time = ( labor_time << 8 ) + I2C2_ByteRead(0x0014);
    labor_time = ( labor_time << 8 ) + I2C2_ByteRead(0x0015);
    labor_time = ( labor_time << 8 ) + I2C2_ByteRead(0x0016);
    printf("\r\n�豸�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
    
    eeprom_PageRead(servers_addr, 0x0017, 30);
    if(servers_addr[0] == 0x01)
    {
        AT_CIPSTART_TCP[18] = '"';
        
        AT_CIPSTART_TCP[19] = servers_addr[26]/100 + 0x30;
        AT_CIPSTART_TCP[20] = (servers_addr[26]%100)/10 + 0x30;
        AT_CIPSTART_TCP[21] = servers_addr[26]%10 + 0x30;
        
        AT_CIPSTART_TCP[22] = '.';
        
        AT_CIPSTART_TCP[23] = servers_addr[27]/100 + 0x30;
        AT_CIPSTART_TCP[24] = (servers_addr[27]%100)/10 + 0x30;
        AT_CIPSTART_TCP[25] = servers_addr[27]%10 + 0x30;
        
        AT_CIPSTART_TCP[26] = '.';
    
        AT_CIPSTART_TCP[27] = servers_addr[28]/100 + 0x30;
        AT_CIPSTART_TCP[28] = (servers_addr[28]%100)/10 + 0x30;
        AT_CIPSTART_TCP[29] = servers_addr[28]%10 + 0x30;
        
        AT_CIPSTART_TCP[30] = '.';
        
        AT_CIPSTART_TCP[31] = servers_addr[29]/100 + 0x30;
        AT_CIPSTART_TCP[32] = (servers_addr[29]%100)/10 + 0x30;
        AT_CIPSTART_TCP[33] = servers_addr[29]%10 + 0x30;

        AT_CIPSTART_TCP[34] = '"';
        AT_CIPSTART_TCP[35] = ',';
        printf("\r\n������IP��ַ��%.3d.%.3d.%.3d.%.3d\r\n",servers_addr[26],servers_addr[27],servers_addr[28],servers_addr[29]);
        
    }
    else if(servers_addr[0] == 0x02)
    {
        AT_CIPSTART_TCP[18] = '"';
        j = 19;
        printf("\r\n������������ַ��");
        for(i = 1; i < 30 ;i++)
        {
            if(servers_addr[i] != 0x00)
            {
                printf("%c",servers_addr[i]);
                AT_CIPSTART_TCP[j] = servers_addr[i];
                j++;
            }
        }
        printf("\r\n");
        AT_CIPSTART_TCP[j] = '"';
        j++;
        AT_CIPSTART_TCP[j] = ',';
        j++;
    }

    port = I2C2_ByteRead(0x0035);
    port = ( port << 8 ) + I2C2_ByteRead(0x0036);
    if(servers_addr[0] == 0x01)
    {
        AT_CIPSTART_TCP[36] = '"';
        AT_CIPSTART_TCP[37] = port/10000 + 0x30;
        AT_CIPSTART_TCP[38] = (port%10000)/1000 + 0x30;
        AT_CIPSTART_TCP[39] = (port%1000)/100 + 0x30;
        AT_CIPSTART_TCP[40] = (port%100)/10 + 0x30;
        AT_CIPSTART_TCP[41] = port%10 + 0x30;
        AT_CIPSTART_TCP[42] = '"';
        AT_CIPSTART_TCP[43] = 0x0D;
        AT_CIPSTART_TCP[44] = 0x0A;        
        
    }
    else if(servers_addr[0] == 0x02)
    {
        AT_CIPSTART_TCP[j] = '"';
        j++;
        AT_CIPSTART_TCP[j] = port/10000 + 0x30;
        j++;
        AT_CIPSTART_TCP[j] = (port%10000)/1000 + 0x30;
        j++;
        AT_CIPSTART_TCP[j] = (port%1000)/100 + 0x30;
        j++;
        AT_CIPSTART_TCP[j] = (port%100)/10 + 0x30;
        j++;
        AT_CIPSTART_TCP[j] = port%10 + 0x30; 
        j++;
        AT_CIPSTART_TCP[j] = '"';
        j++;
        AT_CIPSTART_TCP[j] = 0x0D;
        j++;
        AT_CIPSTART_TCP[j] = 0x0A;
    }
    printf("\r\n�������˿ں�Ϊ��%.5d\r\n",port);
    
    blind_data_size = I2C2_ByteRead(0x0037);
    blind_data_size = ( blind_data_size << 8 ) + I2C2_ByteRead(0x0038);
    printf("\r\n�豸ä����������%d\r\n",blind_data_size);
    
    blind_data_head = I2C2_ByteRead(0x0039);
    blind_data_head = ( blind_data_head << 8 ) + I2C2_ByteRead(0x003A);
    printf("\r\n�豸ä������ͷ��ַ��%d\r\n",blind_data_head);
    
    system_rst = I2C2_ByteRead(0x003B);
    system_rst = ( system_rst << 8 ) + I2C2_ByteRead(0x003C);
    printf("\r\n�豸����������%d\r\n",system_rst);
    
    printf("\r\nϵͳ������ȡ���\r\n");

}
//---------------ϵͳ����ǰ���ݴ���---------------------------------------------
void system_sleep(void)
{
    uscia2_send( (void*)AT_SISC_1 );                                    //�رշ���1
    printf("\r\n�ر�GSM����\r\n");
    __delay_cycles(100000);
    
    GSM_POWER_OFF;      // ��GSM��Դ
    GPS_POWER_OFF;      // ��GPS��Դ
    GPS_POWER_OFF2;      // ��GPS��Դ
    
    if(data_send_flag == 0)//���û�з��ͳɹ�����洢ä������
    {
        printf("\r\nˢ������\r\n");
        pack_data();//ˢ������׼���洢
        eeprom_PageWrite( (void*)data_packet, (128*blind_data_head), 99);
        printf("\r\n�洢ä������\r\n");
        blind_data_size++;
        if(blind_data_size > 500)//���ֻ�ܴ洢500��ä������
            blind_data_size = 500;
        printf("\r\nä����������%d\r\n",blind_data_size);
        blind_data_head++;
        if(blind_data_head > 500)
            blind_data_head = 1;
    }
    I2C2_ByteWrite((uchar)(blind_data_size >> 8), 0x0037);      //��ä�����������ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_size), 0x0038);           //��ä�����������ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_head >> 8), 0x0039);      //��ä��ָ����ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_head), 0x003A);           //��ä��ָ����ֽ�
    __delay_cycles(100000);
        
    labor_time_start = start_hour;
    labor_time_start = labor_time_start * 3600;
    labor_time_start = labor_time_start + start_min * 60 + start_sec;
    //--------------------------------------------------------------------------
    Read_time();
    labor_time_end = RTC_hour;
    labor_time_end = labor_time_end * 3600;
    labor_time_end = labor_time_end + RTC_min * 60 + RTC_sec;
    //--------------------------------------------------------------------------
    if(labor_time_start > labor_time_end)
        labor_time_end += 86400;
    labor_time = labor_time + (labor_time_end - labor_time_start);
    printf("\r\n�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
    
    I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);          //�洢��ʱͳ������
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time), 0x0016);                
    __delay_cycles(100000);
    
    RTC_INIT_ON;
    
    work_type = 0x01;   // ����ǰ�幤��״̬
    GSM_status = 0x00;  // ����ǰ��GSM״̬
    data_type = 0x00;   // ������������״̬
    GPS_analysis_end = 0x00;    // ����ǰ��GPS��������
    GPS_Locate = 'V';       // ����ǰ��GPS��λ��־
    GPS_north_south = 'X';  // ����ǰ���ϱ�γ��־
    GPS_east_west = 'X';    // ����ǰ�嶫������־
    data_send_flag = 0x00;  // ����ǰ�巢�ͳɹ���־
    GPS_start = 0x00;       // ����ǰ��GPS��ʼ������־
    //timing_time = 0xFFFF;   // ����ǰ�嶨ʱʱ��
    time_correction = 0x00; // ����ǰ��Уʱ�ɹ���־
    
    memset(GPS_UTC_time,0x00,sizeof(GPS_UTC_time));     // ����ǰ��GPSʱ��
    memset(GPS_UTC_date,0x00,sizeof(GPS_UTC_date));     // ����ǰ��GPS����
    memset(GPS_latitude,0x00,sizeof(GPS_latitude));     // ����ǰ��GPSγ��
    memset(GPS_Longitude,0x00,sizeof(GPS_Longitude));   // ����ǰ��GPS����
    memset(GPS_Speed,0x00,sizeof(GPS_Speed));           // ����ǰ��GPS�ٶ�
    memset(GPS_direction,0x00,sizeof(GPS_direction));   // ����ǰ��GPS����
    memset(GPS_elevation,0x00,sizeof(GPS_elevation));   // ����ǰ��GPS�߶�
    memset(GPS_employ_satellite,0x00,sizeof(GPS_employ_satellite));     // ����ǰ��GPSʹ�����ǿ���
    memset(GPS_visible_satellite,0x00,sizeof(GPS_visible_satellite));   // ����ǰ��GPS�ɼ����ǿ���
    memset(data_packet,0x00,sizeof(data_packet));       // 
    
    //�͹��Ĵ���
    //--------------------------------------------------------------------------
    printf("\r\n> Close WatchDog.\r\n");
    WDTCTL = WDTPW + WDTHOLD;   // �رտ��Ź�
    __delay_cycles(500);
    
    printf("> Gpio Reinit.\r\n");
    printf("> Enter Power Save Mode.\r\n");

    P1SEL = 0x00;
    P2SEL = 0x00;
    P3SEL = 0x00;
    P4SEL = 0x00;
    P5SEL = 0x00;
    P6SEL = 0x00;
    P7SEL = 0x00;
    P8SEL = 0x00;
    P9SEL = 0x00;
    P10SEL = 0x00;
    P11SEL = 0x00;
    
    RTC_INIT_OFF;
    P1OUT = 0x00;
    P2OUT = 0x00;
    P3OUT = 0x00;
    P4OUT = 0x00;
    P5OUT = 0x00;
    P6OUT = 0x00;
    P7OUT = 0x00;
    P8OUT = 0x00;
    P9OUT = 0x00;
    P10OUT = 0x00;
    P11OUT = 0x00;
    PJOUT = 0x00;
    RTC_INIT_ON;
    
    
    
    GPS_POWER_OFF;
    GPS_POWER_OFF2;
    GSM_POWER_OFF;
    
    ADC12CTL0 &= ~ADC12ENC;
    REFCTL0 &=  ~REFON;
    
    //P9OUT |= (BIT0 + BIT1 + BIT2 + BIT3); // pull up ! 
    P9OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3); // pull up ! 

    
    P2OUT |= BIT7; // pull up ! RTC_INT  
    P6OUT |= BIT1; // pull up ! KEY_IN

    P2OUT &= ~BIT6; // pull up ! i2c rtc
    P3OUT &= ~(BIT1 + BIT2); // pull up ! i2c rtc

    // ���ڶ˿�����
    //P9DIR &= (BIT4 + BIT5);   // gsm com port
    //P3DIR &= (BIT4 + BIT5);   // gps com port
    //P10DIR &= (BIT4 + BIT5);  // debug com port
    
    P8OUT &= ~BIT0; // gsm_rst
    P7OUT &= ~BIT3; // gsm_turm_on
    
    P1DIR = 0xDF;
    P2DIR = 0xFF;
    P3DIR = 0xFF;
    P4DIR = 0xFF;
    P5DIR = 0xFF;
    P6DIR = 0xFF;
    P7DIR = 0xFF;
    P8DIR = 0xFF;
    P9DIR = 0xFF;
    P10DIR = 0xFF;
    P11DIR = 0xFF;
    PJDIR = 0xFF;

    
    P9OUT  &= ~ (BIT4 + BIT5);   // gsm com port
    P3OUT  &= ~ (BIT4 + BIT5);   // gps com port
    P10OUT &= ~ (BIT4 + BIT5);  // debug com port   
    
    P5OUT  &= ~ BIT4;   // led
    P2OUT  &= ~ BIT1;   // led
    
    P6OUT &= ~(BIT4 + BIT5);// AD 
    
    __delay_cycles(10000);
    EXT_INT_INIT;               // ���ⲿ�ж�
    if(activate_status == 0x04 || activate_status == 0x01)
    {
        ALARM_OFF;                // �رչ����ж�
    }
    else
    {
         ALARM_ON;                 // ���������ж�
    } 
    RTC_INIT_ON;                // ��RTC�ж�
    UCSCTL4 = SELM_1 + SELS_1 + SELA_1;       // MCLK = SMCLK = ACLK = VLO
    UCSCTL6 |= XT1OFF;         // �ر�XT1����
    UCSCTL6 |= XT2OFF;         // �ر�XT2����
    work_type_flag = 1;
    
    // ������ָ�8MHz@0.125uS
    _NOP();  
    _NOP();
    _NOP();
    _NOP();
    _NOP();
    _BIS_SR( LPM4_bits + GIE);// ����LPM4�����ж�
    //--------------------------------------------------------------------------
}
//-------------------ģʽ3���ߣ�GPS���õ�Դ�򿪣�-------------------------------
void work3_sleep(void)
{
  #ifdef WDG
  WDTCTL = WDT_ARST_250;
  #endif
    
    uscia2_send( (void*)AT_SISC_1 );                                    //�رշ���1
    printf("\r\n�ر�GSM����\r\n");
    __delay_cycles(100000);
    
    GSM_POWER_OFF;      // ��GSM��Դ
    GPS_POWER_ON;      // ��GPS��Դ
    GPS_POWER_OFF;      // ��GPS����Դ
    
    if(data_send_flag == 0)//���û�з��ͳɹ�����洢ä������
    {
        printf("\r\nˢ������\r\n");
        pack_data();//ˢ������׼���洢
        eeprom_PageWrite( (void*)data_packet, (128*blind_data_head), 99);
        printf("\r\n�洢ä������\r\n");
        blind_data_size++;
        if(blind_data_size > 500)//���ֻ�ܴ洢500��ä������
            blind_data_size = 500;
        printf("\r\nä����������%d\r\n",blind_data_size);
        blind_data_head++;
        if(blind_data_head > 500)
            blind_data_head = 1;
    }
    I2C2_ByteWrite((uchar)(blind_data_size >> 8), 0x0037);      //��ä�����������ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_size), 0x0038);           //��ä�����������ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_head >> 8), 0x0039);      //��ä��ָ����ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_head), 0x003A);           //��ä��ָ����ֽ�
    __delay_cycles(100000);
        
    labor_time_start = start_hour;
    labor_time_start = labor_time_start * 3600;
    labor_time_start = labor_time_start + start_min * 60 + start_sec;
    //--------------------------------------------------------------------------
    Read_time();
    labor_time_end = RTC_hour;
    labor_time_end = labor_time_end * 3600;
    labor_time_end = labor_time_end + RTC_min * 60 + RTC_sec;
    //--------------------------------------------------------------------------
    if(labor_time_start > labor_time_end)
        labor_time_end += 86400;
    labor_time = labor_time + (labor_time_end - labor_time_start);
    printf("\r\n�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
    
    I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);          //�洢��ʱͳ������
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time), 0x0016);                
    __delay_cycles(100000);
    
    RTC_INIT_ON;
    
    work_type = 0x01;   // ����ǰ�幤��״̬
    GSM_status = 0x00;  // ����ǰ��GSM״̬
    data_type = 0x00;   // ������������״̬
    GPS_analysis_end = 0x00;    // ����ǰ��GPS��������
    GPS_Locate = 'V';       // ����ǰ��GPS��λ��־
    GPS_north_south = 'X';  // ����ǰ���ϱ�γ��־
    GPS_east_west = 'X';    // ����ǰ�嶫������־
    data_send_flag = 0x00;  // ����ǰ�巢�ͳɹ���־
    GPS_start = 0x00;       // ����ǰ��GPS��ʼ������־
    //timing_time = 0xFFFF;   // ����ǰ�嶨ʱʱ��
    time_correction = 0x00; // ����ǰ��Уʱ�ɹ���־
    
    memset(GPS_UTC_time,0x00,sizeof(GPS_UTC_time));     // ����ǰ��GPSʱ��
    memset(GPS_UTC_date,0x00,sizeof(GPS_UTC_date));     // ����ǰ��GPS����
    memset(GPS_latitude,0x00,sizeof(GPS_latitude));     // ����ǰ��GPSγ��
    memset(GPS_Longitude,0x00,sizeof(GPS_Longitude));   // ����ǰ��GPS����
    memset(GPS_Speed,0x00,sizeof(GPS_Speed));           // ����ǰ��GPS�ٶ�
    memset(GPS_direction,0x00,sizeof(GPS_direction));   // ����ǰ��GPS����
    memset(GPS_elevation,0x00,sizeof(GPS_elevation));   // ����ǰ��GPS�߶�
    memset(GPS_employ_satellite,0x00,sizeof(GPS_employ_satellite));     // ����ǰ��GPSʹ�����ǿ���
    memset(GPS_visible_satellite,0x00,sizeof(GPS_visible_satellite));   // ����ǰ��GPS�ɼ����ǿ���
    memset(data_packet,0x00,sizeof(data_packet));       // 
    
    //�͹��Ĵ���
    //--------------------------------------------------------------------------
    printf("\r\nϵͳ����\r\n");
    WDTCTL = WDTPW + WDTHOLD;   // �رտ��Ź�
    __delay_cycles(500);
    P1SEL = 0x00;
    P2SEL = 0x00;
    P3SEL = 0x00;
    P4SEL = 0x00;
    P5SEL = 0x00;
    P6SEL = 0x00;
    P7SEL = 0x00;
    P8SEL = 0x00;
    P9SEL = 0x00;
    P10SEL = 0x00;
    P11SEL = 0x00;
    
    RTC_INIT_OFF;
    P1OUT = 0x00;
    P2OUT = 0x04;
    P3OUT = 0x00;
    P4OUT = 0x00;
    P5OUT = 0x00;
    P6OUT = 0x00;
    P7OUT = 0x00;
    P8OUT = 0x00;
    P9OUT = 0x00;
    P10OUT = 0x00;
    P11OUT = 0x00;
    PJOUT = 0x00;
    RTC_INIT_ON;
    
    
    
    GPS_POWER_ON;
    GPS_POWER_OFF2;
    GSM_POWER_OFF;
    
    ADC12CTL0 &= ~ADC12ENC;
    REFCTL0 &=  ~REFON;
    
    P9OUT |= (BIT0 + BIT1 + BIT2);
    P2OUT |= (BIT7 + BIT1);
    P3OUT |= (BIT1 + BIT2);
    
    P6OUT |= (BIT4 + BIT5);
    
    //P9DIR &= (BIT4 + BIT5);
    //P3DIR &= (BIT4 + BIT5);
    //P10DIR &= (BIT4 + BIT5);
    
    P8OUT &= ~BIT0;
    P7OUT &= ~BIT3;
    
    P1DIR = 0xFF;
    P2DIR = 0xFF;
    P3DIR = 0xFF;
    P4DIR = 0xFF;
    P5DIR = 0xFF;
    P6DIR = 0xFF;
    P7DIR = 0xFF;
    P8DIR = 0xFF;
    P9DIR = 0xFF;
    P10DIR = 0xFF;
    P11DIR = 0xFF;
    PJDIR = 0xFF;
    
    EXT_INT_INIT;               // ���ⲿ�ж�
    if(activate_status == 0x04 || activate_status == 0x01)
    {
        ALARM_OFF;                // �رչ����ж�
    }
    else
    {
        ALARM_ON;                 // ���������ж�
    } 
    RTC_INIT_ON;                // ��RTC�ж�
    UCSCTL4 = SELM_1 + SELS_1 + SELA_1;       // MCLK = SMCLK = ACLK = VLO
    UCSCTL6 |= XT1OFF;         // �ر�XT1����
    UCSCTL6 |= XT2OFF;         // �ر�XT2����
    work_type_flag = 1;
    
    _BIS_SR( LPM4_bits + GIE);// ����LPM4�����ж�
    
    
    
}

void enter_work3_process(void)
{
#ifdef WDG
WDTCTL = WDT_ARST_250;
#endif
    GSM_POWER_OFF;      // ��GSM��Դ
    GPS_POWER_ON;      // ��GPS��Դ
    GPS_POWER_OFF;      // ��GPS����Դ
    
    if(data_send_flag == 0)//���û�з��ͳɹ�����洢ä������
    {
        printf("\r\nˢ������\r\n");
        pack_data();//ˢ������׼���洢
        eeprom_PageWrite( (void*)data_packet, (128*blind_data_head), 99);
        printf("\r\n�洢ä������\r\n");
        blind_data_size++;
        if(blind_data_size > 500)//���ֻ�ܴ洢500��ä������
            blind_data_size = 500;
        printf("\r\nä����������%d\r\n",blind_data_size);
        blind_data_head++;
        if(blind_data_head > 500)
            blind_data_head = 1;
    }
    I2C2_ByteWrite((uchar)(blind_data_size >> 8), 0x0037);      //��ä�����������ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_size), 0x0038);           //��ä�����������ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_head >> 8), 0x0039);      //��ä��ָ����ֽ�
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(blind_data_head), 0x003A);           //��ä��ָ����ֽ�
    __delay_cycles(100000);
        
    labor_time_start = start_hour;
    labor_time_start = labor_time_start * 3600;
    labor_time_start = labor_time_start + start_min * 60 + start_sec;
    //--------------------------------------------------------------------------
    Read_time();
    labor_time_end = RTC_hour;
    labor_time_end = labor_time_end * 3600;
    labor_time_end = labor_time_end + RTC_min * 60 + RTC_sec;
    //--------------------------------------------------------------------------
    if(labor_time_start > labor_time_end)
        labor_time_end += 86400;
    labor_time = labor_time + (labor_time_end - labor_time_start);
    printf("\r\n�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
    
    I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);          //�洢��ʱͳ������
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time), 0x0016);                
    __delay_cycles(100000);
    
    RTC_INIT_ON;
    
    work_type = 0x01;   // ����ǰ�幤��״̬
    GSM_status = 0x00;  // ����ǰ��GSM״̬
    data_type = 0x00;   // ������������״̬
    GPS_analysis_end = 0x00;    // ����ǰ��GPS��������
    GPS_Locate = 'V';       // ����ǰ��GPS��λ��־
    GPS_north_south = 'X';  // ����ǰ���ϱ�γ��־
    GPS_east_west = 'X';    // ����ǰ�嶫������־
    data_send_flag = 0x00;  // ����ǰ�巢�ͳɹ���־
    GPS_start = 0x00;       // ����ǰ��GPS��ʼ������־
    //timing_time = 0xFFFF;   // ����ǰ�嶨ʱʱ��
    time_correction = 0x00; // ����ǰ��Уʱ�ɹ���־
    
    memset(GPS_UTC_time,0x00,sizeof(GPS_UTC_time));     // ����ǰ��GPSʱ��
    memset(GPS_UTC_date,0x00,sizeof(GPS_UTC_date));     // ����ǰ��GPS����
    memset(GPS_latitude,0x00,sizeof(GPS_latitude));     // ����ǰ��GPSγ��
    memset(GPS_Longitude,0x00,sizeof(GPS_Longitude));   // ����ǰ��GPS����
    memset(GPS_Speed,0x00,sizeof(GPS_Speed));           // ����ǰ��GPS�ٶ�
    memset(GPS_direction,0x00,sizeof(GPS_direction));   // ����ǰ��GPS����
    memset(GPS_elevation,0x00,sizeof(GPS_elevation));   // ����ǰ��GPS�߶�
    memset(GPS_employ_satellite,0x00,sizeof(GPS_employ_satellite));     // ����ǰ��GPSʹ�����ǿ���
    memset(GPS_visible_satellite,0x00,sizeof(GPS_visible_satellite));   // ����ǰ��GPS�ɼ����ǿ���
    memset(data_packet,0x00,sizeof(data_packet));       // 
}

void exit_work3_process(void)
{
  #ifdef WDG
  WDTCTL = WDT_ARST_250;
  #endif
    GSM_POWER_OFF;      // ��GSM��Դ
    GPS_POWER_ON;      // ��GPS��Դ
    GPS_POWER_OFF;      // ��GPS����Դ
        
    labor_time_start = start_hour;
    labor_time_start = labor_time_start * 3600;
    labor_time_start = labor_time_start + start_min * 60 + start_sec;
    //--------------------------------------------------------------------------
    Read_time();
    labor_time_end = RTC_hour;
    labor_time_end = labor_time_end * 3600;
    labor_time_end = labor_time_end + RTC_min * 60 + RTC_sec;
    //--------------------------------------------------------------------------
    if(labor_time_start > labor_time_end)
        labor_time_end += 86400;
    labor_time = labor_time + (labor_time_end - labor_time_start);
    printf("\r\n�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
    
    I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);          //�洢��ʱͳ������
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);                
    __delay_cycles(100000);
    I2C2_ByteWrite((uchar)(labor_time), 0x0016);                
    __delay_cycles(100000);
    
    RTC_INIT_ON;
    
    work_type = 0x01;   // ����ǰ�幤��״̬
    GSM_status = 0x00;  // ����ǰ��GSM״̬
    data_type = 0x00;   // ������������״̬
    GPS_analysis_end = 0x00;    // ����ǰ��GPS��������
    GPS_Locate = 'V';       // ����ǰ��GPS��λ��־
    GPS_north_south = 'X';  // ����ǰ���ϱ�γ��־
    GPS_east_west = 'X';    // ����ǰ�嶫������־
    data_send_flag = 0x00;  // ����ǰ�巢�ͳɹ���־
    GPS_start = 0x00;       // ����ǰ��GPS��ʼ������־
    //timing_time = 0xFFFF;   // ����ǰ�嶨ʱʱ��
    time_correction = 0x00; // ����ǰ��Уʱ�ɹ���־
    
    memset(GPS_UTC_time,0x00,sizeof(GPS_UTC_time));     // ����ǰ��GPSʱ��
    memset(GPS_UTC_date,0x00,sizeof(GPS_UTC_date));     // ����ǰ��GPS����
    memset(GPS_latitude,0x00,sizeof(GPS_latitude));     // ����ǰ��GPSγ��
    memset(GPS_Longitude,0x00,sizeof(GPS_Longitude));   // ����ǰ��GPS����
    memset(GPS_Speed,0x00,sizeof(GPS_Speed));           // ����ǰ��GPS�ٶ�
    memset(GPS_direction,0x00,sizeof(GPS_direction));   // ����ǰ��GPS����
    memset(GPS_elevation,0x00,sizeof(GPS_elevation));   // ����ǰ��GPS�߶�
    memset(GPS_employ_satellite,0x00,sizeof(GPS_employ_satellite));     // ����ǰ��GPSʹ�����ǿ���
    memset(GPS_visible_satellite,0x00,sizeof(GPS_visible_satellite));   // ����ǰ��GPS�ɼ����ǿ���
    memset(data_packet,0x00,sizeof(data_packet));       // 
}