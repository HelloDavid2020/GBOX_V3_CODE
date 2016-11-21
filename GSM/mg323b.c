#include "MSP430F5438A.h"
#include "mg323b.h"
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "string.h"
#include "packdata.h"
#include "at24c512.h"
#include "sd2068.h"
#include "led.h"
#include "stdbool.h"


uchar GSM_TX0_buf[128];		// GSM���ͻ�����
uchar GSM_RX0_buf[512];		// GSM���ܻ�����

uchar ICCID[20];
uchar ICCID_temp;
//uchar p;

uchar LCA[5];				// λ����
uchar CELL[5];				// С����
uchar CSQ[2];				// �ź�ǿ��

uint  AT_CREG_num = 0;		// ��ѯ��ǰ����ע��״̬���Դ���
uint  CCR0_time;			// ��ʱʱ��
uchar GSM_RX0_buf_num;		// ����0���ܼ���
uchar GSM_status;           // GSM״̬����
uchar GSM_time_flag;        // GSM��ʱ��־λ
uchar retry;                // ATָ�����Դ���
uchar send_retry = 0;       // �������Դ���
uint config_flag;           // ���óɹ���־
uchar data_send_flag;        // GSM���ͳɹ���־


uchar gsm_buf_index=0;
uchar gsm_buf_len=0;
uchar gsm_buf_size=0;
uchar gsm_buf_offset = 0;
//uchar buf[168];
uchar buf[255];

uchar buf1[100];
char *p;

//---------------------GSMģ���ʼ�����AT����----------------------------------
const char AT[]={'A','T',0x0D,0X0A};//GSM��������
const char ATE0[]={'A','T','E','0',0x0D,0X0A};//�رջ���
const char ATE1[]={'A','T','E','1',0x0D,0X0A};//�رջ���
const char AT_CRSM_176_12258_0_0_10[]={'A','T','+','C','R','S','M','=','1','7','6',',','1','2','2','5','8',',','0',',','0',',','1','0',0x0D,0x0A};//��ѯICCID
const char AT_CGREG_0[]={'A','T','+','C','G','R','E','G','=','0',0x0D,0X0A};//��������״̬����
const char AT_CMGF_1[]={'A','T','+','C','M','G','F','=','1',0x0D,0X0A};//���ö���Ϣ��ʽΪ�ı�ģʽ
const char AT_CNMI_0_0_0_0_0[]={'A','T','+','C','N','M','I','=','0',',','0',',','0',',','0',',','0',0x0D,0X0A};//���ö���Ϣ���ϱ�
const char AT_CNMI_2_2_0_0_0[]={'A','T','+','C','N','M','I','=','2',',','2',',','0',',','0',',','0',0x0D,0X0A};//���ö���Ϣ�ϱ�
const char AT_CREG_0[]={'A','T','+','C','R','E','G','=','0',0x0D,0X0A};//��ֹCREG�����ϱ�
const char AT_CREG_[]={'A','T','+','C','R','E','G','?',0x0D,0X0A};//��ѯ��ǰ����ע��״̬(������Ҫѭ������Ƿ�����)
const char AT_SMONC[]={'A','T','^','S','M','O','N','C',0x0D,0X0A};//��ȡС�������Ϣ
const char AT_CSQ[]={'A','T','+','C','S','Q',0x0D,0X0A};//��ѯ���������ź�ǿ�ȣ����31����С0
//----------------------GPRS��ʼ�����AT����------------------------------------
const char AT_CGATT_[]={'A','T','+','C','G','A','T','T','=','?',0x0D,0x0A};//����GPRS����״̬
//const char AT_SICS_0_CONTYPE_GPRS0[]={'A','T','^','S','I','C','S','=','0',',','C','O','N','T','Y','P','E',',','G','P','R','S','0',0x0D,0x0A};//��IDΪ0��Profile��conType����ΪGPRS0
const char AT_SICS_0_CONTYPE_GPRS0[]="AT^SICS=0,\"CONTYPE\",\"GPRS0\"\r\n";//��IDΪ0��Profile��conType����ΪGPRS0

const char AT_SICS_0_ANP_CMNET[]={'A','T','^','S','I','C','S','=','0',',','\"','A','P','N','\"',',','\"','C','M','N','E','T','\"',0x0D,0x0A};//��IDΪ0��Profile��apn����Ϊ1234
//const char AT_SICS_0_ANP_CMNET[]="AT^SICS=0,\"APN\",\"CMNET\"\r\n";//��IDΪ0��Profile��conType����ΪGPRS0


//const char AT_SICS_0_INACTTO_20[]={'A','T','^','S','I','C','S','=','0',',','\"','I','N','A','C','T','T','O','\"',',','2','0',0x0D,0x0A};//����IDΪ0�ڲ���ʱʱ��
const char AT_SICS_0_INACTTO_20[]="AT^SICS=1,\"INACTTO\",20\r\n";//��IDΪ0��Profile��conType����ΪGPRS0
const char AT_IOMODE_1_1[]="AT^IOMODE=1,1\r\n";
//const char AT_IOMODE_1_1[]={'A','T','^','I','O','M','O','D','E','=','1',',','1',0x0D,0x0A};//�Խ������ݽ���ת������ʹ�û��棨ʹ�û���ʱ���׶�ʧ���ݣ�
//----------------------GPRS���ӷ�����AT����------------------------------------
const char AT_SISS_1_SRVTYPE_SOCKET[]={'A','T','^','S','I','S','S','=','1',',','\"','S','R','V','T','Y','P','E','\"',',','\"','S','O','C','K','E','T','\"',0x0D,0x0A};//
const char AT_SISS_1_CONID_0[]={'A','T','^','S','I','S','S','=','1',',','\"','C','O','N','I','D','\"',',','0',0x0D,0x0A};//
char AT_SISS_1_ADDRESS[60]={'A','T','^','S','I','S','S','=','1',',','\"','A','D','D','R','E','S','S','\"',',','\"','S','O','C','K','T','C','P',':','/','/','2','1','1','.','1','0','3','.','1','7','9','.','2','3','4',':','0','7','7','7','8','\"','"',0x0D,0x0A};//��������IP�˿�

