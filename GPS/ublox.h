#ifndef __ublox_H
#define __ublox_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>   
#include <time.h>
#include <math.h>

#define uchar unsigned char 
#define uint unsigned int

#define     GPS_POWER_INIT      P1DIR |= BIT4
#define     GPS_POWER_ON        P1OUT |= BIT4
#define     GPS_POWER_OFF       P1OUT &= ~BIT4
#define     GPS_POWER_RESET     P1DIR &= ~BIT4


#define     GPS_POWER_INIT2     P1DIR |= BIT2
#define     GPS_POWER_ON2       P1OUT |= BIT2
#define     GPS_POWER_OFF2      P1OUT &= ~BIT2
#define     GPS_POWER_RESET2    P1DIR &= ~BIT2

#define     GPS_analysis_num    3

#define NMEA_COUNT_MAX 512 

#define enable_gps_irq()          UCA0IE |= UCRXIE
#define disable_gps_irq()         UCA0IE &=~UCRXIE



#define GPS_VALID       1   
#define GPS_INVALID     0 
	 
#define NORTH_LATITUDE    1 
#define SOUTH_LATITUDE    0
	 
#define EAST_LONGTITUDE   1
#define WEST_LONGTITUDE   0

	 
typedef struct
{
  uchar UtcTime[6]; 
  uchar Status;   
  uchar Latitude[11];	
  uchar LatitudeNS; 
  uchar Longitude[12];	 
  uchar LongitudeEW;
  uchar Speed[6];	    
  uchar Azimuth[6];	
  uchar UtcDate[7]; 
  uchar Altitude[8];   
  uchar SatelliteNumS[3];//??? 
}GPSINFO;

typedef struct
{
  uchar SatelliteNumS; 
  uchar Status;  
  uchar Speed;	    
  uchar UTC_Time[7];  // UTC--->BTC
  
  uchar LatitudeNS; 
  uchar LongitudeEW;
  
  int Altitude;    // �߳�
  uint Azimuth;	// �����  
                  
  float Latitude;	
  float Longitude;	 
}GPSDATA;  

typedef struct
{
  uchar sec;
  uchar min;
  uchar hour;
  uchar day;
  uchar mon;
  uchar year;
  uchar week;	
}TIME;  
extern TIME UTC_Time; // 
extern TIME BTC_Time;
extern GPSINFO GpsInfo; // GPS ��������
extern GPSDATA GpsData; // GPS �ϴ�����


extern uchar Flag_CloseGps;
extern uchar gps_buffer[NMEA_COUNT_MAX];
extern uint dataLen;

extern uchar gpsRxFlag;



extern uchar GPS_start;                // GPS��ʼ������־
extern uchar GPS_analysis_end;         // GPS��λ���������
extern uchar GPS_Locate;               // GPS��λ��־$GPRMC�ֶ�2
extern uchar GPS_UTC_time[7];          // UTCʱ��$GPRMC�ֶ�1
extern uchar GPS_UTC_date[7];          // UTC����$GPRMC�ֶ�9
extern uchar GPS_latitude[11];         // GPSγ��$GPRMC�ֶ�3
extern uchar GPS_north_south;          // �ϱ�γ$GPRMC�ֶ�4
extern uchar GPS_east_west;            // ������$GPRMC�ֶ�6
extern uchar GPS_Longitude[12];        // GPS����$GPRMC�ֶ�5
extern uchar GPS_Speed[7];             // GPS�ٶ�$GPVTG�ֶ�4��λ����
extern uchar GPS_direction[7];         // GPS����$GPRMC�ֶ�8
extern uchar GPS_elevation[8];         // GPS���θ߶�$GPGGA�ֶ�9
extern uchar GPS_employ_satellite[3];  // GPSʹ�����ǿ���$GPGGA�ֶ�7
extern uchar GPS_visible_satellite[3]; // GPS�������ǿ���

extern uint FindStr(char *str,char *ptr);

extern void GPS_analysis( uchar GPS_temp );
extern void GPRMC( uchar GPS_temp );
extern void GPVTG( uchar GPS_temp );
extern void GPGGA( uchar GPS_temp );
extern void GPGLL( uchar GPS_temp );
extern void GPGSA( uchar GPS_temp );
extern void GPGSV( uchar GPS_temp );

#endif