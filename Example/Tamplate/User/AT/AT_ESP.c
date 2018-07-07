#include "AT_ESP.h"
#include "./usart2/bsp_usart2.h"
#include "./systick/bsp_systick.h"
#include <string.h>
#include <stdio.h>




char ESPBuffer[128];
__IO uint16_t ESPRxIndex = 0x00;
const char ResponseOK[] = "OK\r\n";
const char SendResponse[] = "OK\r\n> ";


#define ESP_UART_CONFIG        USART2_Config
#define SendMessageToESP       USART2_SendString
#define SendMessageToESP2      USART2_SendData
#define Delay_ms               Delay_s 


//���жϽ���ESP�ķ�����Ϣ
void USART2_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) == SET)
	{
		USART_ReceiveData( USART2 );
		USART_ClearITPendingBit(USART2,USART_IT_ORE);
	}
	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
	{
		if(ESPRxIndex < 128)
		{
			ESPBuffer[ESPRxIndex++] = USART_ReceiveData(USART2);
		}
		else
		{
			USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
		}
	}
}       
/*
-------------------------------------------------------------------------------
�������һЩ��������
-------------------------------------------------------------------------------
*/

static void delay()
{
	int i,j;
	i = 1000;
	while(i--)
	{
		j = 350;
		while(j--);
	}
}

static void ClearESPBuffer()
{
	ESPRxIndex = 0x00;
	memset(ESPBuffer, '\0', ESP8266_RX_BUFFER_LEN);
}


static void SendEspCommand(char *order)
{
	ClearESPBuffer();
	SendMessageToESP(order);
}

static void SendEspCommand2(char *data,int len)
{
	ClearESPBuffer();
	SendMessageToESP2(data,len);
}


static char CheckResponse()
{
	if(strstr(ESPBuffer,ResponseOK) == NULL)
	{
		return 0;
	}
	return 1;
}

/*
-------------------------------------------------------------------------------
������������
-------------------------------------------------------------------------------
*/

/*
-------------------------------------------------------------------------------
��������ESP01��API
-------------------------------------------------------------------------------
*/


/*
����˵������ⵥƬ����ESP-01�Ƿ���ȷͨ��
����˵����void
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 CheckEsp(void)
{
	SendEspCommand("AT\r\n");
	delay();
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}

/*
����˵��������ESP-01�ĵ�ǰ��WIFIģʽ
����˵����mode����Ҫ���õ�ģʽ������ģʽ���ATָ���ֲ�
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 SetEspMode(char mode)
{
	char tmp[3];
	tmp[0] = mode;
	tmp[1] = '\r';
	tmp[2] = '\n';
	SendEspCommand("AT+CWMODE_CUR=");
	SendEspCommand(tmp);
	delay();
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}

/*
����˵������ESP-01����ָ��WIFI
����˵����SSID:��Ҫ���ӵ�WIFI����Passwd:��Ӧ��WIFI����
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 ConnectWiFi(char *SSID,char *Passwd)
{
	SendEspCommand("AT+CWJAP=\"");
	SendEspCommand(SSID);
	SendEspCommand("\",\"");
	SendEspCommand(Passwd);
	SendEspCommand("\"\r\n");
	Delay_ms(6500);
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}
/*
����˵������ȡESP-01�ĵ�ǰ��IPaddress��ע�������ESP-01����WIFI��ʹ��
����˵����RecvBuffer:���շ��ص�IPaddress��Buffer
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 GetIpAddress(char *RecvBuffer)
{
	char i = 0;
	char *p;
	SendEspCommand("AT+CIFSR\r\n");
	delay();
	if(CheckResponse())
	{
		p = strtok(ESPBuffer,"\"");
		p = strtok(NULL,"\"");
		while(*p)
		{
			RecvBuffer[i] = *p;
			p++;
			i++;
		}
		return 1;
	}
	return 0;
}

/*
����˵������ȡESP-01��MACaddress
����˵����RecvBuffer:���շ��ص�MACaddress��Buffer
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 GetMACaddress(char *Recvbuffer)
{
	char i = 0;
	char *p;
	SendEspCommand("AT+CIFSR\r\n");
	delay();
	if(CheckResponse())
	{
		p = strtok(ESPBuffer,"\"");
		p = strtok(NULL,"\"");
		p = strtok(NULL,"\"");
		p = strtok(NULL,"\"");
		while(*p)
		{
			Recvbuffer[i] = *p;
			p++;
			i++;
		}
		return 1;
	}
	return 0;
}

/*
����˵�����Ͽ�ESP-01�����ӵ�WIFI
����˵����void
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 DisconnectWiFi()
{
	SendEspCommand("AT+CWQAP\r\n");
	delay();
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}
/*
����˵��������ָ����TCPServer����UDPServer
����˵����IPAddress��TCPServer����UDPServer��IPAddress��port����Ӧ�������Ķ˿�
					mode:����Server�����ͣ�0ΪTCP,1ΪUDP
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 ConnectServer(char *IPAddress,char *port,char mode)
{
	if(mode == 0)
	{
		SendEspCommand("AT+CIPSTART=\"TCP\",\"");
		SendEspCommand(IPAddress);
		SendEspCommand("\",");
		SendEspCommand(port);
		SendEspCommand("\r\n");
		Delay_ms(1000);
		if(CheckResponse())
		{
			return 1;
		}
		return 0;
	}
	else if(mode == 1)
	{
		SendEspCommand("AT+CIPSTART=\"UDP\",\"");
		SendEspCommand(IPAddress);
		SendEspCommand("\",");
		SendEspCommand(port);
		SendEspCommand("\r\n");
		Delay_ms(1000);
		if(CheckResponse())
		{
			return 1;
		}
		return 0;
	}
	else
	{
		return 0;
	}
}

/*
����˵�����ر������ӵ�TCPServer or UDPServer
����˵����void
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
u8 CloseTCPOrUDPConnect(void)
{
	SendEspCommand("AT+CIPCLOSE\r\n");
	Delay_ms(1000);
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}

/*
����˵�������Ѿ����ӵ�Sever�����ַ�������
����˵����Data:Ҫ���͵��ַ�����Ϣָ��
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע����������Server������TCPServer����UDPServer���Ӻ�����֮ǰ���ӵ�Server
					���������ʹ���������֮ǰʹ�õ���TCPServer����ô�ú��������������ӵ�
					TCPServer�����ַ������ݣ�����UDPServer��Ҳ����ˡ�
�Ѳ���
*/
u8 SendStringDataToServer(char *Data)
{
	char params[8];
	int len;
	len = strlen(Data);
	sprintf(params,"%d",len);
	SendEspCommand("AT+CIPSEND=");
	SendEspCommand(params);
	SendEspCommand("\r\n");
	delay();
	if(CheckResponse())
	{
		SendEspCommand(Data);
		delay();
		delay();
		return 1;
	}
	return 0;
}

