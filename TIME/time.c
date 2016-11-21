#include "MSP430F5438A.h"
#include "time.h"

void TimeA0_init( void )
{
    TA1CCTL0 = CCIE;                            // 允许中断
    TA1CCR0 = 0;                                // 停止计数
    TA1CTL = TASSEL_1 + MC_1 + TACLR;           // 时钟选择ACLK,增模式定时器计数到TACCR0,定时器清零
}