//char AT_SISS_1_ADDRESS[]={'A','T','^','S','I','S','S','=','1',',','A','D','D','R','E','S','S',',','"','S','O','C','K','T','C','P',':','/','/','1','2','4','.','2','0','2','.','1','9','8','.','2','4','2',':','0','9','0','0','1','"',0x0D,0x0A};//��������IP�˿�
const char AT_SISC_1[]={'A','T','^','S','I','S','C','=','1',0x0D,0x0A};//������ǰ����ȹر�һ������
const char AT_SISO_1[]={'A','T','^','S','I','S','O','=','1',0x0D,0x0A};//�򿪷���1

//----------------------GPRS��������AT����--------------------------------------
const char AT_SISW_1_99[]={'A','T','^','S','I','S','W','=','1',',','9','9',0x0D,0x0A};//�����ش���ä����������73���ֽڻ���
const char AT_SISW_1_113[]={'A','T','^','S','I','S','W','=','1',',','1','1','3',0x0D,0x0A};//д����TCP��Ӧ����14���ֽڻ���
const char AT_CIPSEND_99[]={'A','T','+','C','I','P','S','E','N','D','=','9','9',0x0D,0x0A};//��ģ������99���ֽڻ�����
const char AT_CIPSEND_113[]={'A','T','+','C','I','P','S','E','N','D','=','1','1','3',0x0D,0x0A};//��ģ������113���ֽڻ�����
char AT_CIPSTART_TCP[60]={'A','T','+','C','I','P','S','T','A','R','T','=','"','T','C','P','"',',','"','2','0','2','.','0','8','5','.','2','0','9','.','0','8','0','"',',','"','0','9','0','0','1','"',0x0D,0x0A};//����TCP����


//----------------------GPRS��ȡ����AT����--------------------------------------
//const char AT_SISR_1_1500[]={'A','T','^','S','I','S','R','=','1',',','1','5','0','0',0x0D,0x0A};

uint FindStr(char *str,char *ptr)
{
  uint index=0;
  char *STemp=NULL;
  char *PTemp=NULL;
  char *MTemp=NULL;
  if(0==str||0==ptr)
  return 0;
  for(STemp=str; *STemp!='\0'; STemp++)	 
  {
    index++;   
    MTemp=STemp;  

    for(PTemp=ptr;*PTemp!='\0';PTemp++)
    {	
      if(*PTemp!=*MTemp)
      break;
      MTemp++;
    }
    if(*PTemp=='\0')  
      break;
   }
   return index;
}



