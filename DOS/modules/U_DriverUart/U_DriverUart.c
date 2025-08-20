/*
 * U_DriverUart.c
 *
 *  Created on: May 29, 2025
 *      Author: Admin
 */
#include "U_DriverUart.h"
#include "stdlib.h"
#include "string.h"

Queue Queue_Recieve;
Queue Queue_Transmit;
Ring Ring_Obj;
Ring Ring_Transmit;

void Uart_InitDriver(void)
{
	Queue_Recieve.count = 0;
	Queue_Recieve.head = 0;
	Queue_Recieve.tail = 0;

	Queue_Transmit.count = 0;
	Queue_Transmit.head = 0;
	Queue_Transmit.tail = 0;

	for(uint8_t count = 0; count < QUEUE_BUFFER_SIZE; count ++)
	{
		Queue_Recieve.objects[count].length = 0;
		Queue_Transmit.objects[count].length = 0;
	}

	Ring_Obj.head = 0;
	Ring_Obj.tail = 0;

	Ring_Transmit.head = 0;
	Ring_Transmit.tail = 0;

}



uint8_t Uart_ReadData(uint8_t *Data, uint16_t *Length)
{
	return Dequeue(&Queue_Recieve, Data, Length);
}

uint8_t Uart_SendData(uint8_t *Data, uint16_t Length)
{
	Slip_Encode(Data, Length);
	return 0;
//	return Enqueue(&Queue_Transmit, Data, Length);
}

uint8_t Uart_SendData_LoopBack(uint8_t *Data, uint16_t Length)
{
	return Enqueue(&Queue_Recieve, Data, Length);
}

uint8_t Enqueue(Queue* _Queue, void* data, uint16_t length) {
  if (_Queue->count == QUEUE_BUFFER_SIZE) {
    return NOT_OK;  // _Queue is full, cannot add object
  }
  if (data == NULL)
  {
	  return NOT_OK;
  }
  memcpy(_Queue->objects[_Queue->tail].data, data, length);
  _Queue->objects[_Queue->tail].length = length;

  /* Move the tail to the next position in the circular buffer */
  _Queue->tail = (uint16_t)((_Queue->tail + 1) % QUEUE_BUFFER_SIZE);
  _Queue->count++;

  return OK;
}

uint8_t Dequeue(Queue* _Queue, void* data, uint16_t* length) {
  if (_Queue->count == 0) {
    return NOT_OK;  // FIFO is empty, cannot dequeue object
  }
  if (data == NULL)
  {
	  return NOT_OK;
  }
  /* Copy the object's data from the head */
  *length = _Queue->objects[_Queue->head].length;
  memcpy(data, _Queue->objects[_Queue->head].data, *length);

  /* Free the memory of the dequeued object */
  _Queue->objects[_Queue->head].length = 0;

  /* Move the head to the next position in the circular buffer */
  _Queue->head = (uint16_t)((_Queue->head + 1) % (uint16_t)QUEUE_BUFFER_SIZE);
  _Queue->count--;

  return OK;
}

uint16_t Queue_GetLength(Queue* _Queue)
{
	return _Queue->objects[_Queue->head].length;
}

uint8_t Ring_PushData(Ring* _ring, uint8_t* data, uint16_t length) {
  for (uint16_t CountByte = 0; CountByte < length; CountByte++) {
    _ring->data[_ring->tail] = data[CountByte];
    if (++_ring->tail == RING_BUFFER_SIZE) {
      _ring->tail = 0;
    }

    if (_ring->tail == _ring->head) {
    	if(_ring->tail == 0)
    	{
    		_ring->tail = (uint16_t)(RING_BUFFER_SIZE - 1);
    	}
    	else
    	{
    		_ring->tail --;

    	}
      _ring->data[_ring->tail - 1] = data[CountByte];
    }
  }
  return OK;
}

uint16_t Ring_PullData(Ring* _ring, uint8_t* data, uint16_t length) {
  uint16_t countData = Ring_CountByte(_ring);
  uint16_t maxByteRead = (countData >= length) ? length : countData;

  for (uint16_t CountByte = 0; CountByte < maxByteRead; CountByte++) {
    data[CountByte] = _ring->data[_ring->head];
    if (++_ring->head == RING_BUFFER_SIZE) {
      _ring->head = 0;
    }
  }
  return maxByteRead;
}
uint16_t Ring_CountByte(Ring* _ring) {
  uint16_t count = 0;
  Ring* ring = _ring;
  if (ring->tail >= ring->head) {
    count = ring->tail - ring->head;
  } else {
    count = (uint16_t)(RING_BUFFER_SIZE - ring->head + ring->tail);
  }
  return count;
}

void Ring_Delete(Ring* _ring)
{
	_ring->tail = 0;
	_ring->head = 0;
}

