#include "MSP430F5438A.h"
#include "mg323b.h"
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "string.h"
#include "packdata.h"
#include "at24c512.h"
#include "sd2068.h"
#include "led.h"
#include "stdbool.h"


uchar GSM_TX0_buf[128];		// GSM发送缓存区
uchar GSM_RX0_buf[512];		// GSM接受缓存区

uchar ICCID[20];
uchar ICCID_temp;
//uchar p;

uchar LCA[5];				// 位置码
uchar CELL[5];				// 小区码
uchar CSQ[2];				// 信号强度

uint  AT_CREG_num = 0;		// 查询当前网络注册状态重试次数
uint  CCR0_time;			// 超时时间
uchar GSM_RX0_buf_num;		// 串口0接受计数
uchar GSM_status;           // GSM状态变量
uchar GSM_time_flag;        // GSM定时标志位
uchar retry;                // AT指令重试次数
uchar send_retry = 0;       // 发送重试次数
uint config_flag;           // 配置成功标志
uchar data_send_flag;        // GSM发送成功标志


uchar gsm_buf_index=0;
uchar gsm_buf_len=0;
uchar gsm_buf_size=0;
uchar gsm_buf_offset = 0;
//uchar buf[168];
uchar buf[255];

uchar buf1[100];
char *p;

//---------------------GSM模块初始化相关AT命令----------------------------------
const char AT[]={'A','T',0x0D,0X0A};//GSM握手命令
const char ATE0[]={'A','T','E','0',0x0D,0X0A};//关闭回显
const char ATE1[]={'A','T','E','1',0x0D,0X0A};//关闭回显
const char AT_CRSM_176_12258_0_0_10[]={'A','T','+','C','R','S','M','=','1','7','6',',','1','2','2','5','8',',','0',',','0',',','1','0',0x0D,0x0A};//查询ICCID
const char AT_CGREG_0[]={'A','T','+','C','G','R','E','G','=','0',0x0D,0X0A};//禁用网络状态报告
const char AT_CMGF_1[]={'A','T','+','C','M','G','F','=','1',0x0D,0X0A};//设置短消息格式为文本模式
const char AT_CNMI_0_0_0_0_0[]={'A','T','+','C','N','M','I','=','0',',','0',',','0',',','0',',','0',0x0D,0X0A};//设置短消息不上报
const char AT_CNMI_2_2_0_0_0[]={'A','T','+','C','N','M','I','=','2',',','2',',','0',',','0',',','0',0x0D,0X0A};//设置短消息上报
const char AT_CREG_0[]={'A','T','+','C','R','E','G','=','0',0x0D,0X0A};//禁止CREG主动上报
const char AT_CREG_[]={'A','T','+','C','R','E','G','?',0x0D,0X0A};//查询当前网络注册状态(后续需要循环检测是否入网)
const char AT_SMONC[]={'A','T','^','S','M','O','N','C',0x0D,0X0A};//获取小区相关信息
const char AT_CSQ[]={'A','T','+','C','S','Q',0x0D,0X0A};//查询当地网络信号强度，最大31，最小0
//----------------------GPRS初始化相关AT命令------------------------------------
const char AT_CGATT_[]={'A','T','+','C','G','A','T','T','=','?',0x0D,0x0A};//设置GPRS附着状态
//const char AT_SICS_0_CONTYPE_GPRS0[]={'A','T','^','S','I','C','S','=','0',',','C','O','N','T','Y','P','E',',','G','P','R','S','0',0x0D,0x0A};//将ID为0的Profile的conType设置为GPRS0
const char AT_SICS_0_CONTYPE_GPRS0[]="AT^SICS=0,\"CONTYPE\",\"GPRS0\"\r\n";//将ID为0的Profile的conType设置为GPRS0

const char AT_SICS_0_ANP_CMNET[]={'A','T','^','S','I','C','S','=','0',',','\"','A','P','N','\"',',','\"','C','M','N','E','T','\"',0x0D,0x0A};//将ID为0的Profile的apn设置为1234
//const char AT_SICS_0_ANP_CMNET[]="AT^SICS=0,\"APN\",\"CMNET\"\r\n";//将ID为0的Profile的conType设置为GPRS0


//const char AT_SICS_0_INACTTO_20[]={'A','T','^','S','I','C','S','=','0',',','\"','I','N','A','C','T','T','O','\"',',','2','0',0x0D,0x0A};//设置ID为0内部超时时间
const char AT_SICS_0_INACTTO_20[]="AT^SICS=1,\"INACTTO\",20\r\n";//将ID为0的Profile的conType设置为GPRS0
const char AT_IOMODE_1_1[]="AT^IOMODE=1,1\r\n";
//const char AT_IOMODE_1_1[]={'A','T','^','I','O','M','O','D','E','=','1',',','1',0x0D,0x0A};//对接受数据进行转换，不使用缓存（使用缓存时容易丢失数据）
//----------------------GPRS连接服务器AT命令------------------------------------
const char AT_SISS_1_SRVTYPE_SOCKET[]={'A','T','^','S','I','S','S','=','1',',','\"','S','R','V','T','Y','P','E','\"',',','\"','S','O','C','K','E','T','\"',0x0D,0x0A};//
const char AT_SISS_1_CONID_0[]={'A','T','^','S','I','S','S','=','1',',','\"','C','O','N','I','D','\"',',','0',0x0D,0x0A};//
char AT_SISS_1_ADDRESS[60]={'A','T','^','S','I','S','S','=','1',',','\"','A','D','D','R','E','S','S','\"',',','\"','S','O','C','K','T','C','P',':','/','/','2','1','1','.','1','0','3','.','1','7','9','.','2','3','4',':','0','7','7','7','8','\"','"',0x0D,0x0A};//设置连接IP端口

