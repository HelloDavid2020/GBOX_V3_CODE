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


//-------------------工作参数相关变量-------------------------------------------
uchar vendor_number;                    // 厂商编号
uchar device_number[8];                 // 设备号
uchar hardware;                         // 设备硬件版本
uchar activate_status;                  // 设备状态
uchar work_mode;                        // 工作模式
uchar work_time;                        // 工作时间
uint sleep_time;                        // 休眠时间
uint takt_time;                         // 间隔时间
uint timing_time = 0xFFFF;              // 定时时间
uint blind_data_size;                   // 盲区数据量
uint blind_data_head;                   // 盲区数据头地址
uchar servers_addr[30];                 // 服务器地址
uint port;                              // 端口
uint system_rst;                        // 系统重启次数
volatile uchar work_type;                        // 工作类型
uchar work_type_flag;                   // 工作状态标志
uchar takt_time_flag;                   // 间隔时间标志
volatile uchar alarm;                   // 光敏报警标志
uchar activation_send_num;              // 激活期已发送次数 
uchar activation_max_send_num;          // 激活期需发送次数

unsigned long int labor_time;           // 累计工时
unsigned long int labor_time_start;     // 
unsigned long int labor_time_end;       //


// 123,57,41,22:7001
const uchar config_data[128]={
0x01,                                                                       // 厂家编号         地址：0x0000 - 0x0000
'G','B',0x30,0x30,0x30,0x30,0x30,0x30,                                    // 设备编号         地址：0x0001 - 0x0008
0x00,                                                                       // 硬件版本号       地址：0x0009 - 0x0009
0x01,                                                                       // 设备状态         地址：0x000A - 0x000A
0x02,                                                                       // 工作模式         地址：0x000B - 0x000B
0x03,                                                                       // 工作时间         地址：0x000C - 0x000C
0x00,0x04,                                                                  // 休眠时间         地址：0x000D - 0x000E
0x00,0x3C,                                                                  // 间隔时间         地址：0x000F - 0x0010
0xFF,0xFF,                                                                  // 定时时间         地址：0x0011 - 0x0012
0x00,0x00,0x00,0x00,                                                        // 累计工时         地址：0x0013 - 0x0016
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 服务器地址       地址：0x0017 --------
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,123,57,41,22,        // 服务器地址       地址：-------- 0x0034    
0x1B,0x59,                                                                  // 服务器端口号     地址：0x0035 - 0x0036
0x00,0x00,                                                                  // 盲区数据量       地址：0x0037 - 0x0038
0x00,0x01,                                                                  // 盲区数据头地址   地址：0x0039 - 0x003A
0x00,0x00,                                                                  // 设备启动次数     地址：0x003B - 0x003C
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,                                    // 设备维修编号     地址：0x003D - 0x0044
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,                                    // 设备生产批次号   地址：0x0045 - 0x004C
0x00,0x00,                                                                  // 设备启动次数     地址：0x004D - 0x004E
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 保留空间         地址：0x004F --------
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 保留空间         地址：---------------
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 保留空间         地址：-------- 0x007B
0x00,                                                                       // 激活时已发送次数 地址：0x007C - 0x007C
0x0A,                                                                       // 激活时需发送次数 地址：0x007D - 0x007D
0xAA,0x55                                                                   // EEP密码          地址：0x007E - 0x007F
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
    
    uart_flag = 0;//清接受标志位
    memset( config_buf,0x00,sizeof( config_buf ) );//缓存区清零
    config_num = 0;//清接受数据量
    UCA3IE |= UCRXIE;           // 开串口3接受中断
    
    if( EEPROM_Password != ( (I2C2_ByteRead(126) << 8) | I2C2_ByteRead(127) ) )// 判断设备设备是否为首次启动
    {
        __delay_cycles(10000);
        if( EEPROM_Password != ( (I2C2_ByteRead(126) << 8) | I2C2_ByteRead(127) ) )
        {
            printf("\r\n设备首次启动\r\n");
            printf("\r\n正在初始化系统参数\r\n");
            eeprom_PageWrite( (void*)config_data, 0x0000, 128);
            __delay_cycles(100000);
            RTC_year = 0x0E;
            RTC_mon = 0x01;
            RTC_day = 0x01;
            RTC_hour = 0x00;
            RTC_min = 0x00;
            RTC_sec = 0x00;
            Write_time();
            printf("\r\n系统初始化完成\r\n");
        }
    }
    if((I2C0_ByteRead(0x0F) & 0x01) == 0x01)
    {
        printf("\r\n设备曾断电，时钟信息无效，开始初始化时钟\r\n");
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
    case 0x01:// 标准模式 LBS
      lbs_mode();
      break;
    
    case 0x02:// 精准模式 
       gps_mode();
       break;
     
    case 0x03:// 追车模式
       tracker_mode();
       break;
    default:
        mode=1;
        printf("\r\n\a工作模式非法！！！\r\n");
      break;	
  }
}



