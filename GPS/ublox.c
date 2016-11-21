#include "MSP430F5438A.h"
#include "ublox.h"
#include "system.h"

uchar GPS_analysis_end = 0;		                                                        // GPS定位后解析次数
uchar GPS_Locate = 'V';			                                                        // GPS定位标志$GPRMC字段2
uchar GPS_UTC_time[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};			                    // UTC时间$GPRMC字段1
uchar GPS_UTC_date[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};			                    // UTC日期$GPRMC字段9
uchar GPS_latitude[11]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};        // GPS纬度$GPRMC字段3
uchar GPS_north_south = 'X';	                                                        // 南北纬$GPRMC字段4
uchar GPS_east_west = 'X';		                                                        // 东西经$GPRMC字段6
uchar GPS_Longitude[12]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  // GPS经度$GPRMC字段5
uchar GPS_Speed[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};                                // GPS速度$GPVTG字段4单位公里
uchar GPS_direction[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};                            // GPS方向$GPRMC字段8
uchar GPS_elevation[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};                       // GPS海拔高度$GPGGA字段9
uchar GPS_employ_satellite[3]={0x00,0x00,0x00};                                         // GPS使用卫星颗数$GPGGA字段7
uchar GPS_visible_satellite[3]={0x00,0x00,0x00};                                        // GPS可视卫星颗数

uchar Comma_num;				                                                        // 逗号计数
uchar Bytes_num;				                                                        // 字节计数
uchar Command;					


uchar gps_buffer[NMEA_COUNT_MAX] = {0x00};
uint dataLen = 0;
uchar gpsRxFlag = 0;
TIME UTC_Time; // 
TIME BTC_Time;
GPSINFO GpsInfo; // GPS 接收数据
GPSDATA GpsData; // GPS 上传数据


// 语句状态
uchar GPS_start;
// GPS语句开始标志

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
              GpsInfo.Azimuth[BufIndex]=Sbuf;	 // o??ò??
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



//-------------GPS数据解析------------------------------------------------------
void GPS_analysis( uchar GPS_temp )
{
    if( GPS_temp == '$')//语句开始标志
	{
		Command = 0;
		GPS_start = 1;
		Bytes_num = 0;
		Comma_num = 0;
		return;
	}
	if( GPS_start == 1 )
	{
		if( GPS_temp == ',' )			//如果是','号
		{
			Comma_num++;				//逗号计数加1
			Bytes_num = 0;				//清空段字节数
			return;
		}
		else if( GPS_temp == '*' )		//如果是'*'号，说明语句结束
		{
			GPS_start = 0;				// 标志清0
			return;
		}
		if( Comma_num == 0 )			//如果是第0个逗号开始判断语句
		{
			if( Bytes_num == 3 )		//通过第4个字节判断语句
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
					case 'X'://ublox版权语句退出
                        GPS_start = 0;
						break;
                    default://无效数据退出
                        GPS_start = 0;
                        break;
				}
			}
			if( Bytes_num == 4 )//通过第5个字节判断其余两条语句
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
		else//如果不是0个','号调用各自语句解析子函数
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
        
		case 1:// 第一段UTC时间信息
			if( Bytes_num < 6)
			{
				GPS_UTC_time[Bytes_num] = GPS_temp;
			}
			break;
		case 2:// 第二段定位是否有效信息
			GPS_Locate = GPS_temp;
			break;
		case 3:// 第三段纬度信息
			if( Bytes_num < 10)
			{
				GPS_latitude[Bytes_num] = GPS_temp;
			}
			break;
		case 4: // 第四段南北纬标志
			GPS_north_south = GPS_temp;
			break;
		case 5: // 第五段经度信息
			if( Bytes_num < 11)
			{
				GPS_Longitude[Bytes_num] = GPS_temp;
			}
			break;
		case 6:// 第六段东西经标志
			GPS_east_west = GPS_temp;
			break;
		case 8:// 第八段地面航向
			if( Bytes_num < 8)
			{
				GPS_direction[Bytes_num] = GPS_temp;
			}
			break;
		case 9:// 第九段UTC日期
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
            if( Bytes_num < 6)// 第四段速度单位公里
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
		case 7://第七个字段正在使用的GPS卫星颗数
			if( Bytes_num < 2)
			{
				GPS_employ_satellite[Bytes_num] = GPS_temp;
			}
			break;
		case 9://第九个字段GPS海拔高度
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
            if( Bytes_num < 2)// 第三段可见卫星颗数
			{
				GPS_visible_satellite[Bytes_num] = GPS_temp;
			}
            break;
        default:
            break;            
    } 	
}
