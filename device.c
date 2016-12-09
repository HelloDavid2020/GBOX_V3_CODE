#include "MSP430F5438A.h"
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
#include "system.h"
#include "config.h"
#include "string.h"
//#include "isr.h"
#include "device.h"
#include "key.h"


uchar gps_start = 0;                    //  


//-------------------����������ر���-------------------------------------------
uchar vendor_number;                    // ���̱��
uchar device_number[8];                 // �豸��
uchar hardware;                         // �豸Ӳ���汾
uchar activate_status;                  // �豸״̬
uchar work_mode;                        // ����ģʽ
uchar work_time;                        // ����ʱ��
uint sleep_time;                        // ����ʱ��
uint takt_time;                         // ���ʱ��
uint timing_time = 0xFFFF;              // ��ʱʱ��
uint blind_data_size;                   // ä��������
uint blind_data_head;                   // ä������ͷ��ַ
uchar servers_addr[30];                 // ��������ַ
uint port;                              // �˿�
uint system_rst;                        // ϵͳ��������
volatile uchar work_type;                        // ��������
uchar work_type_flag;                   // ����״̬��־
uchar takt_time_flag;                   // ���ʱ���־
volatile uchar alarm;                   // ����������־
uchar activation_send_num;              // �������ѷ��ʹ��� 
uchar activation_max_send_num;          // �������跢�ʹ���

unsigned long int labor_time;           // �ۼƹ�ʱ
unsigned long int labor_time_start;     // 
unsigned long int labor_time_end;       //


// 123,57,41,22:7001
const uchar config_data[128]={
0x01,                                                                       // ���ұ��         ��ַ��0x0000 - 0x0000
'G','B',0x30,0x30,0x30,0x30,0x30,0x30,                                    // �豸���         ��ַ��0x0001 - 0x0008
0x00,                                                                       // Ӳ���汾��       ��ַ��0x0009 - 0x0009
0x01,                                                                       // �豸״̬         ��ַ��0x000A - 0x000A
0x02,                                                                       // ����ģʽ         ��ַ��0x000B - 0x000B
0x03,                                                                       // ����ʱ��         ��ַ��0x000C - 0x000C
0x00,0x04,                                                                  // ����ʱ��         ��ַ��0x000D - 0x000E
0x00,0x3C,                                                                  // ���ʱ��         ��ַ��0x000F - 0x0010
0xFF,0xFF,                                                                  // ��ʱʱ��         ��ַ��0x0011 - 0x0012
0x00,0x00,0x00,0x00,                                                        // �ۼƹ�ʱ         ��ַ��0x0013 - 0x0016
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ��������ַ       ��ַ��0x0017 --------
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,123,57,41,22,        // ��������ַ       ��ַ��-------- 0x0034    
0x1B,0x59,                                                                  // �������˿ں�     ��ַ��0x0035 - 0x0036
0x00,0x00,                                                                  // ä��������       ��ַ��0x0037 - 0x0038
0x00,0x01,                                                                  // ä������ͷ��ַ   ��ַ��0x0039 - 0x003A
0x00,0x00,                                                                  // �豸��������     ��ַ��0x003B - 0x003C
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,                                    // �豸ά�ޱ��     ��ַ��0x003D - 0x0044
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,                                    // �豸�������κ�   ��ַ��0x0045 - 0x004C
0x00,0x00,                                                                  // �豸��������     ��ַ��0x004D - 0x004E
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // �����ռ�         ��ַ��0x004F --------
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // �����ռ�         ��ַ��---------------
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // �����ռ�         ��ַ��-------- 0x007B
0x00,                                                                       // ����ʱ�ѷ��ʹ��� ��ַ��0x007C - 0x007C
0x0A,                                                                       // ����ʱ�跢�ʹ��� ��ַ��0x007D - 0x007D
0xAA,0x55                                                                   // EEP����          ��ַ��0x007E - 0x007F
};

void show_device_status(void)
{


}


