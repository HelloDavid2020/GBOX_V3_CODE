#include "MSP430F5438A.h"
#include "time.h"

void TimeA0_init( void )
{
    TA1CCTL0 = CCIE;                            // �����ж�
    TA1CCR0 = 0;                                // ֹͣ����
    TA1CTL = TASSEL_1 + MC_1 + TACLR;           // ʱ��ѡ��ACLK,��ģʽ��ʱ��������TACCR0,��ʱ������
}
