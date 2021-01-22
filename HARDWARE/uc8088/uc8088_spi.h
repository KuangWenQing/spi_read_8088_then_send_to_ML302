#ifndef __UC8088_SPI_H__
#define __UC8088_SPI_H__

#include "stdlib.h"
#include "sys.h"
#define SPI2_CS 				 PBout(12)

#define READ_CMD			0x0B	//������
#define WRITE_CMD			0x02	//д����

#define Buf_addr			(0x33ac00)
#define SPI_BUF_LEN		(4096)
//typedef struct
//{
//	u32 sWrite;
//	u32 sRead;
//	u8 ucBuf[SPI_BUF_LEN];
//}STU_SPI_QUEUE;


//typedef struct
//{
//  u8  cmd_type;
//  u8  cmd_data[32];
//}TracealyzerCommandType;

//#define iRescvBufBaseAddr  (0x0033b490)	//this is the base address in spi device, it must be changed accordingly
//#define config_enable_Addr  (iRescvBufBaseAddr + 16)
//#define config_struct_Addr  (iRescvBufBaseAddr + 40)

//#define buffer_head1_Addr  (iRescvBufBaseAddr + 104)
//#define buffer_head2_Addr  (iRescvBufBaseAddr + 116)

//#define buffer1_Addr  (iRescvBufBaseAddr + 132)
//#define buffer2_Addr  (iRescvBufBaseAddr + 132 + 2560)


extern void* memset ( void * ptr, int value, size_t num );
void uc8088_init(void);

u16 uc8088_read_memory(const u32 Addr, register u8* pBuffer, u16 NumByteToRead);	//��Num��Byte
void uc8088_write_memory(const u32 Addr, u8* pBuffer, u16 NumByteToRead);	//дnum���ֽ�

void uc8088_write_u8(const u32 addr, u8 wdata);			//дһ���ֽ�
void uc8088_write_u16(const u32 addr, u16 wdata);		//дһ��u16���ݣ�2���ֽڣ�
void uc8088_write_u32(const u32 addr, u32 wdata);		//дһ��u32���ݣ�4���ֽڣ�
u8 uc8088_read_u8(const u32 addr);									//��һ��u8����
u16 uc8088_read_u16(const u32 addr);								//��һ��u16����
u32 uc8088_read_u32(const u32 addr);								//��һ��u32����
void uc8088_read_2_u32(const u32 addr, u32* r1, u32* r2);							//������u32����
#endif
