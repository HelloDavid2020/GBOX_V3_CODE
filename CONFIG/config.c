#include "MSP430F5438A.h"
#include "config.h"
#include "string.h"
#include "stdlib.h" 
#include "stdio.h"
#include "ctype.h"
#include "uart.h"
#include "at24c512.h"
#include "main.h"
#include "sd2068.h"
#include "packdata.h"
#include "adc.h"
#include "mg323b.h"
#include "ublox.h"
#include "isr.h"
char config_buf[100];
uchar config_array[50];
uchar config_num = 0;
uchar data_num;
uchar uart_flag = 0;
uchar server_len;
uint server_type;
uint command;
uint config_ip1,config_ip2,config_ip3,config_ip4;
uint config_year,config_mon,config_day,config_hour,config_min,config_sec;
unsigned long int config_temp_data;
uint w,q,a;

char command_rec_enable = 0;
uchar debug =0;
// 加入一个壳子shell，只有收到特定指令，才允许进行EEPROM读写操作。
// AT+RW=ENBALE
void scan_command(void)
{
  if(command_rec_enable==0)
  {
    
    if(uart_flag == 1)
    {  
      if(strstr(config_buf, "AT+RW=ENABLE") != NULL)
      {
        command_rec_enable=1;
        printf("\r\n> Config Enabled.\r\n");
      }
      uart_flag = 0;//清接受标志位
      memset( config_buf,0x00,sizeof( config_buf ) );//缓存区清零
      config_num = 0;//清接受数据量
      UCA3IE |= UCRXIE;           // 开串口3接受中断

    }
  }
  
}


