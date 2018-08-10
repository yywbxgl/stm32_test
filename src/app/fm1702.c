#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "fm1702.h"
#include "usart.h"
#include "delay.h"


#define uchar unsigned char

uchar       cardtype;
uchar     	tagtype[2];	        /* 卡片标识字符 */

/* FM1702变量定义 */
uchar     	buf[10];            /* FM1702命令发送接收缓冲区 */
uchar     	UID[5];             /* 序列号 */
uchar       Secnr;			    /* 扇区号 */

void delay(unsigned int dlength)
{ 
	unsigned int  i;
	unsigned char j;
	for (i=0;i<dlength;i++)
	{
		for (j=0;j<100;j++);
	}
}

//SPI接收
unsigned char rev(uchar tem)
{
    uchar i;
    unsigned char var=0,var_snt=0;

    var=0;
    for (i=0;i<8;i++)                     // 8位字节带进位左移
    { 
        var_snt=tem&0x80;                   // 选择高位
        if (var_snt==0x80) MOSI_H;          // 输出高位
        else               MOSI_L;	
        tem<<=1;	
        var<<=1;

        if (READ_MISO==1) 
            var += 1;                        // 带进位左移
        SCK_H;                             // 产生SCK脉冲	
        delay(1);		
        SCK_L;	 
    }	
    return(var);                         // 返回
} 


//SPI发送
void Send(unsigned char var) 
{ 
    unsigned char i,tem;

    for (i=0;i<8;i++)         // 8位字节输出
    {
        delay(1);
        tem=var&0x80;         // 选择高位
        if (tem==0x80) 
            MOSI_H;            // 输出高位
        else
            MOSI_L;
        delay(1);
        SCK_H;                 // 使SCK为1
        var<<=1;               // 左移1位
        delay(1);
        SCK_L;                  // 使SCK为低电平
    }
}

//读寄存器
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

//写寄存器
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
/*名称: Clear_FIFO                                              */
/*功能: 该函数实现清縁FIFO的数据                                */
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出:                                                         */
/*	    TRUE, FIFO被清空                                        */
/*	    FALSE, FIFO未被清空  	                                  */
/****************************************************************/
uchar Clear_FIFO(void)
{
	uchar temp;
	uint  i;
	
	temp =read_reg(Control);				//清空FIFO
	temp = (temp | 0x01);
	write_reg(Control, temp);
	for(i = 0; i < RF_TimeOut; i++)			//检查FIFO是否被清空
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
/*名称: Write_FIFO                                              */
/*功能: 该函数实现向RC531的FIFO中写入x bytes数据                */
/*												       			                          */
/*输入:                                                         */
/*      count, 待写入字节的长度                                 */
/*	    buff, 指向待写入数据的指针                              */
/*                                                              */
/*输出:                                                         */
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
/*名称: Read_FIFO                                               */
/*功能: 该函数实现从RC531的FIFO中读出x bytes数据                */
/*												        		                          */
/*输入:                                                         */
/*       buff, 指向读出数据的指针                               */
/*                                                              */
/*输出:                                                         */
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
	if (temp >= 24)						//temp=255时,会进入死循环
	{									        //因此增加FIFO_Length越限判断
		temp = 24;						 
	}
	for(i = 0;i < temp; i++)
	{
 		*(buff + i) =read_reg(FIFO);
	}
	return temp;
 }

