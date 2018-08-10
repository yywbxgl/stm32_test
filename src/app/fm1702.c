#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "fm1702.h"
#include "usart.h"
#include "delay.h"


#define uchar unsigned char

uchar       cardtype;
uchar     	tagtype[2];	        /* ��Ƭ��ʶ�ַ� */

/* FM1702�������� */
uchar     	buf[10];            /* FM1702����ͽ��ջ����� */
uchar     	UID[5];             /* ���к� */
uchar       Secnr;			    /* ������ */

void delay(unsigned int dlength)
{ 
	unsigned int  i;
	unsigned char j;
	for (i=0;i<dlength;i++)
	{
		for (j=0;j<100;j++);
	}
}

//SPI����
unsigned char rev(uchar tem)
{
    uchar i;
    unsigned char var=0,var_snt=0;

    var=0;
    for (i=0;i<8;i++)                     // 8λ�ֽڴ���λ����
    { 
        var_snt=tem&0x80;                   // ѡ���λ
        if (var_snt==0x80) MOSI_H;          // �����λ
        else               MOSI_L;	
        tem<<=1;	
        var<<=1;

        if (READ_MISO==1) 
            var += 1;                        // ����λ����
        SCK_H;                             // ����SCK����	
        delay(1);		
        SCK_L;	 
    }	
    return(var);                         // ����
} 


//SPI����
void Send(unsigned char var) 
{ 
    unsigned char i,tem;

    for (i=0;i<8;i++)         // 8λ�ֽ����
    {
        delay(1);
        tem=var&0x80;         // ѡ���λ
        if (tem==0x80) 
            MOSI_H;            // �����λ
        else
            MOSI_L;
        delay(1);
        SCK_H;                 // ʹSCKΪ1
        var<<=1;               // ����1λ
        delay(1);
        SCK_L;                  // ʹSCKΪ�͵�ƽ
    }
}

//���Ĵ���
uchar read_reg(uchar SpiAddress)
{
	uchar rdata;
	SpiAddress=SpiAddress<<1;
	SpiAddress=SpiAddress | 0x80; 
	NSS_L;	
	Send(SpiAddress);
	rdata=rev(0);
	NSS_H;		
	return(rdata);
}

//д�Ĵ���
void write_reg(uchar SpiAddress,uchar dat)
{
	SpiAddress = SpiAddress << 1;
	SpiAddress = SpiAddress & 0x7f;
	NSS_L; 
	Send(SpiAddress);		
	Send(dat);	
	NSS_H;
}