//char AT_SISS_1_ADDRESS[]={'A','T','^','S','I','S','S','=','1',',','A','D','D','R','E','S','S',',','"','S','O','C','K','T','C','P',':','/','/','1','2','4','.','2','0','2','.','1','9','8','.','2','4','2',':','0','9','0','0','1','"',0x0D,0x0A};//设置连接IP端口
const char AT_SISC_1[]={'A','T','^','S','I','S','C','=','1',0x0D,0x0A};//打开连接前最好先关闭一下连接
const char AT_SISO_1[]={'A','T','^','S','I','S','O','=','1',0x0D,0x0A};//打开服务1

//----------------------GPRS发送数据AT命令--------------------------------------
const char AT_SISW_1_99[]={'A','T','^','S','I','S','W','=','1',',','9','9',0x0D,0x0A};//正常回传、盲区补发申请73个字节缓存
const char AT_SISW_1_113[]={'A','T','^','S','I','S','W','=','1',',','1','1','3',0x0D,0x0A};//写参数TCP响应申请14个字节缓存
const char AT_CIPSEND_99[]={'A','T','+','C','I','P','S','E','N','D','=','9','9',0x0D,0x0A};//向模块申请99个字节缓冲区
const char AT_CIPSEND_113[]={'A','T','+','C','I','P','S','E','N','D','=','1','1','3',0x0D,0x0A};//向模块申请113个字节缓冲区
char AT_CIPSTART_TCP[60]={'A','T','+','C','I','P','S','T','A','R','T','=','"','T','C','P','"',',','"','2','0','2','.','0','8','5','.','2','0','9','.','0','8','0','"',',','"','0','9','0','0','1','"',0x0D,0x0A};//建立TCP连接


//----------------------GPRS读取数据AT命令--------------------------------------
//const char AT_SISR_1_1500[]={'A','T','^','S','I','S','R','=','1',',','1','5','0','0',0x0D,0x0A};

uint FindStr(char *str,char *ptr)
{
  uint index=0;
  char *STemp=NULL;
  char *PTemp=NULL;
  char *MTemp=NULL;
  if(0==str||0==ptr)
  return 0;
  for(STemp=str; *STemp!='\0'; STemp++)	 
  {
    index++;   
    MTemp=STemp;  

    for(PTemp=ptr;*PTemp!='\0';PTemp++)
    {	
      if(*PTemp!=*MTemp)
      break;
      MTemp++;
    }
    if(*PTemp=='\0')  
      break;
   }
   return index;
}