void device_start(void)
{
  uchar buf[100];
//    GPS_POWER_INIT;
//    GPS_POWER_OFF;
//    GPS_POWER_INIT2;
//    GPS_POWER_OFF2;
//    GSM_POWER_INIT;
//    GSM_POWER_OFF;
    PULL_UP_ON;
    //led_start();
    
    SerialPutString("\r\n\r\n");
    SerialPutString("====================================\r\n");
    SerialPutString("Welcom to GPS Tracker.  G-BOXV104 <GCODE_V104_150202d>\r\n");
    eeprom_PageRead(device_number, 0x0001, 8); 
    sprintf(buf,    "Device S/N: %.*s\r\n", 8, device_number);
    SerialPutString( buf);
    SerialPutString("====================================\r\n\r\n");    
    
    uart_flag = 0;//����ܱ�־λ
    memset( config_buf,0x00,sizeof( config_buf ) );//����������
    config_num = 0;//�����������
    UCA3IE |= UCRXIE;           // ������3�����ж�
    
    if( EEPROM_Password != ( (I2C2_ByteRead(126) << 8) | I2C2_ByteRead(127) ) )// �ж��豸�豸�Ƿ�Ϊ�״�����
    {
        __delay_cycles(10000);
        if( EEPROM_Password != ( (I2C2_ByteRead(126) << 8) | I2C2_ByteRead(127) ) )
        {
            printf("\r\n�豸�״�����\r\n");
            printf("\r\n���ڳ�ʼ��ϵͳ����\r\n");
            eeprom_PageWrite( (void*)config_data, 0x0000, 128);
            __delay_cycles(100000);
            RTC_year = 0x0E;
            RTC_mon = 0x01;
            RTC_day = 0x01;
            RTC_hour = 0x00;
            RTC_min = 0x00;
            RTC_sec = 0x00;
            Write_time();
            printf("\r\nϵͳ��ʼ�����\r\n");
        }
    }
    if((I2C0_ByteRead(0x0F) & 0x01) == 0x01)
    {
        printf("\r\n�豸���ϵ磬ʱ����Ϣ��Ч����ʼ��ʼ��ʱ��\r\n");
        RTC_year = 0x0E;
        RTC_mon = 0x01;
        RTC_day = 0x01;
        RTC_hour = 0x00;
        RTC_min = 0x00;
        RTC_sec = 0x00;
        Write_time();
    }
    
    __delay_cycles(10000);
    system_rst = I2C2_ByteRead(0x003B);
    __delay_cycles(10000);
    system_rst = (system_rst << 8) + I2C2_ByteRead(0x003C);
    __delay_cycles(10000);
    system_rst++;
    I2C2_ByteWrite((uchar)(system_rst >> 8), 0x003B);
    __delay_cycles(10000);
    I2C2_ByteWrite((uchar)(system_rst), 0x003C);
    __delay_cycles(10000);

    //check_key();
}

void work_mode_handler(uchar  mode)
{
  switch(mode)
  {            
    case 0x01:// ��׼ģʽ LBS
      lbs_mode();
      break;
    
    case 0x02:// ��׼ģʽ 
       gps_mode();
       break;
     
    case 0x03:// ׷��ģʽ
       tracker_mode();
       break;
    default:
        mode=1;
        printf("\r\n\a����ģʽ�Ƿ�������\r\n");
      break;	
  }
}



