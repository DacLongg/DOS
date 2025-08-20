#include "mainApp.h"
#include "list.h"
#include "semaphore.h"
#include "mutex.h"
#include "user_uart.h"
#include "U_DriverUart.h"

uint8_t pid_Calc (uint8_t ID);
uint8_t checksum_Calc (uint8_t PID, uint8_t *data, int size);
void LIN_Master_SenData(uint8_t ID, uint8_t *Data, uint16_t length);
void LIN_Slave_SenData(uint8_t ID, uint8_t *Data, uint16_t length);

typedef struct
{
	uint8_t BreakField;
	uint8_t SyncField;
	uint8_t ID;
	uint8_t Data[8];
	uint16_t Time_Keep_Data;
}Mesage_LIN_mng_t;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
uint8_t DataRX1[3];
uint8_t DataRX3[8];

Queue Queue_Lin_Master;
Queue Queue_Lin_Slave;

void UartFunctCallBack(uint8_t data)
{
	Slip_Decode(data);
}

uint8_t StateMessageLIN = 0;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == (UART_HandleTypeDef *)&huart1)
	{

		Enqueue(&Queue_Lin_Slave, DataRX1, 3);
		HAL_UARTEx_ReceiveToIdle_IT(&huart1, DataRX1, 0xFFFF);
	}
	else
	{
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, DataRX3, 0xFFFF);
		Enqueue(&Queue_Lin_Master, DataRX3, 8);
	}
}



void mainApp(void)
{
	uart_cf_t UartConFig;
	UartConFig.ModulUart = USART_2;
	UartConFig.baudrate = 9600;
	UartConFig.Parity.PC = PARITY_DISABLE;
	UartConFig.StopBit = MODE_1BIT;
	UartConFig.WordLength = MODE_8BIT;
	UartConFig.INT_mode = INT_ENABLE;
	uart_init(UartConFig);

	uart_recive_add_callback(UartFunctCallBack);
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, DataRX1, 0xFFFF);
	HAL_UARTEx_ReceiveToIdle_IT(&huart3, DataRX3, 0xFFFF);
	Mesage_LIN_mng_t Datarx;
	Mesage_LIN_mng_t SL_Datatx;
	Mesage_LIN_mng_t MT_Datatx;
	uint16_t Length;

	uint8_t Data_FromPC[20];
	memset(Data_FromPC, 0, 20);
	memset((uint8_t *)&SL_Datatx, 0, 20);
	uint32_t TickTime = 0;
	uint32_t count_time_keep_data = 0;
	while(1)
	{
		if(Uart_ReadData(Data_FromPC, &Length) == OK)	// nhận từ PC
		{
			count_time_keep_data = HAL_GetTick() ;
			SL_Datatx.Time_Keep_Data = Data_FromPC[0] << 8 | Data_FromPC[1];
			SL_Datatx.ID = Data_FromPC[2];			// ID

			for(uint8_t count = 0; count < 8; count ++)
			{
				SL_Datatx.Data[count] = Data_FromPC[count + 3];	//Data
			}
		}

		if(Dequeue(&Queue_Lin_Slave, (uint8_t *)&Datarx, &Length) == OK)		// slave nhận từ master
		{
			Datarx.ID = Datarx.ID & 0x3F;
			if(Datarx.ID == SL_Datatx.ID)				// kiểm tra ID
			{
				LIN_Slave_SenData(Datarx.ID, SL_Datatx.Data, 4);		// slave gửi vào bus
				Uart_SendData(SL_Datatx.Data, 4);			// gửi lại dữ liệu lên máy tính
			}
		}

//		if(HAL_GetTick() - TickTime > 20)
//		{
//			TickTime = HAL_GetTick() ;
//			if( ++ MT_Datatx.ID >= 0x3F)
//			{
//				MT_Datatx.ID = 0;
//			}
//			LIN_Master_SenData(MT_Datatx.ID, NULL, 0);		// giả lập master gửi
//		}
		if(HAL_GetTick() - count_time_keep_data > SL_Datatx.Time_Keep_Data)
		{
			count_time_keep_data = HAL_GetTick() ;
			if(SL_Datatx.Time_Keep_Data != 0xFFFF)
			{
				memset((uint8_t *)&SL_Datatx, 0, 20);
			}
		}

	}
}



void LIN_Master_SenData(uint8_t ID, uint8_t *Data, uint16_t length)
{
	uint8_t TxData[15];
	TxData[0] = 0x55;  // sync field
	TxData[1] = pid_Calc(ID);
	for (int i=0; i<length; i++)
	{
	  TxData[i+2] = Data[i];
	}
	TxData[length + 2] = checksum_Calc(TxData[1], TxData+2, length);   //lin 2.1 includes PID, for line v1 use PID =0

	HAL_LIN_SendBreak(&huart3);
	if(length == 0)
	{
		HAL_UART_Transmit(&huart3, TxData, 2, 1000);
	}
	else
	{
		HAL_UART_Transmit(&huart3, TxData, length + 3, 1000);
	}
}

void LIN_Slave_SenData(uint8_t ID, uint8_t *Data, uint16_t length)
{
	uint8_t TxData[15];
	TxData[0] = 0x55;  // sync field
	TxData[1] = pid_Calc(ID);
	for (int i=0; i<length; i++)
	{
	  TxData[i+2] = Data[i];
	}
	TxData[length + 2] = checksum_Calc(TxData[1], TxData+2, length);   //lin 2.1 includes PID, for line v1 use PID =0

	HAL_LIN_SendBreak(&huart1);
	if(length == 0)
	{
		HAL_UART_Transmit(&huart1, TxData, 2, 1000);
	}
	else
	{
		HAL_UART_Transmit(&huart1, TxData, length + 3, 1000);
	}
}


uint8_t pid_Calc (uint8_t ID)
{
	if (ID > 0x3F) Error_Handler();
	uint8_t IDBuf[6];
	for (int i=0; i<6; i++)
	{
		IDBuf[i] = (ID>>i)&0x01;
	}
	uint8_t P0 = (IDBuf[0]^IDBuf[1]^IDBuf[2]^IDBuf[4])&0x01;
	uint8_t P1 = ~((IDBuf[1]^IDBuf[3]^IDBuf[4]^IDBuf[5])&0x01);
	ID = ID | (P0<<6) | (P1<<7);
	return ID;
}

uint8_t checksum_Calc (uint8_t PID, uint8_t *data, int size)
{
	uint8_t buffer[size+2];
	uint16_t sum = 0;
	buffer[0] = PID;
	for (int i=0; i<size; i++)
	{
		buffer[i+1] = data[i];
	}

	for (int i=0; i<size+1; i++)
	{
		sum += buffer[i];
		if (sum>0xff) sum = sum-0xFF;
	}

	sum = 0xFF-sum;
	return sum;
}