uint8_t Message_handle(Ring* _ring) {

	uint8_t *Data_Send = NULL;
	uint16_t LengthSend = 0;
	static uint8_t countTimeSplitFrame = 0;

	uint8_t* Data_Receive = NULL;
	static uint16_t lengthReceive = 0;
	static uint16_t countCycle = 0;
	uint16_t len = Ring_CountByte(_ring);
	if (len > lengthReceive) {
		lengthReceive = len;

	}
	else if(len > 0)
	{
		if (++countCycle >= 2) {
			/* receive data handle */
			Data_Receive = malloc(lengthReceive);
			if(Data_Receive == NULL)
			{
				return NOT_OK;
			}
			Ring_PullData(_ring, Data_Receive, lengthReceive);
			Enqueue(&Queue_Recieve, Data_Receive, lengthReceive);
			free(Data_Receive);
			Data_Receive = NULL;
			lengthReceive = 0;
			countCycle = 0;
		}
	}

	/* Transmit data handle */
	if(++ countTimeSplitFrame >= 2)
	{
		LengthSend = Queue_GetLength(&Queue_Transmit);
		if(LengthSend > 0)
		{
			Data_Send = malloc(LengthSend);
			if(Data_Send == NULL)
			{
				return NOT_OK;
			}
			if (Dequeue(&Queue_Transmit, Data_Send, &LengthSend) == OK)
			{
				uart_WriteData(USART_2, ( uint8_t*)Data_Send, (uint32_t)LengthSend);
			}
			free(Data_Send);
		}
		countTimeSplitFrame = 0;
	}

	return OK;
}

/*.....Slip Protocol......*/
#define BYTE_HEAD_FRAME		0xC0
#define BYTE_END_FRAME		0xC0
#define BYTE_ESC_FRAME		0xDB
#define BYTE_ESC_END		0xDC
#define BYTE_ESC_ESC		0xDD


#define START_FRAME			0
#define MIDDLE_FRAME		1
#define END_FRAME			2
#define MIDDLE_FRAME_DB		3

void Slip_Decode(uint8_t Byte)
{
	static 	uint8_t TypeByte 		= 0;
			uint8_t *data 			= NULL;
			uint16_t Length			= 0;
	static  uint8_t CountHeadFrame 	= 0;
			uint8_t Temp_byte		= 0;

	switch(TypeByte)
	{
		case START_FRAME:
		{
			if(Byte == BYTE_HEAD_FRAME)
			{
				if( ++ CountHeadFrame == 2)
				{
					TypeByte = MIDDLE_FRAME;
				}
			}
			else
			{
				CountHeadFrame = 0;
			}
			break;
		}
		case MIDDLE_FRAME:
		{

			if(Byte == 0xDB)
			{
				CountHeadFrame = 0;
				TypeByte = MIDDLE_FRAME_DB;
			}
			else if(Byte == BYTE_END_FRAME && CountHeadFrame == 0)
			{
				TypeByte = START_FRAME;
				/*end frame: push to queue*/
				Length = Ring_CountByte(&Ring_Obj);
				data = malloc(Length);
				if(data == NULL)
				{
					Ring_Delete(&Ring_Obj);
				}
				Ring_PullData(&Ring_Obj, data, Length);
				Enqueue(&Queue_Recieve, data, Length);
				free(data);
				data = NULL;
			}
			else if(Byte != BYTE_END_FRAME)
			{
				CountHeadFrame = 0;
				/*push to buffer*/
				Ring_PushData(&Ring_Obj, &Byte, 1);
			}
			else{

				TypeByte = START_FRAME;
				CountHeadFrame = 0;
				Ring_Delete(&Ring_Obj);
			}
			break;
		}
		case MIDDLE_FRAME_DB:
		{
			CountHeadFrame = 0;
			if(Byte == 0xDC)
			{
				Temp_byte = 0xC0;
				/*push to buffer*/
				Ring_PushData(&Ring_Obj, &Temp_byte, 1);
				TypeByte = MIDDLE_FRAME;
			}
			else if(Byte == 0xDD)
			{
				Temp_byte = 0xDB;
				/*push to buffer*/
				Ring_PushData(&Ring_Obj, &Temp_byte, 1);
				TypeByte = MIDDLE_FRAME;
			}
			else
			{

				/*frame error: pull from buffer and clear*/
				Ring_Delete(&Ring_Obj);
				TypeByte = START_FRAME;
			}
			break;
		}
	}
}

void Slip_Encode(uint8_t *Data, uint16_t length)
{
	uint8_t 	*data 			= NULL;
	uint16_t 	Length			= 0;
    uint8_t 	u8Val;
    uint16_t 	u16CountIdx;
    uint8_t  	datatemp[2] 	= {0, 0};

    datatemp[0] = 0xC0;
    datatemp[1] = 0xC0;
    Ring_PushData(&Ring_Transmit, datatemp, 2);
	for(u16CountIdx = 0; u16CountIdx < length; u16CountIdx ++)
	{
		u8Val = Data[u16CountIdx];
		switch(u8Val)
		{
			case BYTE_HEAD_FRAME:
				datatemp[0] = BYTE_ESC_FRAME;
				datatemp[1] = BYTE_ESC_END;
				Ring_PushData(&Ring_Transmit, datatemp, 2);

				break;
			case BYTE_ESC_FRAME:
				datatemp[0] = BYTE_ESC_FRAME;
				datatemp[1] = BYTE_ESC_ESC;
				Ring_PushData(&Ring_Transmit, datatemp, 2);

				break;
			default:
				datatemp[0] = u8Val;
				Ring_PushData(&Ring_Transmit, datatemp, 1);
				break;
		}
	}
	datatemp[0] = 0xC0;
	Ring_PushData(&Ring_Transmit, datatemp, 1);
	Length = Ring_CountByte(&Ring_Transmit);
	data = malloc(Length);
	if(data == NULL)
	{
		Ring_Delete(&Ring_Transmit);
	}
	Ring_PullData(&Ring_Transmit, data, Length);
	uart_WriteData(USART_2, ( uint8_t*)data, (uint32_t)Length);
	free(data);
	data = NULL;

}