void lbs_mode(void)
{
  Write_alarm(work_time);
  RTC_INIT_ON;
  GSM_POWER_INIT;                                                     // GSM��Դ�̳ܽ�ʼ��
  GSM_POWER_ON;                                                       // ����GSM��Դ
  while(1)
  {
      #ifdef WDG
      WDTCTL = WDT_ARST_250;
      #endif
      GSM();
      
      //uscia_config();
      led_toggle();
   
      if(GSM_status == 0xFF && blind_data_size != 0)//���GSM�ڿ���ģʽ�Ҵ���ä����������ä������
      {
          printf("\r\n����ä������\r\n");
          GSM_status = 0x2C;
          data_type = 2;
      }
      if( GSM_status == 0x2C )//����������Ϸ�����
      {
          if(data_type == 0)
          {
              printf("\r\n�����׼��λ����׼���ϴ�\r\n");
          }
          else if(data_type == 1)
          {
              printf("\r\n�����׼��λ����׼���ϴ�\r\n");
          }
          else if(data_type == 2)
          {
              printf("\r\n���ä������׼���ϴ�\r\n");
          }
          else if(data_type == 3)
          {
              printf("\r\n���TCP��Ӧ����׼���ϴ�\r\n");
          }
          pack_data();  
      }
      if(GSM_status == 0xFF && data_send_flag != 0 && blind_data_size == 0) //���GSM�ڿ���ģʽ�Ҷ�λ�����ѷ����Ҳ�����ä����������������
      {
          work_type = 1;      // ���ù�������Ϊ����
          printf("\r\n���ݴ������\r\n");
      }
      if(work_type == 1 && GSM_status != 0x2E && GSM_status != 0x2F && GSM_status !=0x30)
      {
          if(activate_status == 0x02) // ����յ�����������
          {
              printf("\r\n�豸���������ģʽ\r\n");
              activation_send_num = 0;
              I2C2_ByteWrite(0, 0x007C);
              __delay_cycles(100000);
              activation_send_num = 0;
              ban_alarm(); // ��ֹ����
              printf("\r\n׼������\r\n");
              timing_time = 0xFFFF; // �����ʱ��������ֹ��ͻ
              system_sleep();
              break;
          }
          if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// ����豸�ڼ�����
          {
              printf("\r\n�豸���뼤����\r\n");
              activation_send_num++;

              I2C2_ByteWrite(activation_send_num, 0x007C);
               __delay_cycles(100000);
               
              printf("\r\n���������ѷ���%.2d��\r\n",activation_send_num);
              Write_alarm(3);
              system_sleep();
              break;
          }
          if(timing_time != 0xFFFF) // ����յ���ʱ��������
          {
              printf("\r\n�豸���붨ʱ����ģʽ\r\n");
              printf("\r\n׼������\r\n");
              Write_timer(timing_time);
              timing_time = 0xFFFF;
              if(alarm == 0x04)  // ����豸���
              {
                  enter_work3_process(); // ����ǰ���ݴ���
                  work_type = 0; // ���豸Ϊ����״̬
                  break;
              }
              system_sleep();
              break;
          }
          
          if(work_mode == 0x03) // �����׷��ģʽ
          {
              printf("\r\n\aע�⣺�豸��������׷��ģʽ\r\n");
              enter_work3_process(); // ����ǰ���ݴ���
              work_type = 0;
              __delay_cycles(100);
              break;
          }
          else                  // �������׷��ģʽ
          {
              printf("\r\n׼������\r\n"); 
              Write_alarm(sleep_time);
              if(alarm == 0x04)  // ����豸���
              {
                  enter_work3_process(); // ����ǰ���ݴ���
                  work_type = 0; // ���豸Ϊ����״̬
                  break;
              }
              system_sleep();
              break;
          }
      }      
  }   
}