void lbs_mode(void)
{
  Write_alarm(work_time);
  RTC_INIT_ON;
  GSM_POWER_INIT;                                                     // GSM电源管教初始化
  GSM_POWER_ON;                                                       // 开启GSM电源
  while(1)
  {
      #ifdef WDG
      WDTCTL = WDT_ARST_250;
      #endif
      GSM();
      
      //uscia_config();
      led_toggle();
   
      if(GSM_status == 0xFF && blind_data_size != 0)//如果GSM在空闲模式且存在盲区数据则发送盲区数据
      {
          printf("\r\n发送盲区数据\r\n");
          GSM_status = 0x2C;
          data_type = 2;
      }
      if( GSM_status == 0x2C )//如果已连接上服务器
      {
          if(data_type == 0)
          {
              printf("\r\n打包标准定位数据准备上传\r\n");
          }
          else if(data_type == 1)
          {
              printf("\r\n打包精准定位数据准备上传\r\n");
          }
          else if(data_type == 2)
          {
              printf("\r\n打包盲区数据准备上传\r\n");
          }
          else if(data_type == 3)
          {
              printf("\r\n打包TCP响应数据准备上传\r\n");
          }
          pack_data();  
      }
      if(GSM_status == 0xFF && data_send_flag != 0 && blind_data_size == 0) //如果GSM在空闲模式且定位数据已发送且不存在盲区数据则主动休眠
      {
          work_type = 1;      // 设置工作类型为休眠
          printf("\r\n数据处理完毕\r\n");
      }
      if(work_type == 1 && GSM_status != 0x2E && GSM_status != 0x2F && GSM_status !=0x30)
      {
          if(activate_status == 0x02) // 如果收到待激活命令
          {
              printf("\r\n设备进入待激活模式\r\n");
              activation_send_num = 0;
              I2C2_ByteWrite(0, 0x007C);
              __delay_cycles(100000);
              activation_send_num = 0;
              ban_alarm(); // 禁止报警
              printf("\r\n准备休眠\r\n");
              timing_time = 0xFFFF; // 清除定时启动，防止冲突
              system_sleep();
              break;
          }
          if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// 如果设备在激活期
          {
              printf("\r\n设备进入激活期\r\n");
              activation_send_num++;

              I2C2_ByteWrite(activation_send_num, 0x007C);
               __delay_cycles(100000);
               
              printf("\r\n激活数据已发送%.2d条\r\n",activation_send_num);
              Write_alarm(3);
              system_sleep();
              break;
          }
          if(timing_time != 0xFFFF) // 如果收到定时启动命令
          {
              printf("\r\n设备进入定时启动模式\r\n");
              printf("\r\n准备休眠\r\n");
              Write_timer(timing_time);
              timing_time = 0xFFFF;
              if(alarm == 0x04)  // 如果设备拆除
              {
                  enter_work3_process(); // 结束前数据处理
                  work_type = 0; // 置设备为工作状态
                  break;
              }
              system_sleep();
              break;
          }
          
          if(work_mode == 0x03) // 如果在追车模式
          {
              printf("\r\n\a注意：设备即将进入追车模式\r\n");
              enter_work3_process(); // 结束前数据处理
              work_type = 0;
              __delay_cycles(100);
              break;
          }
          else                  // 如果不在追车模式
          {
              printf("\r\n准备休眠\r\n"); 
              Write_alarm(sleep_time);
              if(alarm == 0x04)  // 如果设备拆除
              {
                  enter_work3_process(); // 结束前数据处理
                  work_type = 0; // 置设备为工作状态
                  break;
              }
              system_sleep();
              break;
          }
      }      
  }   
}


