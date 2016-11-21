#ifndef __mg323b_H
#define __mg323b_H

#define uchar unsigned char 
#define uint unsigned int

#define     GSM_POWER_INIT      P1DIR |= BIT3
#define     GSM_POWER_ON        P1OUT |= BIT3
#define     GSM_POWER_OFF       P1OUT &= ~BIT3

//#define G_BOX 1
#ifdef G_BOX
  #define     GSM_SW_INIT      P3DIR |= BIT3
  #define     GSM_SW_HIGH      P3OUT &= ~BIT3
  #define     GSM_SW_LOW       P3OUT |= BIT3
#else 
  #define     GSM_SW_INIT      P3DIR |= BIT3
  #define     GSM_SW_HIGH      P3OUT |= BIT3 
  #define     GSM_SW_LOW       P3OUT &= ~BIT3
#endif


#define     GSM_RST_INIT      P8DIR |= BIT0
#define     GSM_RST_HIGH      P8OUT |= BIT0
#define     GSM_RST_LOW       P8OUT &= ~BIT0

#define AT_CREG_MAX_num		20	// 查询当前网络注册状态最大重试次数

extern char *p;


extern uchar GSM_TX0_buf[128];		// GSM发送缓存区
extern uchar GSM_RX0_buf[512];		// GSM接受缓存区
extern uchar buf1[100];             // 参数设置命令缓冲区
extern char AT_SISS_1_ADDRESS[];    //  服务器IP地址

extern uchar GSM_status;           // GSM状态变量
extern uint  CCR0_time;			    // 超时时间
extern uchar GSM_RX0_buf_num;		// 串口0接受计数
extern uchar GSM_time_flag;        // GSM定时标志位
extern uint config_flag;           // 配置成功标志
extern uchar data_send_flag;       // 定位数据发送成功标志
extern uchar LCA[5];				// 位置码
extern uchar CELL[5];				// 小区码
extern uchar CSQ[2];				// 信号强度

extern const char AT_SISC_1[];
extern char AT_CIPSTART_TCP[60];



extern void GSM( void );
extern uint FindStr(char *str,char *ptr);
extern void ascii2data(char * des, char * src, int size);
extern void data2ascii(char * des, char * src, int size);

#endif