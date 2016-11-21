#ifndef __main_H
#define __main_H

#define uchar unsigned char 
#define uint unsigned int

//#define ALARM_ON                    P1DIR &= ~BIT5;P1IFG &= ~BIT5;P1IES &= ~BIT5;P1IE |= BIT5
//#define ALARM_OFF                   P1DIR &= ~BIT5;P1IFG &= ~BIT5;P1IES &= ~BIT5;P1IE &= ~BIT5

#define ALARM_ON                    P1IE &= ~BIT5;P1IFG &= ~BIT5;P1IES &= ~BIT5;P1IE |= BIT5
#define ALARM_OFF                   P1IFG &= ~BIT5;P1IES &= ~BIT5;P1IE &= ~BIT5

//#define EXT_INT_INIT                P2DIR &= ~BIT3;P2IFG &= ~BIT3;P2IES &= ~BIT3;P2IE |= BIT3

#define EXT_INT_INIT                P6DIR &= ~BIT1

//#define EXT_INT_INIT                P2DIR &= ~BIT3;P2IFG &= ~BIT3;P2IES &= ~BIT3;P2IE |= BIT3
#define EEPROM_Password 0xAA55

extern const uchar config_data[];               // 寄存器默认数据
extern uchar vendor_number;                    // 厂商编号
extern uchar device_number[8];                 // 设备号
extern uchar hardware;                         // 设备硬件版本
extern uchar activate_status;                  // 设备激活状态
extern uchar work_mode;                        // 工作模式
extern uchar work_time;                        // 工作时间
extern uint sleep_time;                        // 休眠时间
extern uint takt_time;                         // 间隔时间
extern uint timing_time;                       // 定时时间
extern uint blind_data_size;                    // 盲区数据量
extern uint blind_data_head;                    // 盲区数据头地址
extern uchar servers_addr[30];                 // 服务器地址
extern uint port;                              // 端口
extern uint system_rst;                        // 系统重启次数
extern volatile uchar work_type;                        // 工作状态
extern uchar work_type_flag;                   // 工作状态
extern volatile uchar alarm;                            // 光敏报警标志
extern uchar activation_send_num;              // 激活时已发送次数
extern uchar activation_max_send_num;          // 激活时需发送次数
extern unsigned long int labor_time;           // 累计工时
extern unsigned long int labor_time_start;     // 
extern unsigned long int labor_time_end;       //

#endif