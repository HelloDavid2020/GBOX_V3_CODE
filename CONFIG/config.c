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
// ����һ������shell��ֻ���յ��ض�ָ����������EEPROM��д������
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
      uart_flag = 0;//����ܱ�־λ
      memset( config_buf,0x00,sizeof( config_buf ) );//����������
      config_num = 0;//�����������
      UCA3IE |= UCRXIE;           // ������3�����ж�

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
                        printf("\r\n0�����ұ�ųɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0000);
                        printf("\r\n���ұ�ţ�%.2ld\r\n",config_temp_data);
                        break;
                    case 0x01:
                        printf("\r\n1���豸��ųɹ�\r\n");
                        eeprom_PageRead((uchar*)config_array, 0x0001, 8);
                        printf("\r\n�豸��ţ�%.*s\r\n", 8, config_array);
                        break;
                    case 0x02:
                        printf("\r\n2����������ַ�ɹ�\r\n");
                        eeprom_PageRead(config_array, 0x0017, 30);
                        if(config_array[0] == 0x01)
                        {
                            printf("\r\n������IP��ַ��%.3d.%.3d.%.3d.%.3d\r\n",config_array[26],config_array[27],config_array[28],config_array[29]);
                        }
                        else if(config_array[0] == 0x02)
                        {
                            printf("\r\n������������ַ��");
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
                        printf("\r\n3���������˿ںųɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0035);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0036);
                        printf("\r\n�������˿ںţ�%ld\r\n",config_temp_data);
                        break;
                    case 0x04:
                        printf("\r\n4������ģʽ�ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000B);
                        if(config_temp_data == 0x01)
                        {
                            printf("\r\n�豸����ģʽ����׼ģʽ\r\n");       
                        }
                        else if(config_temp_data == 0x02)
                        {
                            printf("\r\n�豸����ģʽ����׼ģʽ\r\n");        
                        }
                        else if(config_temp_data == 0x03)
                        {
                            printf("\r\n�豸����ģʽ��׷��ģʽ\r\n");        
                        }
                        break;
                    case 0x05:
                        printf("\r\n5������ʱ��ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000C);
                        printf("\r\n����ʱ�䣺%ldСʱ%ld����\r\n",config_temp_data / 60,config_temp_data % 60);
                        break;
                    case 0x06:
                        printf("\r\n6������ʱ��ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000D);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x000E);
                        printf("\r\n����ʱ�䣺%ldСʱ%ld����\r\n",config_temp_data / 60,config_temp_data % 60);
                        break;
                    case 0x07:
                        printf("\r\n7����ʱʱ��ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = timing_time;                        
                        if(config_temp_data == 0xFFFF)
                        {
                            printf("\r\n��ʱʱ�䣺δ����\r\n");
                        }
                        else
                        {
                            printf("\r\n��ʱʱ�䣺%ldʱ%ld��\r\n",config_temp_data / 60,config_temp_data % 60);
                        }
                        break;
                    case 0x08:
                        printf("\r\n8�����ʱ��ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000F);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0010);
                        printf("\r\n���ʱ�䣺%ldСʱ%ld����%ld��\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);
                        break;
                    case 0x09:
                        printf("\r\n9���豸״̬�ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x000A);
                        if(config_temp_data == 0x01)
                        {
                            printf("\r\n�豸״̬������\r\n");        
                        }
                        else if(config_temp_data == 0x02)
                        {
                            printf("\r\n�豸״̬��������\r\n");        
                        }
                        else if(config_temp_data == 0x03)
                        {
                            printf("\r\n�豸״̬���Ѽ���\r\n");        
                        }
                        else if(config_temp_data == 0x04)
                        {
                            printf("\r\n�豸״̬���Ѳ��\r\n");        
                        }
                        break;  
                    case 0x0A:
                        printf("\r\n10���ۼƹ�ʱ�ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0013);
                        config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0014);
                        config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0015);
                        config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0016);
                        printf("\r\n�豸�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);
                        break;
                    case 0x0B:
                        printf("\r\n11��RTCʱ��ɹ�\r\n");
                        Read_time();
                        printf("\r\nRTCʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                        break;
                    case 0x0C://��ä��ָ��
                         blind_data_size = ((I2C2_ByteRead(0x0037) << 8) | I2C2_ByteRead(0x0038));   //��ȡä��������
                        __delay_cycles(10000);
                        //--------------------------------------------------------------------------
                        blind_data_head = ((I2C2_ByteRead(0x0039) << 8) | I2C2_ByteRead(0x003A));   //��ȡä��ͷ
                        __delay_cycles(10000);

                        printf("\r\nä����������%.3d\r\n",blind_data_size);
                        printf("\r\nä������ͷ��%.3d\r\n",blind_data_head);
                        
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
                        printf("\r\nä�����ݶ�ȡ���\r\n");
                        break;
                    case 0x0D:
                        printf("\r\n13���豸���������ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x003B);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x003C);
                        printf("\r\n�豸����������%ld��\r\n",config_temp_data);
                        break;
                    case 0x0E:
                        printf("\r\n14���豸ά�޺ųɹ�\r\n");
                        eeprom_PageRead((uchar*)config_array, 0x003D, 8);
                        if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                            && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                        {
                            printf("\r\n�豸ά�޺ţ�δ����\r\n");
                        }
                        else
                        {
                            printf("\r\n�豸ά�޺ţ�%.*s\r\n", 8, config_array);
                        }
                        break;
                    case 0x0F:
                        printf("\r\n15���豸���κųɹ�\r\n");
                        eeprom_PageRead((uchar*)config_array, 0x0045, 8);
                        if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                            && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                        {
                            printf("\r\n�豸���κţ�δ����\r\n");
                        }
                        else
                        {
                            printf("\r\n�豸���κţ�%.*s\r\n", 8, config_array);
                        }
                        break;
                    case 0x10:
                        printf("\r\n16���豸Ӳ���汾�ųɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0009);
                        if(config_temp_data == 0)
                        {
                            printf("\r\n�豸Ӳ���汾�ţ�δ����\r\n");
                        }
                        else
                        {
                            printf("\r\n�豸Ӳ���汾�ţ�V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
                        }
                        break;
                    case 0x11:
                        printf("\r\n17���豸����ʱ�跢�͵��������ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x007D);
                        printf("\r\n�豸����ʱ�跢�͵���������%ld��\r\n",config_temp_data );
                        break;
                    case 0x12:
                        printf("\r\n18���豸RTC��ʱʱ��ɹ�\r\n");
                        config_temp_data = 0;
                        config_temp_data = HEX2DEC(I2C0_ByteRead(0x09) & 0x3F);
                        printf("\r\n�豸RTC��ʱʱ�䣺%.2ldʱ",config_temp_data );
                        config_temp_data = HEX2DEC(I2C0_ByteRead(0x08) & 0x7F);
                        printf("%.2ld��",config_temp_data );
                        config_temp_data = HEX2DEC(I2C0_ByteRead(0x07) & 0x7F);
                        printf("%.2ld��\r\n",config_temp_data );
                        break;
                    default:
                        printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        break;
                }
            }
            else
            {
                printf("\r\n\aָ���޷�ʶ����������ָ���Ƿ�����\r\n");
            }
        }
        else if(strstr(config_buf, "AT+Write") != NULL)
		{
            data_num = sscanf((void*)config_buf,"AT+Write=%d",&command);
            if(data_num == 1)
            {
                switch(command)
                {
                    case 0x00://д���ұ��
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 256)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x0000);
                                __delay_cycles(100000);
                                printf("\r\n0д���ұ�ųɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0000);
                                printf("\r\n���ұ�ţ�%.2ld\r\n",config_temp_data);
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x01://д�豸���
                        memset(config_array,0x00,sizeof(config_array));
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%8s",config_array);
                        if(data_num == 1)
                        {
                            if(isalnum(config_array[0]) && isalnum(config_array[1]) && isalnum(config_array[2]) && isalnum(config_array[3])
                               && isalnum(config_array[4]) && isalnum(config_array[5]) && isalnum(config_array[6]) && isalnum(config_array[7]))
                            {
                                eeprom_PageWrite((uchar*)config_array, 0x0001, 8);
                                __delay_cycles(100000);
                                printf("\r\n1д�豸��ųɹ�\r\n");
                                eeprom_PageRead((uchar*)config_array, 0x0001, 8);
                                printf("\r\n�豸��ţ�%.*s\r\n", 8, config_array);
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
                    case 0x02://д��������ַ
                        
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%d,",&server_type);
                        if(data_num == 1 && server_type == 1)
                        {
                            data_num = sscanf((void*)config_buf,"AT+Write=%*d,%*d,%d.%d.%d.%d",&config_ip1,&config_ip2,&config_ip3,&config_ip4);
                            if(data_num == 4)
                            {
                                if(config_ip1 == 0)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }
                                else if(config_ip1 == 255)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }
                                else if(config_ip1 == 10)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }
                                else if(config_ip1 == 192 && config_ip2 == 168)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }
                                else if(config_ip1 == 172 && config_ip2 >= 16 && config_ip2 <= 31)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }
                                else if(config_ip1 == 127)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }
                                else if(config_ip1 > 255 || config_ip2 > 255 || config_ip3 > 255 || config_ip4 > 255)
                                {
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
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
                                    printf("\r\n2д��������ַ�ɹ�\r\n");
                                    eeprom_PageRead(servers_addr, 0x0017, 30);
                                    printf("\r\n������IP��ַ��%.3d.%.3d.%.3d.%.3d\r\n",servers_addr[26],servers_addr[27],servers_addr[28],servers_addr[29]);
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
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
                                    printf("\r\n2д��������ַ�ɹ�\r\n");
                                    eeprom_PageRead(servers_addr, 0x0017, 30);
                                    printf("\r\n������������ַ��");
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
                                    printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                }  
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x03://д�������Ķ˿�
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
                                printf("\r\n3д�������Ķ˿ڳɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0035);
                                config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0036);
                                printf("\r\n�������Ķ˿ڣ�%ld\r\n",config_temp_data);
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x04://д����ģʽ
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(config_temp_data < 4)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000B);
                                __delay_cycles(100000);
                                printf("\r\n4д����ģʽ�ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000B);
                                if(config_temp_data == 0x01)
                                {
                                    printf("\r\n�豸����ģʽ����׼ģʽ\r\n");       
                                }
                                else if(config_temp_data == 0x02)
                                {
                                    printf("\r\n�豸����ģʽ����׼ģʽ\r\n");        
                                }
                                else if(config_temp_data == 0x03)
                                {
                                    printf("\r\n�豸����ģʽ��׷��ģʽ\r\n");        
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x05://д����ʱ��
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(1 <= config_temp_data && config_temp_data <= 255)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000C);
                                __delay_cycles(100000);
                                printf("\r\n5д����ʱ��ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000C);
                                printf("\r\n����ʱ�䣺%ldСʱ%ld����\r\n",config_temp_data / 60,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x06://д����ʱ��
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
                                printf("\r\n6д����ʱ��ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000D);
                                config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x000E);
                                printf("\r\n����ʱ�䣺%ldСʱ%ld����\r\n",config_temp_data / 60,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x07://д��ʱʱ��
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
                                printf("\r\n7д��ʱʱ��ɹ�\r\n");
                                config_temp_data = 0;
                                //config_temp_data = I2C2_ByteRead(0x0011);
                                //config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0012);
                                config_temp_data = timing_time;
                                if(config_temp_data == 0xFFFF)
                                {
                                    printf("\r\n��ʱʱ�䣺δ����\r\n");
                                }
                                else
                                {
                                    printf("\r\n��ʱʱ�䣺%ldʱ%ld��\r\n",config_temp_data / 60,config_temp_data % 60);
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x08://д���ʱ��
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
                                printf("\r\n8д���ʱ��ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000F);
                                config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0010);
                                printf("\r\n���ʱ�䣺%ldСʱ%ld����%ld��\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x09://д�豸״̬
                        config_temp_data = 0;
                        data_num = sscanf((void*)config_buf,"AT+Write=%*d,%ld",&config_temp_data);
                        if(data_num == 1)
                        {
                            if(0 < config_temp_data && config_temp_data < 5)
                            {
                                I2C2_ByteWrite((uchar)config_temp_data, 0x000A);
                                __delay_cycles(100000);
                                printf("\r\n9д�豸״̬�ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x000A);
                                if(config_temp_data == 0x01)
                                {
                                    printf("\r\n�豸״̬������\r\n");        
                                }
                                else if(config_temp_data == 0x02)
                                {
                                    printf("\r\n�豸״̬��������\r\n");        
                                }
                                else if(config_temp_data == 0x03)
                                {
                                    printf("\r\n�豸״̬���Ѽ���\r\n");        
                                }
                                else if(config_temp_data == 0x04)
                                {
                                    printf("\r\n�豸״̬���Ѳ��\r\n");        
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x0A://д�ۼƹ�ʱ
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
                                printf("\r\n10д�ۼƹ�ʱ�ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0013);
                                config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0014);
                                config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0015);
                                config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0016);
                                printf("\r\n�豸�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",config_temp_data / 3600,config_temp_data / 60 % 60 ,config_temp_data % 60);
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x0B://д����
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
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                            else if(1 > RTC_mon || RTC_mon > 12)
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                            else if(RTC_hour > 23)
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }  
                            else if(RTC_min > 59)
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                            else if(RTC_sec > 59)
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                            else
                            {
                                if(RTC_mon == 1 || RTC_mon == 3 || RTC_mon == 5 || RTC_mon == 7 || RTC_mon == 8 || RTC_mon == 10 || RTC_mon == 12)
                                {
                                    if(1 > RTC_day || RTC_day > 31)
                                    {
                                        printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                    }
                                    else
                                    {
                                        Write_time();
                                        __delay_cycles(100000);
                                        printf("\r\n11д����ʱ��ɹ�\r\n");
                                        Read_time();
                                        printf("\r\nRTCʱ�䣺%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                    }
                                    
                                }
                                else if(RTC_mon == 4 || RTC_mon == 6 || RTC_mon == 9 || RTC_mon == 11)
                                {
                                    if(1 > RTC_day || RTC_day > 30)
                                    {
                                        printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                    }
                                    else
                                    {
                                        Write_time();
                                        __delay_cycles(100000);
                                        printf("\r\n11д����ʱ��ɹ�\r\n");
                                        Read_time();
                                        printf("\r\nRTCʱ�䣺%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                    }
                                }
                                else if(RTC_mon == 2)
                                {
                                    if(RTC_year%4 == 0)
                                    {
                                        if(1 > RTC_day || RTC_day > 29)
                                        {
                                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                        }
                                        else
                                        {
                                            Write_time();
                                            __delay_cycles(100000);
                                            printf("\r\n11д����ʱ��ɹ�\r\n");
                                            Read_time();
                                            printf("\r\nRTCʱ�䣺%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                        }
                                    }
                                    else
                                    {
                                        if(1 > RTC_day || RTC_day > 28)
                                        {
                                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                                        }
                                        else
                                        {
                                            Write_time();
                                            __delay_cycles(100000);
                                            printf("\r\n11д����ʱ��ɹ�\r\n");
                                            Read_time();
                                            printf("\r\nRTCʱ�䣺%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
                                        }
                                    }
                                } 
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    case 0x0C:
                        printf("\r\n12��ä�����ݳɹ�\r\n");
                        I2C2_ByteWrite(0x00, 0x0037);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x00, 0x0038);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x00, 0x0039);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x01, 0x003A);
                        __delay_cycles(100000);
                        blind_data_size = ((I2C2_ByteRead(0x0037) << 8) | I2C2_ByteRead(0x0038));   //��ȡä��������
                        __delay_cycles(10000);
                        //--------------------------------------------------------------------------
                        blind_data_head = ((I2C2_ByteRead(0x0039) << 8) | I2C2_ByteRead(0x003A));   //��ȡä��ͷ
                        __delay_cycles(10000);
                        printf("\r\nä��������:%.3d\r\n",blind_data_size);
                        printf("\r\nä������ָ��:%.3d\r\n",blind_data_head);
                        break;
                    case 0x0D:
                        printf("\r\n13�����������ɹ�\r\n");
                        I2C2_ByteWrite(0x00, 0x003B);
                        __delay_cycles(100000);
                        I2C2_ByteWrite(0x00, 0x003C);
                        __delay_cycles(100000);
                        config_temp_data = 0;
                        config_temp_data = I2C2_ByteRead(0x0022);
                        config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0023);
                        printf("\r\n�豸����������%ld\r\n",config_temp_data);
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
                                printf("\r\n14д�豸ά�޺ųɹ�\r\n");
                                eeprom_PageRead((uchar*)config_array, 0x003D, 8);
                                if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                                    && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                                {
                                    printf("\r\n�豸ά�޺ţ�δ����\r\n");
                                }
                                else
                                {
                                    printf("\r\n�豸ά�޺ţ�%.*s\r\n", 8, config_array);
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
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
                                printf("\r\n15д�������κųɹ�\r\n");
                                eeprom_PageRead((uchar*)config_array, 0x0045, 8);
                                if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                                    && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
                                {
                                    printf("\r\n�豸���κţ�δ����\r\n");
                                }
                                else
                                {
                                    printf("\r\n�豸���κţ�%.*s\r\n", 8, config_array);
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
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
                                printf("\r\n16дӲ���汾�ųɹ�\r\n");
                                I2C2_ByteWrite(hardware, 0x0009);
                                __delay_cycles(100000);
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x0009);
                                if(config_temp_data == 0)
                                {
                                    printf("\r\n�豸Ӳ���汾�ţ�δ����\r\n");
                                }
                                else
                                {
                                    printf("\r\n�豸Ӳ���汾�ţ�V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
                                }
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
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
                                printf("\r\n17д�豸����ʱ�跢�͵��������ɹ�\r\n");
                                config_temp_data = 0;
                                config_temp_data = I2C2_ByteRead(0x007D);
                                printf("\r\n�豸����ʱ�跢�͵���������%ld��\r\n",config_temp_data );
                            }
                            else
                            {
                                printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                            }
                        }
                        else
                        {
                            printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                        }
                        break;
                    default:
                        printf("\r\nָ���޷�ʶ����������ָ���Ƿ�����\r\n");
                    break;
                }
            }
            else
            {
                printf("\r\n\aָ���޷�ʶ����������ָ���Ƿ�����\r\n");
            }
        }
        else if(strstr(config_buf, "AT+Reset") != NULL)
        {
            eeprom_PageWrite( (void*)config_data, 0x0000, 128);
            __delay_cycles(100000);
            printf("\r\n�ָ�Ĭ�����óɹ�\r\n");
            //------------------------------------------------------------------

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0000);
            printf("\r\n���ұ�ţ�%.2ld\r\n",config_temp_data);

            eeprom_PageRead((uchar*)config_array, 0x0001, 8);
            printf("\r\n�豸��ţ�%.*s\r\n", 8, config_array);

            eeprom_PageRead(config_array, 0x0017, 30);
            if(config_array[0] == 0x01)
            {
            printf("\r\n������IP��ַ��%.3d.%.3d.%.3d.%.3d\r\n",config_array[26],config_array[27],config_array[28],config_array[29]);
            }
            else if(config_array[0] == 0x02)
            {
            printf("\r\n������������ַ��");
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
            printf("\r\n�������˿ںţ�%ld\r\n",config_temp_data);

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000B);
            if(config_temp_data == 0x01)
            {
            printf("\r\n�豸����ģʽ����׼ģʽ\r\n");       
            }
            else if(config_temp_data == 0x02)
            {
            printf("\r\n�豸����ģʽ����׼ģʽ\r\n");        
            }
            else if(config_temp_data == 0x03)
            {
            printf("\r\n�豸����ģʽ��׷��ģʽ\r\n");        
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000C);
            printf("\r\n����ʱ�䣺%ldСʱ%ld����\r\n",config_temp_data / 60,config_temp_data % 60);

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000D);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x000E);
            printf("\r\n����ʱ�䣺%ldСʱ%ld����\r\n",config_temp_data / 60,config_temp_data % 60);

            config_temp_data = 0;
            config_temp_data = timing_time;                        
            if(config_temp_data == 0xFFFF)
            {
            printf("\r\n��ʱʱ�䣺δ����\r\n");
            }
            else
            {
            printf("\r\n��ʱʱ�䣺%ldʱ%ld��\r\n",config_temp_data / 60,config_temp_data % 60);
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000F);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x0010);
            printf("\r\n���ʱ�䣺%ldСʱ%ld����%ld��\r\n",config_temp_data / 3600,config_temp_data / 60 % 60,config_temp_data % 60);

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x000A);
            if(config_temp_data == 0x01)
            {
            printf("\r\n�豸״̬������\r\n");        
            }
            else if(config_temp_data == 0x02)
            {
            printf("\r\n�豸״̬��������\r\n");        
            }
            else if(config_temp_data == 0x03)
            {
            printf("\r\n�豸״̬���Ѽ���\r\n");        
            }
            else if(config_temp_data == 0x04)
            {
            printf("\r\n�豸״̬���Ѳ��\r\n");        
            }

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0013);
            config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0014);
            config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0015);
            config_temp_data = ( config_temp_data << 8 ) + I2C2_ByteRead(0x0016);
            printf("\r\n�豸�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",config_temp_data / 3600,config_temp_data % 60 / 60,config_temp_data % 60);

            Read_time();
            printf("\r\nRTCʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);

            blind_data_size = ((I2C2_ByteRead(0x0037) << 8) | I2C2_ByteRead(0x0038));   //��ȡä��������
            __delay_cycles(10000);
            //--------------------------------------------------------------------------
            blind_data_head = ((I2C2_ByteRead(0x0039) << 8) | I2C2_ByteRead(0x003A));   //��ȡä��ͷ
            __delay_cycles(10000);

            printf("\r\nä����������%.3d\r\n",blind_data_size);
            printf("\r\nä������ͷ��%.3d\r\n",blind_data_head);

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
            printf("\r\nä�����ݶ�ȡ���\r\n");

            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x003B);
            config_temp_data = (config_temp_data << 8) + I2C2_ByteRead(0x003C);
            printf("\r\n�豸����������%ld��\r\n",config_temp_data);

            eeprom_PageRead((uchar*)config_array, 0x003D, 8);
            if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
            {
                printf("\r\n�豸ά�޺ţ�δ����\r\n");
            }
            else
            {
                printf("\r\n�豸ά�޺ţ�%.*s\r\n", 8, config_array);
            }
            
            eeprom_PageRead((uchar*)config_array, 0x0045, 8);
            if(config_array[0] == 0x30 && config_array[1] == 0x30 && config_array[2] == 0x30 && config_array[3] == 0x30
                && config_array[4] == 0x30 && config_array[5] == 0x30 && config_array[6] == 0x30 && config_array[7] == 0x30)
            {
                printf("\r\n�豸���κţ�δ����\r\n");
            }
            else
            {
                printf("\r\n�豸���κţ�%.*s\r\n", 8, config_array);
            }
            
            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0009);
            if(config_temp_data == 0)
            {
                printf("\r\n�豸Ӳ���汾�ţ�δ����\r\n");
            }
            else
            {
                printf("\r\n�豸Ӳ���汾�ţ�V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
            }

            //------------------------------------------------------------------
        }
        else if(strstr(config_buf, "AT+Test") != NULL)
        {
            __delay_cycles(100000);
            printf("\r\n��ʼ�Լ�\r\n");
            __delay_cycles(100000);
            printf("\r\n����汾�ţ�V%d.%d\r\n",procedure_VER / 10,procedure_VER % 10);
            config_temp_data = 0;
            config_temp_data = I2C2_ByteRead(0x0009);
            if(config_temp_data == 0)
            {
                printf("\r\n�豸Ӳ���汾�ţ�δ����\r\n");
            }
            else
            {
                printf("\r\n�豸Ӳ���汾�ţ�V%ld.%ld\r\n",config_temp_data / 10,config_temp_data % 10);
            }
            
            AD_collect();
            printf("\r\n��ص�ѹ��%d.%dV\r\n",AD_voltage / 1000,AD_voltage % 1000);
            
            I2C2_ByteWrite(0x55, 0xFFFD);
            __delay_cycles(100000);
            I2C2_ByteWrite(0xAA, 0xFFFE);
            __delay_cycles(100000);
            I2C2_ByteWrite(0x55, 0xFFFF);
            __delay_cycles(100000);
            if(I2C2_ByteRead(0xFFFD) == 0x55 && I2C2_ByteRead(0xFFFE) == 0xAA && I2C2_ByteRead(0xFFFF) == 0x55)
            {
                printf("\r\nEEPģ������\r\n");
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
                    printf("\r\nEEPģ������\r\n");
                    I2C2_ByteWrite(0x00, 0xFFFD);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0xFFFE);
                    __delay_cycles(100000);
                    I2C2_ByteWrite(0x00, 0xFFFF);
                    __delay_cycles(100000);
                }
                else
                {
                    printf("\r\nEEPģ�����\a\r\n");
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
                printf("\r\nRTCģ������\r\n");
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
                    printf("\r\nRTCģ������\r\n");
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
                    printf("\r\nRTCģ�����\a\r\n");
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
                if((P1IN & BIT5) != ray)
                {
                    ray = (P1IN & BIT5);
                    if(ray == 0x00)
                    {
                        printf("\r\n����δ����\r\n");
                    }
                    else if(ray == 0x20)
                    {
                        printf("\r\n�����Ѽ���\r\n");
                    }
                }
                if(GSM_status != 0xFF)
                {
                    GSM();
                }
                if(GSM_status == 0x2C)
                {
                    printf("\r\nGSMģ������\r\n");
                    GSM_status = 0xFF;
                    GSM_POWER_OFF;//�ر�GSM��Դ
                    
                    gps_debug_flag = 1;
                    debug = 0;
                    
                }
                if(GPS_analysis_end > GPS_analysis_num && GPS_analysis_end != 0xFF)
                {
                    printf("\r\nGPSģ������\r\n");
                    printf("\r\n��λ��־��%c\r\n",GPS_Locate);
                    printf("\r\nUTCʱ�䣺%.*s\r\n", 6, GPS_UTC_time);
                    printf("\r\nUTC���ڣ�%.*s\r\n", 6, GPS_UTC_date);
                    printf("\r\nGPS�ϱ�γ��%c\r\n", GPS_north_south);
                    printf("\r\nGPSγ�ȣ�%.*s\r\n", 10, GPS_latitude);
                    printf("\r\nGPS��������%c\r\n", GPS_east_west);
                    printf("\r\nGPS���ȣ�%.*s\r\n", 11, GPS_Longitude);
                    printf("\r\nGPS�ٶȣ�%.*s\r\n", 5, GPS_Speed);
                    printf("\r\nGPS����%.*s\r\n", 6, GPS_direction);
                    printf("\r\nGPS���Σ�%.*s\r\n", 7, GPS_elevation);
                    printf("\r\nʹ�����ǿ�����%.*s\r\n", 2, GPS_employ_satellite);
                    printf("\r\n�������ǿ�����%.*s\r\n", 2, GPS_employ_satellite);
                    GPS_POWER_OFF;
                    GPS_analysis_end = 0xFF;
                }
                if(GPS_analysis_end == 0xFF && GSM_status == 0xFF)
                {
                    printf("\r\n�Լ����\r\n");
                    
                    I2C2_ByteWrite(0x00, 0x003B);   // ���������� ä��������
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
                    
                    GSM_status = 0x00;  // ����ǰ��GSM״̬
                    data_type = 0x00;   // ������������״̬
                    GPS_analysis_end = 0x00;    // ����ǰ��GPS��������
                    GPS_Locate = 'V';       // ����ǰ��GPS��λ��־
                    GPS_north_south = 'X';  // ����ǰ���ϱ�γ��־
                    GPS_east_west = 'X';    // ����ǰ�嶫������־
                    data_send_flag = 0x00;  // ����ǰ�巢�ͳɹ���־
                    GPS_start = 0x00;       // ����ǰ��GPS��ʼ������־
                    timing_time = 0xFFFF;   // ����ǰ�嶨ʱʱ��
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
                    
                    break;
                }
                if((P2IN & BIT3) == 0x00)
                {
                    printf("\r\n�豸�˳�����ģʽ\r\n");
                    break;
                }
            }
            
        }
        else
		{
			printf("\r\n\aָ���޷�ʶ����������ָ���Ƿ�����\r\n");
		}
        uart_flag = 0;//����ܱ�־λ
        memset( config_buf,0x00,sizeof( config_buf ) );//����������
        config_num = 0;//�����������
        UCA3IE |= UCRXIE;           // ������3�����ж�
    }
}