/****************************************************************/
/*名称: Judge_Req                                               */
/*功能: 该函数实现对卡片复位应答信号的判断                      */
/*												       			                          */
/*输入:                                                         */
/*       *buff, 指向应答数据的指针                              */
/*                                                              */
/*输出:                                                         */
/*	     TRUE, 卡片应答信号正确                                 */
/*       FALSE, 卡片应答信号错误                                */
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
/*名称: Check_UID                                               */
/*功能: 该函数实现对收到的卡片的序列号的判断                    */
/*输入: N/A                                                     */
/*输出: TRUE: 序列号正确                                        */
/* FALSE: 序列号错误                                            */
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
/*名称: Save_UID                                                */
/*功能: 该函数实现保存卡片收到的序列号                          */
/*输入: row: 产生冲突的行                                       */
/* col: 产生冲突的列                                            */
/* length: 接収到的UID数据长度                                  */
/*输出: N/A                                                     */
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
/*名称: Set_BitFraming                                          */
/*功能: 该函数设置待发送数据的字节数                            */
/*输入: row: 产生冲突的行                                       */
/*      col: 产生冲突的列                                       */
/*输出: N/A                                                     */
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
/*名称: FM1702_Bus_Sel                                          */
/*功能: 该函数实现对FM1702操作的总线方式(并行总线,SPI)选择      */ 
/*												       			                          */
/*输入:                                                         */
/*      N/A                                                     */
/*                                                              */
/*输出:                                                         */
/*	    TRUE,  总线选择成功                                     */
/*	    FALSE, 总线选择失败  	                                  */
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

//FM1702初始化
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


    RST_H;  //复位
    for (i = 0; i < 0x3fff; i++);
    RST_L;
    for (i = 0; i < 0x3fff; i++);
    SCK_L;	 

    //temp = read_reg(0x05);
    //if(temp == 0x60)
    {
        FM1702_Bus_Sel();		 	//总线选择
    }


}

/****************************************************************/
/*名称: Command_Send                                            */
/*功能: 该函数实现向RC531发送命令集的功能                                   */
/*                                                          */
/*输入:                                                         */
/*       count, 待发送命令集的长度                              */
/*	     buff, 指向待发送数据的指针                             */
/*       Comm_Set, 命令码                                       */
/*                                                            */
/*输出:                                                         */
/*	     TRUE, 命令被正确执行                                   */
/*	     FALSE, 命令执行错误  	                                */
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
	
	write_reg(Command, Comm_Set);				//命令执行
	
	for(j = 0; j< RF_TimeOut; j++)				//检查命令执行否
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
/*名称: MIF_Halt                                                */
/*功能: 该函数实现暂停MIFARE卡                                  */
/*输入: N/A                                                     */
/*输出: FM1702_OK: 应答正确                                     */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_NOTAGERR: 无卡                                        */
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
// 转换密钥格式
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
/*名称: Load_keyE2                                              */
/*功能: 该函数实现把E2中密码存入FM1702的keybuf中             */
/*输入: Secnr: EE起始地址                                       */
/*输出: True: 密钥装载成功                                      */
/* False: 密钥装载失败                                          */
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
/*名称: Request                                                 */
/*功能: 该函数实现对放入RC531操作范围之内的卡片的Request操作    */ 
/*												       			                          */
/*输入:                                                         */
/*      mode: ALL(监测所以RC531操作范围之内的卡片)			   	    */
/*	    STD(监测在RC531操作范围之内处于HALT状态的卡片)          */
/*                                                              */
/*输出:                                                         */
/*    FM222_NOTAGERR: 无卡                                     */
/*      FM222_OK: 应答正确                                        */
/*    FM222_REQERR: 应答错误	                                  */
/****************************************************************/
uchar Request(uchar mode)
{
    uchar  temp;

    write_reg(TxControl,0x58);
    delay(1);
    write_reg(TxControl,0x5b);		 
    write_reg(CRCPresetLSB,0x63);
    write_reg(CWConductance,0x3f);
    buf[0] = mode;                          //Request模式选择
    write_reg(Bit_Frame,0x07);              //发送7bit
    write_reg(ChannelRedundancy,0x03);      //关闭CRC
    write_reg(TxControl,0x5b); 
    write_reg(Control,0x01);          		 //屏蔽CRYPTO1位
    temp = Command_Send(1, buf, Transceive);
    if(temp == FALSE)
    {
        return FM1702_NOTAGERR;
    }

    Read_FIFO(buf);					         //从FIFO中读取应答信息
    temp = Judge_Req(buf);			         //判断应答信号是否正确
    if (temp == TRUE)
    {
        tagtype[0] = buf[0];
        tagtype[1] = buf[1];
        return FM1702_OK;
    }
    return FM1702_REQERR;
}