// GPS ����60��󣬿���GSM
void gps_mode(void)
{
  // ����1�������Ӵ��� ---> ����GSM
  //Write_alarm(1);
  RTC_INIT_ON;
   
  // ����GPS
  GPS_POWER_INIT;     // GPS��Դ�ܽų�ʼ��
  GPS_POWER_ON;       // ��GPS��Դ
//  GPS_POWER_INIT2;     // GPS��Դ�ܽų�ʼ��
//  GPS_POWER_ON2;       // ��GPS��Դ
      __delay_cycles(10000);
    __delay_cycles(10000);
    __delay_cycles(10000);

  // ϵͳ����
    //--------------------------------------------------------------------------
  //debug  =1;
  WDTCTL = WDTPW + WDTHOLD;   // �رտ��Ź�
  __delay_cycles(500);
  Read_time();  
  
  gps_start = 0;

  printf("\r\n> Close WatchDog.\r\n");
  printf("\r\n> ������ Enter LPM3 mode. <%d> ������ 20%02d-%02d-%02d %02d:%02d:%02d\r\n",gps_start,RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
  __delay_cycles(500);
  
  LED_OFF;

//  LPM3;   
//  gps_start=1;
//  _NOP();  
//  _NOP();
//  _NOP();
//  _NOP();
//  _NOP();
//  
//  Read_time();
  printf("\r\n> ������ Exit LPM3 mode.  <%d> ������ 20%02d-%02d-%02d %02d:%02d:%02d\r\n",gps_start,RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
//  Write_alarm(work_time);
//  RTC_INIT_ON;
  
//  GSM_POWER_INIT;     // GSM��Դ�ܽų�ʼ��
//  GSM_POWER_ON;       // ��GSM��Դ

  while(1)
  {
//    #ifdef WDG
//    WDTCTL = WDT_ARST_250;
//    #endif
    GSM();
    
    //uscia_config();
    led_toggle();
   if(gpsRxFlag==1)
   {
    gpsRxFlag = 0;
    GPSParse();
    
    if(GpsData.Status == 1 && gps_start ==1)
    {
      // GPS��λ���������
      GPS_analysis_end++;
      if(GPS_analysis_end > 100)
          GPS_analysis_end = 100;
      
      if(GPS_analysis_end > 3)
      {
        // close GPS
        gps_start = 0;
        GPS_POWER_INIT;     // GPS��Դ�ܽų�ʼ��
        GPS_POWER_OFF;       // ��GPS��Դ
        GPS_POWER_INIT2;     // GPS��Դ�ܽų�ʼ��
        GPS_POWER_OFF2;       // ��GPS��Դ
              
      }
#ifdef DEBUG
      LED_ON;
      printf("\r\n \a\a\a GPS�Ѷ�λ %d \a\a\a \r\n",GPS_analysis_end);
#endif      
    }
    
    enable_gps_irq();
   }
    
    if(GPS_analysis_end > GPS_analysis_num && GSM_status == 0xFF && data_send_flag != 2)//���GSM�ڿ���ģʽ��GPS�Ѷ�λ���;�׼��λ����
    {
        printf("\r\nGPS�Ѷ�λ,׼�����;�׼��λ\r\n");
        GSM_status = 0x2C;
        data_type = 1;
    }
    if(time_correction == 0x00 && GPS_analysis_end > GPS_analysis_num )
    {
        printf("\r\nGPS�Ѷ�λ,��ʼУʱ\r\n");
        BTC_sec = UTC_Time.sec;
        BTC_min = UTC_Time.min;
        BTC_hour =UTC_Time.hour + 8 ;
        BTC_day = UTC_Time.day;
        BTC_mon = UTC_Time.mon;
        BTC_year = UTC_Time.year;
        if( BTC_hour >= 24 )
        {
            BTC_hour = BTC_hour - 24;
            BTC_day++;
        }
        if( BTC_mon == 1 || BTC_mon == 3 || BTC_mon == 5 || BTC_mon == 7 || BTC_mon == 8 || BTC_mon == 10 || BTC_mon == 12 )
        {
            if( BTC_day > 31 )
            {
                BTC_day = 1;
                BTC_mon++;
            }
        }
        else if( BTC_mon == 4 || BTC_mon == 6 || BTC_mon == 9 || BTC_mon == 11 )
        {
            if( BTC_day > 30 )
            {
                BTC_day = 1;
                BTC_mon++;
            }
        }
        else if( BTC_mon == 2 )
        {
            if( BTC_year%4 == 0 )
            {
                if( BTC_day > 29)
                {
                    BTC_day = 1;
                    BTC_mon++;
                }
            }
            else 
            {
                if( BTC_day > 28 )
                {
                    BTC_day = 1;
                    BTC_mon++;
                }
            }
        }
        if( BTC_mon > 12 )
        {
            BTC_mon = 1;
            BTC_year++;
        }
        
        Correction_Time(BTC_year,BTC_mon,BTC_day,BTC_hour,BTC_min,BTC_sec);
        time_correction = 1;
    }
    if(GSM_status == 0xFF && blind_data_size != 0)//���GSM�ڿ���ģʽ�Ҵ���ä����������ä������
    {
        printf("\r\n����ä������\r\n");
                                GSM_status = 0x2C;
                                data_type = 2;
    }
    if( GSM_status == 0x2C )//����������Ϸ�����
    {
        if(GPS_analysis_end > GPS_analysis_num && data_type != 3 && data_type != 2)
        {
            data_type = 1;
        }
        if(data_type == 0)
        {
            printf("\r\n�����׼��λ����׼���ϴ�\r\n");
        }
        else if(data_type == 1)
        {
            printf("\r\n�����׼��λ����׼���ϴ�\r\n");
        }
        else if(data_type == 2)
        {
            printf("\r\n���ä������׼���ϴ�\r\n");
        }
        else if(data_type == 3)
        {
            printf("\r\n���TCP��Ӧ����׼���ϴ�\r\n");
        }
        pack_data();  
    }
    if(GSM_status == 0xFF && data_send_flag == 2 && blind_data_size == 0) //���GSM�ڿ���ģʽ�Ҿ�׼��λ�����ѷ����Ҳ�����ä����������������
    {
        work_type = 1;      // ���ù�������Ϊ����
    }
    if(work_type == 1 && GSM_status != 0x2E && GSM_status != 0x2F && GSM_status !=0x30)
    { 
        if(activate_status == 0x02) // ����յ�����������
        {
            printf("\r\n�豸���������ģʽ\r\n");
            activation_send_num = 0;
            I2C2_ByteWrite(0, 0x007C);
            __delay_cycles(100000);
            ban_alarm(); // ��ֹ����
            printf("\r\n׼������\r\n");
            timing_time = 0xFFFF; // �����ʱ��������ֹ��ͻ
            system_sleep();
            break;
        }
        if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// ����豸�ڼ�����
        {
            printf("\r\n�豸���뼤����\r\n");
            activation_send_num++;
            
            I2C2_ByteWrite(activation_send_num, 0x007C);
            __delay_cycles(100000);
            
            printf("\r\n���������ѷ���%.2d��\r\n",activation_send_num);
            Write_alarm(3);
            system_sleep();
            break;
        }
        if(timing_time != 0xFFFF) // ����յ���ʱ��������
        {
            printf("\r\n�豸���붨ʱ����ģʽ\r\n");
            printf("\r\n׼������\r\n");
            Write_timer(timing_time);
            timing_time = 0xFFFF;
            if(alarm == 0x04)  // ����豸���
            {
                enter_work3_process(); // ����ǰ���ݴ���
                work_type = 0; // ���豸Ϊ����״̬
                break;
            }
            system_sleep();
            break;
        }
        
        if(work_mode == 0x03) // �����׷��ģʽ
        {
            printf("\r\n\aע�⣺�豸��������׷��ģʽ\r\n");
            enter_work3_process(); // ����ǰ���ݴ���
            work_type = 0;
            __delay_cycles(100);
            break;
        }
        else                  // �������׷��ģʽ
        {
            printf("\r\n׼������\r\n"); 
            Write_alarm(sleep_time);
            if(alarm == 0x04)  // ����豸���
            {
                enter_work3_process(); // ����ǰ���ݴ���
                work_type = 0; // ���豸Ϊ����״̬
                break;
            }
            system_sleep();
            break;
        } 
    }      
  }
}

void tracker_mode(void)
{
  if(takt_time >= 180) // ���׷��ģʽ���ڵ���3����
  {
    Write_alarm_sec(120);
    RTC_INIT_ON;
    GSM_POWER_INIT;     // GSM��Դ�ܽų�ʼ��
    GSM_POWER_ON;       // ��GSM��Դ
    GPS_POWER_INIT;     // GPS��Դ�ܽų�ʼ��
    GPS_POWER_ON;       // ��GPS��Դ
    GPS_POWER_INIT2;     // GPS��Դ�ܽų�ʼ��
    GPS_POWER_ON2;       // ��GPS��Դ
    while(1)
    {
      #ifdef WDG
      WDTCTL = WDT_ARST_250;
      #endif
       GSM();
        
      //uscia_config();
      led_toggle();
        
        
        if(GPS_analysis_end > GPS_analysis_num && GSM_status == 0xFF && data_send_flag != 2)//���GSM�ڿ���ģʽ��GPS�Ѷ�λ���;�׼��λ����
        {
            printf("\r\nGPS�Ѷ�λ,׼�����;�׼��λ\r\n");
            GSM_status = 0x2C;
            data_type = 1;
        }
        if( GSM_status == 0x2C )//����������Ϸ�����
        {
            if(GPS_analysis_end > 7 && data_type != 3 && data_type != 2)
            {
                data_type = 1;
            }
            if(data_type == 0)
            {
                printf("\r\n�����׼��λ����׼���ϴ�\r\n");
            }
            else if(data_type == 1)
            {
                printf("\r\n�����׼��λ����׼���ϴ�\r\n");
            }
            else if(data_type == 2)
            {
                printf("\r\n���ä������׼���ϴ�\r\n");
            }
            else if(data_type == 3)
            {
                printf("\r\n���TCP��Ӧ����׼���ϴ�\r\n");
            }
            pack_data();  
        }
        if(GSM_status == 0xFF && data_send_flag == 2) //���GSM�ڿ���ģʽ�Ҿ�׼��λ�����ѷ�������������
        {
            work_type = 1;      // ���ù�������Ϊ����
        }
        if(work_type == 1 && GSM_status != 0x2E && GSM_status != 0x2F && GSM_status !=0x30)
        { 
            if(activate_status == 0x02) // ����յ�����������
            {
                printf("\r\n�豸���������ģʽ\r\n");
                activation_send_num = 0;
                I2C2_ByteWrite(0, 0x007C);
                __delay_cycles(100000);
                ban_alarm(); // ��ֹ����
                printf("\r\n׼������\r\n");
                timing_time = 0xFFFF; // �����ʱ��������ֹ��ͻ
                system_sleep();
                break;
            }
            if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// ����豸�ڼ�����
            {
                printf("\r\n�豸���뼤����\r\n");
                
                activation_send_num++;
                
                I2C2_ByteWrite(activation_send_num, 0x007C);
                __delay_cycles(100000);
                
                printf("\r\n���������ѷ���%.2d��\r\n",activation_send_num);
                Write_alarm(3);
                system_sleep();
                break;
            }
            if(timing_time != 0xFFFF) // ����յ���ʱ��������
            {
                printf("\r\n�豸���붨ʱ����ģʽ\r\n");
                printf("\r\n׼������\r\n");
                Write_timer(timing_time);
                timing_time = 0xFFFF;
                if(alarm == 0x04)  // ����豸���
                {
                    enter_work3_process(); // ����ǰ���ݴ���
                    work_type = 0; // ���豸Ϊ����״̬
                    break;
                }
                system_sleep();
                break;
            }
            if(work_mode == 0x03) // �����׷��ģʽ
            {
                if(takt_time >= 180) // ������ʱ����ڵ���3����
                {
                    Write_alarm_sec(takt_time);
                    if(alarm == 0x04)  // ����豸���
                    {
                        enter_work3_process(); // ����ǰ���ݴ���
                        work_type = 0; // ���豸Ϊ����״̬
                        break;
                    }
                    work3_sleep(); 
                    break;
                }
                else                 // ������ʱ��С3����
                {
                    exit_work3_process(); // ����ǰ���ݴ���
                    break;
                }
            }
            else                    // �������׷��ģʽ
            {
                printf("\r\n\aע�⣺�豸�����˳�׷��ģʽ\r\n");
                printf("\r\n׼������\r\n");
                work_type = 1;
                Write_alarm(sleep_time);
                if(alarm == 0x04)  // ����豸���
                {
                    enter_work3_process(); // ����ǰ���ݴ���
                    work_type = 0; // ���豸Ϊ����״̬
                    break;
                }
                system_sleep();
                break;
            }
        }
    }
  }
  else // ���׷��ģʽС��3����
  {
    Write_takt(takt_time); // ���ü��ʱ��
    takt_time_flag = 0;
    RTC_INIT_ON;
    GSM_POWER_INIT;     // GSM��Դ�ܽų�ʼ��
    GSM_POWER_ON;       // ��GSM��Դ
    GPS_POWER_INIT;     // GPS��Դ�ܽų�ʼ��
    GPS_POWER_ON;       // ��GPS��Դ
    GPS_POWER_INIT2;     // GPS��Դ�ܽų�ʼ��
    GPS_POWER_ON2;       // ��GPS��Դ
    while(1)
    {
        #ifdef WDG
        WDTCTL = WDT_ARST_250;
        #endif
        GSM();
        if(GSM_status == 0xFF && takt_time_flag == 1)  // ���GSM�ڿ���ģʽ�Ҽ��ʱ�䵽
        {
            
            
            if(activate_status == 0x02) // ����յ�����������
            {
                printf("\r\n�豸���������ģʽ\r\n");
                activation_send_num = 0;
                I2C2_ByteWrite(0, 0x007C);
                __delay_cycles(100000);
                ban_alarm(); // ��ֹ����
                printf("\r\n׼������\r\n");
                timing_time = 0xFFFF; // �����ʱ��������ֹ��ͻ
                system_sleep();
                break;
            }
            if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// ����豸�ڼ�����
            {
                printf("\r\n�豸���뼤����\r\n");
                activation_send_num++;
                
                I2C2_ByteWrite(activation_send_num, 0x007C);
                __delay_cycles(100000);
                
                printf("\r\n���������ѷ���%.2d��\r\n",activation_send_num);
                Write_alarm(3);
                system_sleep();
                break;
            }
            if(timing_time != 0xFFFF) // ����յ���ʱ��������
            {
                printf("\r\n�豸���붨ʱ����ģʽ\r\n");
                printf("\r\n׼������\r\n");
                Write_timer(timing_time);
                timing_time = 0xFFFF;
                system_sleep();
                break;
            }
            if(work_mode == 0x03) // �����׷��ģʽ
            {
                if(takt_time >= 180) // ������ʱ����ڵ���3����
                {
                    exit_work3_process();
                    work_type = 0;
                    break;
                }
                else                 // ������ʱ��С3����
                {
                    AD_collect();
                    //*****************�湤ʱ***************************
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
                    //******************�湤ʱ����**********************
                    Read_time();
                    start_year = RTC_year;
                    start_mon = RTC_mon;
                    start_day = RTC_day;
                    start_hour = RTC_hour;
                    start_min = RTC_min;
                    start_sec = RTC_sec;
                    Write_takt(takt_time);  // ���ü��ʱ��
                    takt_time_flag = 0;
                    GSM_status = 0x2C;
                }   
            }
            else                  // �������׷��ģʽ
            {
                printf("\r\n\aע�⣺�豸�����˳�׷��ģʽ\r\n");
                printf("\r\n׼������\r\n"); 
                work_type = 1;
                Write_alarm(sleep_time);
                takt_time_flag = 0;
                if(alarm == 0x04)  // ����豸���
                {
                    enter_work3_process(); // ����ǰ���ݴ���
                    work_type = 0; // ���豸Ϊ����״̬
                    break;
                }
                
                system_sleep();
                break;
            }
        }
        if(alarm == 0x04)  // ����豸���
        {
            I2C2_ByteWrite(0x04, 0x000A);
            alarm = 0x00;
            activate_status = 0x04;
        }
        if( GSM_status == 0x2C )//����������Ϸ�����
        {
            if(GPS_analysis_end > 7 && data_type != 3 && data_type != 2)
            {
                data_type = 1;
            }
            if(data_type == 0)
            {
                printf("\r\n�����׼��λ����׼���ϴ�\r\n");
            }
            else if(data_type == 1)
            {
                printf("\r\n�����׼��λ����׼���ϴ�\r\n");
            }
            else if(data_type == 2)
            {
                printf("\r\n���ä������׼���ϴ�\r\n");
            }
            else if(data_type == 3)
            {
                printf("\r\n���TCP��Ӧ����׼���ϴ�\r\n");
            }
            pack_data();  
        }   
    }
  }
}