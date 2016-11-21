#include "MSP430F5438A.h"
#include "packdata.h"
#include "stdio.h"
#include "uart.h"
#include "string.h"
#include "stdlib.h"
#include "mg323b.h"
#include "main.h"
#include "sd2068.h"
#include "ublox.h"
#include "at24c512.h"
#include "adc.h"

uchar data_type;            // ��������
uchar data_packet[113];      // ���ݰ�



void pack_data( void )
{
    uchar q;
    unsigned long int temp_data;
    double temp;
    
    memset(data_packet,0x00,sizeof(data_packet));
	
    if( data_type == 0 || data_type == 1 )
    {
        // ֡ͷ�̶�0xFF
        data_packet[0] = 0xFF;  
        data_packet[1] = 0xFF;  
        data_packet[2] = 0xFF;  
        data_packet[3] = 0xFF;  
        data_packet[4] = 0xFF;
        // ���ݳ��� �̶�0x5D
        data_packet[5] = 0x5D;
        // ���ұ��
        data_packet[6] = vendor_number;
        printf("\r\n���ұ�ţ�%.2d\r\n",vendor_number);
        // ������ �̶�0x01
        data_packet[7] = 0x01;
        // �ش�Э��汾��
        data_packet[8] = protocol_VER;
        printf("\r\nЭ��汾��V%d.%d\r\n",protocol_VER / 10,protocol_VER % 10);
        // �ն��豸���
        data_packet[9] = device_number[0];
        data_packet[10] = device_number[1];
        data_packet[11] = device_number[2];
        data_packet[12] = device_number[3];
        data_packet[13] = device_number[4];
        data_packet[14] = device_number[5];
        data_packet[15] = device_number[6];
        data_packet[16] = device_number[7];
        printf("\r\n�豸��ţ�%.*s\r\n", 8,device_number);
        // �豸����汾
        data_packet[17] = procedure_VER;
        printf("\r\n�豸����汾��V%d.%d\r\n",procedure_VER / 10,procedure_VER % 10);
        // �豸Ӳ���汾
        data_packet[18] = hardware;
        if(hardware == 0)
        {
            printf("\r\n�豸Ӳ���汾�ţ�δд��\r\n");
        }
        else
        {
            printf("\r\n�豸Ӳ���汾�ţ�V%d.%d\r\n",hardware / 10,hardware % 10);
        }
        // �ն��豸ʱ��
        Read_time();
        data_packet[19] = RTC_year;
        data_packet[20] = RTC_mon;
        data_packet[21] = RTC_day;
        data_packet[22] = RTC_hour;
        data_packet[23] = RTC_min;
        data_packet[24] = RTC_sec;
        printf("\r\nRTCʱ�䣺20%.2d-%.2d-%.2d.%.2d:%.2d:%.2d\r\n",RTC_year,RTC_mon,RTC_day,RTC_hour,RTC_min,RTC_sec);
        // �豸����״̬
        data_packet[25] = activate_status;
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
        // �豸����ģʽ
        data_packet[26] = work_mode;
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
        // �豸����ʱ��
        data_packet[27] = work_time;
        printf("\r\n����ʱ�䣺%dСʱ%d����\r\n",work_time / 60,work_time % 60);
        // �豸����ʱ��
        data_packet[28] = sleep_time >> 8;
        data_packet[29] = sleep_time;
        printf("\r\n����ʱ�䣺%dСʱ%d����\r\n",sleep_time / 60,sleep_time % 60);
        // �豸���ʱ��
        data_packet[30] = takt_time >> 8;
        data_packet[31] = takt_time;
        printf("\r\n���ʱ�䣺%dСʱ%d����%d��\r\n",takt_time / 3600,takt_time / 60 % 60,takt_time % 60);
        // �豸��ص�ѹ
        data_packet[32] = (uchar)(AD_voltage >> 8);
        data_packet[33] = (uchar)(AD_voltage);
        printf("\r\n��ص�ѹ��%d.%dV\r\n",AD_voltage / 1000,AD_voltage % 1000);
        // �豸�ۼƹ�ʱ
        data_packet[34] = labor_time >> 24;
        data_packet[35] = labor_time >> 16;
        data_packet[36] = labor_time >> 8;
        data_packet[37] = labor_time;
        printf("\r\n�豸�ۼƹ�ʱ��%ldСʱ%ld����%ld��\r\n",labor_time / 3600,labor_time / 60 % 60,labor_time % 60);
        // ä����������
        data_packet[38] = blind_data_size >> 8;
        data_packet[39] = blind_data_size;
        printf("\r\n�豸ä������������%d\r\n",blind_data_size);
        // �豸��������
        data_packet[40] = system_rst >> 8;
        data_packet[41] = system_rst;
        printf("\r\n�豸����������%d\r\n",system_rst);
        // ���������ֽ�
        data_packet[42] = 0x00;
        data_packet[43] = 0x00;
        data_packet[44] = 0x00;
        data_packet[45] = 0x00;
        data_packet[46] = 0x00;
        data_packet[47] = 0x00;
        data_packet[48] = 0x00;
        data_packet[49] = 0x00;
        data_packet[50] = 0x00;
        data_packet[51] = 0x00;
        data_packet[52] = 0x00;
        data_packet[53] = 0x00;
        data_packet[54] = 0x00;
        data_packet[55] = 0x00;
        data_packet[56] = 0x00;
        data_packet[57] = 0x00;
        data_packet[58] = 0x00;
        data_packet[59] = 0x00;
        data_packet[60] = 0x00;
        // ä�����ݱ�־
        data_packet[61] = 0x02;     //01Ϊä������ 02Ϊ��������
        printf("\r\n�豸ä�����ݱ�ǣ���������\r\n");
        if(GPS_analysis_end > GPS_analysis_num)
        {
            // �豸��λ��Ϣ
            data_packet[62] = GpsInfo.Status;
            
            printf("\r\nGPS��λ��־��%c\r\n",GpsInfo.Status);
            // �ϱ�����
            data_packet[63] = GpsInfo.LatitudeNS;
            printf("\r\n�ϱ�γ��־��%c\r\n", GpsInfo.LatitudeNS);
            // γ��
            temp_data = 0;
            temp = 0.0;
            temp = atof((void*)GpsInfo.Latitude);
            temp_data = (unsigned long int)(((uint)(temp/100) + (temp - (uint)(temp/100)*100)/60)*1000000);
            data_packet[64] = temp_data >> 24;
            data_packet[65] = temp_data >> 16;
            data_packet[66] = temp_data >> 8;
            data_packet[67] = temp_data;
            printf("\r\nγ�ȣ�%ld\r\n",temp_data);
            // ��������
            data_packet[68] = GpsInfo.LongitudeEW;
            printf("\r\n��������־��%c\r\n",GpsInfo.LongitudeEW);
            // ����
            temp_data = 0;
            temp = 0.0;
            temp = atof((void*)GpsInfo.Longitude);
            temp_data = (unsigned long int)(((uint)(temp/100) + (temp - (uint)(temp/100)*100)/60)*1000000);
            data_packet[69] = temp_data >> 24;
            data_packet[70] = temp_data >> 16;
            data_packet[71] = temp_data >> 8;
            data_packet[72] = temp_data;
            printf("\r\n���ȣ�%ld\r\n",temp_data);
            // GPS�ٶ�
            temp = 0.0;
            temp = atof((void*)GpsInfo.Speed);
            temp_data = (unsigned long int)(temp * 10);
            data_packet[73] = temp_data >> 8;
            data_packet[74] = temp_data;
            printf("\r\n�ٶȣ�%ld\r\n",temp_data);
            // GPS����
            temp = 0.0;
            temp = atof((void*)GpsInfo.Azimuth); // ����
            temp_data = (unsigned long int)(temp * 10);
            data_packet[75] = temp_data >> 8;
            data_packet[76] = temp_data;
            printf("\r\n����%ld\r\n",temp_data);
            // GPS���θ���
            temp = 0.0;
            temp = atof((void*)GpsInfo.Altitude);
            temp_data = (unsigned long int)(temp * 10);
            data_packet[77] = temp_data >> 16;
            data_packet[78] = temp_data >> 8;
            data_packet[79] = temp_data;
            printf("\r\n���Σ�%ld\r\n",temp_data);
            // ʹ�����ǿ���
            temp_data = 0;
            temp_data = atoi((void*)GpsInfo.SatelliteNumS);
            data_packet[80] = temp_data;
            printf("\r\nʹ�����ǿ�����%ld\r\n",temp_data);
            // �������ǿ���
            temp_data = 0;
            temp_data = atoi((void*)GpsInfo.SatelliteNumS);
            temp_data +=rand()%(5-1);

            data_packet[81] = temp_data;
            printf("\r\n�������ǿ�����%ld\r\n",temp_data);
        }
        else
        {
            // �豸��λ��Ϣ
            data_packet[62] = 'V';
            printf("\r\nGPS��λ��־��V\r\n");
        }
        // GSM�ź�ǿ��
        temp_data = 0;
        temp_data = atoi((void*)CSQ);
        data_packet[82] = temp_data;
        printf("\r\nGSM�ź�ǿ�ȣ�%ld\r\n",temp_data);
        // �����վ��Ϣ
        temp_data = 0;
        for(q=0;q<4;q++)
		{
			if('0'<=LCA[q] && LCA[q]<= '9')
			{
				temp_data = ((temp_data << 4) + (LCA[q] -'0'));
			}
			else if('a'<= LCA[q] && LCA[q] <= 'f')
			{
				temp_data = ((temp_data << 4) + (LCA[q] -0x57));
			}
		}
        data_packet[83] = temp_data >> 8;
        data_packet[84] = temp_data;
        printf("\r\n��վLCA��%ld\r\n",temp_data);
        temp_data = 0;
        for(q=0;q<4;q++)
		{
			if('0'<= CELL[q] && CELL[q] <= '9')
			{
				temp_data = ((temp_data << 4) + (CELL[q] -'0'));
			}
			else if('a'<= CELL[q] && CELL[q] <= 'f')
			{
				temp_data = ((temp_data << 4) + (CELL[q] -0x57));
			}
		}
        data_packet[85] = temp_data >> 8;
        data_packet[86] = temp_data;
        printf("\r\n��վCELL��%ld\r\n",temp_data);
        // ������վ��Ϣ1
        data_packet[87] = 0x00;
        data_packet[88] = 0x00;
        data_packet[89] = 0x00;
        data_packet[90] = 0x00;
        // ������վ��Ϣ2
        data_packet[91] = 0x00;
        data_packet[92] = 0x00;
        data_packet[93] = 0x00;
        data_packet[94] = 0x00;
        // ������վ��Ϣ3
        data_packet[95] = 0x00;
        data_packet[96] = 0x00;
        data_packet[97] = 0x00;
        data_packet[98] = 0x00;
    }
    else if( data_type == 2 )// ä������
    {
        if(blind_data_head == 1)
        {
            eeprom_PageRead((void*)data_packet, 64000, 75);
        }
        else
        {
            eeprom_PageRead((void*)data_packet, ((blind_data_head - 1)*128), 99);
        }
        data_packet[61] = 0x01; //01Ϊä������ 02Ϊ��������
        
    }
    else if( data_type == 3 )// TCP��Ӧ����
    {
        // ֡ͷ�̶�0xFF
        data_packet[0] = 0xFF;  
        data_packet[1] = 0xFF;  
        data_packet[2] = 0xFF;  
        data_packet[3] = 0xFF;  
        data_packet[4] = 0xFF;
        // ���ݳ��� �̶�0x5D
        data_packet[5] = 0x5D;
        // ���ұ��
        data_packet[6] = vendor_number;
        // ������ �̶�0x01
        data_packet[7] = 0x01;
        // �ش�Э��汾��
        data_packet[8] = protocol_VER;
        // �ն��豸���
        data_packet[9] = device_number[0];
        data_packet[10] = device_number[1];
        data_packet[11] = device_number[2];
        data_packet[12] = device_number[3];
        data_packet[13] = device_number[4];
        data_packet[14] = device_number[5];
        data_packet[15] = device_number[6];
        data_packet[16] = device_number[7];
        // �豸����汾
        data_packet[17] = procedure_VER;
        // �豸Ӳ���汾
        data_packet[18] = hardware;
        // �ն��豸ʱ��
        Read_time();
        data_packet[19] = RTC_year;
        data_packet[20] = RTC_mon;
        data_packet[21] = RTC_day;
        data_packet[22] = RTC_hour;
        data_packet[23] = RTC_min;
        data_packet[24] = RTC_sec;
        // �豸����״̬
        data_packet[25] = activate_status;
        // �豸����ģʽ
        data_packet[26] = work_mode;
        // �豸����ʱ��
        data_packet[27] = work_time;
        // �豸����ʱ��
        data_packet[28] = sleep_time >> 8;
        data_packet[29] = sleep_time;
        // �豸���ʱ��
        data_packet[30] = takt_time >> 8;
        data_packet[31] = takt_time;
        // �豸��ص�ѹ
        data_packet[32] = (uchar)(AD_voltage >> 8);
        data_packet[33] = (uchar)(AD_voltage);
        // �豸�ۼƹ�ʱ
        data_packet[34] = labor_time >> 24;
        data_packet[35] = labor_time >> 16;
        data_packet[36] = labor_time >> 8;
        data_packet[37] = labor_time;
        // ä����������
        data_packet[38] = blind_data_size >> 8;
        data_packet[39] = blind_data_size;
        // �豸��������
        data_packet[40] = system_rst >> 8;
        data_packet[41] = system_rst;
        // ���������ֽ�
        data_packet[42] = 0x00;
        data_packet[43] = 0x00;
        data_packet[44] = 0x00;
        data_packet[45] = 0x00;
        data_packet[46] = 0x00;
        data_packet[47] = 0x00;
        data_packet[48] = 0x00;
        data_packet[49] = 0x00;
        data_packet[50] = 0x00;
        data_packet[51] = 0x00;
        data_packet[52] = 0x00;
        data_packet[53] = 0x00;
        data_packet[54] = 0x00;
        data_packet[55] = 0x00;
        data_packet[56] = 0x00;
        data_packet[57] = 0x00;
        data_packet[58] = 0x00;
        data_packet[59] = 0x00;
        data_packet[60] = 0x00;
        // ä�����ݱ�־
        data_packet[61] = 0x02;     //01Ϊä������ 02Ϊ��������
        if(GPS_analysis_end > GPS_analysis_num)
        {
            // �豸��λ��Ϣ
            data_packet[62] = GPS_Locate;
            // �ϱ�����
            data_packet[63] = GPS_north_south;
            // γ��
            temp_data = 0;
            temp = 0.0;
            temp = atof((void*)GPS_latitude);
            temp_data = (unsigned long int)(((uint)(temp/100) + (temp - (uint)(temp/100)*100)/60)*1000000);
            data_packet[64] = temp_data >> 24;
            data_packet[65] = temp_data >> 16;
            data_packet[66] = temp_data >> 8;
            data_packet[67] = temp_data;
            // ��������
            data_packet[68] = GPS_east_west;
            // ����
            temp_data = 0;
            temp = 0.0;
            temp = atof((void*)GPS_Longitude);
            temp_data = (unsigned long int)(((uint)(temp/100) + (temp - (uint)(temp/100)*100)/60)*1000000);
            data_packet[69] = temp_data >> 24;
            data_packet[70] = temp_data >> 16;
            data_packet[71] = temp_data >> 8;
            data_packet[72] = temp_data;
            // GPS�ٶ�
            temp = 0.0;
            temp = atof((void*)GPS_Speed);
            temp_data = (unsigned long int)(temp * 10);
            data_packet[73] = temp_data >> 8;
            data_packet[74] = temp_data;
            // GPS����
            temp = 0.0;
            temp = atof((void*)GPS_direction);
            temp_data = (unsigned long int)(temp * 10);
            data_packet[75] = temp_data >> 8;
            data_packet[76] = temp_data;
            // GPS���θ���
            temp = 0.0;
            temp = atof((void*)GPS_elevation);
            temp_data = (unsigned long int)(temp * 10);
            data_packet[77] = temp_data >> 16;
            data_packet[78] = temp_data >> 8;
            data_packet[79] = temp_data;
            // ʹ�����ǿ���
            temp_data = 0;
            temp_data = atoi((void*)GPS_employ_satellite);
            data_packet[80] = temp_data;
            // �������ǿ���
            temp_data = 0;
            temp_data = atoi((void*)GPS_visible_satellite);
            data_packet[81] = temp_data;
        }
        else
        {
            // �豸��λ��Ϣ
            data_packet[62] = 'V';
        }
        // GSM�ź�ǿ��
        temp_data = 0;
        temp_data = atoi((void*)CSQ);
        data_packet[82] = temp_data;
        // �����վ��Ϣ
        temp_data = 0;
        for(q=0;q<4;q++)
		{
			if('0'<=LCA[q] && LCA[q]<= '9')
			{
				temp_data = ((temp_data << 4) + (LCA[q] -'0'));
			}
			else if('a'<= LCA[q] && LCA[q] <= 'f')
			{
				temp_data = ((temp_data << 4) + (LCA[q] -0x57));
			}
		}
        data_packet[83] = temp_data >> 8;
        data_packet[84] = temp_data;
        temp_data = 0;
        for(q=0;q<4;q++)
		{
			if('0'<= CELL[q] && CELL[q] <= '9')
			{
				temp_data = ((temp_data << 4) + (CELL[q] -'0'));
			}
			else if('a'<= CELL[q] && CELL[q] <= 'f')
			{
				temp_data = ((temp_data << 4) + (CELL[q] -0x57));
			}
		}
        data_packet[85] = temp_data >> 8;
        data_packet[86] = temp_data;
        // ������վ��Ϣ1
        data_packet[87] = 0x00;
        data_packet[88] = 0x00;
        data_packet[89] = 0x00;
        data_packet[90] = 0x00;
        // ������վ��Ϣ2
        data_packet[91] = 0x00;
        data_packet[92] = 0x00;
        data_packet[93] = 0x00;
        data_packet[94] = 0x00;
        // ������վ��Ϣ3
        data_packet[95] = 0x00;
        data_packet[96] = 0x00;
        data_packet[97] = 0x00;
        data_packet[98] = 0x00;
        //----------------------------------------------------------------------
        // ֡ͷ�̶�0xFF
        data_packet[99] = 0xFF;  
        data_packet[100] = 0xFF;  
        data_packet[101] = 0xFF;  
        data_packet[102] = 0xFF;  
        data_packet[103] = 0xFF;
        // ���ݳ��ȹ̶�
        data_packet[104] = 0x08;
        // �̶�
        data_packet[105] = 0x00;
        // ������̶�0x02
        data_packet[106] = 0x02;
        // ��ˮ��
        data_packet[107] = buf1[8];  
        data_packet[108] = buf1[9];  
        data_packet[109] = buf1[10];  
        data_packet[110] = buf1[11];
        // ��������ִ�н��
        data_packet[111] = (uchar)( config_flag >> 8 );
        data_packet[112] = (uchar)(config_flag);
    }
}