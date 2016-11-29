#include "MSP430F5438A.h"
#include "at24c512.h"

//-----------------IIC2��ʼ���Ӻ���---------------------------------------------
void I2C2_Init(void)
{
    //EEP_WP_INIT;                                // EEP_WP IO��ʼ��
    //EEP_WP_ENABLE;                              // EEPд����ʹ��
    IIC2_IO_INIT;                               // ���ù��ܿ�
    UCB2CTL1 |= UCSWRST;                        // ʹ�������λ
    UCB2CTL0 = UCMST + UCMODE_3 + UCSYNC;       // ���ص�ַ7λ�����豸��ַ7λ���������������豸ģʽ��i2cģʽ��ͬ��ģʽ
    UCB2CTL1 |= UCSSEL_2 + UCSWRST;             // USCIʱ��Դѡ��SMCLK=XT2=8M�����������λ
    UCB2BR0 = 80;				// ��Ƶ������ֵ
    UCB2BR1 = 0;				// fSCL = SMCLK/UCB1BR0 = 8000KHz/40 = ~250kHz
    UCB2I2CSA = EEPROM_ADDRESS;                 // EEPROM��ַ
    UCB2CTL0 &= ~UCSLA10;                   	// ȷ��7λ��ַģʽ
    UCB2CTL1 &= ~UCSWRST;                       // ��������λ���ָ�����
}
//-----------------IIC2дһ���ֽ��Ӻ���-----------------------------------------
void I2C2_ByteWrite(uchar TXdata, uint Addr)
{
    __delay_cycles(100000);
    //EEP_WP_DISABLE;                                 // д������ֹ
    while( UCB2CTL1 & UCTXSTP );                    // ȷ��ֹͣ����û�в���
    UCB2CTL1 |= UCTR;                               // дģʽ
    UCB2CTL1 |= UCTXSTT;                            // ��������λ
    UCB2TXBUF = (uchar)( ( Addr & 0xFF00 ) >> 8 );  // ���ʹ��ֽڸߵ�ַ
    while( !( UCB2IFG & UCTXIFG ) );                // �ȴ�������ɣ�UCB2TXBUFΪ��ʱUCTXIFGλ����1��
    while( UCB2CTL1 & UCTXSTT );                    // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB2TXBUF = (uchar)( Addr & 0x00FF );           // ���ʹ��ֽڵ͵�ַ
    while( !( UCB2IFG & UCTXIFG ) );                // �ȴ��������

    UCB2TXBUF = TXdata;                             // ��������
    while( !( UCB2IFG & UCTXIFG ) );                // �ȴ�������ɣ�UCB2TXBUFΪ��ʱUCTXIFGλ����1��
    UCB2CTL1 |= UCTXSTP;                            // ����ֹͣλ
    while( UCB2CTL1 & UCTXSTP );                    // �ȴ��������  
    //EEP_WP_ENABLE;                                  // д����ʹ��
}
//-----------------IIC2��һ���ֽ��Ӻ���-----------------------------------------
uchar I2C2_ByteRead(uint Addr)
{
    uchar re;
    
    __delay_cycles(100000);
    while( UCB2CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
    UCB2CTL1 |= UCTR;                                   // дģʽ
    UCB2CTL1 |= UCTXSTT;                                // ��������λ
    UCB2TXBUF = (uchar)( ( Addr & 0xFF00 ) >> 8 );      // ���ʹ��ֽڸߵ�ַ
    while( !( UCB2IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB2TXBUFΪ��ʱUCTXIFGλ����1��
    while( UCB2CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB2TXBUF = (uchar)( Addr & 0x00FF );               // ���ʹ��ֽڵ͵�ַ
    while( !( UCB2IFG & UCTXIFG ) );                    // �ȴ��������
    
    UCB2CTL1 &= ~UCTR;                                  // ��ģʽ
    UCB2CTL1 |= UCTXSTT;                                // ��������λ
    while( UCB2CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB2CTL1 |= UCTXSTP;                                // ����ֹͣλ
    while( UCB2CTL1 & UCTXSTP );                        // �ȴ��������  
    while( !( UCB2IFG & UCRXIFG ) );                    // �յ�����
    re = UCB2RXBUF;                                     // ��ȡ�ֽ�����
    return re;                                          // ��������
}
//-----------IIC2д����ֽڣ����128���ֽڣ��Ӻ���------------------------------
void eeprom_PageWrite(uchar* pBuffer, uint WriteAddr, uchar NumByteToWrite)
{
    return;

    __delay_cycles(100000);
	//EEP_WP_DISABLE;                                     // д������ֹ
	while( UCB2CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
    UCB2CTL1 |= UCTR;                                   // дģʽ
	UCB2CTL1 |= UCTXSTT;                                // ��������λ
    UCB2TXBUF = (uchar)( ( WriteAddr & 0xFF00 ) >> 8 ); // ���ʹ��ֽڸߵ�ַ
	while( !( UCB2IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB2TXBUFΪ��ʱUCTXIFGλ����1��
    while( UCB2CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB2TXBUF = (uchar)( WriteAddr & 0x00FF );          // ���ʹ��ֽڵ͵�ַ
    while( !( UCB2IFG & UCTXIFG ) );                    // �ȴ��������

	while(NumByteToWrite-- )                            // ѭ���������ݰ� 
	{
		UCB2TXBUF = *pBuffer++;       
		while( !( UCB2IFG & UCTXIFG ) );
	}
	UCB2CTL1 |= UCTXSTP;                                // ����ֹͣλ
	while( UCB2CTL1 & UCTXSTP );                        // �ȴ��������  
    //EEP_WP_ENABLE;                                      // д����ʹ��
}
//-----------IIC2������ֽڣ����128���ֽڣ��Ӻ���------------------------------
void eeprom_PageRead(uchar* pBuffer, uint WriteAddr, uchar NumByteToRead)
{
  return;
    __delay_cycles(100000);
	while( UCB2CTL1 & UCTXSTP );                        // ȷ��ֹͣ����û�в���
    UCB2CTL1 |= UCTR;                                   // дģʽ
	UCB2CTL1 |= UCTXSTT;                                // ��������λ
    UCB2TXBUF = (uchar)( ( WriteAddr & 0xFF00 ) >> 8 ); // ���ʹ��ֽڸߵ�ַ
    while( !( UCB2IFG & UCTXIFG ) );                    // �ȴ�������ɣ�UCB2TXBUFΪ��ʱUCTXIFGλ����1��
    while( UCB2CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
    UCB2TXBUF = (uchar)( WriteAddr & 0x00FF );          // ���ʹ��ֽڵ͵�ַ
    while( !( UCB2IFG & UCTXIFG ) );                    // �ȴ��������
    
    UCB2CTL1 &= ~UCTR;                                  // ��ģʽ
    UCB2CTL1 |= UCTXSTT;                                // ��������λ
    while( UCB2CTL1 & UCTXSTT );                        // �ȴ����豸Ӧ���յ����豸Ӧ��ʱUCTXSTT�����㣩
	NumByteToRead -- ;
	while(NumByteToRead)
	{
        while( !( UCB2IFG & UCRXIFG ) );                // �յ�����
		*pBuffer++ = UCB2RXBUF;                         //��ȡ�ֽ�����
		NumByteToRead--;

	}

	UCB2CTL1 |= UCTXSTP;                                // ����ֹͣλ
    while( UCB2CTL1 & UCTXSTP );                        // �ȴ��������  
    while( !( UCB2IFG & UCRXIFG ) );                    // �յ�����
	*pBuffer = UCB2RXBUF;                               //��ȡ�ֽ�����       
	 
}
