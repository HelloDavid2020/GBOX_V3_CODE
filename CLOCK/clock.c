
//******************************************************************************
//   MSP430F543xA Demo - Software Toggle P1.0 with 12MHz DCO
//
//   Description: Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//   ACLK is rought out on pin P11.0, SMCLK is brought out on P11.2, and MCLK
//   is brought out on pin P11.1.
//   ACLK = REFO = 32kHz, MCLK = SMCLK = 12MHz
//   PMMCOREV = 1 to support up to 12MHz clock
//
//                 MSP430F5438A
//             -----------------
//         /|\|                 |
//          | |            P11.0|-->ACLK
//          --|RST         P11.1|-->MCLK
//            |            P11.2|-->SMCLK
//            |                 |
//            |             P1.0|-->LED
//
//   Note: 
//   In order to run the system at up to 12MHz, VCore must be set at 1.6V 
//   or higher. This is done by invoking function SetVCore(), which requires 
//   2 files, hal_pmm.c and hal_pmm.h, to be included in the project.
//   hal_pmm.c and hal_pmm.h are located in the same folder as the code 
//   example. 
//
//   D. Dang
//   Texas Instruments Inc.
//   December 2009
//   Built with CCS Version: 4.0.2 and IAR Embedded Workbench Version: 4.21.8
//******************************************************************************

#include "MSP430F5438A.h"
#include "clock.h"

void Clock_init( void )
{
    P5SEL |= BIT2+BIT3;         // 端口功能选择振荡器
    UCSCTL6 |= XT1OFF;          // 关闭XT1振荡器
    UCSCTL6 &= ~XT2OFF;         // 打开XT2振荡器
    UCSCTL3 |= SELREF_2;        // FLLref = REFO
    UCSCTL4 |= SELA_1;          // ACLK=VLOCLK,SMCLK=DCO,MCLK=DCO      
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);         // 清除 XT2,XT1,DCO 错误标志                                                          
        SFRIFG1 &= ~OFIFG; 
    }while(SFRIFG1&OFIFG);                                              // 检测振荡器错误标志
    UCSCTL6 &= ~(XT2DRIVE0 + XT2DRIVE1);                                // XT2 驱动模式 4~8MHz                                            
    UCSCTL4 |= SELS_5 + SELM_5;                                         // SMCLK = MCLK = XT2
    UCSCTL5 |= DIVA_3;                                                  // ACLK 8分频
}


void CLK_Init(void)
{
  WDTCTL = WDTPW + WDTHOLD;   // Stop WDT
  //P11DIR |= 0x07;        // ACLK, MCLK, SMCLK set out to pins 
  //P11SEL |= 0x07;      // P11.0,1,2 for debugging purposes. 
  __bis_SR_register(SCG0);  // Disable the FLL control loop +OSCOFF 
  // Initialize LFXT1 ,XT2
  P5SEL |= 0x0C;        // Port select XT2 
  P7SEL |= 0x03;        // Port select XT1 
  UCSCTL6 &= ~(XT1OFF + XT2OFF);    // Set XT1 & XT2 On 
  UCSCTL6 |= XCAP_3; // Internal load cap 	
  // Loop until XT1,XT2 & DCO fault flag is cleared 
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
    // Clear XT2,XT1,DCO fault flags 
    SFRIFG1 &= ~OFIFG;                  // Clear fault flags
  }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag 
  UCSCTL6 &= ~XT2DRIVE1;              // Decrease XT2 Drive according to
  // expected frequency 
  UCSCTL5 |= DIVS_3;                     // SMCLK/1=8Mhz
  UCSCTL4 |= SELS_5 + SELM_5;            // SMCLK=MCLK=XT2=8Mhz 
}



void dco_int(void) // 开启内部时钟源， 倍频到 7.3728 MHz
{
  UCSCTL6 |= XT1OFF + XT2OFF;             // Set XT1 & XT2 Off
  //SetVCore(PMMCOREV_1);                 // Set VCore = 1.6V for 12MHz clock  主频太高时配置内核电压

  //P11DIR |= (BIT0+BIT1+BIT2);                // ACLK, MCLK, SMCLK set out to pins
  //P11SEL |= (BIT0+BIT1+BIT2);                // P11.0,1,2 for debugging purposes.

  UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
  UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

  
//  // NOTE: Change core voltage one level at a time..
  
  __bis_SR_register(SCG0);                  // Disable the FLL control loop
  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
  UCSCTL1 = DCORSEL_5;                      // Select DCO range 24MHz operation
  UCSCTL2 = FLLD_1 + 223;                   // Set DCO Multiplier for 7.3728MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (223 + 1) * 32768 = 8MHz
                                            // Set FLL Div = fDCOCLK/2
  __bic_SR_register(SCG0);                  // Enable the FLL control loop

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  // 32 x 32 x 7.3728 MHz / 32,768 Hz = 230400= MCLK cycles for DCO to settle
  __delay_cycles(230400);
    __delay_cycles(230400);      
  // Loop until XT1,XT2 & DCO fault flag is cleared
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
                                            // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
  
  
  //UCSCTL5 |= DIVA_3;                    // ACLK 8分频
  //UCSCTL4 |= SELS__DCOCLK + SELM__DCOCLK;           // SMCLK = MCLK = DCO
  //UCSCTL4 = UCSCTL4&(~(SELS_7|SELM_7))|SELS_3|SELM_3;  

  
}