void GSM( void )
{
  char tmp_buf[100]={0};
  uchar i = 0;
  
  uchar config_array[50]={0x00};
  
  uchar GSM_Plus = 0; 
  uchar GSM_Comma = 0;
  uchar w;
  bool GSM_start = 0;
  uint a;
  
  
  switch( GSM_status )
  {
    case 0x00:
      GSM_SW_INIT;
      GSM_SW_HIGH;              //�����ܽ���1
      GSM_status = 0x01;        //����GSM״̬
      GSM_time_flag = 0;        //�嶨ʱ��־λ
      TA1CCR0 = 3750;           //����һ����Լ3��Ķ�ʱ
      CCR0_time = 2500;         //����һ����Լ2��Ķ�ʱ        
      printf("> Gsm power on...\r\n");
      break;
      case 0x01:
        if (GSM_time_flag)		                                            //��ʱʱ�䵽
        {
            GSM_status = 0x02;	                                            //����GSM״̬
            GSM_time_flag = 0;	                                            //�嶨ʱ��־λ
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
        }
        break;
        
        case 0x02:
            GSM_SW_LOW;			//�����ܽ���0
            GSM_status = 0x03;		//����GSM״̬
            GSM_time_flag = 0;		//�嶨ʱ��־λ
            TA1CCR0 = 3750;		//����һ����Լ3��Ķ�ʱ
            CCR0_time = 2500;		//����һ����Լ2��Ķ�ʱ
            //printf("\r\n�����ܽ���0\r\n");
            break;
        case 0x03:
            if (GSM_time_flag)		                                            //��ʱʱ�䵽
            {
              GSM_status = 0x04;	                                            //����GSM״̬
              GSM_time_flag = 0;	                                            //�嶨ʱ��־λ
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
            break;
        case 0x04:
            delay_ms(3000);
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0; 
            uscia2_send( (void*)AT);                                           //��������
            GSM_status = 0x05;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
            break;
        case 0x05:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                GSM_status = 0x06;
              }
              else
              {
                GSM_status = 0x06;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
            break;
        case 0x06:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0; 
            //������0
            uscia2_send( (void*)"ATE0\r\n" );                                         //�رջ���
            GSM_status = 0x07;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
            break;
        case 0x07:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                  printf("\r\n�ػ���ʧ�ܣ�����\r\n");
                  GSM_status = 0x06;
                  retry++;
                }
              }
              else
              {
                retry = 0;                                                  //���Դ�������
                GSM_status = 0x08;                                          //����GSM״̬
                GSM_status = 0x0C;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
            break;
        case 0x08:
          memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
          GSM_RX0_buf_num = 0; 

          uscia2_send( (void*)"AT+CCID\r\n" );                     //��ѯICCID��Ϣ
          GSM_status = 0x09;
          //GSM_status = 0x10;   

          //����GSM״̬
          GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
          TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
          CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
          break;

        case 0x09:

        if (GSM_time_flag)                                                  //��ʱʱ�䵽
        {	
          if(strstr(GSM_RX0_buf, "OK") == NULL)
          {
            if( retry >= 3 )
            {
              printf("\r\n���Դ�������3�Σ����¿���\r\n");
              GSM_POWER_OFF;
              delay_ms(1000);
              delay_ms(1000);
              GSM_POWER_ON;
              delay_ms(500);
              GSM_status = 0x00;
            }
            else
            {
              printf("\r\n��ѯICCIDʧ�ܣ�����\r\n");
              GSM_status = 0x08;
              retry++;
            }
          }
          else
          {
               eeprom_PageRead((uchar*)config_array, 0x0001, 8);
              if( (config_array[2]=='0') &&  ( config_array[3]=='2')  )
              {
                 printf("\r\n�豸��ţ�%.*s\r\n", 8, config_array);
              }
               else
              {

                memcpy(config_array+2,GSM_RX0_buf+16,6);
                config_array[0]='G';
                config_array[1]='B';
                eeprom_PageWrite((uchar*)config_array, 0x0001, 8);
                __delay_cycles(100000);
                printf("\r\n1д�豸��ųɹ�\r\n");
                eeprom_PageRead((uchar*)config_array, 0x0001, 8);
                printf("\r\n�豸��ţ�%.*s\r\n", 8, config_array);
                
              }
            
            retry = 0;                                                  //���Դ�������

            GSM_status = 0x0C;                                          //����GSM״̬
            GSM_time_flag = 0;                                          //�嶨ʱ��־λ
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
          }
         }
         break;
        case 0x0A:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)"AT+CNMI=0,0,0,0,0\r\n" );                            //���ö���Ϣ���ϱ�
            GSM_status = 0x0B;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
            printf("\r\n���ö���Ϣ���ϱ�\r\n");
            break;
            
         case 0x0B:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                  printf("\r\n���ö���Ϣ���ϱ�ʧ�ܣ�����\r\n");
                  GSM_status = 0x0A;
                  retry++;
                  }
              }
              else
              {
                retry = 0;                                                  //���Դ�������
                GSM_status = 0x0C;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
              }
            break;
                    
  case 0x0C:

     memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)"AT+CREG=1\r\n" );                                    //�ر���ʾ����ע��δ֪��
            GSM_status = 0x0D;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 375;                                                    //����һ����Լ300����Ķ�ʱ
            printf("\r\n�ر���ʾ����ע��δ֪��\r\n");
            break;
        case 0x0D:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                 }
                 else
                {
                  printf("\r\n�ر���ʾ����ע��δ֪��ʧ�ܣ�����\r\n");
                  GSM_status = 0x0C;
                  retry++;
                }
              }
              else
              {
                retry = 0;                                                  //���Դ�������
                GSM_status = 0x0E;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
               }
            }
            break;
            case 0x0E:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)"AT+CREG?\r\n" );                                     //��ѯ����ע��״̬
            GSM_status = 0x0F;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                   //����һ����Լ1��Ķ�ʱ
            printf("\r\n��ѯGSM����ע��״̬\r\n");
            break;
        case 0x0F:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
          {	
          if(strstr(GSM_RX0_buf, "+CREG:") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
          printf("\r\n��ѯGSM����ע��״̬ʧ�ܣ�����\r\n");
          GSM_status = 0x0E;
          retry++;
          }
          }
          else
          {
          p = (void *)strstr(GSM_RX0_buf, "+CREG:");
          if(*(p + 9) != '1' && *(p + 9) != '5')
          {
          AT_CREG_num++;                                          //���Դ�����1
          GSM_status = 0x0E;                                      //����GSM״̬
          GSM_time_flag = 0;                                      //�嶨ʱ��־λ
          printf("\r\n�ȴ�GSM����ע��:%d\r\n",AT_CREG_num);
          TA1CCR0 = 0;
          TA1CTL = TASSEL_1 + MC_1 + TACLR;
          CCR0_time = 0;
          }
              else if( *(p + 9) == '1' )
              {
                  retry = 0;                                              //���Դ�������
                GSM_status = 0x10;                                      //����GSM״̬
              GSM_time_flag = 0;                                      //�嶨ʱ��־λ
                  printf("\r\n��ע�᱾��GSM����\r\n");
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
              }
              else if( *(p + 9) == '5' )
              {
                retry = 0;                                              //���Դ�������
                GSM_status = 0x10;                                      //����GSM״̬
                GSM_time_flag = 0;                                      //�嶨ʱ��־λ
                printf("\r\n��ע������GSM����\r\n");
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
          }
          break;            

    case 0x10:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)"AT+CENG=1,1\r\n" );                                  //�򿪹���ģʽ
            GSM_status = 0x11;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
            printf("\r\n�򿪹���ģʽ\r\n");
            break;
        case 0x11:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
              if( retry >= 3 )
              {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;

              }
              else
              {
                printf("\r\n�򿪹���ģʽʧ�ܣ�����\r\n");
                GSM_status = 0x10;
                retry++;
              }
            }
            else
            {
              retry = 0;                                                  //���Դ�������
              GSM_status = 0x12;                                          //����GSM״̬
              GSM_time_flag = 0;                                          //�嶨ʱ��־λ
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
            }
            break;
          
          
     
        case 0x12:
          memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
          GSM_RX0_buf_num = 0;                                                //������0
          AT_CREG_num = 0;                                                    //���Դ�����0
          uscia2_send( (void*)"AT+CENG?\r\n" );                                     //��ȡС�������Ϣ
          GSM_status = 0x13;                                                  //����GSM״̬
          GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
          TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
          CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
          printf("\r\n��ȡС�������Ϣ\r\n");
        break;
        case 0x13:
          if (GSM_time_flag)                                                  //��ʱʱ�䵽
          {	
          if(strstr(GSM_RX0_buf, "+CENG:") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
          printf("\r\n��ѯС����վ��Ϣʧ�ܣ�����\r\n");
          GSM_status = 0x12;
          retry++;
          }
          }
          else
          {
          CELL[0] = '0';CELL[1] = '0';CELL[2] = '0';CELL[3] = '0';
          LCA[0] = '0';LCA[1] = '0';LCA[2] = '0';LCA[3] = '0';
          //----����GSM��վ��Ϣ
          for(a=0;GSM_RX0_buf[a] != 0x00;a++)
          {
          if(GSM_RX0_buf[a]=='+')
          {
          GSM_start = 1;
          GSM_Plus++;
          GSM_Comma = 0;
          }
          if(GSM_start)
          {
          switch(GSM_Plus)
          {
          case 0x01:
          break;
          case 0x02:
          if(GSM_RX0_buf[a] != ',')
          {
          if(GSM_Comma == 7)
          {
          CELL[0] = CELL[1];
          CELL[1] = CELL[2];
          CELL[2] = CELL[3];
          CELL[3] = GSM_RX0_buf[a];
          }
          if(GSM_Comma == 10)
          {
          LCA[0] = LCA[1];
          LCA[1] = LCA[2];
          LCA[2] = LCA[3];
          LCA[3] = GSM_RX0_buf[a];
          }
          }
          break;
          case 0x03:
          break;
          case 0x04:
          break;
          case 0x05:
          break;
          case 0x06:
          break;
          case 0x07:
          break;
          case 0x08:
          break;
          default:
          break;

          }
          }
          if(GSM_start)
          {
          if(GSM_RX0_buf[a]==',')
          {
          GSM_Comma++;
          }

          }   
          }
          if( LCA[0] == '0' && LCA[1] == '0' && LCA[2] == '0' && LCA[3] == '0' && CELL[0] == '0' && CELL[1] == '0' && CELL[2] == '0' && CELL[3] == '0')
          {
          GSM_status = 0x12;                                  //��ȡС����Ϣ
          printf("\r\nС����ϢΪ�գ���ȡС����Ϣ\r\n");
          }
          else
          {
          printf("\r\nGSMС����CELL��%.*s\r\n", 4, CELL);
          printf("\r\nGSMλ����LCA��%.*s\r\n", 4, LCA);
          GSM_status = 0x14;
          }
          retry = 0;                                                  //���Դ�������                                         //����GSM״̬
          GSM_time_flag = 0;                                          //�嶨ʱ��־λ
          TA1CCR0 = 0;
          TA1CTL = TASSEL_1 + MC_1 + TACLR;
          CCR0_time = 0;
          }
          }
            break;
        case 0x14:
            memset( GSM_RX0_buf,0x00,sizeof( GSM_RX0_buf ) );                   //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)AT_CSQ );                                       //��ѯ���������ź�ǿ��
            GSM_status = 0x15;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
            printf("\r\n��ѯ�ź�ǿ��\r\n");
            break;
        case 0x15:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              printf(GSM_RX0_buf);
              if(strstr(GSM_RX0_buf, "OK") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                    printf("\r\n��ѯ�ź�ǿ��ʧ�ܣ�����\r\n");
                    GSM_status = 0x14;
                    retry++;
                }
              }
              else
              {
                retry = 0;                                                  //���Դ�������
                GSM_status = 0x18;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
                //ȡ�����������ź�ǿ��
                if( GSM_RX0_buf[9] == ',' )
                {
                    CSQ[0] = '0';
                    if( '0' <= GSM_RX0_buf[8] && GSM_RX0_buf[9] <= '9' )
                    {
                        CSQ[1] = GSM_RX0_buf[8];
                    }
                    else
                    {
                        CSQ[1] = 0;
                    }
                }
        else
        {
                if( '0' <= GSM_RX0_buf[8] && GSM_RX0_buf[8] <= '9' )
                {
                    CSQ[0] = GSM_RX0_buf[8];
                }
                else
                {
                    CSQ[1] = 0;
                }
                if( '0' <= GSM_RX0_buf[9] && GSM_RX0_buf[9] <= '9' )
                {
                    CSQ[1] = GSM_RX0_buf[9];
                }
                else
                {
                    CSQ[1] = 0;
                }
              }
              printf("\r\nGSM�ź�ǿ�ȣ�%.*s\r\n", 2, CSQ);
                  }
          }
            break;
        case 0x18:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)AT_CGATT_ );                                    //��ѯGPRS����״̬
            GSM_status = 0x19;                                                  //����GSM״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ
            printf("\r\n��ѯGPRS����\r\n");
            break;
        case 0x19:
          if (GSM_time_flag)                                                  //��ʱʱ�䵽
          {	
            if(strstr(GSM_RX0_buf, "OK") == NULL)
            {
            if( retry >= 3 )
            {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
            }
            else
            {
              printf("\r\n��ѯGPRS����ʧ�ܣ�����\r\n");
              GSM_status = 0x18;
              retry++;
            }
            }
            else
            {
              retry = 0;                                                  //���Դ�������
              GSM_status = 0x1A;                                          //����GSM״̬
              GSM_time_flag = 0;                                          //�嶨ʱ��־λ
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
          }
            break;
        case 0x1A:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
            GSM_RX0_buf_num = 0;         //������0
            uscia2_send("AT+CIPHEAD=1\r\n\r\n");
            //uscia2_send( (void*)AT_SICS_0_CONTYPE_GPRS0 );                      //��IDΪ0��Profile��conType����ΪGPRS0
            GSM_status = 0x1B;                                                  //����GPRS״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ	
            printf("\r\n������ǰ��IPͷ\r\n");
            break;
        case 0x1B:
          if (GSM_time_flag)                                                  //��ʱʱ�䵽
          {	
          printf(GSM_RX0_buf);

          if(strstr(GSM_RX0_buf, "OK") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
            printf("\r\n������ǰ��IPͷʧ�ܣ�����\r\n");
            GSM_status = 0x1A;
            retry++;
         
          }
          }
          else
          {
            retry = 0;                                                  //���Դ�������
            GSM_status = 0x1C;                                          //����GSM״̬
            GSM_time_flag = 0;                                          //�嶨ʱ��־λ
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
          }
          }
            break;
        case 0x1C:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
           GSM_RX0_buf_num = 0;      
            //������0
           uscia2_send("AT+CSTT=\"CMNET\"\r\n");
            //uscia2_send( (void*)AT_SICS_0_ANP_CMNET );                          //��IDΪ0��Profile��apn����ΪCMNET
            GSM_status = 0x1D;                                                  //����GPRS״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 2500;                                                    //����һ����Լ300����Ķ�ʱ	
            printf("\r\n����CMNET\r\n");	
            break;
        case 0x1D:
        if (GSM_time_flag)                                                  //��ʱʱ�䵽
        {	
          printf(GSM_RX0_buf);

          if(strstr(GSM_RX0_buf, "OK") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
            printf("\r\n����CMNETʧ�ܣ�����\r\n");
            GSM_status = 0x1C;
            retry++;
          }
          }
          else
          {
            retry = 0;                                                  //���Դ�������
            GSM_status = 0x1E;                                          //����GSM״̬
            //GSM_status = 0x22;                                          //����GSM״̬

            GSM_time_flag = 0;                                          //�嶨ʱ��־λ
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
           }
          }
            break;
          case 0x1E:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
            GSM_RX0_buf_num = 0;                                                //������0
            uscia2_send( (void*)"AT+CIICR\r\n" );                                     //����������·
            GSM_status = 0x1F;                                                  //����GPRS״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
			TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ	
            printf("\r\n����������·\r\n");
            break;
        case 0x1F:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
            if(strstr(GSM_RX0_buf, "OK") == NULL)
            {
            if( retry >= 3 )
            {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
            }
            else
            {
            printf("\r\n����������·������\r\n");
            GSM_status = 0x1E;
            retry++;
            }
            }
            else
            {
              retry = 0;                                                  //���Դ�������
              GSM_status = 0x20;                                          //����GSM״̬
              GSM_status = 0x22;                                          //����GSM״̬
              
              GSM_time_flag = 0;                                          //�嶨ʱ��־λ
              TA1CCR0 = 0;
              TA1CTL = TASSEL_1 + MC_1 + TACLR;
              CCR0_time = 0;
            }
            }
            break;        
        
        
        case 0x20:    // AT+CIFSR ��ñ���IP��ַ 

          memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
          GSM_RX0_buf_num = 0;                                                //������0
          uscia2_send( (void*)"AT+CIFSR\r\n" );                                     //��ȡ����IP��ַ
          GSM_status = 0x21;                                                  //����GPRS״̬
          GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
          TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
          CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ	
          printf("\r\n��ȡ����IP��ַ\r\n");
          break;
        case 0x21:
        if (GSM_time_flag)                                                  //��ʱʱ�䵽
        {	
          if(strstr(GSM_RX0_buf, ".") == NULL)
          {
          if( retry >= 3 )
          {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
          }
          else
          {
            printf("\r\n��ȡ����IP��ַ������\r\n");
            GSM_status = 0x20;
            retry++;
          }
          }
          else
          {

            retry = 0;                                                  //���Դ�������
            GSM_status = 0x22;                                          //����GSM״̬
            printf("\r\n����IP��%s", GSM_RX0_buf+2);
            delay100ms();
            GSM_time_flag = 0;                                          //�嶨ʱ��־λ
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
          }
          }
          break;
        case 0x22:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
            GSM_RX0_buf_num = 0;  //������0    
            printf("\r\n����TCP������������\r\n");
            printf((void*)AT_CIPSTART_TCP);

            uscia2_send( (void*)AT_CIPSTART_TCP );                              //����TCP������������                  delay_ms(1000);
            delay_ms(1000);
            delay_ms(1000);
            delay_ms(1000);
            GSM_status = 0x23;                                                  //����GPRS״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 3750;                                                    //����һ����Լ300����Ķ�ʱ
            break;
        case 0x23:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              if(strstr(GSM_RX0_buf, "OK") == NULL)
            {
            if( retry >= 3 )
            {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
            }
            else
            {
            printf("\r\n����TCP������������ʧ�ܣ�����\r\n");
            GSM_status = 0x22;
            retry++;
            }
            }
            else
            {
              printf("\r\n�����������TCP���ӳɹ�\r\n");
            retry = 0;                                                  //���Դ�������
            GSM_status = 0x2C;                                          //����GSM״̬
            GSM_time_flag = 0;                                          //�嶨ʱ��־λ
            TA1CCR0 = 0;
            TA1CTL = TASSEL_1 + MC_1 + TACLR;
            CCR0_time = 0;
            }
            }
            break;
  
        
        case 0x2C:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
            GSM_RX0_buf_num = 0;                                                //������0
            if(data_type == 0 || data_type == 1 || data_type == 2)              //������ä���ش�
            {
              uscia2_send( (void*)AT_CIPSEND_99 );                             //��ģ������99���ֽڻ�����
            }
            else if(data_type == 3)                                             //д����TCP��Ӧ
            {
              uscia2_send( (void*)AT_CIPSEND_113 );                             //��ģ������14���ֽڻ�����
            }
            GSM_status = 0x2D;                                                  //����GPRS״̬
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                     //����һ����Լ3��Ķ�ʱ
            CCR0_time = 1250;                                                    //����һ����Լ300����Ķ�ʱ	
            printf("\r\n��ģ�����뻺����\r\n");
            break;
        case 0x2D:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
              if(strstr(GSM_RX0_buf, ">") == NULL)
              {
                if( retry >= 3 )
                {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                    printf("\r\n���뻺����ʧ�ܣ�����\r\n");
                    GSM_status = 0x2C;
                    retry++;
                }
              }
              else
              {
                retry = 0;                                                  //���Դ�������
                GSM_status = 0x2E;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
              }
            }
            break;
        case 0x2E:
            memset(GSM_RX0_buf,0x00,sizeof(GSM_RX0_buf));                       //����������
            GSM_RX0_buf_num = 0;                                                //������0
            if(data_type == 0 || data_type == 1 || data_type == 2)              //������ä���ش�
            {
              send_PostBack_data( (void*)data_packet,99 );                   //��ģ�黺����д��99���ֽ�
              GSM_status = 0x2F;                                              //����GPRS״̬
            }
            else if(data_type == 3)                                             //TCP��Ӧ
            {
              send_PostBack_data( (void*)data_packet,113);                    //��ģ�黺����д��113���ֽ�
              GSM_status = 0x31;                                              //����GPRS״̬
                    
            }
            GSM_time_flag = 0;                                                  //�嶨ʱ��־λ
            TA1CCR0 = 3750;                                                    //����һ����Լ3��Ķ�ʱ
            CCR0_time = 2500;                                                   //����һ����Լ2��Ķ�ʱ		
            printf("\r\n��ģ�黺����д������\r\n");
            break;
        case 0x2F:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	
	      if(strstr(GSM_RX0_buf, "+IPD,9:") != NULL)
               {
                GSM_status = 0x30;                                          //����GSM״̬
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                CCR0_time = 0;
                send_retry = 0;
                if(data_type == 0)                                          //�������������
                {
                data_send_flag = 1;// ��׼��λ���ݷ��ͳɹ���־��λ
                }
                if(data_type == 1)
                {
                data_send_flag = 2;// ��׼��λ���ݷ��ͳɹ���־��λ
                }
                if(data_type == 2)//�����ä������
                {
                blind_data_size--;//ä����������1
                printf("\r\nä��������Ϊ��%d\r\n",blind_data_size);
                blind_data_head--;//ä������ͷ��1
                if(blind_data_head < 1)
                blind_data_head = 500;
                }
                printf("\r\n�յ�������Ӧ�������ϴ��ɹ�\r\n");
              }
              else
              {
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                CCR0_time = 0;
                if( send_retry >= 3 )
                {
                  printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;
                }
                else
                {
                  printf("\r\nû���յ���������Ӧ�������ϴ�\r\n");
                  GSM_status = 0x28;                                      //����GSM״̬
                  send_retry++;
                }
              }
                

            }
            break;	
            case 0x30:
              
             GSM_RX0_buf[20]=0x31;
            GSM_RX0_buf[21]=0x31;
            GSM_RX0_buf[22]=0x31;
            GSM_RX0_buf[23]=0x31;
            GSM_RX0_buf[24]=0x31;
            if(strstr(GSM_RX0_buf, "+IPD,83:") == NULL)
            {
                printf("\r\nû�м�⵽��������֡ͷ\r\n");
            }
            else
            {
                printf("\r\n�Ѿ���⵽��������֡ͷ\r\n");
            }
            
            //if(sscanf((char *)strstr(GSM_RX0_buf, "+IPD,83:"), "^SISR: 1, 83, %s",buf) == 1)
			if(strstr(GSM_RX0_buf, "+IPD,83:") != NULL)
            {
				printf("\r\n��⵽���������������ʼ��������\r\n");
				//ascii2data((void*)buf1,(void*)buf,168);
                p = (void *)strstr(GSM_RX0_buf, "+IPD,83:");
                p = p + 8;
                for(w = 0; w < 83; w++)
                {
                    buf1[w] = *p;
                    p++;
                }
                if( buf1[0] == 0xFF && buf1[1] == 0xFF && buf1[2] == 0xFF && buf1[3] == 0xFF && buf1[4] == 0xFF )
                {
                    // ������������ַ
                    if( buf1[12] == 0x01 || buf1[12] == 0x02)
                    {
                        servers_addr[0] = buf1[12];
                        servers_addr[1] = buf1[13];
                        servers_addr[2] = buf1[14];
                        servers_addr[3] = buf1[15];
                        servers_addr[4] = buf1[16];
                        servers_addr[5] = buf1[17];
                        servers_addr[6] = buf1[18];
                        servers_addr[7] = buf1[19];
                        servers_addr[8] = buf1[20];
                        servers_addr[9] = buf1[21];
                        servers_addr[10] = buf1[22];
                        servers_addr[11] = buf1[23];
                        servers_addr[12] = buf1[24];
                        servers_addr[13] = buf1[25];
                        servers_addr[14] = buf1[26];
                        servers_addr[15] = buf1[27];
                        servers_addr[16] = buf1[28];
                        servers_addr[17] = buf1[29];
                        servers_addr[18] = buf1[30];
                        servers_addr[19] = buf1[31];
                        servers_addr[20] = buf1[32];
                        servers_addr[21] = buf1[33];
                        servers_addr[22] = buf1[34];
                        servers_addr[23] = buf1[35];
                        servers_addr[24] = buf1[36];
                        servers_addr[25] = buf1[37];
                        servers_addr[26] = buf1[38];
                        servers_addr[27] = buf1[39];
                        servers_addr[28] = buf1[40];
                        servers_addr[29] = buf1[41];
                        eeprom_PageWrite(servers_addr, 0x0017, 30);
                        printf("\r\n��������ַ�������洢���\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸ķ�������ַ\r\n");
                    }
                    // �����������˿�
                    if( buf1[42] != 0x00 || buf1[43] != 0x00 )
                    {
                        port = buf1[42];
                        port = ( port << 8 ) + buf1[43];
                        I2C2_ByteWrite((uchar)(port >> 8), 0x0035);
                        I2C2_ByteWrite((uchar)(port), 0x0036);
                        printf("\r\n�������˿ڽ������洢���\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸ķ������˿�\r\n");
                    }
                    // ��������ʱ��
                    printf("\r\n��ʼ��������ʱ�䲢Уʱ\r\n");
                    Correction_Time(buf1[44] , buf1[45] ,buf1[46], buf1[47] , buf1[48] , buf1[49] );
                    // ��������״̬
                    if( buf1[50] != 0x00)
                    {
                        activate_status = buf1[50];
                        I2C2_ByteWrite( activate_status, 0x000A );
                        printf("\r\n�豸����״̬�������洢���\r\n");
                        //-------����豸����Ϊ������ģʽ��������������-----------
                        if( buf1[50] == 0x02 )
                        {
                            I2C2_ByteWrite(0x00, 0x007C);
                            __delay_cycles(100000);
                        }
                    }
                    else
                    {
                        printf("\r\n���޸ļ���״̬\r\n");
                    }
                    // ��������ģʽ
                    if( buf1[51] != 0x00)
                    {
                        work_mode = buf1[51];
                        I2C2_ByteWrite( work_mode, 0x000B );
                        printf("\r\n�豸����ģʽ�������洢���\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸Ĺ���ģʽ\r\n");
                    }
                    // ��������ʱ��
                    if( buf1[52] != 0x00)
                    {
                        work_time = buf1[52];
                        I2C2_ByteWrite( work_time, 0x000C );
                        printf("\r\n�豸����ʱ��������洢���\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸Ĺ���ʱ��\r\n");
                    }
                    // ��������ʱ��
                    if( buf1[53] != 0x00 || buf1[54] != 0x00 )
                    {
                        sleep_time = buf1[53];
                        sleep_time = ( sleep_time << 8 ) + buf1[54];
                        I2C2_ByteWrite((uchar)(sleep_time >> 8), 0x000D);
                        I2C2_ByteWrite((uchar)(sleep_time), 0x000E);
                        printf("\r\n�豸����ʱ��������洢���\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸�����ʱ��\r\n");
                    }
                    // ������ʱʱ��
                    if( buf1[55] != 0xFF || buf1[56] != 0xFF )
                    {
                        timing_time = buf1[55];
                        timing_time = ( timing_time << 8 ) + buf1[56];
                        //I2C2_ByteWrite((uchar)(timing_time >> 8), 0x0011);
                        //I2C2_ByteWrite((uchar)(timing_time), 0x0012);
                        printf("\r\n�豸��ʱʱ��������\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸Ķ�ʱʱ��\r\n");
                    }
                    // �������ʱ��
                    if( buf1[57] != 0x00 || buf1[58] != 0x00 )
                    {
                        takt_time = buf1[57];
                        takt_time = ( takt_time << 8 ) + buf1[58];
                        I2C2_ByteWrite((uchar)(takt_time >> 8), 0x000F);
                        I2C2_ByteWrite((uchar)(takt_time), 0x0010);
                        printf("\r\n�豸���ʱ��������洢���\r\n");
                    }
                    else
                    {
                        printf("\r\n���޸ļ��ʱ��\r\n");
                    }
                    // �����ۼƹ�ʱ
                    if( buf1[59] != 0xFF || buf1[60] != 0xFF || buf1[61] != 0xFF || buf1[62] != 0xFF )
                    {
                        labor_time = buf1[59];
                        labor_time = ( labor_time << 8 ) + buf1[60];
                        labor_time = ( labor_time << 8 ) + buf1[61];
                        labor_time = ( labor_time << 8 ) + buf1[62];
                        I2C2_ByteWrite((uchar)(labor_time >> 24), 0x0013);
                        I2C2_ByteWrite((uchar)(labor_time >> 16), 0x0014);
                        I2C2_ByteWrite((uchar)(labor_time >> 8), 0x0015);
                        I2C2_ByteWrite((uchar)(labor_time), 0x0016);
                        printf("\r\n�豸�ۼƹ�ʱ�������洢���\r\n");
                        //--------�����ʱ�������豸��������һ������------------
                        if(buf1[59] == 0x00 && buf1[60] == 0x00 && buf1[61] == 0x00 && buf1[62] == 0x00)
                        {
                            I2C2_ByteWrite(0x00, 0x003B);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x00, 0x003C);
                            __delay_cycles(100000);
                            system_rst = 0;
                        }
                        //-------�����ʱ�������豸ä������һ������-------------
                        if(buf1[59] == 0x00 && buf1[60] == 0x00 && buf1[61] == 0x00 && buf1[62] == 0x00)
                        {
                            I2C2_ByteWrite(0x00, 0x0037);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x00, 0x0038);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x00, 0x0039);
                            __delay_cycles(100000);
                            I2C2_ByteWrite(0x01, 0x003A);
                            __delay_cycles(100000);
                        }
                    }
                    else
                    {
                        printf("\r\n���޸��ۼƹ�ʱ\r\n");
                    }
                    GSM_status = 0x2C;                                          // ׼����Ӧ������
                    config_flag = 0x0000;                                       // �������óɹ�
					data_type = 3;                                              //д������Ӧ
                    printf("\r\n��������������������\r\n");
                    
                }
                else
                {
                    printf("\r\n��������������Ƿ�����������\r\n");
                    GSM_status = 0x2C;                                          // ׼����Ӧ������
                    config_flag = 0xFFFF;                                       // �������óɹ�
					data_type = 3;                                              //д������Ӧ
                }
				
			}
			else
			{
				printf("\r\nû�з�������������\r\n");
                GSM_status = 0xFF;                                              //����GSM����״̬
				data_type = 0;                                                  //�ָ������ش�ģʽ
			}
            break;
              
              
              
            case 0x31:
            if (GSM_time_flag)                                                  //��ʱʱ�䵽
            {	                
              if(strstr(GSM_RX0_buf, "OK") == NULL)

                //if(strstr(GSM_RX0_buf, "^SISW") == NULL)
                {
                  if( retry >= 3 )
                  {
                    printf("\r\n���Դ�������3�Σ����¿���\r\n");
                    GSM_POWER_OFF;
                    delay_ms(1000);
                    delay_ms(1000);
                    GSM_POWER_ON;
                    delay_ms(500);
                    GSM_status = 0x00;
                  }
                  else
                  {
                    printf("\r\nTCP��Ӧʧ�ܣ����·���ATָ��\r\n");
                    GSM_status = 0x2C;
                    retry++;
                  }
              }
              else
              {
                retry = 0;                                                  //���Դ�������
                GSM_status = 0xFF;                                          //����GSM״̬
                data_type = 0;                                              //�ָ������ش�ģʽ
                GSM_time_flag = 0;                                          //�嶨ʱ��־λ
                TA1CCR0 = 0;
                TA1CTL = TASSEL_1 + MC_1 + TACLR;
                CCR0_time = 0;
                printf("\r\nTCP��Ӧ�ɹ�\r\n");
              }
            }
            break;
        case 0x40:
                  //printf("\r\n���Դ�������3�Σ����¿���\r\n");
                  GSM_POWER_OFF;
                  delay_ms(1000);
                  delay_ms(1000);
                  GSM_POWER_ON;
                  delay_ms(500);
                  GSM_status = 0x00;                                                //����GPRS״̬
            break;
        case 0xFF:
            break;
        default:
          
            printf("GSM״̬�Ƿ�: %d\r\n",GSM_status);
            printf("\r\n���¿���\r\n");
            GSM_POWER_OFF;
            delay_ms(1000);
            delay_ms(1000);
            GSM_POWER_ON;
            delay_ms(500);
            GSM_status = 0x00;                                             //����GPRS״̬
            break;
    }
}
//--------------asciiת16�����Ӻ���---------------------------------------------
void ascii2data(char * des, char * src, int size)
{
	uint i = 0;
	uint j = 0;
	uchar highData;
	uchar lowData;
	i++;
	while (i < (size - 1)) 
	{
		if (src[i] <= '9' && src[i] >= '0')
			highData = src[i] - '0';
		else if(src[i] <= 'F' && src[i] >= 'A')
			highData = src[i] - 'A' + 0xa;
		i++;
		if (src[i] <= '9' && src[i] >= '0')
			lowData = src[i] - '0';
		else
			lowData = src[i] - 'A' + 0xa;
		des[j] = lowData + (highData << 4);
		j++;
		i++;
	}
}
//--------------16����תascii�Ӻ���---------------------------------------------
void data2ascii(char * des, char * src, int size)
{
	uint i = 0;
	uint j = 0;
	uchar highData;
	uchar lowData;
	while (i < size) 
    {
		highData	= (src[i] >> 4) & 0xf;
		lowData		= src[i] & 0xf;
		if(highData <= 0x09)
    		des[j] = highData+'0';
		else
    		des[j] = highData - 0x0A + 'A';
		j++;
		if(lowData <= 0x09)
    		des[j] = lowData + '0';
		else
    		des[j] = lowData - 0x0A + 'A';
		j++;
		i++;
	}	
}