// GPS 开机60秒后，开启GSM
void gps_mode(void)
{
  // 设置1分钟闹钟触发 ---> 开启GSM
  //Write_alarm(1);
  RTC_INIT_ON;
   
  // 开启GPS
  GPS_POWER_INIT;     // GPS电源管脚初始化
  GPS_POWER_ON;       // 打开GPS电源
//  GPS_POWER_INIT2;     // GPS电源管脚初始化
//  GPS_POWER_ON2;       // 打开GPS电源
      __delay_cycles(10000);
    __delay_cycles(10000);
    __delay_cycles(10000);

  // 系统休眠
    //--------------------------------------------------------------------------
  //debug  =1;
  WDTCTL = WDTPW + WDTHOLD;   // 关闭看门狗
  __delay_cycles(500);
  Read_time();  
  
  gps_start = 0;

  printf("\r\n> Close WatchDog.\r\n");
  printf("\r\n> △△△ Enter LPM3 mode. <%d> △△△ 20%02d-%02d-%02d %02d:%02d:%02d\r\n",gps_start,RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
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
  printf("\r\n> □□□ Exit LPM3 mode.  <%d> □□□ 20%02d-%02d-%02d %02d:%02d:%02d\r\n",gps_start,RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
//  Write_alarm(work_time);
//  RTC_INIT_ON;
  
//  GSM_POWER_INIT;     // GSM电源管脚初始化
//  GSM_POWER_ON;       // 打开GSM电源

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
      // GPS定位后解析次数
      GPS_analysis_end++;
      if(GPS_analysis_end > 100)
          GPS_analysis_end = 100;
      
      if(GPS_analysis_end > 3)
      {
        // close GPS
        gps_start = 0;
        GPS_POWER_INIT;     // GPS电源管脚初始化
        GPS_POWER_OFF;       // 打开GPS电源
        GPS_POWER_INIT2;     // GPS电源管脚初始化
        GPS_POWER_OFF2;       // 打开GPS电源
              
      }
#ifdef DEBUG
      LED_ON;
      printf("\r\n \a\a\a GPS已定位 %d \a\a\a \r\n",GPS_analysis_end);
#endif      
    }
    
    enable_gps_irq();
   }
    
    if(GPS_analysis_end > GPS_analysis_num && GSM_status == 0xFF && data_send_flag != 2)//如果GSM在空闲模式且GPS已定位则发送精准定位数据
    {
        printf("\r\nGPS已定位,准备发送精准定位\r\n");
        GSM_status = 0x2C;
        data_type = 1;
    }
    if(time_correction == 0x00 && GPS_analysis_end > GPS_analysis_num )
    {
        printf("\r\nGPS已定位,开始校时\r\n");
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
    if(GSM_status == 0xFF && blind_data_size != 0)//如果GSM在空闲模式且存在盲区数据则发送盲区数据
    {
        printf("\r\n发送盲区数据\r\n");
                                GSM_status = 0x2C;
                                data_type = 2;
    }
    if( GSM_status == 0x2C )//如果已连接上服务器
    {
        if(GPS_analysis_end > GPS_analysis_num && data_type != 3 && data_type != 2)
        {
            data_type = 1;
        }
        if(data_type == 0)
        {
            printf("\r\n打包标准定位数据准备上传\r\n");
        }
        else if(data_type == 1)
        {
            printf("\r\n打包精准定位数据准备上传\r\n");
        }
        else if(data_type == 2)
        {
            printf("\r\n打包盲区数据准备上传\r\n");
        }
        else if(data_type == 3)
        {
            printf("\r\n打包TCP响应数据准备上传\r\n");
        }
        pack_data();  
    }
    if(GSM_status == 0xFF && data_send_flag == 2 && blind_data_size == 0) //如果GSM在空闲模式且精准定位数据已发送且不存在盲区数据则主动休眠
    {
        work_type = 1;      // 设置工作类型为休眠
    }
    if(work_type == 1 && GSM_status != 0x2E && GSM_status != 0x2F && GSM_status !=0x30)
    { 
        if(activate_status == 0x02) // 如果收到待激活命令
        {
            printf("\r\n设备进入待激活模式\r\n");
            activation_send_num = 0;
            I2C2_ByteWrite(0, 0x007C);
            __delay_cycles(100000);
            ban_alarm(); // 禁止报警
            printf("\r\n准备休眠\r\n");
            timing_time = 0xFFFF; // 清除定时启动，防止冲突
            system_sleep();
            break;
        }
        if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// 如果设备在激活期
        {
            printf("\r\n设备进入激活期\r\n");
            activation_send_num++;
            
            I2C2_ByteWrite(activation_send_num, 0x007C);
            __delay_cycles(100000);
            
            printf("\r\n激活数据已发送%.2d条\r\n",activation_send_num);
            Write_alarm(3);
            system_sleep();
            break;
        }
        if(timing_time != 0xFFFF) // 如果收到定时启动命令
        {
            printf("\r\n设备进入定时启动模式\r\n");
            printf("\r\n准备休眠\r\n");
            Write_timer(timing_time);
            timing_time = 0xFFFF;
            if(alarm == 0x04)  // 如果设备拆除
            {
                enter_work3_process(); // 结束前数据处理
                work_type = 0; // 置设备为工作状态
                break;
            }
            system_sleep();
            break;
        }
        
        if(work_mode == 0x03) // 如果在追车模式
        {
            printf("\r\n\a注意：设备即将进入追车模式\r\n");
            enter_work3_process(); // 结束前数据处理
            work_type = 0;
            __delay_cycles(100);
            break;
        }
        else                  // 如果不在追车模式
        {
            printf("\r\n准备休眠\r\n"); 
            Write_alarm(sleep_time);
            if(alarm == 0x04)  // 如果设备拆除
            {
                enter_work3_process(); // 结束前数据处理
                work_type = 0; // 置设备为工作状态
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
  if(takt_time >= 180) // 如果追车模式大于等于3分钟
  {
    Write_alarm_sec(120);
    RTC_INIT_ON;
    GSM_POWER_INIT;     // GSM电源管脚初始化
    GSM_POWER_ON;       // 打开GSM电源
    GPS_POWER_INIT;     // GPS电源管脚初始化
    GPS_POWER_ON;       // 打开GPS电源
    GPS_POWER_INIT2;     // GPS电源管脚初始化
    GPS_POWER_ON2;       // 打开GPS电源
    while(1)
    {
      #ifdef WDG
      WDTCTL = WDT_ARST_250;
      #endif
       GSM();
        
      //uscia_config();
      led_toggle();
        
        
        if(GPS_analysis_end > GPS_analysis_num && GSM_status == 0xFF && data_send_flag != 2)//如果GSM在空闲模式且GPS已定位则发送精准定位数据
        {
            printf("\r\nGPS已定位,准备发送精准定位\r\n");
            GSM_status = 0x2C;
            data_type = 1;
        }
        if( GSM_status == 0x2C )//如果已连接上服务器
        {
            if(GPS_analysis_end > 7 && data_type != 3 && data_type != 2)
            {
                data_type = 1;
            }
            if(data_type == 0)
            {
                printf("\r\n打包标准定位数据准备上传\r\n");
            }
            else if(data_type == 1)
            {
                printf("\r\n打包精准定位数据准备上传\r\n");
            }
            else if(data_type == 2)
            {
                printf("\r\n打包盲区数据准备上传\r\n");
            }
            else if(data_type == 3)
            {
                printf("\r\n打包TCP响应数据准备上传\r\n");
            }
            pack_data();  
        }
        if(GSM_status == 0xFF && data_send_flag == 2) //如果GSM在空闲模式且精准定位数据已发送则主动休眠
        {
            work_type = 1;      // 设置工作类型为休眠
        }
        if(work_type == 1 && GSM_status != 0x2E && GSM_status != 0x2F && GSM_status !=0x30)
        { 
            if(activate_status == 0x02) // 如果收到待激活命令
            {
                printf("\r\n设备进入待激活模式\r\n");
                activation_send_num = 0;
                I2C2_ByteWrite(0, 0x007C);
                __delay_cycles(100000);
                ban_alarm(); // 禁止报警
                printf("\r\n准备休眠\r\n");
                timing_time = 0xFFFF; // 清除定时启动，防止冲突
                system_sleep();
                break;
            }
            if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// 如果设备在激活期
            {
                printf("\r\n设备进入激活期\r\n");
                
                activation_send_num++;
                
                I2C2_ByteWrite(activation_send_num, 0x007C);
                __delay_cycles(100000);
                
                printf("\r\n激活数据已发送%.2d条\r\n",activation_send_num);
                Write_alarm(3);
                system_sleep();
                break;
            }
            if(timing_time != 0xFFFF) // 如果收到定时启动命令
            {
                printf("\r\n设备进入定时启动模式\r\n");
                printf("\r\n准备休眠\r\n");
                Write_timer(timing_time);
                timing_time = 0xFFFF;
                if(alarm == 0x04)  // 如果设备拆除
                {
                    enter_work3_process(); // 结束前数据处理
                    work_type = 0; // 置设备为工作状态
                    break;
                }
                system_sleep();
                break;
            }
            if(work_mode == 0x03) // 如果在追车模式
            {
                if(takt_time >= 180) // 如果间隔时间大于等于3分钟
                {
                    Write_alarm_sec(takt_time);
                    if(alarm == 0x04)  // 如果设备拆除
                    {
                        enter_work3_process(); // 结束前数据处理
                        work_type = 0; // 置设备为工作状态
                        break;
                    }
                    work3_sleep(); 
                    break;
                }
                else                 // 如果间隔时间小3分钟
                {
                    exit_work3_process(); // 结束前数据处理
                    break;
                }
            }
            else                    // 如果不在追车模式
            {
                printf("\r\n\a注意：设备即将退出追车模式\r\n");
                printf("\r\n准备休眠\r\n");
                work_type = 1;
                Write_alarm(sleep_time);
                if(alarm == 0x04)  // 如果设备拆除
                {
                    enter_work3_process(); // 结束前数据处理
                    work_type = 0; // 置设备为工作状态
                    break;
                }
                system_sleep();
                break;
            }
        }
    }
  }
  else // 如果追车模式小于3分钟
  {
    Write_takt(takt_time); // 设置间隔时间
    takt_time_flag = 0;
    RTC_INIT_ON;
    GSM_POWER_INIT;     // GSM电源管脚初始化
    GSM_POWER_ON;       // 打开GSM电源
    GPS_POWER_INIT;     // GPS电源管脚初始化
    GPS_POWER_ON;       // 打开GPS电源
    GPS_POWER_INIT2;     // GPS电源管脚初始化
    GPS_POWER_ON2;       // 打开GPS电源
    while(1)
    {
        #ifdef WDG
        WDTCTL = WDT_ARST_250;
        #endif
        GSM();
        if(GSM_status == 0xFF && takt_time_flag == 1)  // 如果GSM在空闲模式且间隔时间到
        {
            
            
            if(activate_status == 0x02) // 如果收到待激活命令
            {
                printf("\r\n设备进入待激活模式\r\n");
                activation_send_num = 0;
                I2C2_ByteWrite(0, 0x007C);
                __delay_cycles(100000);
                ban_alarm(); // 禁止报警
                printf("\r\n准备休眠\r\n");
                timing_time = 0xFFFF; // 清除定时启动，防止冲突
                system_sleep();
                break;
            }
            if(activate_status == 0x03 && activation_send_num < activation_max_send_num)// 如果设备在激活期
            {
                printf("\r\n设备进入激活期\r\n");
                activation_send_num++;
                
                I2C2_ByteWrite(activation_send_num, 0x007C);
                __delay_cycles(100000);
                
                printf("\r\n激活数据已发送%.2d条\r\n",activation_send_num);
                Write_alarm(3);
                system_sleep();
                break;
            }
            if(timing_time != 0xFFFF) // 如果收到定时启动命令
            {
                printf("\r\n设备进入定时启动模式\r\n");
                printf("\r\n准备休眠\r\n");
                Write_timer(timing_time);
                timing_time = 0xFFFF;
                system_sleep();
                break;
            }
            if(work_mode == 0x03) // 如果在追车模式
            {
                if(takt_time >= 180) // 如果间隔时间大于等于3分钟
                {
                    exit_work3_process();
                    work_type = 0;
                    break;
                }
                else                 // 如果间隔时间小3分钟
                {
                    AD_collect();
                    //*****************存工时***************************
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
                    printf("\r\n累计工时：%ld小时%ld分钟%ld秒\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
                    
                    I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);          //存储工时统计数据
                    __delay_cycles(100000);
                    I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);                
                    __delay_cycles(100000);
                    I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);                
                    __delay_cycles(100000);
                    I2C2_ByteWrite((uchar)(labor_time), 0x0016);                
                    __delay_cycles(100000);
                    //******************存工时结束**********************
                    Read_time();
                    start_year = RTC_year;
                    start_mon = RTC_mon;
                    start_day = RTC_day;
                    start_hour = RTC_hour;
                    start_min = RTC_min;
                    start_sec = RTC_sec;
                    Write_takt(takt_time);  // 设置间隔时间
                    takt_time_flag = 0;
                    GSM_status = 0x2C;
                }   
            }
            else                  // 如果不在追车模式
            {
                printf("\r\n\a注意：设备即将退出追车模式\r\n");
                printf("\r\n准备休眠\r\n"); 
                work_type = 1;
                Write_alarm(sleep_time);
                takt_time_flag = 0;
                if(alarm == 0x04)  // 如果设备拆除
                {
                    enter_work3_process(); // 结束前数据处理
                    work_type = 0; // 置设备为工作状态
                    break;
                }
                
                system_sleep();
                break;
            }
        }
        if(alarm == 0x04)  // 如果设备拆除
        {
            I2C2_ByteWrite(0x04, 0x000A);
            alarm = 0x00;
            activate_status = 0x04;
        }
        if( GSM_status == 0x2C )//如果已连接上服务器
        {
            if(GPS_analysis_end > 7 && data_type != 3 && data_type != 2)
            {
                data_type = 1;
            }
            if(data_type == 0)
            {
                printf("\r\n打包标准定位数据准备上传\r\n");
            }
            else if(data_type == 1)
            {
                printf("\r\n打包精准定位数据准备上传\r\n");
            }
            else if(data_type == 2)
            {
                printf("\r\n打包盲区数据准备上传\r\n");
            }
            else if(data_type == 3)
            {
                printf("\r\n打包TCP响应数据准备上传\r\n");
            }
            pack_data();  
        }   
    }
  }
}