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

//-------------------工作参数相关变量-------------------------------------------

#define MODE_LBS    1
#define MODE_GPS    2
#define MODE_TRACE  3

extern uchar gps_start;

extern uchar vendor_number;                    // 厂商编号
extern uchar device_number[8];                 // 设备号
extern uchar hardware;                         // 设备硬件版本
extern uchar activate_status;                  // 设备状态
extern uchar work_mode;                        // 工作模式
extern uchar work_time;                        // 工作时间
extern uint sleep_time;                        // 休眠时间
extern uint takt_time;                         // 间隔时间
extern uint timing_time;              // 定时时间
extern uint blind_data_size;                   // 盲区数据量
extern uint blind_data_head;                   // 盲区数据头地址
extern uchar servers_addr[30];                 // 服务器地址
extern uint port;                              // 端口
extern uint system_rst;                        // 系统重启次数
extern volatile uchar work_type;                        // 工作类型
extern uchar work_type_flag;                   // 工作状态标志
extern uchar takt_time_flag;                   // 间隔时间标志
extern volatile uchar alarm;                   // 光敏报警标志
extern uchar activation_send_num;              // 激活期已发送次数 
extern uchar activation_max_send_num;          // 激活期需发送次数
extern unsigned long int labor_time;           // 累计工时
extern unsigned long int labor_time_start;     // 
extern unsigned long int labor_time_end;       //
extern const uchar config_data[128];
extern void device_start(void);
extern void tracker_mode(void);
extern void gps_mode(void);
extern void lbs_mode(void);
extern void work_mode_handler(uchar  mode);


















#endif
