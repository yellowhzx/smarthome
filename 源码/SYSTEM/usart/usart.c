//硬件驱动
#include "usart.h"
#include "delay.h"
#include "timer.h"

//C库
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

extern uint8_t Uart3_Buffer[2];       	//接收缓冲区
extern uint8_t Uart3_Rx;             	//Uart3_Buffer下标

extern u8 alarmFlag;	//报警标志部分
extern u8 alarmBusy;	//报警空闲部分

/*
************************************************************
*	函数名称：	Usart1_Init
*
*	函数功能：	串口1初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PA9		RX-PA10
************************************************************
*/
void Usart1_Init(unsigned int baud)
{

    GPIO_InitTypeDef gpio_initstruct;
    USART_InitTypeDef usart_initstruct;
    NVIC_InitTypeDef nvic_initstruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    //PA9	TXD
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_9;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);

    //PA10	RXD
    gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_10;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);

    usart_initstruct.USART_BaudRate = baud;
    usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
    usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
    usart_initstruct.USART_Parity = USART_Parity_No;									//无校验
    usart_initstruct.USART_StopBits = USART_StopBits_1;								//1位停止位
    usart_initstruct.USART_WordLength = USART_WordLength_8b;							//8位数据位
    USART_Init(USART1, &usart_initstruct);

    USART_Cmd(USART1, ENABLE);														//使能串口

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//使能接收中断

    nvic_initstruct.NVIC_IRQChannel = USART1_IRQn;
    nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 2;
    nvic_initstruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvic_initstruct);

}

/*
************************************************************
*	函数名称：	Usart2_Init
*
*	函数功能：	串口2初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PA2		RX-PA3
************************************************************
*/
void Usart2_Init(unsigned int baud)
{

    GPIO_InitTypeDef gpio_initstruct;
    USART_InitTypeDef usart_initstruct;
    NVIC_InitTypeDef nvic_initstruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    //PA2	TXD
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_2;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);

    //PA3	RXD
    gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_3;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);

    usart_initstruct.USART_BaudRate = baud;
    usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
    usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
    usart_initstruct.USART_Parity = USART_Parity_No;									//无校验
    usart_initstruct.USART_StopBits = USART_StopBits_1;								//1位停止位
    usart_initstruct.USART_WordLength = USART_WordLength_8b;							//8位数据位
    USART_Init(USART2, &usart_initstruct);

    USART_Cmd(USART2, ENABLE);														//使能串口

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);									//使能接收中断

    nvic_initstruct.NVIC_IRQChannel = USART2_IRQn;
    nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;
    nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_initstruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic_initstruct);

}

/*
************************************************************
*	函数名称：	Usart3_Init
*
*	函数功能：	串口3初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PB10		RX-PB11
************************************************************
*/
void Usart3_Init(unsigned int baud)
{
    GPIO_InitTypeDef gpio_initstruct;     //GPIO初始化结构体声明
    USART_InitTypeDef usart_initstruct;   //串口初始化结构体声明
    NVIC_InitTypeDef nvic_initstruct;  //NVIC初始化结构体声明

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);      //使能PB端口时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);    //USART3时钟

    //USART3的Tx---GPIO----PB10
	gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_10;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&gpio_initstruct);

    //USART3的Rx---GPIO----PB11
    gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_11;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&gpio_initstruct);

    usart_initstruct.USART_BaudRate = baud;
    usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
    usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
    usart_initstruct.USART_Parity = USART_Parity_No;									//无校验
    usart_initstruct.USART_StopBits = USART_StopBits_1;								//1位停止位
    usart_initstruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3,&usart_initstruct);	        //调用USART_Init()函数完成端口初始化

    USART_Cmd(USART3,ENABLE);                           //使能串口3

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);      //使能接收中断

    nvic_initstruct.NVIC_IRQChannel = USART3_IRQn;             //中断通道为USART3_IRQn
	nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;               //通道使能    
	nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 1;     //设置抢占优先级2
    nvic_initstruct.NVIC_IRQChannelSubPriority = 1;            //设置响应优先级2
    NVIC_Init(&nvic_initstruct);                      //调用NVIC_Init()函数完成端口初始化

}

/*
************************************************************
*	函数名称：	Usart_SendString
*
*	函数功能：	串口数据发送
*
*	入口参数：	USARTx：串口组
*				str：要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：
************************************************************
*/
void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len)
{

    unsigned short count = 0;

    for(; count < len; count++)
    {
        USART_SendData(USARTx, *str++);									//发送数据
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);		//等待发送完成
    }

}

/*
************************************************************
*	函数名称：	UsartPrintf
*
*	函数功能：	格式化打印
*
*	入口参数：	USARTx：串口组
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：
************************************************************
*/
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...)
{

    unsigned char UsartPrintfBuf[296];
    va_list ap;
    unsigned char *pStr = UsartPrintfBuf;

    va_start(ap, fmt);
    vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
    va_end(ap);

    while(*pStr != 0)
    {
        USART_SendData(USARTx, *pStr++);
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
    }
}

/*
************************************************************
*	函数名称：	DEBUG_LOG
*
*	函数功能：	格式化输出调试信息
*
*	入口参数：	fmt：不定长参

*	返回参数：	无
*
*	说明：
************************************************************
*/
void DEBUG_LOG(char *fmt,...)
{
    unsigned char UsartPrintfBuf[296];
    va_list ap;
    unsigned char *pStr = UsartPrintfBuf;

    va_start(ap, fmt);
    vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
    va_end(ap);
    UsartPrintf(USART_DEBUG, "[LOG] /> ");
    while(*pStr != 0)
    {
        USART_SendData(USART_DEBUG, *pStr++);
        while(USART_GetFlagStatus(USART_DEBUG, USART_FLAG_TC) == RESET);
    }
    UsartPrintf(USART_DEBUG, "\r\n");
}

/*
************************************************************
*	函数名称：	USART1_IRQHandler
*
*	函数功能：	串口1收发中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：
************************************************************
*/
void USART1_IRQHandler(void)
{

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收中断
    {
        USART_ClearITPendingBit(USART1, USART_FLAG_RXNE);
    }

}

/*
************************************************************
*	函数名称：	USART3_IRQHandler
*
*	函数功能：	串口3收发中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：
************************************************************
*/
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收中断
    {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		Uart3_Buffer[Uart3_Rx] = USART_ReceiveData(USART3); 
		Uart3_Rx++;
	}
}