/****************************************************************/
/*名称: AntiColl                                                */
/*功能: 该函数实现对放入FM1702操作范围之内的卡片的防冲突检测    */
/*输入: N/A                                                     */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_SERNRERR: 卡片序列号应答错误                          */
/* FM1702_OK: 卡片应答正确                                      */
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
	write_reg(0x22,0x03);	                   // 关闭CRC,打开奇偶校验
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
		Save_UID(row, col, temp);			        // 将收到的UID放入UID数组中
	
		temp = read_reg(0x0A);				        // 判断接収数据是否出错
		temp = temp & 0x01;
		if(temp == 0x00)
		{
			temp = Check_UID();			            // 校验收到的UID
			if(temp == FALSE)
			{
				return(FM1702_SERNRERR);
			}

			return(FM1702_OK);
		}
		else
		{
			temp = read_reg(0x0B);             // 读取冲突检测寄存器 
			row = temp / 8;
			col = temp % 8;
			buf[0] = RF_CMD_ANTICOL;
			Set_BitFraming(row + pre_row, col);	// 设置待发送数据的字节数 
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
/*名称: Select_Card                                             */
/*功能: 该函数实现对放入FM1702操作范围之内的某张卡片进行选择    */
/*输入: N/A                                                     */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_OK: 应答正确                                          */
/* FM1702_SELERR: 选卡出错                                      */
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

	write_reg(0x22,0x0f);	                       // 开启CRC,奇偶校验校验 
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
		Read_FIFO(buf);	                      // 从FIFO中读取应答信息 
		temp = *buf;
		//判断应答信号是否正确 
		if((temp == 0x08) || (temp == 0x88) || (temp == 0x53) ||(temp == 0x18)) //S70 temp = 0x18	
			return(FM1702_OK);
		else
			return(FM1702_SELERR);
	}
}

/****************************************************************/
/*名称: Authentication                                          */
/*功能: 该函数实现密码认证的过程                                */
/*输入: UID: 卡片序列号地址                                     */
/* SecNR: 扇区号                                                */
/* mode: 模式                                                   */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_OK: 应答正确                                          */
/* FM1702_AUTHERR: 权威认证有错                                 */
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

	write_reg(0x22,0x0f);	                     // 开启CRC,奇偶校验校验 
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
	temp1 = temp1 & 0x08;	                     // Crypto1on=1验证通过 
	if(temp1 == 0x08)
	{
		return FM1702_OK;
	}

	return FM1702_AUTHERR;
}

/****************************************************************/
/*名称: MIF_Read                                                */
/*功能: 该函数实现读MIFARE卡块的数值                            */
/*输入: buff: 缓冲区首地址                                      */
/* Block_Adr: 块地址                                            */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_OK: 应答正确                                          */
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
	if(temp == 0x10)	                      // 8K卡读数据长度为16
	{
		Read_FIFO(buff);
		return FM1702_OK;
	}
	else if(temp == 0x04)	                  // Token卡读数据长度为16
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
/*名称: MIF_Write                                               */
/*功能: 该函数实现写MIFARE卡块的数值                            */
/*输入: buff: 缓冲区首地址                                      */
/* Block_Adr: 块地址                                            */
/*输出: FM1702_NOTAGERR: 无卡                                   */
/* FM1702_BYTECOUNTERR: 接收字节错误                            */
/* FM1702_NOTAUTHERR: 未经权威认证                              */
/* FM1702_EMPTY: 数据溢出错误                                   */
/* FM1702_CRCERR: CRC校验错                                     */
/* FM1702_PARITYERR: 奇偶校验错                                 */
/* FM1702_WRITEERR: 写卡块数据出错                              */
/* FM1702_OK: 应答正确                                          */
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
		case 0x00:	return(FM1702_NOTAUTHERR);	     // 暂时屏蔽掉写错误
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

 