void uscia_config( void )
{
    uchar ray;

    if(uart_flag == 1)
    {
      if(strstr(config_buf, "ATE1") != NULL)
      {
        debug  =1;
      }
      else  if(strstr(config_buf, "ATE0") != NULL)
      {
        debug  =0;
      }
      
      else  if(strstr(config_buf, "GPS1") != NULL)
      {
        gps_debug_flag  =1;debug  =0;
      }
      else  if(strstr(config_buf, "GPS0") != NULL)
      {
        gps_debug_flag  =0;
      }
              
        else if(strstr(config_buf, "AT+Read") != NULL)
        {
            data_num = sscanf((void*)config_buf,"AT+Read=%d",&command);
            if(data_num == 1)
            {
                switch(command)
                {
                    case 0x00:
                        printf("\r\n0读厂家编号成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0000);
                        printf("\r\n厂家编号：%.2ld\r\n",config_temp_data);
                        break;
                    case 0x01:
                        printf("\r\n1读设备编号成功\r\n");
                        eeprom_PageRead((uchar*)config_array, 0x0001, 8);
                        printf("\r\n设备编号：%.*s\r\n", 8, config_array);
                        break;
                    case 0x02:
                        printf("\r\n2读服务器地址成功\r\n");
                        eeprom_PageRead(config_array, 0x0017, 30);
                        if(config_array[0] == 0x01)
                        {
                            printf("\r\n服务器IP地址：%.3d.%.3d.%.3d.%.3d\r\n",config_array[26],config_array[27],config_array[28],config_array[29]);
                        }
                        else if(config_array[0] == 0x02)
                        {
                            printf("\r\n服务器域名地址：");
                            for(w = 1; w < 30 ;w++)
                            {
                                if(config_array[w] != 0x00)
                                {
                                    printf("%c",config_array[w]);
                                }
                            }
                            printf("\r\n");   
                        }
                        break;
                    case 0x03:
                        printf("\r\n3读服务器端口号成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0035);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0036);
                        printf("\r\n服务器端口号：%ld\r\n",config_temp_data);
                        break;
                    case 0x04:
                        printf("\r\n4读工作模式成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000B);
                        if(config_temp_data == 0x01)
                        {
                            printf("\r\n设备工作模式：标准模式\r\n");       
                        }
                        else if(config_temp_data == 0x02)
                        {
                            printf("\r\n设备工作模式：精准模式\r\n");        
                        }
                        else if(config_temp_data == 0x03)
                        {
                            printf("\r\n设备工作模式：追车模式\r\n");        
                        }
                        break;
                    case 0x05:
                        printf("\r\n5读工作时间成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000C);
                        printf("\r\n工作时间：%ld小时%ld分钟\r\n",config_temp_data / 60,config_temp_data % 60);
                        break;
                    case 0x06:
                        printf("\r\n6读休眠时间成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000D);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x000E);
                        printf("\r\n休眠时间：%ld小时%ld分钟\r\n",config_temp_data / 60,config_temp_data % 60);
                        break;
                    case 0x07:
                        printf("\r\n7读定时时间成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = timing_time;                        
                        if(config_temp_data == 0xFFFF)
                        {
                            printf("\r\n定时时间：未设置\r\n");
                        }
                        else
                        {
                            printf("\r\n定时时间：%ld时%ld分\r\n",config_temp_data / 60,config_temp_data % 60);
                        }
                        break;
                    case 0x08:
                        printf("\r\n8读间隔时间成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000F);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0010);
                        printf("\r\n间隔时间：%ld小时%ld分钟%ld秒\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);
                        break;
                    case 0x09:
                        printf("\r\n9读设备状态成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000A);
                        if(config_temp_data == 0x01)
                        {
                            printf("\r\n设备状态：测试\r\n");        
                        }
                        else if(config_temp_data == 0x02)
                        {
                            printf("\r\n设备状态：待激活\r\n");        
                        }
                        else if(config_temp_data == 0x03)
                        {
                            printf("\r\n设备状态：已激活\r\n");        
                        }
                        else if(config_temp_data == 0x04)
                        {
                            printf("\r\n设备状态：已拆除\r\n");        
                        }
                        break;  
                    case 0x0A:
                        printf("\r\n10读累计工时成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0013);
                        config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0014);
                        config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0015);
                        config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0016);
                        printf("\r\n设备累计工时：%ld小时%ld分钟%ld秒\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);
                        break;
                    case 0x0B:
                        printf("\r\n11读RTC时间成功\r\n");
                        Read_time();
                        printf("\r\nRTC时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                        break;
                    case 0x0C://读盲区指令
                         blind_data_size = ((I2C2_ByteRead(0x0037) << 8) | I2C2_ByteRead(0x0038));   //读取盲区数据量
                        __delay_cycles(10000);
                        //--------------------------------------------------------------------------
                        blind_data_head = ((I2C2_ByteRead(0x0039) << 8) | I2C2_ByteRead(0x003A));   //读取盲区头
                        __delay_cycles(10000);

                        printf("\r\n盲区数据量：%.3d\r\n",blind_data_size);
                        printf("\r\n盲区数据头：%.3d\r\n",blind_data_head);
                        
                        w = blind_data_head;
                        for(q = blind_data_size; q > 0; q--)
                        {
                            w--;
                            if(w == 0)
                                w = 500;
                            eeprom_PageRead((void*)data_packet, (w*128), 99);
                            printf("\r\n%.3d:\r\n",w);
                            for(a = 0; a < 99; a++)
                                printf("%02X ",data_packet[a]);
                            printf("\r\n");

                        }
                        printf("\r\n盲区数据读取完毕\r\n");
                        break;
                    case 0x0D:
                        printf("\r\n13读设备启动次数成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x003B);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x003C);
                        printf("\r\n设备启动次数：%ld次\r\n",config_temp_data);
                        break;
                    case 0x0E:
                        printf("\r\n14读设备维修号成功\r\n");
                        eeprom_PageRead((uchar*)config_array, 0x003D, 8);
                        if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                            && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                        {
                            printf("\r\n设备维修号：未设置\r\n");
                        }
                        else
                        {
                            printf("\r\n设备维修号：%.*s\r\n", 8, config_array);
                        }
                        break;
                    case 0x0F:
                        printf("\r\n15读设备批次号成功\r\n");
                        eeprom_PageRead((uchar*)config_array, 0x0045, 8);
                        if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                            && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                        {
                            printf("\r\n设备批次号：未设置\r\n");
                        }
                        else
                        {
                            printf("\r\n设备批次号：%.*s\r\n", 8, config_array);
                        }
                        break;
                    case 0x10:
                        printf("\r\n16读设备硬件版本号成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0009);
                        if(config_temp_data == 0)
                        {
                            printf("\r\n设备硬件版本号：未设置\r\n");
                        }
                        else
                        {
                            printf("\r\n设备硬件版本号：V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
                        }
                        break;
                    case 0x11:
                        printf("\r\n17读设备激活时需发送的数据量成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x007D);
                        printf("\r\n设备激活时需发送的数据量：%ld条\r\n",config_temp_data );
                        break;
                    case 0x12:
                        printf("\r\n18读设备RTC定时时间成功\r\n");
                        config_temp_data = 0;
                        config_temp_data = HEX2DEC(I2C0_ByteRead(0x09) & 0x3F);
                        printf("\r\n设备RTC定时时间：%.2ld时",config_temp_data );
                        config_temp_data = HEX2DEC(I2C0_ByteRead(0x08) & 0x7F);
                        printf("%.2ld分",config_temp_data );
                        config_temp_data = HEX2DEC(I2C0_ByteRead(0x07) & 0x7F);
                        printf("%.2ld秒\r\n",config_temp_data );
                        break;
                    default:
                        printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        break;
                }
            }
            else
            {
                printf("\r\n\a指令无法识别，请检查配置指令是否有误\r\n");
            }
        }
        else if(strstr(config_buf, "AT+Write") != NULL)
		{
            data_num = sscanf((void*)config_buf,"AT+Write=%d",&command);
            if(data_num == 1)
            {
                switch(command)
                {
                    case 0x00://写厂家编号
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 256)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x0000);
                                __delay_cycles(100000);
                                printf("\r\n0写厂家编号成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0000);
                                printf("\r\n厂家编号：%.2ld\r\n",config_temp_data);
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x01://写设备编号
                        memset(config_array,0x00,sizeof(config_array));
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%8s",config_array);
                        if(data_num == 1)
                        {
                            if(isalnum(config_array[0]) && isalnum(config_array[1]) && isalnum(config_array[2]) && isalnum(config_array[3])
                               && isalnum(config_array[4]) && isalnum(config_array[5]) && isalnum(config_array[6]) && isalnum(config_array[7]))
                            {
                                eeprom_PageWrite((uchar*)config_array, 0x0001, 8);
                                __delay_cycles(100000);
                                printf("\r\n1写设备编号成功\r\n");
                                eeprom_PageRead((uchar*)config_array, 0x0001, 8);
                                printf("\r\n设备编号：%.*s\r\n", 8, config_array);
                            }
                            else
                            {
                                printf("Error\a");
                            }
                        }
                        else
                        {
                            printf("Error\a");
                        }
                        break;
                    case 0x02://写服务器地址
                        
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%d,",&server_type);
                        if(data_num == 1 && server_type == 1)
                        {
                            data_num = sscanf((void*)config_buf,"AT+Write=%*d,%*d,%d.%d.%d.%d",&config_ip1,&config_ip2,&config_ip3,&config_ip4);
                            if(data_num == 4)
                            {
                                if(config_ip1 == 0)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else if(config_ip1 == 255)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else if(config_ip1 == 10)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else if(config_ip1 == 192 && config_ip2 == 168)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else if(config_ip1 == 172 && config_ip2 >= 16 && config_ip2 <= 31)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else if(config_ip1 == 127)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else if(config_ip1 > 255 || config_ip2 > 255 || config_ip3 > 255 || config_ip4 > 255)
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }
                                else
                                {
                                    I2C2_ByteWrite((uchar)0x01, 0x0017);
                                    __delay_cycles(100000);
                                    I2C2_ByteWrite((uchar)config_ip1, 0x0031);
                                    __delay_cycles(100000);
                                    I2C2_ByteWrite((uchar)config_ip2, 0x0032);
                                    __delay_cycles(100000);
                                    I2C2_ByteWrite((uchar)config_ip3, 0x0033);
                                    __delay_cycles(100000);
                                    I2C2_ByteWrite((uchar)config_ip4, 0x0034);
                                    __delay_cycles(100000);
                                    printf("\r\n2写服务器地址成功\r\n");
                                    eeprom_PageRead(servers_addr, 0x0017, 30);
                                    printf("\r\n服务器IP地址：%.3d.%.3d.%.3d.%.3d\r\n",servers_addr[26],servers_addr[27],servers_addr[28],servers_addr[29]);
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else if(data_num == 1 && server_type == 2)
                        {
                            memset(config_array,0x00,sizeof(config_array));
                            data_num = sscanf((void*)config_buf,"AT+Write=%*d,%*d,%s",config_array);
                            if(data_num == 1)
                            {
                                server_len = strlen((char*)config_array);
                                if(0 < server_len && server_len < 30)
                                {
                                    memset(servers_addr,0x00,sizeof(servers_addr));
                                    servers_addr[0] = 0x02;
                                    q = 30 - server_len;
                                    for(w = 0; w < server_len; w++)
                                    {
                                        servers_addr[q] = config_array[w];
                                        q++;
                                    }
                                    eeprom_PageWrite(servers_addr, 0x0017, 30);
                                    __delay_cycles(100000);
                                    printf("\r\n2写服务器地址成功\r\n");
                                    eeprom_PageRead(servers_addr, 0x0017, 30);
                                    printf("\r\n服务器域名地址：");
                                    for(w = 1; w < 30 ;w++)
                                    {
                                        if(servers_addr[w] != 0x00)
                                        {
                                            printf("%c",servers_addr[w]);
                                        }
                                    }
                                    printf("\r\n");
                                }
                                else
                                {
                                    printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                }  
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x03://写数据中心端口
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(1023 < config_temp_data && config_temp_data <65536)
                            {
                                I2C2_ByteWrite((uchar)(config_temp_data >> 8), 0x0035);
                                __delay_cycles(100000);
                                I2C2_ByteWrite((uchar)config_temp_data, 0x0036);
                                __delay_cycles(100000);
                                printf("\r\n3写数据中心端口成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0035);
                                config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0036);
                                printf("\r\n数据中心端口：%ld\r\n",config_temp_data);
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x04://写工作模式
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 4)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000B);
                                __delay_cycles(100000);
                                printf("\r\n4写工作模式成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000B);
                                if(config_temp_data == 0x01)
                                {
                                    printf("\r\n设备工作模式：标准模式\r\n");       
                                }
                                else if(config_temp_data == 0x02)
                                {
                                    printf("\r\n设备工作模式：精准模式\r\n");        
                                }
                                else if(config_temp_data == 0x03)
                                {
                                    printf("\r\n设备工作模式：追车模式\r\n");        
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x05://写工作时间
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(1 <= config_temp_data && config_temp_data <= 255)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000C);
                                __delay_cycles(100000);
                                printf("\r\n5写工作时间成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000C);
                                printf("\r\n工作时间：%ld小时%ld分钟\r\n",config_temp_data / 60,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x06://写休眠时间
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(2 <= config_temp_data && config_temp_data <= 1440)
                            {
                                I2C2_ByteWrite((uchar)(config_temp_data >> 8), 0x000D);
                                __delay_cycles(100000);
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000E);
                                __delay_cycles(100000);
                                printf("\r\n6写休眠时间成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000D);
                                config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x000E);
                                printf("\r\n休眠时间：%ld小时%ld分钟\r\n",config_temp_data / 60,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x07://写定时时间
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 1440 || config_temp_data == 0xFFFF)
                            {
                                //I2C1_ByteWrite((uchar)(config_temp_data >> 8), 0x0011);
                                //__delay_cycles(100000);
                                //I2C1_ByteWrite((uchar)config_temp_data, 0x0012);
                                //__delay_cycles(100000);
                                timing_time = (uint)(config_temp_data);
                                printf("\r\n7写定时时间成功\r\n");
                                config_temp_data = 0;
                                //config_temp_data = I2C2_ByteRead(0x0011);
                                //config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0012);
                                config_temp_data = timing_time;
                                if(config_temp_data == 0xFFFF)
                                {
                                    printf("\r\n定时时间：未设置\r\n");
                                }
                                else
                                {
                                    printf("\r\n定时时间：%ld时%ld分\r\n",config_temp_data / 60,config_temp_data % 60);
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x08://写间隔时间
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(30 <= config_temp_data && config_temp_data <= 65535)
                            {
                                I2C2_ByteWrite((uchar)(config_temp_data >> 8), 0x000F);
                                __delay_cycles(100000);
                                I2C2_ByteWrite((uchar)config_temp_data, 0x0010);
                                __delay_cycles(100000);
                                printf("\r\n8写间隔时间成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000F);
                                config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0010);
                                printf("\r\n间隔时间：%ld小时%ld分钟%ld秒\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x09://写设备状态
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(0 < config_temp_data && config_temp_data < 5)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000A);
                                __delay_cycles(100000);
                                printf("\r\n9写设备状态成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000A);
                                if(config_temp_data == 0x01)
                                {
                                    printf("\r\n设备状态：测试\r\n");        
                                }
                                else if(config_temp_data == 0x02)
                                {
                                    printf("\r\n设备状态：待激活\r\n");        
                                }
                                else if(config_temp_data == 0x03)
                                {
                                    printf("\r\n设备状态：已激活\r\n");        
                                }
                                else if(config_temp_data == 0x04)
                                {
                                    printf("\r\n设备状态：已拆除\r\n");        
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x0A://写累计工时
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 0xFFFFFFFF)
                            {
                                I2C2_ByteWrite((uchar)(config_temp_data >> 24), 0x0013);
                                __delay_cycles(100000);
                                I2C2_ByteWrite((uchar)(config_temp_data >> 16), 0x0014);
                                __delay_cycles(100000);
                                I2C2_ByteWrite((uchar)(config_temp_data >> 8), 0x0015);
                                __delay_cycles(100000);
                                I2C2_ByteWrite((uchar)(config_temp_data), 0x0016);
                                __delay_cycles(100000);
                                printf("\r\n10写累计工时成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0013);
                                config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0014);
                                config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0015);
                                config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0016);
                                printf("\r\n设备累计工时：%ld小时%ld分钟%ld秒\r\n",config_temp_data / 3600,config_temp_data / 60 % 60 ,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x0B://写日期
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%d-%d-%d.%d:%d:%d",&config_year,&config_mon,&config_day,&config_hour,&config_min,&config_sec);
                        if(data_num == 6)
                        {
                            RTC_year = (uchar)config_year;
                            RTC_mon = (uchar)config_mon;
                            RTC_day = (uchar)config_day;
                            RTC_hour = (uchar)config_hour;
                            RTC_min = (uchar)config_min;
                            RTC_sec = (uchar)config_sec;
                            if(13 > RTC_year || RTC_year > 99)
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                            else if(1 > RTC_mon || RTC_mon > 12)
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                            else if(RTC_hour > 23)
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }  
                            else if(RTC_min > 59)
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                            else if(RTC_sec > 59)
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                            else
                            {
                                if(RTC_mon == 1 || RTC_mon == 3 || RTC_mon == 5 || RTC_mon == 7 || RTC_mon == 8 || RTC_mon == 10 || RTC_mon == 12)
                                {
                                    if(1 > RTC_day || RTC_day > 31)
                                    {
                                        printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                    }
                                    else
                                    {
                                        Write_time();
                                        __delay_cycles(100000);
                                        printf("\r\n11写日期时间成功\r\n");
                                        Read_time();
                                        printf("\r\nRTC时间：%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                    }
                                    
                                }
                                else if(RTC_mon == 4 || RTC_mon == 6 || RTC_mon == 9 || RTC_mon == 11)
                                {
                                    if(1 > RTC_day || RTC_day > 30)
                                    {
                                        printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                    }
                                    else
                                    {
                                        Write_time();
                                        __delay_cycles(100000);
                                        printf("\r\n11写日期时间成功\r\n");
                                        Read_time();
                                        printf("\r\nRTC时间：%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                    }
                                }
                                else if(RTC_mon == 2)
                                {
                                    if(RTC_year%4 == 0)
                                    {
                                        if(1 > RTC_day || RTC_day > 29)
                                        {
                                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                        }
                                        else
                                        {
                                            Write_time();
                                            __delay_cycles(100000);
                                            printf("\r\n11写日期时间成功\r\n");
                                            Read_time();
                                            printf("\r\nRTC时间：%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                        }
                                    }
                                    else
                                    {
                                        if(1 > RTC_day || RTC_day > 28)
                                        {
                                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                                        }
                                        else
                                        {
                                            Write_time();
                                            __delay_cycles(100000);
                                            printf("\r\n11写日期时间成功\r\n");
                                            Read_time();
                                            printf("\r\nRTC时间：%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                        }
                                    }
                                } 
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x0C:
                        printf("\r\n12清盲区数据成功\r\n");
                        I2C2_ByteWrite(0x00, 0x0037);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x00, 0x0038);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x00, 0x0039);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x01, 0x003A);
                        __delay_cycles(100000);
                        blind_data_size = ((I2C2_ByteRead(0x0037) << 8) | I2C2_ByteRead(0x0038));   //读取盲区数据量
                        __delay_cycles(10000);
                        //--------------------------------------------------------------------------
                        blind_data_head = ((I2C2_ByteRead(0x0039) << 8) | I2C2_ByteRead(0x003A));   //读取盲区头
                        __delay_cycles(10000);
                        printf("\r\n盲区数据量:%.3d\r\n",blind_data_size);
                        printf("\r\n盲区数据指针:%.3d\r\n",blind_data_head);
                        break;
                    case 0x0D:
                        printf("\r\n13清重启次数成功\r\n");
                        I2C2_ByteWrite(0x00, 0x003B);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x00, 0x003C);
                        __delay_cycles(100000);
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0022);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0023);
                        printf("\r\n设备重启次数：%ld\r\n",config_temp_data);
                        break;
                    case 0x0E:
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%8s",config_array);
                        if(data_num == 1)
                        {
                            if(isalnum(config_array[0]) && isalnum(config_array[1]) && isalnum(config_array[2]) && isalnum(config_array[3])
                               && isalnum(config_array[4]) && isalnum(config_array[5]) && isalnum(config_array[6]) && isalnum(config_array[7]))
                            {
                                eeprom_PageWrite((uchar*)config_array, 0x003D, 8);
                                __delay_cycles(100000);
                                printf("\r\n14写设备维修号成功\r\n");
                                eeprom_PageRead((uchar*)config_array, 0x003D, 8);
                                if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                                    && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                                {
                                    printf("\r\n设备维修号：未设置\r\n");
                                }
                                else
                                {
                                    printf("\r\n设备维修号：%.*s\r\n", 8, config_array);
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x0F:
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%8s",config_array);
                        if(data_num == 1)
                        {
                            if(isalnum(config_array[0]) && isalnum(config_array[1]) && isalnum(config_array[2]) && isalnum(config_array[3])
                               && isalnum(config_array[4]) && isalnum(config_array[5]) && isalnum(config_array[6]) && isalnum(config_array[7]))
                            {
                                eeprom_PageWrite((uchar*)config_array, 0x0045, 8);
                                __delay_cycles(100000);
                                printf("\r\n15写生产批次号成功\r\n");
                                eeprom_PageRead((uchar*)config_array, 0x0045, 8);
                                if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                                    && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                                {
                                    printf("\r\n设备批次号：未设置\r\n");
                                }
                                else
                                {
                                    printf("\r\n设备批次号：%.*s\r\n", 8, config_array);
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    case 0x10:
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld.%*ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            hardware = config_temp_data * 10;
                            data_num = sscanf((void*)config_buf,"AT+Write=%*d,%*ld.%ld",&config_temp_data);
                            if(data_num == 1)
                            {
                                hardware = hardware + config_temp_data;
                                printf("\r\n16写硬件版本号成功\r\n");
                                I2C2_ByteWrite(hardware, 0x0009);
                                __delay_cycles(100000);
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0009);
                                if(config_temp_data == 0)
                                {
                                    printf("\r\n设备硬件版本号：未设置\r\n");
                                }
                                else
                                {
                                    printf("\r\n设备硬件版本号：V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
                                }
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        
                        
                        break;
                    case 0x11:
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 256)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x007D);
                                __delay_cycles(100000);
                                printf("\r\n17写设备激活时需发送的数据量成功\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x007D);
                                printf("\r\n设备激活时需发送的数据量：%ld条\r\n",config_temp_data );
                            }
                            else
                            {
                                printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                        }
                        break;
                    default:
                        printf("\r\n指令无法识别，请检查配置指令是否有误\r\n");
                    break;
                }
            }
            else
            {
                printf("\r\n\a指令无法识别，请检查配置指令是否有误\r\n");
            }
        }
        else if(strstr(config_buf, "AT+Reset") != NULL)
        {
            eeprom_PageWrite( (void*)config_data, 0x0000, 128);
            __delay_cycles(100000);
            printf("\r\n恢复默认设置成功\r\n");
            //------------------------------------------------------------------

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0000);
            printf("\r\n厂家编号：%.2ld\r\n",config_temp_data);

            eeprom_PageRead((uchar*)config_array, 0x0001, 8);
            printf("\r\n设备编号：%.*s\r\n", 8, config_array);

            eeprom_PageRead(config_array, 0x0017, 30);
            if(config_array[0] == 0x01)
            {
            printf("\r\n服务器IP地址：%.3d.%.3d.%.3d.%.3d\r\n",config_array[26],config_array[27],config_array[28],config_array[29]);
            }
            else if(config_array[0] == 0x02)
            {
            printf("\r\n服务器域名地址：");
            for(w = 1; w < 30 ;w++)
            {
            if(config_array[w] != 0x00)
            {
                printf("%c",config_array[w]);
            }
            }
            printf("\r\n");   
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0035);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0036);
            printf("\r\n服务器端口号：%ld\r\n",config_temp_data);

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000B);
            if(config_temp_data == 0x01)
            {
            printf("\r\n设备工作模式：标准模式\r\n");       
            }
            else if(config_temp_data == 0x02)
            {
            printf("\r\n设备工作模式：精准模式\r\n");        
            }
            else if(config_temp_data == 0x03)
            {
            printf("\r\n设备工作模式：追车模式\r\n");        
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000C);
            printf("\r\n工作时间：%ld小时%ld分钟\r\n",config_temp_data / 60,config_temp_data % 60);

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000D);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x000E);
            printf("\r\n休眠时间：%ld小时%ld分钟\r\n",config_temp_data / 60,config_temp_data % 60);

            config_temp_data = 0;
            config_temp_data = timing_time;                        
            if(config_temp_data == 0xFFFF)
            {
            printf("\r\n定时时间：未设置\r\n");
            }
            else
            {
            printf("\r\n定时时间：%ld时%ld分\r\n",config_temp_data / 60,config_temp_data % 60);
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000F);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0010);
            printf("\r\n间隔时间：%ld小时%ld分钟%ld秒\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000A);
            if(config_temp_data == 0x01)
            {
            printf("\r\n设备状态：测试\r\n");        
            }
            else if(config_temp_data == 0x02)
            {
            printf("\r\n设备状态：待激活\r\n");        
            }
            else if(config_temp_data == 0x03)
            {
            printf("\r\n设备状态：已激活\r\n");        
            }
            else if(config_temp_data == 0x04)
            {
            printf("\r\n设备状态：已拆除\r\n");        
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0013);
            config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0014);
            config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0015);
            config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0016);
            printf("\r\n设备累计工时：%ld小时%ld分钟%ld秒\r\n",config_temp_data / 3600,config_temp_data % 60 / 60,config_temp_data % 60);

            Read_time();
            printf("\r\nRTC时间：20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);

            blind_data_size = ((I2C2_ByteRead(0x0037) << 8) | I2C2_ByteRead(0x0038));   //读取盲区数据量
            __delay_cycles(10000);
            //--------------------------------------------------------------------------
            blind_data_head = ((I2C2_ByteRead(0x0039) << 8) | I2C2_ByteRead(0x003A));   //读取盲区头
            __delay_cycles(10000);

            printf("\r\n盲区数据量：%.3d\r\n",blind_data_size);
            printf("\r\n盲区数据头：%.3d\r\n",blind_data_head);

            w = blind_data_head;
            for(q = blind_data_size; q > 0; q--)
            {
            w--;
            if(w == 0)
            w = 500;
            eeprom_PageRead((void*)data_packet, (w*128), 99);
            printf("\r\n%.3d:\r\n",w);
            for(a = 0; a < 99; a++)
            printf("%02X ",data_packet[a]);
            printf("\r\n");

            }
            printf("\r\n盲区数据读取完毕\r\n");

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x003B);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x003C);
            printf("\r\n设备启动次数：%ld次\r\n",config_temp_data);

            eeprom_PageRead((uchar*)config_array, 0x003D, 8);
            if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
            {
                printf("\r\n设备维修号：未设置\r\n");
            }
            else
            {
                printf("\r\n设备维修号：%.*s\r\n", 8, config_array);
            }
            
            eeprom_PageRead((uchar*)config_array, 0x0045, 8);
            if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
            {
                printf("\r\n设备批次号：未设置\r\n");
            }
            else
            {
                printf("\r\n设备批次号：%.*s\r\n", 8, config_array);
            }
            
            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0009);
            if(config_temp_data == 0)
            {
                printf("\r\n设备硬件版本号：未设置\r\n");
            }
            else
            {
                printf("\r\n设备硬件版本号：V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
            }

            //------------------------------------------------------------------
        }
        else if(strstr(config_buf, "AT+Test") != NULL)
        {
            __delay_cycles(100000);
            printf("\r\n开始自检\r\n");
            __delay_cycles(100000);
            printf("\r\n软件版本号：V%d.%d\r\n",procedure_VER / 10,procedure_VER % 10);
            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0009);
            if(config_temp_data == 0)
            {
                printf("\r\n设备硬件版本号：未设置\r\n");
            }
            else
            {
                printf("\r\n设备硬件版本号：V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
            }
            
            AD_collect();
            printf("\r\n电池电压：%d.%dV\r\n",AD_voltage / 1000,AD_voltage % 1000);
            
            I2C2_ByteWrite(0x55, 0xFFFD);
            __delay_cycles(100000);
            I2C2_ByteWrite(0xAA, 0xFFFE);
            __delay_cycles(100000);
            I2C2_ByteWrite(0x55, 0xFFFF);
            __delay_cycles(100000);
            if(I2C2_ByteRead(0xFFFD) == 0x55 && I2C2_ByteRead(0xFFFE) == 0xAA && I2C2_ByteRead(0xFFFF) == 0x55)
            {
                printf("\r\nEEP模块正常\r\n");
                I2C2_ByteWrite(0x00, 0xFFFD);
                __delay_cycles(100000);
                I2C2_ByteWrite(0x00, 0xFFFE);
                __delay_cycles(100000);
                I2C2_ByteWrite(0x00, 0xFFFF);
                __delay_cycles(100000);
            }
            else
            {
                if(I2C2_ByteRead(0xFFFD) == 0x55 && I2C2_ByteRead(0xFFFE) == 0xAA && I2C2_ByteRead(0xFFFF) == 0x55)
                {
                    printf("\r\nEEP模块正常\r\n");
                    I2C2_ByteWrite(0x00, 0xFFFD);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0xFFFE);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0xFFFF);
                    __delay_cycles(100000);
                }
                else
                {
                    printf("\r\nEEP模块错误\a\r\n");
                    I2C2_ByteWrite(0x00, 0xFFFD);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0xFFFE);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0xFFFF);
                    __delay_cycles(100000);
                }
            }
            
            RTC_year = 0x01;
            RTC_mon = 0x02;
            RTC_day = 0x03;
            RTC_hour = 0x04;
            RTC_min = 0x05;
            RTC_sec = 0x06;
            Write_time();
            __delay_cycles(100000);
            Read_time();
            __delay_cycles(10000);
            if(RTC_year == 0x01 && RTC_mon == 0x02 && RTC_day == 0x03 && RTC_hour == 0x04 && RTC_min ==0x05 && RTC_sec == 0x06)
            {
                printf("\r\nRTC模块正常\r\n");
                RTC_year = 0x0D;
                RTC_mon = 0x01;
                RTC_day = 0x01;
                RTC_hour = 0x00;
                RTC_min = 0x00;
                RTC_sec = 0x00;
                Write_time();
                __delay_cycles(100000);
            }
            else
            {
                Read_time();
                __delay_cycles(10000);
                if(RTC_year == 0x01 && RTC_mon == 0x02 && RTC_day == 0x03 && RTC_hour == 0x04 && RTC_min ==0x05 && RTC_sec == 0x06)
                {
                    printf("\r\nRTC模块正常\r\n");
                    RTC_year = 0x0D;
                    RTC_mon = 0x01;
                    RTC_day = 0x01;
                    RTC_hour = 0x00;
                    RTC_min = 0x00;
                    RTC_sec = 0x00;
                    Write_time();
                    __delay_cycles(100000);
                }
                else
                {
                    printf("\r\nRTC模块错误\a\r\n");
                    RTC_year = 0x0D;
                    RTC_mon = 0x01;
                    RTC_day = 0x01;
                    RTC_hour = 0x00;
                    RTC_min = 0x00;
                    RTC_sec = 0x00;
                    Write_time();
                    __delay_cycles(100000);
                }
            }
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
                if((P1IN & BIT5) != ray)
                {
                    ray = (P1IN & BIT5);
                    if(ray == 0x00)
                    {
                        printf("\r\n光敏未见光\r\n");
                    }
                    else if(ray == 0x20)
                    {
                        printf("\r\n光敏已见光\r\n");
                    }
                }
                if(GSM_status != 0xFF)
                {
                    GSM();
                }
                if(GSM_status == 0x2C)
                {
                    printf("\r\nGSM模块正常\r\n");
                    GSM_status = 0xFF;
                    GSM_POWER_OFF;//关闭GSM电源
                    
                    gps_debug_flag = 1;
                    debug = 0;
                    
                }
                if(GPS_analysis_end > GPS_analysis_num && GPS_analysis_end != 0xFF)
                {
                    printf("\r\nGPS模块正常\r\n");
                    printf("\r\n定位标志：%c\r\n",GPS_Locate);
                    printf("\r\nUTC时间：%.*s\r\n", 6, GPS_UTC_time);
                    printf("\r\nUTC日期：%.*s\r\n", 6, GPS_UTC_date);
                    printf("\r\nGPS南北纬：%c\r\n", GPS_north_south);
                    printf("\r\nGPS纬度：%.*s\r\n", 10, GPS_latitude);
                    printf("\r\nGPS东西经：%c\r\n", GPS_east_west);
                    printf("\r\nGPS经度：%.*s\r\n", 11, GPS_Longitude);
                    printf("\r\nGPS速度：%.*s\r\n", 5, GPS_Speed);
                    printf("\r\nGPS方向：%.*s\r\n", 6, GPS_direction);
                    printf("\r\nGPS海拔：%.*s\r\n", 7, GPS_elevation);
                    printf("\r\n使用卫星颗数：%.*s\r\n", 2, GPS_employ_satellite);
                    printf("\r\n可视卫星颗数：%.*s\r\n", 2, GPS_employ_satellite);
                    GPS_POWER_OFF;
                    GPS_analysis_end = 0xFF;
                }
                if(GPS_analysis_end == 0xFF && GSM_status == 0xFF)
                {
                    printf("\r\n自检结束\r\n");
                    
                    I2C2_ByteWrite(0x00, 0x003B);   // 请启动次数 盲区数据量
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0x003C);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0x0037);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0x0038);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0x0039);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x01, 0x003A);
                    __delay_cycles(100000);
                    
                    GSM_status = 0x00;  // 启动前清GSM状态
                    data_type = 0x00;   // 启动数据类型状态
                    GPS_analysis_end = 0x00;    // 启动前清GPS解析次数
                    GPS_Locate = 'V';       // 启动前清GPS定位标志
                    GPS_north_south = 'X';  // 启动前清南北纬标志
                    GPS_east_west = 'X';    // 启动前清东西经标志
                    data_send_flag = 0x00;  // 启动前清发送成功标志
                    GPS_start = 0x00;       // 启动前清GPS开始解析标志
                    timing_time = 0xFFFF;   // 启动前清定时时间
                    time_correction = 0x00; // 启动前清校时成功标志
                    
                    memset(GPS_UTC_time,0x00,sizeof(GPS_UTC_time));     // 启动前清GPS时间
                    memset(GPS_UTC_date,0x00,sizeof(GPS_UTC_date));     // 启动前清GPS日期
                    memset(GPS_latitude,0x00,sizeof(GPS_latitude));     // 启动前清GPS纬度
                    memset(GPS_Longitude,0x00,sizeof(GPS_Longitude));   // 启动前清GPS经度
                    memset(GPS_Speed,0x00,sizeof(GPS_Speed));           // 启动前清GPS速度
                    memset(GPS_direction,0x00,sizeof(GPS_direction));   // 启动前清GPS方向
                    memset(GPS_elevation,0x00,sizeof(GPS_elevation));   // 启动前清GPS高度
                    memset(GPS_employ_satellite,0x00,sizeof(GPS_employ_satellite));     // 启动前清GPS使用卫星颗数
                    memset(GPS_visible_satellite,0x00,sizeof(GPS_visible_satellite));   // 启动前清GPS可见卫星颗数
                    memset(data_packet,0x00,sizeof(data_packet));       // 
                    
                    break;
                }
                if((P2IN & BIT3) == 0x00)
                {
                    printf("\r\n设备退出配置模式\r\n");
                    break;
                }
            }
            
        }
        else
		{
			printf("\r\n\a指令无法识别，请检查配置指令是否有误\r\n");
		}
        uart_flag = 0;//清接受标志位
        memset( config_buf,0x00,sizeof( config_buf ) );//缓存区清零
        config_num = 0;//清接受数据量
        UCA3IE |= UCRXIE;           // 开串口3接受中断
    }
}