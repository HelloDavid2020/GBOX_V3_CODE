#ifndef _DEVICE_H_20141018
#define _DEVICE_H_20141018
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
#include "led.h"

#include "device.h"

//-------------------����������ر���-------------------------------------------

#define MODE_LBS    1
#define MODE_GPS    2
#define MODE_TRACE  3

extern uchar gps_start;

extern uchar vendor_number;                    // ���̱��
extern uchar device_number[8];                 // �豸��
extern uchar hardware;                         // �豸Ӳ���汾
extern uchar activate_status;                  // �豸״̬
extern uchar work_mode;                        // ����ģʽ
extern uchar work_time;                        // ����ʱ��
extern uint sleep_time;                        // ����ʱ��
extern uint takt_time;                         // ���ʱ��
extern uint timing_time;              // ��ʱʱ��
extern uint blind_data_size;                   // ä��������
extern uint blind_data_head;                   // ä������ͷ��ַ
extern uchar servers_addr[30];                 // ��������ַ
extern uint port;                              // �˿�
extern uint system_rst;                        // ϵͳ��������
extern volatile uchar work_type;                        // ��������
extern uchar work_type_flag;                   // ����״̬��־
extern uchar takt_time_flag;                   // ���ʱ���־
extern volatile uchar alarm;                   // ����������־
extern uchar activation_send_num;              // �������ѷ��ʹ��� 
extern uchar activation_max_send_num;          // �������跢�ʹ���
extern unsigned long int labor_time;           // �ۼƹ�ʱ
extern unsigned long int labor_time_start;     // 
extern unsigned long int labor_time_end;       //
extern const uchar config_data[128];
extern void device_start(void);
extern void tracker_mode(void);
extern void gps_mode(void);
extern void lbs_mode(void);
extern void work_mode_handler(uchar  mode);


















#endif
