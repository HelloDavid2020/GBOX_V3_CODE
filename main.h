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

extern const uchar config_data[];               // �Ĵ���Ĭ������
extern uchar vendor_number;                    // ���̱��
extern uchar device_number[8];                 // �豸��
extern uchar hardware;                         // �豸Ӳ���汾
extern uchar activate_status;                  // �豸����״̬
extern uchar work_mode;                        // ����ģʽ
extern uchar work_time;                        // ����ʱ��
extern uint sleep_time;                        // ����ʱ��
extern uint takt_time;                         // ���ʱ��
extern uint timing_time;                       // ��ʱʱ��
extern uint blind_data_size;                    // ä��������
extern uint blind_data_head;                    // ä������ͷ��ַ
extern uchar servers_addr[30];                 // ��������ַ
extern uint port;                              // �˿�
extern uint system_rst;                        // ϵͳ��������
extern volatile uchar work_type;                        // ����״̬
extern uchar work_type_flag;                   // ����״̬
extern volatile uchar alarm;                            // ����������־
extern uchar activation_send_num;              // ����ʱ�ѷ��ʹ���
extern uchar activation_max_send_num;          // ����ʱ�跢�ʹ���
extern unsigned long int labor_time;           // �ۼƹ�ʱ
extern unsigned long int labor_time_start;     // 
extern unsigned long int labor_time_end;       //

#endif