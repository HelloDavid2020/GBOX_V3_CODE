#include "MSP430F5438A.h"
#include "ublox.h"
#include "system.h"

uchar GPS_analysis_end = 0;		                                                        // GPS��λ���������
uchar GPS_Locate = 'V';			                                                        // GPS��λ��־$GPRMC�ֶ�2
uchar GPS_UTC_time[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};			                    // UTCʱ��$GPRMC�ֶ�1
uchar GPS_UTC_date[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};			                    // UTC����$GPRMC�ֶ�9
uchar GPS_latitude[11]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};        // GPSγ��$GPRMC�ֶ�3
uchar GPS_north_south = 'X';	                                                        // �ϱ�γ$GPRMC�ֶ�4
uchar GPS_east_west = 'X';		                                                        // ������$GPRMC�ֶ�6
uchar GPS_Longitude[12]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  // GPS����$GPRMC�ֶ�5
uchar GPS_Speed[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};                                // GPS�ٶ�$GPVTG�ֶ�4��λ����
uchar GPS_direction[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};                            // GPS����$GPRMC�ֶ�8
uchar GPS_elevation[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};                       // GPS���θ߶�$GPGGA�ֶ�9
uchar GPS_employ_satellite[3]={0x00,0x00,0x00};                                         // GPSʹ�����ǿ���$GPGGA�ֶ�7
uchar GPS_visible_satellite[3]={0x00,0x00,0x00};                                        // GPS�������ǿ���

uchar Comma_num;				                                                        // ���ż���
uchar Bytes_num;				                                                        // �ֽڼ���
uchar Command;					


uchar gps_buffer[NMEA_COUNT_MAX] = {0x00};
uint dataLen = 0;
uchar gpsRxFlag = 0;
TIME UTC_Time; // 
TIME BTC_Time;
GPSINFO GpsInfo; // GPS ��������
GPSDATA GpsData; // GPS �ϴ�����


// ���״̬
uchar GPS_start;
// GPS��俪ʼ��־

