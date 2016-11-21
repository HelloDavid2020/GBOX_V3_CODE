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

#define AT_CREG_MAX_num		20	// ��ѯ��ǰ����ע��״̬������Դ���

extern char *p;


extern uchar GSM_TX0_buf[128];		// GSM���ͻ�����
extern uchar GSM_RX0_buf[512];		// GSM���ܻ�����
extern uchar buf1[100];             // ���������������
extern char AT_SISS_1_ADDRESS[];    //  ������IP��ַ

extern uchar GSM_status;           // GSM״̬����
extern uint  CCR0_time;			    // ��ʱʱ��
extern uchar GSM_RX0_buf_num;		// ����0���ܼ���
extern uchar GSM_time_flag;        // GSM��ʱ��־λ
extern uint config_flag;           // ���óɹ���־
extern uchar data_send_flag;       // ��λ���ݷ��ͳɹ���־
extern uchar LCA[5];				// λ����
extern uchar CELL[5];				// С����
extern uchar CSQ[2];				// �ź�ǿ��

extern const char AT_SISC_1[];
extern char AT_CIPSTART_TCP[60];



extern void GSM( void );
extern uint FindStr(char *str,char *ptr);
extern void ascii2data(char * des, char * src, int size);
extern void data2ascii(char * des, char * src, int size);

#endif