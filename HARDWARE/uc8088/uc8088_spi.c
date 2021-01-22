#include "uc8088_spi.h"
#include "spi.h"
#include "delay.h"


void uc8088_init(void)
{
	int i ;
	char wr_buf4[2] = {0x11, 0x1F};	
//	TracealyzerCommandType config_st;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ�� 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 ���� 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	
	SPI2_CS = 1;
	SPI2_Init();
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);			//Ƶ�ʹ��߶Ű��߿����ܲ���
	
	SPI2_CS = 0;
	for (i =0; i<2; i++)
	{
		SPI2_ReadWriteByte(wr_buf4[i]);
		delay_us(10);
	}
	SPI2_CS = 1;
	delay_us(20);
//flag_config:
//	config_st.cmd_type = 0x03;
//	memset(config_st.cmd_data, 0xff, sizeof(config_st.cmd_data));
//	uc8088_write_memory(config_struct_Addr, (u8*)&config_st, 4);
//	if (uc8088_read_u32(config_struct_Addr) != 0x03ffffff){
//		PEout(6) = PEout(5);
//		PEout(5) = ~PEout(5);
//		goto flag_config;
//	}
//	
//	uc8088_write_u32(config_enable_Addr, (uc8088_read_u32(config_enable_Addr)&(0x01ffffff))|(0x01<<24));
	
}


/*****************************************************************
 *	�������ܣ���ȡNumByteToRead���ֽڵ�pBuffer
 *	Addr �����4���������� ������ǣ�uc8088Ӳ������ǰȡ��4���������ĵ�ַ
 *	NumByteToRead �����4��������, �������, �������ᶪ����������ֽ�
******************************************************************/
u16 uc8088_read_memory(const u32 Addr, register u8* pBuffer, u16 NumByteToRead)
{
	u8 temp, temp2;
	register u16 i;
	
	temp = Addr % 4;
	if (NumByteToRead < 4)
		return 0;
	else
		temp2 = (NumByteToRead - (4 - temp)) % 4;
		
	SPI2_CS = 0;
	SPI2_ReadWriteByte(READ_CMD);							//��������
	SPI2_ReadWriteByte((u8)((Addr) >> 24));  	//����32bit��ַ    
	SPI2_ReadWriteByte((u8)((Addr) >> 16)); 
  SPI2_ReadWriteByte((u8)((Addr) >> 8));   
  SPI2_ReadWriteByte((u8)Addr); 
	
	for(i=0; i<4 + temp; i++)									// ����demo �� ����ǰ����ֽ�
		SPI2_ReadWriteByte(0xFF);
	
	for(i=0; i<NumByteToRead - temp2; i++)						//ѭ������
			*pBuffer++ = SPI2_ReadWriteByte(0XFF);
	
	SPI2_CS = 1;
	delay_us(10);
	return i;
}

void uc8088_write_memory(const u32 Addr, u8* pBuffer, u16 NumByteToRead)
{
	u16 i;
	if (NumByteToRead < 1)
		return;
	
	SPI2_CS = 0;
	
	SPI2_ReadWriteByte(WRITE_CMD);							//��д����
	SPI2_ReadWriteByte((u8)((Addr) >> 24));  	//����32bit��ַ    
	SPI2_ReadWriteByte((u8)((Addr) >> 16)); 
  SPI2_ReadWriteByte((u8)((Addr) >> 8));   
  SPI2_ReadWriteByte((u8)Addr); 
	
	for(i=0; i<NumByteToRead; i++)
			SPI2_ReadWriteByte(pBuffer[i]);   	//ѭ��д
	
	SPI2_CS = 1;
	delay_us(10);
}



////�������ܣ�дһ��u8�������ݵ���ַaddr
//void uc8088_write_u8(const u32 addr, u8 wdata)
//{
//	int i;
//	u8 wr_buf[6] = {0};
//	
//	wr_buf[0] = WRITE_CMD;
//	wr_buf[1] = addr >> 24;
//	wr_buf[2] = addr >> 16;
//	wr_buf[3] = addr >> 8;
//	wr_buf[4] = addr;
//	
//	wr_buf[5] = wdata;
//	
//	SPI2_CS = 0;
//	for(i=0; i<6; i++)
//		SPI2_ReadWriteByte(wr_buf[i]);
//	
//	SPI2_CS = 1;
//	delay_us(20);
//}

////�������ܣ�дһ��u16�������ݵ���ַaddr
//void uc8088_write_u16(const u32 addr, u16 wdata)
//{
//	int i;
//	u8 wr_buf[7] = {0};
//	
//	wr_buf[0] = WRITE_CMD;
//	wr_buf[1] = addr >> 24;
//	wr_buf[2] = addr >> 16;
//	wr_buf[3] = addr >> 8;
//	wr_buf[4] = addr;
//	
//	wr_buf[5] = wdata>>8;
//	wr_buf[6] = wdata;
//	
//	SPI2_CS = 0;
//	for(i=0; i<7; i++)
//		SPI2_ReadWriteByte(wr_buf[i]);
//	SPI2_ReadWriteByte(0xFF);
//	SPI2_ReadWriteByte(0xFF);				// ��֤��4���ֽڶ�д����
//	
//	
//	SPI2_CS = 1;
//	delay_us(20);
//}

