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

u8 alarmFlag = 0;   //������־����
u8 alarmBusy = 0;   //�������в���

u8 humidityH;       //ʪ����������
u8 humidityL;       //ʪ��С������
u8 temperatureH;    //�¶���������
u8 temperatureL;    //�¶�С������
u8 Led_Status = 0;  //LED����
float Light = 0;    //���նȲ���

char Pub_Buf[256];//�ϴ�����
const char *topic_Sub[] = {"/home/sub"};//��������
const char topic_Pub[] = "/home/pub";//��������

unsigned short timeCount = 0;	//���ͼ������
unsigned short timeflag2s = 0;	//���������ͱ�־����
unsigned short timeflag5s = 0;	//esp���ͱ�־����

uint8_t Uart3_Buffer[2];        //���ջ�����
uint8_t Uart3_Rx = 0;             //Uart3_Buffer�±�

void sensor_task()
{
    if(timeflag2s)//���ͼ��1s
    {
        /*********** ��ʪ�ȴ�������ȡ����*************/
        DHT11_Read_Data(&humidityH,&humidityL,&temperatureH,&temperatureL);
        UsartPrintf(USART_DEBUG,"�¶ȣ�%d.%d  ʪ�ȣ�%d.%d",temperatureH,temperatureL,humidityH,humidityL);

        /*********** ���նȴ�������ȡ����*************/
        if (!i2c_CheckDevice(BH1750_Addr))
        {
            Light = LIght_Intensity();
            UsartPrintf(USART_DEBUG,"���նȣ�%.1f lx\r\n",Light);
        }

        /******************����ģ��*******************/
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
    /********** esp8266ģ��**************/
    if(timeflag5s)//���ͼ��5s
    {
        Led_Status = !GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);//��ȡLED0��״̬
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
    key = KEY_Scan(0);		//�õ���ֵ
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
        case 0x10://�ص�
            LED0 = 1;
            break;
        case 0x11://����
            LED0 = 0;
            break;
        case 0x20://�رձ�����
            alarmFlag = 0;
            alarmBusy = 0;
            break;
        case 0x21://�򿪱�����
            alarmFlag = 1;
            alarmBusy = 1;
            break;
        case 0x30://��ǰ�¶�
            UsartPrintf(USART3,"%c",temperatureH);
            break;
        case 0x31://��ǰʪ��
            UsartPrintf(USART3,"%c",humidityH);
            break;
		case 0x32://��ǰ���ն�
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
    delay_init();               //��ʱ������ʼ��

    LED_Init();                 //��ʼ����LED���ӵ�Ӳ���ӿ�

    BEEP_Init();

    KEY_Init();

    DHT11_Init();

    BH1750_Init();

    OLED_Init();
    OLED_ColorTurn(0);  //0������ʾ 1 ��ɫ��ʾ
    OLED_DisplayTurn(0);//0������ʾ 1 ��Ļ��ת��ʾ
    OLED_Clear();

    TIM2_Int_Init(2499,7199);   //2499ms
    TIM3_Int_Init(4999,7199);   //4999ms

    Usart1_Init(115200);//DEBUG����
    Usart2_Init(115200);//stm32-esp8266ͨ�Ŵ���
    Usart3_Init(115200);//����ģ�鴮��

    UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
    ESP8266_Init();					//��ʼ��ESP8266

    while(OneNet_DevLink())			//����OneNET
        delay_ms(500);

    BEEP = 0;				        //������ʾ����ɹ�
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
