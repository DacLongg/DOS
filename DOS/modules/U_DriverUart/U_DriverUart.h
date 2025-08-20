/*
 * U_DriverUart.h
 *
 *  Created on: May 29, 2025
 *      Author: Admin
 */

#ifndef U_CODEHANDLE_U_DRIVERUART_U_DRIVERUART_H_
#define U_CODEHANDLE_U_DRIVERUART_U_DRIVERUART_H_

#include "main.h"
#include "user_uart.h"



#define NOT_OK 		1
#define OK 			0

#define QUEUE_BUFFER_SIZE	 	5		// contain max 5 packet
#define RING_BUFFER_SIZE 		280		// contain max 40 byte in ring buffer
#define MAX_LENGTH_DATA_QUEUE	280		// each packet contain max 40 byte

typedef struct
{
  volatile uint16_t head;
  volatile uint16_t tail;
  uint8_t data[RING_BUFFER_SIZE];
} Ring;
/* Structure for LiteLink object */
typedef struct
{
  uint8_t data[MAX_LENGTH_DATA_QUEUE];
  volatile uint16_t length;
  uint16_t id;
} Queue_Object;
/* Structure for managing FIFO queue in LiteLink */
typedef struct
{
  Queue_Object objects[QUEUE_BUFFER_SIZE];
  volatile uint16_t count;
  volatile uint16_t head;
  volatile uint16_t tail;
} Queue;

extern Ring Ring_Obj;
extern Queue Queue_Recieve;
extern Queue Queue_Transmit;

Queue* newQueuen(void);
uint8_t Enqueue(Queue* _Queue, void* data, uint16_t length);
uint8_t Dequeue(Queue* _Queue, void* data, uint16_t* length);
uint16_t Queue_GetLength(Queue* _Queue);
Ring* newRing();
uint8_t Ring_PushData(Ring* _ring, uint8_t* data, uint16_t length);
uint16_t Ring_PullData(Ring* _ring, uint8_t* data, uint16_t length);
uint16_t Ring_CountByte(Ring* _ring);
void Ring_Delete(Ring* _ring);
uint8_t Message_handle(Ring* _ring);

void Slip_Decode(uint8_t Byte);
void Slip_Encode(uint8_t *Data, uint16_t length);

void Uart_InitDriver(void);
uint8_t Uart_ReadData(uint8_t *Data, uint16_t *Length);
uint8_t Uart_SendData(uint8_t *Data, uint16_t Length);
uint8_t Uart_SendData_LoopBack(uint8_t *Data, uint16_t Length);

typedef void (*PfnOnDataReadCompleteCallback)(void);


#endif /* U_CODEHANDLE_U_DRIVERUART_U_DRIVERUART_H_ */
