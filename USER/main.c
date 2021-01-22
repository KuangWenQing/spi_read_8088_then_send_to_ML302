#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
//#include "key.h"     
//#include "exti.h"
   
#include "uc8088_spi.h"
#include "wdg.h"

#include <stdbool.h>

volatile bool which_buf = 0;
volatile u8  send_flag = 1;
volatile u16 str_len[2] = {0};
u8 Buffer[2][SPI_BUF_LEN] = {0};


void ByteChange(register u8 *pBuf, s16 len)
{
	register s16 i;
	register u8 ucTmp;
	
	for(i = 0; i < len; i += 4)
	{
			ucTmp = pBuf[i+3];
			pBuf[i+3] = pBuf[i];
			pBuf[i] = ucTmp;
			ucTmp = pBuf[i+2];
			pBuf[i+2] = pBuf[i+1];
			pBuf[i+1] = ucTmp;
	}
}

void ML302_init()
{
	printf("AT\r\n");			
	delay_ms(10);
	printf("AT+CPIN?\r\n");		//��ѯSIM��״̬
	delay_ms(10);
	printf("AT+CSQ\r\n");			//��ѯ�ź������� С��10˵���źŲ�
	delay_ms(10);
//	printf("AT+CGDCONT=1,\"IP\",\"CMIOT\"\r\n");		//����APN
//	delay_ms(10);
	printf("AT+CGACT=1,1\r\n");		//����PDP
	delay_ms(20);
	printf("AT+MIPOPEN=1,\"TCP\",\"server.natappfree.cc\",43328\r\n");	//���ӷ�����
//		printf("AT+MIPOPEN=1,\"TCP\",\"47.104.157.10\",15884\r\n");	//���ӷ�����
	delay_ms(20);
	
//	printf("AT+CMUX=0,0,6,127,10,3,30,10,2\r\n");	//����������Ϊ230400
	delay_ms(10);
	printf("ATE0\r\n");				//�رջ���
	delay_ms(10);
	memset(USART_RX_BUF, 0, USART_REC_LEN);
	USART_RX_STA = 0;
}


void uart_send_data_2_ML302(register u8 *TX_BUF, u16 len)
{
	char num_char[5];
	register u16 i;
	sprintf(num_char, "%d", len);
	printf("AT+MIPSEND=1,%s\r\n", num_char);
	//TX_BUF[len] = '\0';
	//printf("%s", TX_BUF);
	for(i=0; i<len; i++){
		USART1->DR = TX_BUF[i];
		while((USART1->SR&0X40)==0);//�ȴ����ַ��������   
	}
}


void Resend()
{
		printf("Resend\r\n");
		send_flag = 0;
		uart_send_data_2_ML302(Buffer[!which_buf], str_len[!which_buf]);
}