/*
����˵�������Ѿ����ӵ�Sever���ͷ��ַ�����Ϣ(��������ṹ����ڴ������Ϣ)
����˵����Data:Ҫ�������ݵ�ͷָ�룬len���������ݳ���
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע����������Server������TCPServer����UDPServer���Ӻ�����֮ǰ���ӵ�Server
					���������ʹ���������֮ǰʹ�õ���TCPServer����ô�ú��������������ӵ�
					TCPServer�����ַ������ݣ�����UDPServer��Ҳ����ˡ�
�Ѳ���
*/
u8 SendDataToServer(char *Data,int len)
{
	char params[8];
	sprintf(params,"%d",len);
	SendEspCommand("AT+CIPSEND=");
	SendEspCommand(params);
	SendEspCommand("\r\n");
	delay();
	if(CheckResponse())
	{
		SendEspCommand2(Data,len);
		delay();
		delay();
		return 1;
	}
	return 0;
}

/*
����˵������ESP01����modem_sleep״̬
����˵����void
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
������
*/
u8 ModemSleep_Getinto(void)
{
	SendEspCommand("AT+SLEEP=2\r\n");
	delay();
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}

/*
����˵������ESP01�˳�modem_sleep״̬
����˵����void
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
������
*/
u8 ModemSleep_Quit(void)
{
	SendEspCommand("AT+SLEEP=0\r\n");
	delay();
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}

/*
����˵������ESP01����Deep_Sleep״̬
����˵����void
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�������ģʽ��ֻ��ͨ��Ӳ�����ѣ���ϸ˵���鿴�ٷ�ATָ��
������
*/
u8 DeepSleep_Getinto(void)
{
	SendEspCommand("AT+GSLP=0\r\n");   //����0��������ESP����Ӳ�����ѣ��������ỽ��.
	delay();
	if(CheckResponse())
	{
		return 1;
	}
	return 0;
}



/*
����˵����esp8266�ṹ��ĳ�ʼ���Ͷ�ӦUART�ĳ�ʼ��
����˵����IPAddress��TCPServer��IPAddress��port����Ӧ�������Ķ˿�
����ֵ˵��:��������ֵΪ1��success,0Ϊfail
ע�����void
�Ѳ���
*/
void esp8266Init(esp8266 *handle)
{
	ESP_UART_CONFIG();                  //��ʼ��ESP-01��Ӧ�Ĵ���
	handle->CheckESP = CheckEsp;        //��Щ���ú���ָ���Ӧ��Ӧ�ĺ���
	handle->SetEspMode = SetEspMode;
	handle->ConnectWiFi = ConnectWiFi;
	handle->DisconnectWiFi = DisconnectWiFi;
	handle->GetIpAddress = GetIpAddress;
	handle->GetMACaddress = GetMACaddress;
	handle->ConnectServer = ConnectServer;
	handle->CloseTCPOrUDPConnect = CloseTCPOrUDPConnect;
	handle->SendStringDataToServer = SendStringDataToServer;
	handle->SendDataToServer = SendDataToServer;
	handle->ModemSleep_Getinto = ModemSleep_Getinto;
	handle->ModemSleep_Quit = ModemSleep_Quit;
	handle->DeepSleep_Getinto = DeepSleep_Getinto;
}