void GPSParse(void)
{
  uchar CommaNum=0; 
  uchar BufIndex=0;
  uchar Sbuf;
  char *Pstr;
  uint index;

  index= FindStr((void *)gps_buffer,"$GPRMC,");
  if(index)
  {
    CommaNum=0;
    Pstr=gps_buffer+index+6;	 
    do
    {       
      Sbuf=*Pstr++;	

      switch(Sbuf)
      {
        case ',':
                CommaNum++;  
                BufIndex=0;
        break;
        
        default:
        switch(CommaNum)
        {
            case 0:
              GpsInfo.UtcTime[BufIndex]=Sbuf;
              GpsData.UTC_Time[3] = (GpsInfo.UtcTime[0] - '0') * 10 + (GpsInfo.UtcTime[1] - '0');
              GpsData.UTC_Time[4] = (GpsInfo.UtcTime[2] - '0') * 10 + (GpsInfo.UtcTime[3] - '0');
              GpsData.UTC_Time[5] = (GpsInfo.UtcTime[4] - '0') * 10 + (GpsInfo.UtcTime[5] - '0');
      
              UTC_Time.hour = GpsData.UTC_Time[3];
              UTC_Time.min = GpsData.UTC_Time[4];
              UTC_Time.sec = GpsData.UTC_Time[5];
            break; 
            
            case 1:
              GpsInfo.Status=Sbuf;
              if(GpsInfo.Status == 'A')
                GpsData.Status = GPS_VALID; 
              
              else if(GpsInfo.Status == 'V')
                GpsData.Status = GPS_INVALID;                 
            break;
            
            case 2:
              GpsInfo.Latitude[BufIndex]=Sbuf;	
              GpsInfo.Latitude[10]='\0';	
      
              GpsData.Latitude =	atof((void *)GpsInfo.Latitude);
            break;
            
            case 3:
              GpsInfo.LatitudeNS = Sbuf;
              if(GpsInfo.LatitudeNS == 'N')
                      GpsData.LatitudeNS = NORTH_LATITUDE; //
              else if(GpsInfo.LatitudeNS == 'S')
                      GpsData.LatitudeNS = SOUTH_LATITUDE; 
             break;
            
            case 4:
              GpsInfo.Longitude[BufIndex] = Sbuf;
              GpsInfo.Longitude[11] = '\0';
      
              GpsData.Longitude =	atof((void *)GpsInfo.Longitude);
            
            break;
            
            case 5:
              GpsInfo.LongitudeEW = Sbuf;	
              if(GpsInfo.LongitudeEW == 'E')
                      GpsData.LongitudeEW = EAST_LONGTITUDE; //
              else if(GpsInfo.LongitudeEW == 'W')
                      GpsData.LongitudeEW = WEST_LONGTITUDE;  // 							

            break;
            
            case 6:
              GpsInfo.Speed[BufIndex]=Sbuf;		
              GpsData.Speed =	atoi((void *)GpsInfo.Speed);
      
            break;
            
            case 7: 
              GpsInfo.Azimuth[BufIndex]=Sbuf;	 // o??��??
              GpsData.Azimuth =	atoi((void *)GpsInfo.Azimuth);
            
            break;
            
            case 8:
              GpsInfo.UtcDate[BufIndex]=Sbuf;	
              GpsData.UTC_Time[0] = (GpsInfo.UtcDate[4] - '0') * 10 + (GpsInfo.UtcDate[5] - '0');
              GpsData.UTC_Time[1] = (GpsInfo.UtcDate[2] - '0') * 10 + (GpsInfo.UtcDate[3] - '0');
              GpsData.UTC_Time[2] = (GpsInfo.UtcDate[0] - '0') * 10 + (GpsInfo.UtcDate[1] - '0');
      
              UTC_Time.day 	= GpsData.UTC_Time[2];
              UTC_Time.mon 	= GpsData.UTC_Time[1];
              UTC_Time.year	= GpsData.UTC_Time[0];							
            break;
            
            default:break;
          }
          BufIndex++;	//
          break;
        }
    }while(Sbuf!='*');
          
  }
  index= FindStr((void *)gps_buffer,"$GPGGA,");
  if(index)
  {
    CommaNum=0;
    Pstr=gps_buffer+index+6;
    do
    {
      Sbuf=*Pstr++ ;
      switch(Sbuf)
      {
        case ',':CommaNum++;
          BufIndex=0;
          break;
        
        default:
          switch(CommaNum)
          {
            case 0:break;
            case 1:break;
            case 2:break;
            case 3:break;
            case 4:break;
            case 5:break;
            case 6:
            GpsInfo.SatelliteNumS[BufIndex]= Sbuf;
            GpsInfo.SatelliteNumS[2]= '\0';

            GpsData.SatelliteNumS = atoi((void *)GpsInfo.SatelliteNumS);
            break;
                                            
            case 7:break;
            
            case 8:
              GpsInfo.Altitude[BufIndex]=Sbuf; 
              GpsData.Altitude = atoi((void *)GpsInfo.Altitude);

              break;
              
            default:break;
          }
          BufIndex++;
          break;
      }
    }while(Sbuf!='*');	
  } 
  
  gpsRxFlag = 0;
  dataLen = 0;
  memset(gps_buffer, 0x00, sizeof(gps_buffer));
  _EINT();
  
}



