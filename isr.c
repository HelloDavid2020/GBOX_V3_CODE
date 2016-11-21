#include "isr.h"
#include "key.h"
uchar gps_debug_flag = 0;
uchar gsm_debug_flag = 0;
//------------串口0中断子函数---------------------------------------------------
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
              disable_gps_irq();  // 禁止接收中断
              dataLen = 0;
              gpsRxFlag = 1; // 接收完成标志置位
            } 
          
          
          //GPS_analysis(UCA0RXBUF);        // 调用GPS解析函数
          if(gps_debug_flag == 1)
          SerialPutChar(UCA0RXBUF); 
            
            break;
        case 4:
            break;                                      // Vector 4 - TXIFG
        default: 
            break;
    } 
}
//------------串口2中断子函数---------------------------------------------------
#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR(void)
{
    switch(__even_in_range(UCA2IV,4))
    {
        case 0:
            break;                          // Vector 0 - no interrupt
        case 2:                             // Vector 2 - RXIFG
            TA1CCR0 = CCR0_time;                        //串口接受到数据定时器A0开始/重新计数
            TA1CTL = TASSEL_1 + MC_1 + TACLR;           // 时钟选择ACLK,增模式定时器计数到TACCR0,定时器清零
            GSM_RX0_buf[GSM_RX0_buf_num] = UCA2RXBUF;	//数据送入接受缓冲区 
            GSM_RX0_buf_num++; 
              putchar(UCA2RXBUF);     
            break;
        case 4:
            break;                          // Vector 4 - TXIFG
        default: 
            break;
    }  
}
//------------串口3中断子函数-----------------
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
                UCA3IE &= ~UCRXIE;           // 关串口0接受中断
                uart_flag = 1;
            }
            

            config_buf[config_num] = UCA3RXBUF;		//数据送入配置接受缓冲区
            config_num++;
            if(config_num > 99)
            {
                config_num = 0;
                uart_flag = 0;
            }
            
            
            uscia_config();  // 串口配置 
            
            
            
            break;
        case 4:
            break;                          // Vector 4 - TXIFG
        default: 
            break;
    } 
}
//------------TA0定时中断子函数-------------------------------------------------
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    GSM_time_flag = 1;
    GSM_RX0_buf_num=0;
}
//----------------光敏中断子函数------------------------------------------------
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
        case 12://光敏报警
            P1IFG &= ~BIT5;
            if(activate_status == 0x02)// 如果在测试模式
            {
                alarm = 0x03;   // 激活产品
            }
            if(activate_status == 0x03 && activation_send_num >= activation_max_send_num)// 如果在激活模式且不再激活期
            {
                alarm = 0x04;   // 拆除报警
            }
            if(work_type_flag == 1)
            {
                work_type = 0;
                _BIC_SR_IRQ(LPM4_bits);					//退出低功耗
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
//----------------时钟中断、外部中断子函数---------------------------------------
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
            //       WDTCTL = 0X00;  // 写错误看门狗密码，软重启设备
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
            if(work_type == 0) // 如果是工作状态
            {
              if(work_mode == MODE_GPS && gps_start == 0)
              {                
                gps_start=1;
                LPM3_EXIT;
              }
              else
                work_type = 1; // 置设备为休眠状态
            }
            
            else               // 如果是休眠状态
            {
                work_type = 0; // 置设备为工作状态
                _BIC_SR_IRQ(LPM4_bits);					//退出低功耗 
            }
            takt_time_flag = 1;
            break;            
        default: 
            break;
    } 

}