/****************************************************************/
/*����: Clear_FIFO                                              */
/*����: �ú���ʵ����FFIFO������                                */
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE, FIFO�����                                        */
/*	    FALSE, FIFOδ�����  	                                  */
/****************************************************************/
uchar Clear_FIFO(void)
{
	uchar temp;
	uint  i;
	
	temp =read_reg(Control);				//���FIFO
	temp = (temp | 0x01);
	write_reg(Control, temp);
	for(i = 0; i < RF_TimeOut; i++)			//���FIFO�Ƿ����
	{
		temp = read_reg(FIFO_Length);
		if(temp == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*����: Write_FIFO                                              */
/*����: �ú���ʵ����RC531��FIFO��д��x bytes����                */
/*												       			                          */
/*����:                                                         */
/*      count, ��д���ֽڵĳ���                                 */
/*	    buff, ָ���д�����ݵ�ָ��                              */
/*                                                              */
/*���:                                                         */
/*	    N/A                                                 		*/
/****************************************************************/
void Write_FIFO(uchar count,uchar *buff)
{
	uchar i;
	
	for(i = 0; i < count; i++)
	{
		write_reg(FIFO,*(buff + i));
	}
}

/****************************************************************/
/*����: Read_FIFO                                               */
/*����: �ú���ʵ�ִ�RC531��FIFO�ж���x bytes����                */
/*												        		                          */
/*����:                                                         */
/*       buff, ָ��������ݵ�ָ��                               */
/*                                                              */
/*���:                                                         */
/*	     N/A                                                 		*/
/****************************************************************/
uchar Read_FIFO(uchar *buff)
{
	uchar temp;
	uchar i;
	
	temp =read_reg(FIFO_Length);
	if (temp == 0)
	{
		return 0;
	}
	if (temp >= 24)						//temp=255ʱ,�������ѭ��
	{									        //�������FIFO_LengthԽ���ж�
		temp = 24;						 
	}
	for(i = 0;i < temp; i++)
	{
 		*(buff + i) =read_reg(FIFO);
	}
	return temp;
 }

/****************************************************************/
/*����: Judge_Req                                               */
/*����: �ú���ʵ�ֶԿ�Ƭ��λӦ���źŵ��ж�                      */
/*												       			                          */
/*����:                                                         */
/*       *buff, ָ��Ӧ�����ݵ�ָ��                              */
/*                                                              */
/*���:                                                         */
/*	     TRUE, ��ƬӦ���ź���ȷ                                 */
/*       FALSE, ��ƬӦ���źŴ���                                */
/****************************************************************/
uchar Judge_Req(uchar *buff)
{
	uchar temp1,temp2;
	
	temp1 = *buff;
	temp2 = *(buff + 1);

	if((temp1 != 0x00) && (temp2 == 0x00))
	{
		return TRUE;
	}
	return FALSE;
}

/****************************************************************/
/*����: Check_UID                                               */
/*����: �ú���ʵ�ֶ��յ��Ŀ�Ƭ�����кŵ��ж�                    */
/*����: N/A                                                     */
/*���: TRUE: ���к���ȷ                                        */
/* FALSE: ���кŴ���                                            */
/****************************************************************/
uchar Check_UID(void)
{
	uchar	temp;
	uchar	i;

	temp = 0x00;
	for(i = 0; i < 5; i++)
	{
		temp = temp ^ UID[i];
	}

	if(temp == 0)
	{
		return TRUE;
	}

	return FALSE;
}

/****************************************************************/
/*����: Save_UID                                                */
/*����: �ú���ʵ�ֱ��濨Ƭ�յ������к�                          */
/*����: row: ������ͻ����                                       */
/* col: ������ͻ����                                            */
/* length: �Ӆ�����UID���ݳ���                                  */
/*���: N/A                                                     */
/****************************************************************/
void Save_UID(uchar row, uchar col, uchar length)
{
	uchar	i;
	uchar	temp;
	uchar	temp1;

	if((row == 0x00) && (col == 0x00))
	{
		for(i = 0; i < length; i++)
		{
			UID[i] = buf[i];
		}
	}
	else
	{
		temp = buf[0];
		temp1 = UID[row - 1];
		switch(col)
		{
		case 0:		temp1 = 0x00; row = row + 1; break;
		case 1:		temp = temp & 0xFE; temp1 = temp1 & 0x01; break;
		case 2:		temp = temp & 0xFC; temp1 = temp1 & 0x03; break;
		case 3:		temp = temp & 0xF8; temp1 = temp1 & 0x07; break;
		case 4:		temp = temp & 0xF0; temp1 = temp1 & 0x0F; break;
		case 5:		temp = temp & 0xE0; temp1 = temp1 & 0x1F; break;
		case 6:		temp = temp & 0xC0; temp1 = temp1 & 0x3F; break;
		case 7:		temp = temp & 0x80; temp1 = temp1 & 0x7F; break;
		default:	break;
		}

		buf[0] = temp;
		UID[row - 1] = temp1 | temp;
		for(i = 1; i < length; i++)
		{
			UID[row - 1 + i] = buf[i];
		}
	}
}

/****************************************************************/
/*����: Set_BitFraming                                          */
/*����: �ú������ô��������ݵ��ֽ���                            */
/*����: row: ������ͻ����                                       */
/*      col: ������ͻ����                                       */
/*���: N/A                                                     */
/****************************************************************/
void Set_BitFraming(uchar row, uchar col)
{
	switch(row)
	{
	case 0:		buf[1] = 0x20; break;
	case 1:		buf[1] = 0x30; break;
	case 2:		buf[1] = 0x40; break;
	case 3:		buf[1] = 0x50; break;
	case 4:		buf[1] = 0x60; break;
	default:	break;
	}

	switch(col)
	{
	case 0:		write_reg(0x0F,0x00);  break;
	case 1:		write_reg(0x0F,0x11); buf[1] = (buf[1] | 0x01); break;
	case 2:		write_reg(0x0F,0x22); buf[1] = (buf[1] | 0x02); break;
	case 3:		write_reg(0x0F,0x33); buf[1] = (buf[1] | 0x03); break;
	case 4:		write_reg(0x0F,0x44); buf[1] = (buf[1] | 0x04); break;
	case 5:		write_reg(0x0F,0x55); buf[1] = (buf[1] | 0x05); break;
	case 6:		write_reg(0x0F,0x66); buf[1] = (buf[1] | 0x06); break;
	case 7:		write_reg(0x0F,0x77); buf[1] = (buf[1] | 0x07); break;
	default:	break;
	}
}

/****************************************************************/
/*����: FM1702_Bus_Sel                                          */
/*����: �ú���ʵ�ֶ�FM1702���������߷�ʽ(��������,SPI)ѡ��      */ 
/*												       			                          */
/*����:                                                         */
/*      N/A                                                     */
/*                                                              */
/*���:                                                         */
/*	    TRUE,  ����ѡ��ɹ�                                     */
/*	    FALSE, ����ѡ��ʧ��  	                                  */
/****************************************************************/
uchar FM1702_Bus_Sel(void)
{
	uchar i,temp;
	
	write_reg(Page_Sel,0x80);
	for(i = 0; i < RF_TimeOut; i++)
	{
		temp=read_reg(Command);
		if(temp == 0x00)
		{
			write_reg(Page_Sel,0x00);
			return TRUE;
		}
	}
	return FALSE;
}

//FM1702��ʼ��
void Init_FM1702(void)
{

    uint    i;
    //uchar temp;

    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Enable the GPIO Clock */
    RCC_APB2PeriphClockCmd(MF522_RST_CLK, ENABLE);

    /* Configure the GPIO pin */
    GPIO_InitStructure.GPIO_Pin = MF522_RST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(MF522_RST_PORT, &GPIO_InitStructure);
      
    /* Enable the GPIO Clock */
    RCC_APB2PeriphClockCmd(MF522_MISO_CLK, ENABLE);

    /* Configure the GPIO pin */
    GPIO_InitStructure.GPIO_Pin = MF522_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(MF522_MISO_PORT, &GPIO_InitStructure);

    /* Enable the GPIO Clock */
    RCC_APB2PeriphClockCmd(MF522_MOSI_CLK, ENABLE);

    /* Configure the GPIO pin */
    GPIO_InitStructure.GPIO_Pin = MF522_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(MF522_MOSI_PORT, &GPIO_InitStructure);
      
    /* Enable the GPIO Clock */
    RCC_APB2PeriphClockCmd(MF522_SCK_CLK, ENABLE);

    /* Configure the GPIO pin */
    GPIO_InitStructure.GPIO_Pin = MF522_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(MF522_SCK_PORT, &GPIO_InitStructure);
      
    /* Enable the GPIO Clock */
    RCC_APB2PeriphClockCmd(MF522_NSS_CLK, ENABLE);

    /* Configure the GPIO pin */
    GPIO_InitStructure.GPIO_Pin = MF522_NSS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(MF522_NSS_PORT, &GPIO_InitStructure);


    RST_H;  //��λ
    for (i = 0; i < 0x3fff; i++);
    RST_L;
    for (i = 0; i < 0x3fff; i++);
    SCK_L;	 

    //temp = read_reg(0x05);
    //if(temp == 0x60)
    {
        FM1702_Bus_Sel();		 	//����ѡ��
    }


}

/****************************************************************/
/*����: Command_Send                                            */
/*����: �ú���ʵ����RC531��������Ĺ���                                   */
/*                                                          */
/*����:                                                         */
/*       count, ����������ĳ���                              */
/*	     buff, ָ����������ݵ�ָ��                             */
/*       Comm_Set, ������                                       */
/*                                                            */
/*���:                                                         */
/*	     TRUE, �����ȷִ��                                   */
/*	     FALSE, ����ִ�д���  	                                */
/****************************************************************/
uchar Command_Send(uchar count,uchar * buff,uchar Comm_Set)
{
	uint  j;
	uchar temp;
	
	write_reg(Command, 0x00);
	Clear_FIFO();
  	if (count != 0)
  	{
	  	Write_FIFO(count, buff);
  	}	 
	temp = read_reg(FIFO_Length);
	
	write_reg(Command, Comm_Set);				//����ִ��
	
	for(j = 0; j< RF_TimeOut; j++)				//�������ִ�з�
	{
		temp =read_reg(Command);
		if(temp == 0x00)  
		{
			return TRUE;
		}
	}
	return FALSE;	
}

/****************************************************************/
/*����: MIF_Halt                                                */
/*����: �ú���ʵ����ͣMIFARE��                                  */
/*����: N/A                                                     */
/*���: FM1702_OK: Ӧ����ȷ                                     */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_NOTAGERR: �޿�                                        */
/****************************************************************/
/*
uchar MIF_Halt(void)
{
	uchar	temp;
	uint	i;

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x22,0x07);
	*buf = RF_CMD_HALT;
	*(buf + 1) = 0x00;
	temp = Command_Send(2, buf, Transmit);
	if(temp == TRUE)
	{
		for(i = 0; i < 0x50; i++)
		{
			_nop_();
		}

		return FM1702_OK;
	}
	else
	{
		temp = read_reg(0x0A);
		if((temp & 0x02) == 0x02)
		{
			return(FM1702_PARITYERR);
		}

		if((temp & 0x04) == 0x04)
		{
			return(FM1702_FRAMINGERR);
		}

		return(FM1702_NOTAGERR);
	}
}
*/
///////////////////////////////////////////////////////////////////////
// ת����Կ��ʽ
///////////////////////////////////////////////////////////////////////
char M500HostCodeKey(unsigned char *uncoded, unsigned char *coded)   
{
    unsigned char cnt = 0;
    unsigned char ln  = 0;     
    unsigned char hn  = 0;      
    
    for (cnt = 0; cnt < 6; cnt++)
    {
        ln = uncoded[cnt] & 0x0F;
        hn = uncoded[cnt] >> 4;
        coded[cnt * 2 + 1] = (~ln << 4) | ln;
        coded[cnt * 2 ] = (~hn << 4) | hn;
    }
    return FM1702_OK;
}

/****************************************************************/
/*����: Load_keyE2                                              */
/*����: �ú���ʵ�ְ�E2���������FM1702��keybuf��             */
/*����: Secnr: EE��ʼ��ַ                                       */
/*���: True: ��Կװ�سɹ�                                      */
/* False: ��Կװ��ʧ��                                          */
/****************************************************************/
uchar Load_keyE2_CPY(uchar *uncoded_keys)
{
  uchar temp;
  uchar coded_keys[13];
    
    M500HostCodeKey(uncoded_keys, coded_keys);
	temp = Command_Send(12, coded_keys, LoadKey);
	temp = read_reg(0x0A) & 0x40;
	if (temp == 0x40)
	{
		return FALSE;
	}
	return TRUE;
}

/****************************************************************/
/*����: Request                                                 */
/*����: �ú���ʵ�ֶԷ���RC531������Χ֮�ڵĿ�Ƭ��Request����    */ 
/*												       			                          */
/*����:                                                         */
/*      mode: ALL(�������RC531������Χ֮�ڵĿ�Ƭ)			   	    */
/*	    STD(�����RC531������Χ֮�ڴ���HALT״̬�Ŀ�Ƭ)          */
/*                                                              */
/*���:                                                         */
/*    FM222_NOTAGERR: �޿�                                     */
/*      FM222_OK: Ӧ����ȷ                                        */
/*    FM222_REQERR: Ӧ�����	                                  */
/****************************************************************/
uchar Request(uchar mode)
{
    uchar  temp;

    write_reg(TxControl,0x58);
    delay(1);
    write_reg(TxControl,0x5b);		 
    write_reg(CRCPresetLSB,0x63);
    write_reg(CWConductance,0x3f);
    buf[0] = mode;                          //Requestģʽѡ��
    write_reg(Bit_Frame,0x07);              //����7bit
    write_reg(ChannelRedundancy,0x03);      //�ر�CRC
    write_reg(TxControl,0x5b); 
    write_reg(Control,0x01);          		 //����CRYPTO1λ
    temp = Command_Send(1, buf, Transceive);
    if(temp == FALSE)
    {
        return FM1702_NOTAGERR;
    }

    Read_FIFO(buf);					         //��FIFO�ж�ȡӦ����Ϣ
    temp = Judge_Req(buf);			         //�ж�Ӧ���ź��Ƿ���ȷ
    if (temp == TRUE)
    {
        tagtype[0] = buf[0];
        tagtype[1] = buf[1];
        return FM1702_OK;
    }
    return FM1702_REQERR;
}

/****************************************************************/
/*����: AntiColl                                                */
/*����: �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ�ķ���ͻ���    */
/*����: N/A                                                     */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_SERNRERR: ��Ƭ���к�Ӧ�����                          */
/* FM1702_OK: ��ƬӦ����ȷ                                      */
/****************************************************************/
uchar AntiColl(void)
{
	uchar	temp;
	uchar	i;
	uchar	row, col;
	uchar	pre_row;

	row = 0;
	col = 0;
	pre_row = 0;
	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x13,0x3f);
	buf[0] = RF_CMD_ANTICOL;
	buf[1] = 0x20;
	write_reg(0x22,0x03);	                   // �ر�CRC,����żУ��
	temp = Command_Send(2, buf, Transceive);
	while(1)
	{
		if(temp == FALSE)
		{
			return(FM1702_NOTAGERR);
		}

		temp = read_reg(0x04);
		if(temp == 0)
		{
			return FM1702_BYTECOUNTERR;
		}

		Read_FIFO(buf);
		Save_UID(row, col, temp);			        // ���յ���UID����UID������
	
		temp = read_reg(0x0A);				        // �жϽӅ������Ƿ����
		temp = temp & 0x01;
		if(temp == 0x00)
		{
			temp = Check_UID();			            // У���յ���UID
			if(temp == FALSE)
			{
				return(FM1702_SERNRERR);
			}

			return(FM1702_OK);
		}
		else
		{
			temp = read_reg(0x0B);             // ��ȡ��ͻ���Ĵ��� 
			row = temp / 8;
			col = temp % 8;
			buf[0] = RF_CMD_ANTICOL;
			Set_BitFraming(row + pre_row, col);	// ���ô��������ݵ��ֽ��� 
			pre_row = pre_row + row;
			for(i = 0; i < pre_row + 1; i++)
			{
				buf[i + 2] = UID[i];
			}

			if(col != 0x00)
			{
				row = pre_row + 1;
			}
			else
			{
				row = pre_row;
			}
			temp = Command_Send(row + 2, buf, Transceive);
		}
	}
}

/****************************************************************/
/*����: Select_Card                                             */
/*����: �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵ�ĳ�ſ�Ƭ����ѡ��    */
/*����: N/A                                                     */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_OK: Ӧ����ȷ                                          */
/* FM1702_SELERR: ѡ������                                      */
/****************************************************************/
uchar Select_Card(void)
{
	uchar	temp, i;

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	buf[0] = RF_CMD_SELECT;
	buf[1] = 0x70;
	for(i = 0; i < 5; i++)
	{
		buf[i + 2] = UID[i];
	}

	write_reg(0x22,0x0f);	                       // ����CRC,��żУ��У�� 
	temp = Command_Send(7, buf, Transceive);
	if(temp == FALSE)
	{
		return(FM1702_NOTAGERR);
	}
	else
	{
		temp = read_reg(0x0A);
		if((temp & 0x02) == 0x02) return(FM1702_PARITYERR);
		if((temp & 0x04) == 0x04) return(FM1702_FRAMINGERR);
		if((temp & 0x08) == 0x08) return(FM1702_CRCERR);
		temp = read_reg(0x04);
		if(temp != 1) return(FM1702_BYTECOUNTERR);
		Read_FIFO(buf);	                      // ��FIFO�ж�ȡӦ����Ϣ 
		temp = *buf;
		//�ж�Ӧ���ź��Ƿ���ȷ 
		if((temp == 0x08) || (temp == 0x88) || (temp == 0x53) ||(temp == 0x18)) //S70 temp = 0x18	
			return(FM1702_OK);
		else
			return(FM1702_SELERR);
	}
}

/****************************************************************/
/*����: Authentication                                          */
/*����: �ú���ʵ��������֤�Ĺ���                                */
/*����: UID: ��Ƭ���кŵ�ַ                                     */
/* SecNR: ������                                                */
/* mode: ģʽ                                                   */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_OK: Ӧ����ȷ                                          */
/* FM1702_AUTHERR: Ȩ����֤�д�                                 */
/****************************************************************/
uchar Authentication(uchar *UID, uchar SecNR, uchar mode)
{
	uchar	i;
	uchar	temp, temp1;

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x13,0x3f);
	temp1 = read_reg(0x09);
	temp1 = temp1 & 0xf7;
	write_reg(0x09,temp1);
	if(mode == RF_CMD_AUTH_LB)			            // AUTHENT1 
		buf[0] = RF_CMD_AUTH_LB;
	else
		buf[0] = RF_CMD_AUTH_LA;
	buf[1] = SecNR * 4 + 3;
	for(i = 0; i < 4; i++)
	{
		buf[2 + i] = UID[i];
	}

	write_reg(0x22,0x0f);	                     // ����CRC,��żУ��У�� 
	temp = Command_Send(6, buf, Authent1);
	if(temp == FALSE)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = Command_Send(0, buf, Authent2);	 // AUTHENT2 
	if(temp == FALSE)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp1 = read_reg(0x09);
	temp1 = temp1 & 0x08;	                     // Crypto1on=1��֤ͨ�� 
	if(temp1 == 0x08)
	{
		return FM1702_OK;
	}

	return FM1702_AUTHERR;
}

/****************************************************************/
/*����: MIF_Read                                                */
/*����: �ú���ʵ�ֶ�MIFARE�������ֵ                            */
/*����: buff: �������׵�ַ                                      */
/* Block_Adr: ���ַ                                            */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_OK: Ӧ����ȷ                                          */
/****************************************************************/
uchar MIF_READ(uchar *buff, uchar Block_Adr)
{
	uchar	temp;

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
	write_reg(0x13,0x3f);
	write_reg(0x22,0x0f);

	buff[0] = RF_CMD_READ;
	buff[1] = Block_Adr;
	temp = Command_Send(2, buff, Transceive);
	if(temp == 0)
	{
		return FM1702_NOTAGERR;
	}

	temp = read_reg(0x0A);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = read_reg(0x04);
	if(temp == 0x10)	                      // 8K�������ݳ���Ϊ16
	{
		Read_FIFO(buff);
		return FM1702_OK;
	}
	else if(temp == 0x04)	                  // Token�������ݳ���Ϊ16
	{
		Read_FIFO(buff);
		return FM1702_OK;
	}
	else
	{
		return FM1702_BYTECOUNTERR;
	}
}

/****************************************************************/
/*����: MIF_Write                                               */
/*����: �ú���ʵ��дMIFARE�������ֵ                            */
/*����: buff: �������׵�ַ                                      */
/* Block_Adr: ���ַ                                            */
/*���: FM1702_NOTAGERR: �޿�                                   */
/* FM1702_BYTECOUNTERR: �����ֽڴ���                            */
/* FM1702_NOTAUTHERR: δ��Ȩ����֤                              */
/* FM1702_EMPTY: �����������                                   */
/* FM1702_CRCERR: CRCУ���                                     */
/* FM1702_PARITYERR: ��żУ���                                 */
/* FM1702_WRITEERR: д�������ݳ���                              */
/* FM1702_OK: Ӧ����ȷ                                          */
/****************************************************************/
uchar MIF_Write(uchar *buff, uchar Block_Adr)
{
	uchar	temp;
	uchar	F_buff[2];

	write_reg(0x23,0x63);
	write_reg(0x12,0x3f);
//	F_buff = temp1 + 0x10;
	write_reg(0x22,0x07);
	F_buff[0] = RF_CMD_WRITE;
	F_buff[1] = Block_Adr;
	temp = Command_Send(2, F_buff, Transceive);
	if(temp == FALSE)
	{
		return(FM1702_NOTAGERR);
	}

	temp = read_reg(0x04);
	if(temp == 0)
	{
		return(FM1702_BYTECOUNTERR);
	}

	Read_FIFO(F_buff);
	temp = *F_buff;
	switch(temp)
	{
		case 0x00:	return(FM1702_NOTAUTHERR);	     // ��ʱ���ε�д����
		case 0x04:	return(FM1702_EMPTY);
		case 0x0a:	break;
		case 0x01:	return(FM1702_CRCERR);
		case 0x05:	return(FM1702_PARITYERR);
		default:	return(FM1702_WRITEERR);
	}

	temp = Command_Send(16, buff, Transceive);
	if(temp == TRUE)
	{
		return(FM1702_OK);
	}
	else
	{
		temp = read_reg(0x0A);
		if((temp & 0x02) == 0x02)
			return(FM1702_PARITYERR);
		else if((temp & 0x04) == 0x04)
			return(FM1702_FRAMINGERR);
		else if((temp & 0x08) == 0x08)
			return(FM1702_CRCERR);
		else
			return(FM1702_WRITEERR);
	}
}

 