//-------------GPS���ݽ���------------------------------------------------------
void GPS_analysis( uchar GPS_temp )
{
    if( GPS_temp == '$')//��俪ʼ��־
	{
		Command = 0;
		GPS_start = 1;
		Bytes_num = 0;
		Comma_num = 0;
		return;
	}
	if( GPS_start == 1 )
	{
		if( GPS_temp == ',' )			//�����','��
		{
			Comma_num++;				//���ż�����1
			Bytes_num = 0;				//��ն��ֽ���
			return;
		}
		else if( GPS_temp == '*' )		//�����'*'�ţ�˵��������
		{
			GPS_start = 0;				// ��־��0
			return;
		}
		if( Comma_num == 0 )			//����ǵ�0�����ſ�ʼ�ж����
		{
			if( Bytes_num == 3 )		//ͨ����4���ֽ��ж����
			{
				switch( GPS_temp )
				{
					case 'M'://$GPRMC
						Command = 1;
						break;
					case 'T'://$GPVTG
						Command = 2;
						break;
					case 'G'://$GPGGA
						Command = 3;
						break;
					case 'L'://$GPGLL
						Command = 4;
						break;
					case 'S'://$GPGSA/$GPGSV
						break;
					case 'X'://ublox��Ȩ����˳�
                        GPS_start = 0;
						break;
                    default://��Ч�����˳�
                        GPS_start = 0;
                        break;
				}
			}
			if( Bytes_num == 4 )//ͨ����5���ֽ��ж������������
			{
				if( Command == 0 && GPS_temp == 'A' )
				{
					Command = 5;//$GPGSA
				}
				else if( Command == 0 && GPS_temp == 'V' )
				{
					Command = 6;//$GPGSV
				}
			}
		}
		else//�������0��','�ŵ��ø����������Ӻ���
		{
			switch(Command)
			{
				case 1:
					GPRMC( GPS_temp );
					break;
				case 2:
					GPVTG( GPS_temp );
					break;
				case 3:
					GPGGA( GPS_temp );
					break;
				case 4:
					GPGLL( GPS_temp );
					break;
				case 5:
					GPGSA( GPS_temp );
					break;
				case 6:
					GPGSV( GPS_temp );
					break;
                default:
                    break;                    
			}
		}
		Bytes_num++;
	}
	
	return;
}

void GPRMC( uchar GPS_temp )
{
	switch(Comma_num)
	{
        
		case 1:// ��һ��UTCʱ����Ϣ
			if( Bytes_num < 6)
			{
				GPS_UTC_time[Bytes_num] = GPS_temp;
			}
			break;
		case 2:// �ڶ��ζ�λ�Ƿ���Ч��Ϣ
			GPS_Locate = GPS_temp;
			break;
		case 3:// ������γ����Ϣ
			if( Bytes_num < 10)
			{
				GPS_latitude[Bytes_num] = GPS_temp;
			}
			break;
		case 4: // ���Ķ��ϱ�γ��־
			GPS_north_south = GPS_temp;
			break;
		case 5: // ����ξ�����Ϣ
			if( Bytes_num < 11)
			{
				GPS_Longitude[Bytes_num] = GPS_temp;
			}
			break;
		case 6:// �����ζ�������־
			GPS_east_west = GPS_temp;
			break;
		case 8:// �ڰ˶ε��溽��
			if( Bytes_num < 8)
			{
				GPS_direction[Bytes_num] = GPS_temp;
			}
			break;
		case 9:// �ھŶ�UTC����
			if( Bytes_num < 6)
			{
				GPS_UTC_date[Bytes_num] = GPS_temp;
			}
			break;
        default:
            break;            
	}
}

void GPVTG( uchar GPS_temp )
{
    switch( Comma_num )
	{
        case 4:
            if( Bytes_num < 6)// ���Ķ��ٶȵ�λ����
			{
				GPS_Speed[Bytes_num] = GPS_temp;
			}
            break;
        default:
            break;            
    } 
}

void GPGGA( uchar GPS_temp )
{
	switch( Comma_num )
	{
		case 7://���߸��ֶ�����ʹ�õ�GPS���ǿ���
			if( Bytes_num < 2)
			{
				GPS_employ_satellite[Bytes_num] = GPS_temp;
			}
			break;
		case 9://�ھŸ��ֶ�GPS���θ߶�
			if( Bytes_num < 7)
			{
				GPS_elevation[Bytes_num] = GPS_temp;
			}
			break;
        default:
            break;            
	}
}

void GPGLL( uchar GPS_temp )
{
    switch( Comma_num )
	{
        case 7:
            if(GPS_Locate == 'A')
            {
                GPS_analysis_end++;
                if(GPS_analysis_end > 200)
                    GPS_analysis_end = 200;
            }
            break;
        default:
            break;            
    }      
}

void GPGSA( uchar GPS_temp )
{
	
}

void GPGSV( uchar GPS_temp )
{
    switch( Comma_num )
	{
        case 3:
            if( Bytes_num < 2)// �����οɼ����ǿ���
			{
				GPS_visible_satellite[Bytes_num] = GPS_temp;
			}
            break;
        default:
            break;            
    } 	
}
