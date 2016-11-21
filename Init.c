#include <msp430f5438a.h>
#include "Init.h"

void CLK_Init()
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
  UCSCTL5 |= DIVS_0;                     // SMCLK/1=8Mhz
  UCSCTL4 |= SELS_5 + SELM_5;            // SMCLK=MCLK=XT2=8Mhz 
}