int main(void)
{		
	volatile u8  rp_OK, wp_OK, cnt, wp_stop_flag;
	volatile u16 tmp, tmp1, len;
	volatile u32 rrp;
	u32 wp, rp;

	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(230400);	 	//���ڳ�ʼ��Ϊ230400
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	//KEY_Init();					//��ʼ������
	uc8088_init();		//8088��ʼ��
	
	printf("halt cpu\t");
	rp = uc8088_read_u32(0x1a107018);
	printf("read test = %x\r\n", rp);

	wp = uc8088_read_u32(Buf_addr);
	rp = uc8088_read_u32(Buf_addr + 4);
	printf("wp = %d,   rp = %d\r\n", wp, rp); 
	LED0 = 1;
	ML302_init();
	wp = 65536;
	rrp = rp;

	wp_stop_flag = 0;
	IWDG_Init(5,625);    //���Ƶ��Ϊ128,����ֵΪ625,���ʱ��Ϊ2s
	send_flag = 1;

	while(1){
		if (send_flag == 2){
				//Resend();
				printf("\r\nThe return value is not 'OK'!\r\n");
				send_flag = 1;
		}

		cnt = 0;
		do
		{
			uc8088_read_2_u32(Buf_addr, &wp, &rp);
			if (rp == rrp && wp < SPI_BUF_LEN)
			{
				rp_OK = 1;
				len = (rp <= wp) ? (wp - rp) : (SPI_BUF_LEN - rp + wp);
				if (len < 64){
					wp_OK = 0;
					wp_stop_flag = 1;
				}
				else if((len + str_len[which_buf]) > SPI_BUF_LEN)
					wp_OK = 0;
				else 
					wp_OK = 1;
				break;
			}

			if(cnt++ > 5)
			{
				wp_OK = 0;
				printf("rrp = %d,  rp = %d,  wp = %d!\r\n", rrp, rp, wp);
				break;
			}
		}while(1);
		
		if(rp_OK == 1 && wp_OK == 1){
			LED1 = 0;
			printf("    rp = %d,  wp = %d\r\n", rp, wp);
			
			if (rp < wp){
				tmp = uc8088_read_memory(Buf_addr + 8 + rp, Buffer[which_buf] + str_len[which_buf], len);
				tmp1 = rp+tmp;
				str_len[which_buf] += tmp;
			}
			else{
				tmp = uc8088_read_memory(Buf_addr + 8 + rp, Buffer[which_buf]+ str_len[which_buf], SPI_BUF_LEN - rp);
				tmp1 = uc8088_read_memory(Buf_addr + 8, Buffer[which_buf]+ str_len[which_buf] + tmp, wp);
				tmp += tmp1;
				str_len[which_buf] += tmp;
			}
			
			ByteChange(Buffer[which_buf] + str_len[which_buf] - tmp, tmp);		//�ֽڷ�ת
			
			cnt = 0;  	rp_OK=0;		wp_OK=0;  rrp = 65536;
			
			do{
					uc8088_write_u32(Buf_addr + 4, tmp1);
					rrp = uc8088_read_u32(Buf_addr + 4);
					
					if(cnt++ > 5)		//�޸Ķ�ָ��ʧ��
					{
						printf("write rp = %d, not eq %d, so is error!\r\n", rrp, tmp1);
						break;
					}
			}while(rrp != tmp1);
		}
		
		//�յ�1KB�������ݾͷ���ML302
		if(str_len[which_buf] > 1024 && send_flag == 1)
		{
//			send_flag = 0;
//			LED0 = 0;			//����  æ, STM32 ���ܶ�ȡuc8088, �ʿ��ܶ�����
//			ByteChange(Buffer[which_buf], str_len[which_buf]);		//�ֽڷ�ת
//			cnt = 0;
//			do{
			uart_send_data_2_ML302(Buffer[which_buf], str_len[which_buf]);
			
//				if (cnt++ > 5){
//					printf("send to ML302 error\r\n");
//					break;
//				}
//			}while(ML302_send_result());
//			LED0 = 1;
			send_flag = 0;
			which_buf = !which_buf;
			str_len[which_buf] = 0;
//			memset(Buffer[which_buf], 0, SPI_BUF_LEN);
			IWDG_Feed();		//ι��
		}
		
		if(wp_stop_flag)		//����
		{
			LED1 = 1;
			wp_stop_flag = 0;
			IWDG_Feed();		//ι��
		}
		
	}
}

////�ⲿ�ж�0������� 
//void EXTI0_IRQHandler(void)
//{
//	delay_ms(10);//����
//	if(WK_UP==1)	 	 //WK_UP����
//	{				 
//		if (key0_flag == 0)
//		{
//			/* ���ٶ�д���ر��ļ� */
//			f_close(&fnew);	
//			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
//			f_mount(NULL,"0:",1);
//		}
//		
//		LED0 = 1;
//		LED1 = 1;
//		key0_flag = 1;
//	}
//	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ  
//}

//void EXTI4_IRQHandler(void)
//{
//	delay_ms(10);//����
//	if(KEY0==0)	 //����KEY0
//	{
//		if (key0_flag == 0)
//		{
//			/* ���ٶ�д���ر��ļ� */
//			f_close(&fnew);	
//			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
//			f_mount(NULL,"0:",1);
//		}
//		
//		LED0=!LED0;
//		LED1=!LED1; 
//		key0_flag = 1;
//	}		 
//	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
//}
// 