//�������ܣ�дһ��u32�������ݵ���ַaddr
void uc8088_write_u32(const u32 addr, u32 wdata)
{
	register int i;
	u8 wr_buf[9] = {0};
	u8 *ptrWr = wr_buf;
	
	wr_buf[0] = WRITE_CMD;
	wr_buf[1] = addr >> 24;
	wr_buf[2] = addr >> 16;
	wr_buf[3] = addr >> 8;
	wr_buf[4] = addr;
	
	wr_buf[5] = wdata >> 24;
	wr_buf[6] = wdata >> 16;
	wr_buf[7] = wdata >> 8;
	wr_buf[8] = wdata;
	
	SPI2_CS = 0;
	for(i=0; i<9; i++)
		SPI2_ReadWriteByte(*ptrWr++);
	
	SPI2_CS = 1;
	delay_us(20);
}

//�������ܣ���һ��u8��������
u8 uc8088_read_u8(const u32 addr)
{
	int i, temp;
	u8 wr_buf[5] = {0};
	u8 read_data = 0;
	
	temp = addr % 4;
	
	wr_buf[0] = READ_CMD;
	wr_buf[1] = addr >> 24;
	wr_buf[2] = addr >> 16;
	wr_buf[3] = addr >> 8;
	wr_buf[4] = addr;
	
	SPI2_CS = 0;
	for(i=0; i<5; i++)								// �������� �� ��ַ
		SPI2_ReadWriteByte(wr_buf[i]);
	
	for(i=0; i<4 + temp; i++)					// ����demo �� ����ǰ����ֽ�
		SPI2_ReadWriteByte(0xFF);

	read_data = SPI2_ReadWriteByte(0xFF);
	SPI2_CS = 1;
	delay_us(20);
	
	return read_data;
}

//�������ܣ���һ��u16��������
u16 uc8088_read_u16(const u32 addr)
{
	int i, temp;
	u8 wr_buf[5] = {0};
	u8 read_buf[2] = {0};
	u16 read_data = 0xffff;
	
	temp = addr % 4;

	wr_buf[0] = READ_CMD;
	wr_buf[1] = addr >> 24;
	wr_buf[2] = addr >> 16;
	wr_buf[3] = addr >> 8;
	wr_buf[4] = addr;
	
	SPI2_CS = 0;
	for(i=0; i<5; i++)								// �������� �� ��ַ
		SPI2_ReadWriteByte(wr_buf[i]);
	
	for(i=0; i<4 + temp; i++)					// ����demo �� ����ǰ����ֽ�
		SPI2_ReadWriteByte(0xFF);
	
	read_buf[0] = SPI2_ReadWriteByte(0xFF);
	read_buf[1] = SPI2_ReadWriteByte(0xFF);

	
	SPI2_CS = 1;
	delay_us(20);
	read_data = 	 read_buf[1] 
							| (read_buf[0] << 8);
	
	return read_data;
}

//�������ܣ���һ��u32��������
u32 uc8088_read_u32(const u32 addr)
{
	register int i;
	u8 wr_buf[5] = {0};
	u8 read_buf[4] = {0};
	u8 *ptrRd = read_buf, *ptrWr = wr_buf;
	u32 read_data = 0;
	 
	wr_buf[0] = READ_CMD;
	wr_buf[1] = addr >> 24;
	wr_buf[2] = addr >> 16;
	wr_buf[3] = addr >> 8;
	wr_buf[4] = addr;
	
	SPI2_CS = 0;
	for(i=0; i<5; i++)
		SPI2_ReadWriteByte(*ptrWr++);
	
	for(i=0; i<4; i++)					// ����demo
		SPI2_ReadWriteByte(0xFF);
	for(i=0; i<4; i++)
		*ptrRd++ = SPI2_ReadWriteByte(0xFF);
		
	SPI2_CS = 1;
	delay_us(20);
	read_data = 	 read_buf[3] 
							| (read_buf[2] << 8)
							| (read_buf[1] << 16) 
							| (read_buf[0] << 24);
	
	return read_data;
}

//�������ܣ�������u32��������
void uc8088_read_2_u32(const u32 addr, u32* r1, u32* r2)
{
	register int i;
	u8 wr_buf[5] = {0};
	u8 read_buf[8] = {0};
	u8 *ptrRd = read_buf, *ptrWr = wr_buf;
	
	 
	wr_buf[0] = READ_CMD;
	wr_buf[1] = addr >> 24;
	wr_buf[2] = addr >> 16;
	wr_buf[3] = addr >> 8;
	wr_buf[4] = addr;
	
	SPI2_CS = 0;
	for(i=0; i<5; i++)
		SPI2_ReadWriteByte(*ptrWr++);
	
	for(i=0; i<4; i++)					// ����demo
		SPI2_ReadWriteByte(0xFF);
	for(i=0; i<8; i++)
		*ptrRd++ = SPI2_ReadWriteByte(0xFF);
		
	SPI2_CS = 1;
	delay_us(20);
	*r2 = 	 read_buf[7] 
				| (read_buf[6] << 8)
				| (read_buf[5] << 16) 
				| (read_buf[4] << 24);
							
	*r1 =		read_buf[3]
				| (read_buf[2] << 8)
				| (read_buf[1] << 16) 
				| (read_buf[0] << 24);
}