void GSM( void )
{
  char tmp_buf[100]={0};
  uchar i = 0;
  
  uchar config_array[50]={0x00};
  
  uchar GSM_Plus = 0; 
  uchar GSM_Comma = 0;
  uchar w;
  bool GSM_start = 0;
  uint a;
  
  
  switch( GSM_status )
  {
    case 0x00:
      GSM_SW_INIT;
      GSM_SW_HIGH;              //开机管脚置1
      GSM_status = 0x01;        //设置GSM状态
      GSM_time_flag = 0;        //清定时标志位
      TA1CCR0 = 3750;           //设置一个大约3秒的定时
      CCR0_time = 2500;         //设置一个大约2秒的定时        
      printf("> Gsm power on...\r\n");
      break;
      case 0x01:
        if (GSM_time_flag)		                                            //定时时间到
        {
            GSM_status = 0x02;	                                            //设置GSM状态
            GSM_time_flag = 0;	                                            //清定时标志位
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
        }
        break;
        
        case 0x02:
            GSM_SW_LOW;			//开机管脚清0
            GSM_status = 0x03;		//设置GSM状态
            GSM_time_flag = 0;		//清定时标志位
            TA1CCR0 = 3750;		//设置一个大约3秒的定时
            CCR0_time = 2500;		//设置一个大约2秒的定时
            //printf("\r\n开机管脚清0\r\n");
            break;
        case 0x03:
            if (GSM_time_flag)		                                            //定时时间到
            {
              GSM_status = 0x04;	                                            //设置GSM状态
              GSM_time_flag = 0;	                                            //清定时标志位
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
            break;
        case 0x04:
            delay_ms(3000);
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0; 
            uscia2_send( (void*)AT);                                           //握手命令
            GSM_status = 0x05;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
            break;
        case 0x05:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                GSM_status = 0x06;
              }
              else
              {
                GSM_status = 0x06;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
            break;
        case 0x06:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0; 
            //变量清0
            uscia2_send( (void*)"ATE0\r\n" );                                         //关闭回显
            GSM_status = 0x07;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
            break;
        case 0x07:
            if (GSM_time_flag)                                                  //定时时间到
            {
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                  printf("\r\n关回显失败，重试\r\n");
                  GSM_status = 0x06;
                  retry++;
                }
              }
              else
              {
                retry = 0;                                                  //重试次数清零
                GSM_status = 0x08;                                          //设置GSM状态
                GSM_status = 0x0C;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
            break;
        case 0x08:
          memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
          GSM_RX0_buf_num = 0; 

          uscia2_send( (void*)"AT+CCID\r\n" );                     //查询ICCID信息
          GSM_status = 0x09;
          //GSM_status = 0x10;   

          //设置GSM状态
          GSM_time_flag = 0;                                                  //清定时标志位
          TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
          CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
          break;

        case 0x09:

        if (GSM_time_flag)                                                  //定时时间到
        {	
          if(strstr(GSM_RX0_buf, "OK") == NULL)
          {
            if( retry >= 3 )
            {
              printf("\r\n重试次数超过3次，重新开机\r\n");
              GSM_POWER_OFF;
              delay_ms(1000);
              delay_ms(1000);
              GSM_POWER_ON;
              delay_ms(500);
              GSM_status = 0x00;
            }
            else
            {
              printf("\r\n查询ICCID失败，重试\r\n");
              GSM_status = 0x08;
              retry++;
            }
          }
          else
          {
               eeprom_PageRead((uchar*)config_array, 0x0001, 8);
              if( (config_array[2]=='0') &&  ( config_array[3]=='2')  )
              {
                 printf("\r\n设备编号：%.*s\r\n", 8, config_array);
              }
               else
              {

                memcpy(config_array+2,GSM_RX0_buf+16,6);
                config_array[0]='G';
                config_array[1]='B';
                eeprom_PageWrite((uchar*)config_array, 0x0001, 8);
                __delay_cycles(100000);
                printf("\r\n1写设备编号成功\r\n");
                eeprom_PageRead((uchar*)config_array, 0x0001, 8);
                printf("\r\n设备编号：%.*s\r\n", 8, config_array);
                
              }
            
            retry = 0;                                                  //重试次数清零

            GSM_status = 0x0C;                                          //设置GSM状态
            GSM_time_flag = 0;                                          //清定时标志位
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
          }
         }
         break;
        case 0x0A:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)"AT+CNMI=0,0,0,0,0\r\n" );                            //设置短消息不上报
            GSM_status = 0x0B;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
            printf("\r\n设置短消息不上报\r\n");
            break;
            
         case 0x0B:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                  printf("\r\n设置短消息不上报失败，重试\r\n");
                  GSM_status = 0x0A;
                  retry++;
                  }
              }
              else
              {
                retry = 0;                                                  //重试次数清零
                GSM_status = 0x0C;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
              }
            break;
                    
  case 0x0C:

     memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)"AT+CREG=1\r\n" );                                    //关闭显示网络注册未知码
            GSM_status = 0x0D;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 375;                                                    //设置一个大约300毫秒的定时
            printf("\r\n关闭显示网络注册未知码\r\n");
            break;
        case 0x0D:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                 }
                 else
                {
                  printf("\r\n关闭显示网络注册未知码失败，重试\r\n");
                  GSM_status = 0x0C;
                  retry++;
                }
              }
              else
              {
                retry = 0;                                                  //重试次数清零
                GSM_status = 0x0E;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
               }
            }
            break;
            case 0x0E:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)"AT+CREG?\r\n" );                                     //查询网络注册状态
            GSM_status = 0x0F;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                   //设置一个大约1秒的定时
            printf("\r\n查询GSM网络注册状态\r\n");
            break;
        case 0x0F:
            if (GSM_time_flag)                                                  //定时时间到
          {	
          if(strstr(GSM_RX0_buf, "+CREG:") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
          printf("\r\n查询GSM网络注册状态失败，重试\r\n");
          GSM_status = 0x0E;
          retry++;
          }
          }
          else
          {
          p = (void *)strstr(GSM_RX0_buf, "+CREG:");
          if(*(p + 9) != '1' && *(p + 9) != '5')
          {
          AT_CREG_num++;                                          //重试次数加1
          GSM_status = 0x0E;                                      //设置GSM状态
          GSM_time_flag = 0;                                      //清定时标志位
          printf("\r\n等待GSM网络注册:%d\r\n",AT_CREG_num);
          TA1CCR0 = 0;
          TA1CTL = TASSEL_1 + MC_1 + TACLR;
          CCR0_time = 0;
          }
              else if( *(p + 9) == '1' )
              {
                  retry = 0;                                              //重试次数清零
                GSM_status = 0x10;                                      //设置GSM状态
              GSM_time_flag = 0;                                      //清定时标志位
                  printf("\r\n已注册本地GSM网络\r\n");
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
              }
              else if( *(p + 9) == '5' )
              {
                retry = 0;                                              //重试次数清零
                GSM_status = 0x10;                                      //设置GSM状态
                GSM_time_flag = 0;                                      //清定时标志位
                printf("\r\n已注册漫游GSM网络\r\n");
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
          }
          break;            

    case 0x10:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)"AT+CENG=1,1\r\n" );                                  //打开工程模式
            GSM_status = 0x11;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
            printf("\r\n打开工程模式\r\n");
            break;
        case 0x11:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
              if( retry >= 3 )
              {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;

              }
              else
              {
                printf("\r\n打开工程模式失败，重试\r\n");
                GSM_status = 0x10;
                retry++;
              }
            }
            else
            {
              retry = 0;                                                  //重试次数清零
              GSM_status = 0x12;                                          //设置GSM状态
              GSM_time_flag = 0;                                          //清定时标志位
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
            }
            break;
          
          
     
        case 0x12:
          memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
          GSM_RX0_buf_num = 0;                                                //变量清0
          AT_CREG_num = 0;                                                    //重试次数清0
          uscia2_send( (void*)"AT+CENG?\r\n" );                                     //获取小区相关信息
          GSM_status = 0x13;                                                  //设置GSM状态
          GSM_time_flag = 0;                                                  //清定时标志位
          TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
          CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
          printf("\r\n获取小区相关信息\r\n");
        break;
        case 0x13:
          if (GSM_time_flag)                                                  //定时时间到
          {	
          if(strstr(GSM_RX0_buf, "+CENG:") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
          printf("\r\n查询小区基站信息失败，重试\r\n");
          GSM_status = 0x12;
          retry++;
          }
          }
          else
          {
          CELL[0] = '0';CELL[1] = '0';CELL[2] = '0';CELL[3] = '0';
          LCA[0] = '0';LCA[1] = '0';LCA[2] = '0';LCA[3] = '0';
          //----解析GSM基站信息
          for(a=0;GSM_RX0_buf[a] != 0x00;a++)
          {
          if(GSM_RX0_buf[a]=='+')
          {
          GSM_start = 1;
          GSM_Plus++;
          GSM_Comma = 0;
          }
          if(GSM_start)
          {
          switch(GSM_Plus)
          {
          case 0x01:
          break;
          case 0x02:
          if(GSM_RX0_buf[a] != ',')
          {
          if(GSM_Comma == 7)
          {
          CELL[0] = CELL[1];
          CELL[1] = CELL[2];
          CELL[2] = CELL[3];
          CELL[3] = GSM_RX0_buf[a];
          }
          if(GSM_Comma == 10)
          {
          LCA[0] = LCA[1];
          LCA[1] = LCA[2];
          LCA[2] = LCA[3];
          LCA[3] = GSM_RX0_buf[a];
          }
          }
          break;
          case 0x03:
          break;
          case 0x04:
          break;
          case 0x05:
          break;
          case 0x06:
          break;
          case 0x07:
          break;
          case 0x08:
          break;
          default:
          break;

          }
          }
          if(GSM_start)
          {
          if(GSM_RX0_buf[a]==',')
          {
          GSM_Comma++;
          }

          }   
          }
          if( LCA[0] == '0' && LCA[1] == '0' && LCA[2] == '0' && LCA[3] == '0' && CELL[0] == '0' && CELL[1] == '0' && CELL[2] == '0' && CELL[3] == '0')
          {
          GSM_status = 0x12;                                  //重取小区信息
          printf("\r\n小区信息为空，重取小区信息\r\n");
          }
          else
          {
          printf("\r\nGSM小区码CELL：%.*s\r\n", 4, CELL);
          printf("\r\nGSM位置码LCA：%.*s\r\n", 4, LCA);
          GSM_status = 0x14;
          }
          retry = 0;                                                  //重试次数清零                                         //设置GSM状态
          GSM_time_flag = 0;                                          //清定时标志位
          TA1CCR0 = 0;
          TA1CTL = TASSEL_1 + MC_1 + TACLR;
          CCR0_time = 0;
          }
          }
            break;
        case 0x14:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)AT_CSQ );                                       //查询当地网络信号强度
            GSM_status = 0x15;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
            printf("\r\n查询信号强度\r\n");
            break;
        case 0x15:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              printf(GSM_RX0_buf);
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                    printf("\r\n查询信号强度失败，重试\r\n");
                    GSM_status = 0x14;
                    retry++;
                }
              }
              else
              {
                retry = 0;                                                  //重试次数清零
                GSM_status = 0x18;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
                //取出当地网络信号强度
                if( GSM_RX0_buf[9] == ',' )
                {
                    CSQ[0] = '0';
                    if( '0' <= GSM_RX0_buf[8] && GSM_RX0_buf[9] <= '9' )
                    {
                        CSQ[1] = GSM_RX0_buf[8];
                    }
                    else
                    {
                        CSQ[1] = 0;
                    }
                }
        else
        {
                if( '0' <= GSM_RX0_buf[8] && GSM_RX0_buf[8] <= '9' )
                {
                    CSQ[0] = GSM_RX0_buf[8];
                }
                else
                {
                    CSQ[1] = 0;
                }
                if( '0' <= GSM_RX0_buf[9] && GSM_RX0_buf[9] <= '9' )
                {
                    CSQ[1] = GSM_RX0_buf[9];
                }
                else
                {
                    CSQ[1] = 0;
                }
              }
              printf("\r\nGSM信号强度：%.*s\r\n", 2, CSQ);
                  }
          }
            break;
        case 0x18:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)AT_CGATT_ );                                    //查询GPRS附着状态
            GSM_status = 0x19;                                                  //设置GSM状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时
            printf("\r\n查询GPRS附着\r\n");
            break;
        case 0x19:
          if (GSM_time_flag)                                                  //定时时间到
          {	
            if(strstr(GSM_RX0_buf, "OK") == NULL)
            {
            if( retry >= 3 )
            {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
            }
            else
            {
              printf("\r\n查询GPRS附着失败，重试\r\n");
              GSM_status = 0x18;
              retry++;
            }
            }
            else
            {
              retry = 0;                                                  //重试次数清零
              GSM_status = 0x1A;                                          //设置GSM状态
              GSM_time_flag = 0;                                          //清定时标志位
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
          }
            break;
        case 0x1A:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
            GSM_RX0_buf_num = 0;         //变量清0
            uscia2_send("AT+CIPHEAD=1\r\n\r\n");
            //uscia2_send( (void*)AT_SICS_0_CONTYPE_GPRS0 );                      //将ID为0的Profile的conType设置为GPRS0
            GSM_status = 0x1B;                                                  //设置GPRS状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时	
            printf("\r\n在数据前加IP头\r\n");
            break;
        case 0x1B:
          if (GSM_time_flag)                                                  //定时时间到
          {	
          printf(GSM_RX0_buf);

          if(strstr(GSM_RX0_buf, "OK") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
            printf("\r\n在数据前加IP头失败，重试\r\n");
            GSM_status = 0x1A;
            retry++;
         
          }
          }
          else
          {
            retry = 0;                                                  //重试次数清零
            GSM_status = 0x1C;                                          //设置GSM状态
            GSM_time_flag = 0;                                          //清定时标志位
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
          }
          }
            break;
        case 0x1C:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
           GSM_RX0_buf_num = 0;      
            //变量清0
           uscia2_send("AT+CSTT=\"CMNET\"\r\n");
            //uscia2_send( (void*)AT_SICS_0_ANP_CMNET );                          //将ID为0的Profile的apn设置为CMNET
            GSM_status = 0x1D;                                                  //设置GPRS状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 2500;                                                    //设置一个大约300毫秒的定时	
            printf("\r\n设置CMNET\r\n");	
            break;
        case 0x1D:
        if (GSM_time_flag)                                                  //定时时间到
        {	
          printf(GSM_RX0_buf);

          if(strstr(GSM_RX0_buf, "OK") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
            printf("\r\n设置CMNET失败，重试\r\n");
            GSM_status = 0x1C;
            retry++;
          }
          }
          else
          {
            retry = 0;                                                  //重试次数清零
            GSM_status = 0x1E;                                          //设置GSM状态
            //GSM_status = 0x22;                                          //设置GSM状态

            GSM_time_flag = 0;                                          //清定时标志位
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
           }
          }
            break;
          case 0x1E:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            uscia2_send( (void*)"AT+CIICR\r\n" );                                     //建立无线链路
            GSM_status = 0x1F;                                                  //设置GPRS状态
            GSM_time_flag = 0;                                                  //清定时标志位
			TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时	
            printf("\r\n建立无线链路\r\n");
            break;
        case 0x1F:
            if (GSM_time_flag)                                                  //定时时间到
            {	
            if(strstr(GSM_RX0_buf, "OK") == NULL)
            {
            if( retry >= 3 )
            {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
            }
            else
            {
            printf("\r\n建立无线链路，重试\r\n");
            GSM_status = 0x1E;
            retry++;
            }
            }
            else
            {
              retry = 0;                                                  //重试次数清零
              GSM_status = 0x20;                                          //设置GSM状态
              GSM_status = 0x22;                                          //设置GSM状态
              
              GSM_time_flag = 0;                                          //清定时标志位
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
            }
            break;        
        
        
        case 0x20:    // AT+CIFSR 获得本地IP地址 

          memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
          GSM_RX0_buf_num = 0;                                                //变量清0
          uscia2_send( (void*)"AT+CIFSR\r\n" );                                     //获取本机IP地址
          GSM_status = 0x21;                                                  //设置GPRS状态
          GSM_time_flag = 0;                                                  //清定时标志位
          TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
          CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时	
          printf("\r\n获取本机IP地址\r\n");
          break;
        case 0x21:
        if (GSM_time_flag)                                                  //定时时间到
        {	
          if(strstr(GSM_RX0_buf, ".") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
            printf("\r\n获取本机IP地址，重试\r\n");
            GSM_status = 0x20;
            retry++;
          }
          }
          else
          {

            retry = 0;                                                  //重试次数清零
            GSM_status = 0x22;                                          //设置GSM状态
            printf("\r\n本机IP：%s", GSM_RX0_buf+2);
            delay100ms();
            GSM_time_flag = 0;                                          //清定时标志位
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
          }
          }
          break;
        case 0x22:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
            GSM_RX0_buf_num = 0;  //变量清0    
            printf("\r\n建立TCP连接至服务器\r\n");
            printf((void*)AT_CIPSTART_TCP);

            uscia2_send( (void*)AT_CIPSTART_TCP );                              //建立TCP连接至服务器                  delay_ms(1000);
            delay_ms(1000);
            delay_ms(1000);
            delay_ms(1000);
            GSM_status = 0x23;                                                  //设置GPRS状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 3750;                                                    //设置一个大约300毫秒的定时
            break;
        case 0x23:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
            {
            if( retry >= 3 )
            {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
            }
            else
            {
            printf("\r\n建立TCP连接至服务器失败，重试\r\n");
            GSM_status = 0x22;
            retry++;
            }
            }
            else
            {
              printf("\r\n与服务器建立TCP连接成功\r\n");
            retry = 0;                                                  //重试次数清零
            GSM_status = 0x2C;                                          //设置GSM状态
            GSM_time_flag = 0;                                          //清定时标志位
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
            }
            }
            break;
  
        
        case 0x2C:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            if(data_type == 0 || data_type == 1 || data_type == 2)              //正常、盲区回传
            {
              uscia2_send( (void*)AT_CIPSEND_99 );                             //向模块申请99个字节缓冲区
            }
            else if(data_type == 3)                                             //写参数TCP回应
            {
              uscia2_send( (void*)AT_CIPSEND_113 );                             //向模块申请14个字节缓冲区
            }
            GSM_status = 0x2D;                                                  //设置GPRS状态
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                     //设置一个大约3秒的定时
            CCR0_time = 1250;                                                    //设置一个大约300毫秒的定时	
            printf("\r\n向模块申请缓冲区\r\n");
            break;
        case 0x2D:
            if (GSM_time_flag)                                                  //定时时间到
            {	
              if(strstr(GSM_RX0_buf, ">") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                    printf("\r\n申请缓冲区失败，重试\r\n");
                    GSM_status = 0x2C;
                    retry++;
                }
              }
              else
              {
                retry = 0;                                                  //重试次数清零
                GSM_status = 0x2E;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
            break;
        case 0x2E:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //缓存区清零
            GSM_RX0_buf_num = 0;                                                //变量清0
            if(data_type == 0 || data_type == 1 || data_type == 2)              //正常、盲区回传
            {
              send_PostBack_data( (void*)data_packet,99 );                   //向模块缓冲区写入99个字节
              GSM_status = 0x2F;                                              //设置GPRS状态
            }
            else if(data_type == 3)                                             //TCP回应
            {
              send_PostBack_data( (void*)data_packet,113);                    //向模块缓冲区写入113个字节
              GSM_status = 0x31;                                              //设置GPRS状态
                    
            }
            GSM_time_flag = 0;                                                  //清定时标志位
            TA1CCR0 = 3750;                                                    //设置一个大约3秒的定时
            CCR0_time = 2500;                                                   //设置一个大约2秒的定时		
            printf("\r\n向模块缓冲区写入数据\r\n");
            break;
        case 0x2F:
            if (GSM_time_flag)                                                  //定时时间到
            {	
	      if(strstr(GSM_RX0_buf, "+IPD,9:") != NULL)
               {
                GSM_status = 0x30;                                          //设置GSM状态
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                CCR0_time = 0;
                send_retry = 0;
                if(data_type == 0)                                          //如果是正常数据
                {
                data_send_flag = 1;// 标准定位数据发送成功标志置位
                }
                if(data_type == 1)
                {
                data_send_flag = 2;// 精准定位数据发送成功标志置位
                }
                if(data_type == 2)//如果是盲区数据
                {
                blind_data_size--;//盲区数据量减1
                printf("\r\n盲区数据量为：%d\r\n",blind_data_size);
                blind_data_head--;//盲区数据头减1
                if(blind_data_head < 1)
                blind_data_head = 500;
                }
                printf("\r\n收到服务器应答，数据上传成功\r\n");
              }
              else
              {
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                CCR0_time = 0;
                if( send_retry >= 3 )
                {
                  printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                  printf("\r\n没有收到服务器回应，重新上传\r\n");
                  GSM_status = 0x28;                                      //设置GSM状态
                  send_retry++;
                }
              }
                

            }
            break;	
            case 0x30:
              
             GSM_RX0_buf[20]=0x31;
            GSM_RX0_buf[21]=0x31;
            GSM_RX0_buf[22]=0x31;
            GSM_RX0_buf[23]=0x31;
            GSM_RX0_buf[24]=0x31;
            if(strstr(GSM_RX0_buf, "+IPD,83:") == NULL)
            {
                printf("\r\n没有检测到设置命令帧头\r\n");
            }
            else
            {
                printf("\r\n已经检测到设置命令帧头\r\n");
            }
            
            //if(sscanf((char *)strstr(GSM_RX0_buf, "+IPD,83:"), "^SISR: 1, 83, %s",buf) == 1)
			if(strstr(GSM_RX0_buf, "+IPD,83:") != NULL)
            {
				printf("\r\n检测到服务器配置命令，开始解析……\r\n");
				//ascii2data((void*)buf1,(void*)buf,168);
                p = (void *)strstr(GSM_RX0_buf, "+IPD,83:");
                p = p + 8;
                for(w = 0; w < 83; w++)
                {
                    buf1[w] = *p;
                    p++;
                }
                if( buf1[0] == 0xFF && buf1[1] == 0xFF && buf1[2] == 0xFF && buf1[3] == 0xFF && buf1[4] == 0xFF )
                {
                    // 解析服务器地址
                    if( buf1[12] == 0x01 || buf1[12] == 0x02)
                    {
                        servers_addr[0] = buf1[12];
                        servers_addr[1] = buf1[13];
                        servers_addr[2] = buf1[14];
                        servers_addr[3] = buf1[15];
                        servers_addr[4] = buf1[16];
                        servers_addr[5] = buf1[17];
                        servers_addr[6] = buf1[18];
                        servers_addr[7] = buf1[19];
                        servers_addr[8] = buf1[20];
                        servers_addr[9] = buf1[21];
                        servers_addr[10] = buf1[22];
                        servers_addr[11] = buf1[23];
                        servers_addr[12] = buf1[24];
                        servers_addr[13] = buf1[25];
                        servers_addr[14] = buf1[26];
                        servers_addr[15] = buf1[27];
                        servers_addr[16] = buf1[28];
                        servers_addr[17] = buf1[29];
                        servers_addr[18] = buf1[30];
                        servers_addr[19] = buf1[31];
                        servers_addr[20] = buf1[32];
                        servers_addr[21] = buf1[33];
                        servers_addr[22] = buf1[34];
                        servers_addr[23] = buf1[35];
                        servers_addr[24] = buf1[36];
                        servers_addr[25] = buf1[37];
                        servers_addr[26] = buf1[38];
                        servers_addr[27] = buf1[39];
                        servers_addr[28] = buf1[40];
                        servers_addr[29] = buf1[41];
                        eeprom_PageWrite(servers_addr, 0x0017, 30);
                        printf("\r\n服务器地址解析并存储完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改服务器地址\r\n");
                    }
                    // 解析服务器端口
                    if( buf1[42] != 0x00 || buf1[43] != 0x00 )
                    {
                        port = buf1[42];
                        port = ( port << 8 ) + buf1[43];
                        I2C2_ByteWrite((uchar)(port >> 8), 0x0035);
                        I2C2_ByteWrite((uchar)(port), 0x0036);
                        printf("\r\n服务器端口解析并存储完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改服务器端口\r\n");
                    }
                    // 解析北京时间
                    printf("\r\n开始解析北京时间并校时\r\n");
                    Correction_Time(buf1[44] , buf1[45] ,buf1[46], buf1[47] , buf1[48] , buf1[49] );
                    // 解析激活状态
                    if( buf1[50] != 0x00)
                    {
                        activate_status = buf1[50];
                        I2C2_ByteWrite( activate_status, 0x000A );
                        printf("\r\n设备激活状态解析并存储完成\r\n");
                        //-------如果设备更改为待激活模式，需清除激活过程-----------
                        if( buf1[50] == 0x02 )
                        {
                            I2C2_ByteWrite(0x00, 0x007C);
                            __delay_cycles(100000);
                        }
                    }
                    else
                    {
                        printf("\r\n不修改激活状态\r\n");
                    }
                    // 解析工作模式
                    if( buf1[51] != 0x00)
                    {
                        work_mode = buf1[51];
                        I2C2_ByteWrite( work_mode, 0x000B );
                        printf("\r\n设备工作模式解析并存储完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改工作模式\r\n");
                    }
                    // 解析工作时间
                    if( buf1[52] != 0x00)
                    {
                        work_time = buf1[52];
                        I2C2_ByteWrite( work_time, 0x000C );
                        printf("\r\n设备工作时间解析并存储完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改工作时间\r\n");
                    }
                    // 解析休眠时间
                    if( buf1[53] != 0x00 || buf1[54] != 0x00 )
                    {
                        sleep_time = buf1[53];
                        sleep_time = ( sleep_time << 8 ) + buf1[54];
                        I2C2_ByteWrite((uchar)(sleep_time >> 8), 0x000D);
                        I2C2_ByteWrite((uchar)(sleep_time), 0x000E);
                        printf("\r\n设备休眠时间解析并存储完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改休眠时间\r\n");
                    }
                    // 解析定时时间
                    if( buf1[55] != 0xFF || buf1[56] != 0xFF )
                    {
                        timing_time = buf1[55];
                        timing_time = ( timing_time << 8 ) + buf1[56];
                        //I2C2_ByteWrite((uchar)(timing_time >> 8), 0x0011);
                        //I2C2_ByteWrite((uchar)(timing_time), 0x0012);
                        printf("\r\n设备定时时间解析完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改定时时间\r\n");
                    }
                    // 解析间隔时间
                    if( buf1[57] != 0x00 || buf1[58] != 0x00 )
                    {
                        takt_time = buf1[57];
                        takt_time = ( takt_time << 8 ) + buf1[58];
                        I2C2_ByteWrite((uchar)(takt_time >> 8), 0x000F);
                        I2C2_ByteWrite((uchar)(takt_time), 0x0010);
                        printf("\r\n设备间隔时间解析并存储完成\r\n");
                    }
                    else
                    {
                        printf("\r\n不修改间隔时间\r\n");
                    }
                    // 解析累计工时
                    if( buf1[59] != 0xFF || buf1[60] != 0xFF || buf1[61] != 0xFF || buf1[62] != 0xFF )
                    {
                        labor_time = buf1[59];
                        labor_time = ( labor_time << 8 ) + buf1[60];
                        labor_time = ( labor_time << 8 ) + buf1[61];
                        labor_time = ( labor_time << 8 ) + buf1[62];
                        I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);
                        I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);
                        I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);
                        I2C2_ByteWrite((uchar)(labor_time), 0x0016);
                        printf("\r\n设备累计工时解析并存储完成\r\n");
                        //--------如果工时被清零设备重启次数一并清零------------
                        if(buf1[59] == 0x00 && buf1[60] == 0x00 && buf1[61] == 0x00 && buf1[62] == 0x00)
                        {
                            I2C2_ByteWrite(0x00, 0x003B);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x00, 0x003C);
                            __delay_cycles(100000);
                            system_rst = 0;
                        }
                        //-------如果工时被清零设备盲区数据一并清零-------------
                        if(buf1[59] == 0x00 && buf1[60] == 0x00 && buf1[61] == 0x00 && buf1[62] == 0x00)
                        {
                            I2C2_ByteWrite(0x00, 0x0037);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x00, 0x0038);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x00, 0x0039);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x01, 0x003A);
                            __delay_cycles(100000);
                        }
                    }
                    else
                    {
                        printf("\r\n不修改累计工时\r\n");
                    }
                    GSM_status = 0x2C;                                          // 准备响应服务器
                    config_flag = 0x0000;                                       // 参数设置成功
					data_type = 3;                                              //写参数响应
                    printf("\r\n服务器配置命令解析完成\r\n");
                    
                }
                else
                {
                    printf("\r\n服务器配置命令非法，丢弃数据\r\n");
                    GSM_status = 0x2C;                                          // 准备响应服务器
                    config_flag = 0xFFFF;                                       // 参数设置成功
					data_type = 3;                                              //写参数响应
                }
				
			}
			else
			{
				printf("\r\n没有服务器配置命令\r\n");
                GSM_status = 0xFF;                                              //设置GSM空闲状态
				data_type = 0;                                                  //恢复正常回传模式
			}
            break;
              
              
              
            case 0x31:
            if (GSM_time_flag)                                                  //定时时间到
            {	                
              if(strstr(GSM_RX0_buf, "OK") == NULL)

                //if(strstr(GSM_RX0_buf, "^SISW") == NULL)
                {
                  if( retry >= 3 )
                  {
                    printf("\r\n重试次数超过3次，重新开机\r\n");
                    GSM_POWER_OFF;
                    delay_ms(1000);
                    delay_ms(1000);
                    GSM_POWER_ON;
                    delay_ms(500);
                    GSM_status = 0x00;
                  }
                  else
                  {
                    printf("\r\nTCP响应失败，重新发送AT指令\r\n");
                    GSM_status = 0x2C;
                    retry++;
                  }
              }
              else
              {
                retry = 0;                                                  //重试次数清零
                GSM_status = 0xFF;                                          //设置GSM状态
                data_type = 0;                                              //恢复正常回传模式
                GSM_time_flag = 0;                                          //清定时标志位
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
                printf("\r\nTCP响应成功\r\n");
              }
            }
            break;
        case 0x40:
                  //printf("\r\n重试次数超过3次，重新开机\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;                                                //设置GPRS状态
            break;
        case 0xFF:
            break;
        default:
          
            printf("GSM状态非法: %d\r\n",GSM_status);
            printf("\r\n重新开机\r\n");
            GSM_POWER_OFF;
            delay_ms(1000);
            delay_ms(1000);
            GSM_POWER_ON;
            delay_ms(500);
            GSM_status = 0x00;                                             //设置GPRS状态
            break;
    }
}
//--------------ascii转16进制子函数---------------------------------------------
void ascii2data(char * des, char * src, int size)
{
	uint i = 0;
	uint j = 0;
	uchar highData;
	uchar lowData;
	i++;
	while (i < (size - 1)) 
	{
		if (src[i] <= '9' && src[i] >= '0')
			highData = src[i] - '0';
		else if(src[i] <= 'F' && src[i] >= 'A')
			highData = src[i] - 'A' + 0xa;
		i++;
		if (src[i] <= '9' && src[i] >= '0')
			lowData = src[i] - '0';
		else
			lowData = src[i] - 'A' + 0xa;
		des[j] = lowData + (highData << 4);
		j++;
		i++;
	}
}
//--------------16进制转ascii子函数---------------------------------------------
void data2ascii(char * des, char * src, int size)
{
	uint i = 0;
	uint j = 0;
	uchar highData;
	uchar lowData;
	while (i < size) 
    {
		highData	= (src[i] >> 4) & 0xf;
		lowData		= src[i] & 0xf;
		if(highData <= 0x09)
    		des[j] = highData+'0';
		else
    		des[j] = highData - 0x0A + 'A';
		j++;
		if(lowData <= 0x09)
    		des[j] = lowData + '0';
		else
    		des[j] = lowData - 0x0A + 'A';
		j++;
		i++;
	}	
}
