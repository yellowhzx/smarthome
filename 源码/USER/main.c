#include "stdio.h"
#include "led.h"
#include "delay.h"
#include "timer.h"
#include "key.h"
#include "dht11.h"
#include "usart.h"
#include "bh1750.h"
#include "oled.h"
#include "beep.h"
#include "esp8266.h"
#include "onenet.h"

u8 alarmFlag = 0;   //报警标志部分
u8 alarmBusy = 0;   //报警空闲部分

u8 humidityH;       //湿度整数部分
u8 humidityL;       //湿度小数部分
u8 temperatureH;    //温度整数部分
u8 temperatureL;    //温度小数部分
u8 Led_Status = 0;  //LED部分
float Light = 0;    //光照度部分

char Pub_Buf[256];//上传数据
const char *topic_Sub[] = {"/home/sub"};//上行数据
const char topic_Pub[] = "/home/pub";//订阅命令

unsigned short timeCount = 0;	//发送间隔变量
unsigned short timeflag2s = 0;	//传感器发送标志变量
unsigned short timeflag5s = 0;	//esp发送标志变量

uint8_t Uart3_Buffer[2];        //接收缓冲区
uint8_t Uart3_Rx = 0;             //Uart3_Buffer下标

void sensor_task()
{
    if(timeflag2s)//发送间隔1s
    {
        /*********** 温湿度传感器获取数据*************/
        DHT11_Read_Data(&humidityH,&humidityL,&temperatureH,&temperatureL);
        UsartPrintf(USART_DEBUG,"温度：%d.%d  湿度：%d.%d",temperatureH,temperatureL,humidityH,humidityL);

        /*********** 光照度传感器获取数据*************/
        if (!i2c_CheckDevice(BH1750_Addr))
        {
            Light = LIght_Intensity();
            UsartPrintf(USART_DEBUG,"光照度：%.1f lx\r\n",Light);
        }

        /******************报警模块*******************/
        if(!alarmBusy)
        {
            if(humidityH < 80 && temperatureH < 30 && Light < 10000)
                alarmFlag = 0;
            else
                alarmFlag = 1;
        }
//		UsartPrintf(USART_DEBUG,"alarmBusy = %d\r\n",alarmBusy);
//        UsartPrintf(USART_DEBUG,"alarmFlag = %d\r\n",alarmFlag);
        timeflag2s = 0;
    }
}

void esp8266_task()
{
    unsigned char *dataPtr = NULL;
    /********** esp8266模块**************/
    if(timeflag5s)//发送间隔5s
    {
        Led_Status = !GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);//读取LED0的状态
        UsartPrintf(USART_DEBUG, "OneNet_Publish\r\n");
        sprintf(Pub_Buf,"{\"Temp\":%d.%d,\"Humi\":%d.%d,\"Light\":%.1f,\"Led\":%d,\"Beep\":%d}",
                temperatureH,temperatureL,humidityH,humidityL,Light,Led_Status,alarmFlag);
        OneNet_Publish(topic_Pub,Pub_Buf);

        ESP8266_Clear();
        timeflag5s = 0;
    }
    dataPtr = ESP8266_GetIPD(3);
    if(dataPtr != NULL)
        OneNet_RevPro(dataPtr);

    delay_ms(10);
}

void key_task()
{
    u8 key = 0;
    key = KEY_Scan(0);		//得到键值
    switch(key)
    {
    case KEY0_PRES:
        LED0 = !LED0;
        break;
    case KEY1_PRES:
        alarmFlag = !alarmFlag;
        LED1 = !LED1;
        alarmBusy = !alarmBusy;
        break;
    default:
        delay_ms(10);
    }
}

void voice_task()
{
    u8 i;
    if(Uart3_Buffer[0] == 0xaa)
    {
        switch (Uart3_Buffer[1])
        {
        case 0x10://关灯
            LED0 = 1;
            break;
        case 0x11://开灯
            LED0 = 0;
            break;
        case 0x20://关闭报警器
            alarmFlag = 0;
            alarmBusy = 0;
            break;
        case 0x21://打开报警器
            alarmFlag = 1;
            alarmBusy = 1;
            break;
        case 0x30://当前温度
            UsartPrintf(USART3,"%c",temperatureH);
            break;
        case 0x31://当前湿度
            UsartPrintf(USART3,"%c",humidityH);
            break;
		case 0x32://当前光照度
            UsartPrintf(USART3,"%c",(uint8_t)Light);
		break;
        }
        for(i = 0; i < 2; i++)
        {
            Uart3_Buffer[i] = 0;
        }
        Uart3_Rx = 0;
    }
}

int main(void)
{
    delay_init();               //延时函数初始化

    LED_Init();                 //初始化与LED连接的硬件接口

    BEEP_Init();

    KEY_Init();

    DHT11_Init();

    BH1750_Init();

    OLED_Init();
    OLED_ColorTurn(0);  //0正常显示 1 反色显示
    OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
    OLED_Clear();

    TIM2_Int_Init(2499,7199);   //2499ms
    TIM3_Int_Init(4999,7199);   //4999ms

    Usart1_Init(115200);//DEBUG串口
    Usart2_Init(115200);//stm32-esp8266通信串口
    Usart3_Init(115200);//语音模块串口

    UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
    ESP8266_Init();					//初始化ESP8266

    while(OneNet_DevLink())			//接入OneNET
        delay_ms(500);

    BEEP = 0;				        //鸣叫提示接入成功
    delay_ms(250);
    BEEP = 1;

    OneNet_Subscribe(topic_Sub, 1);

    while(1)
    {
        sensor_task();
        esp8266_task();
        voice_task();
        key_task();
    }
}
