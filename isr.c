#include "isr.h"
#include "key.h"
uchar gps_debug_flag = 0;
uchar gsm_debug_flag = 0;
//------------����0�ж��Ӻ���---------------------------------------------------
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV,4))
    {
        case 0:
            break;                                      // Vector 0 - no interrupt
        case 2:                                         // Vector 2 - RXIFG  
            gps_buffer[dataLen++] = UCA0RXBUF;
            if(dataLen >= NMEA_COUNT_MAX) 
            {    
              disable_gps_irq();  // ��ֹ�����ж�
              dataLen = 0;
              gpsRxFlag = 1; // ������ɱ�־��λ
            } 
          
          
          //GPS_analysis(UCA0RXBUF);        // ����GPS��������
          if(gps_debug_flag == 1)
          SerialPutChar(UCA0RXBUF); 
            
            break;
        case 4:
            break;                                      // Vector 4 - TXIFG
        default: 
            break;
    } 
}
//------------����2�ж��Ӻ���---------------------------------------------------
#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR(void)
{
    switch(__even_in_range(UCA2IV,4))
    {
        case 0:
            break;                          // Vector 0 - no interrupt
        case 2:                             // Vector 2 - RXIFG
            TA1CCR0 = CCR0_time;                        //���ڽ��ܵ����ݶ�ʱ��A0��ʼ/���¼���
            TA1CTL = TASSEL_1 + MC_1 + TACLR;           // ʱ��ѡ��ACLK,��ģʽ��ʱ��������TACCR0,��ʱ������
            GSM_RX0_buf[GSM_RX0_buf_num] = UCA2RXBUF;	//����������ܻ����� 
            GSM_RX0_buf_num++; 
              putchar(UCA2RXBUF);     
            break;
        case 4:
            break;                          // Vector 4 - TXIFG
        default: 
            break;
    }  
}
//------------����3�ж��Ӻ���-----------------
// Debug port
#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
    switch(__even_in_range(UCA3IV,4))
    {
        case 0:
            break;                          // Vector 0 - no interrupt
        case 2:                             // Vector 2 - RXIFG
            if(UCA3RXBUF == 0x0A)
            {
                UCA3IE &= ~UCRXIE;           // �ش���0�����ж�
                uart_flag = 1;
            }
            

            config_buf[config_num] = UCA3RXBUF;		//�����������ý��ܻ�����
            config_num++;
            if(config_num > 99)
            {
                config_num = 0;
                uart_flag = 0;
            }
            
            
            uscia_config();  // �������� 
            
            
            
            break;
        case 4:
            break;                          // Vector 4 - TXIFG
        default: 
            break;
    } 
}
//------------TA0��ʱ�ж��Ӻ���-------------------------------------------------
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    GSM_time_flag = 1;
    GSM_RX0_buf_num=0;
}
//----------------�����ж��Ӻ���------------------------------------------------
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    switch(__even_in_range(P1IV,16))
    {
        case 0:
            break;                          // Vector 0 - no interrupt
        case 2:    
            break;
        case 4:
            break;                          
        case 6:
            break; 
        case 8:
            break; 
        case 10:
            break; 
        case 12://��������
            P1IFG &= ~BIT5;
            if(activate_status == 0x02)// ����ڲ���ģʽ
            {
                alarm = 0x03;   // �����Ʒ
            }
            if(activate_status == 0x03 && activation_send_num >= activation_max_send_num)// ����ڼ���ģʽ�Ҳ��ټ�����
            {
                alarm = 0x04;   // �������
            }
            if(work_type_flag == 1)
            {
                work_type = 0;
                _BIC_SR_IRQ(LPM4_bits);					//�˳��͹���
            }
            
            break;
        case 14:
            break;
        case 16:
            break;            
        default: 
            break;
    } 

}
//----------------ʱ���жϡ��ⲿ�ж��Ӻ���---------------------------------------
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    switch(__even_in_range(P2IV,16))
    {
        case 0:
            break;                          // Vector 0 - no interrupt
        case 2:    
            break;
        case 4:
            break;                          
        case 6:
            break; 
        case 8:
            //P2IFG &= ~BIT3;
            //__delay_cycles(10000);
            //if((P2IN & BIT3) == 0x08)
            //{
            //    __delay_cycles(10000);
            //    if((P2IN & BIT3) == 0x08)
            //    {
            //       WDTCTL = 0X00;  // д�����Ź����룬�������豸
            //    }
            //}
            break; 
        case 10:
            break; 
        case 12:
            break;
        case 14:
            break;
        case 16:
            P2IFG &= ~BIT7;
            if(work_type == 0) // ����ǹ���״̬
            {
              if(work_mode == MODE_GPS && gps_start == 0)
              {                
                gps_start=1;
                LPM3_EXIT;
              }
              else
                work_type = 1; // ���豸Ϊ����״̬
            }
            
            else               // ���������״̬
            {
                work_type = 0; // ���豸Ϊ����״̬
                _BIC_SR_IRQ(LPM4_bits);					//�˳��͹��� 
            }
            takt_time_flag = 1;
            break;            
        default: 
            break;
    } 

}
