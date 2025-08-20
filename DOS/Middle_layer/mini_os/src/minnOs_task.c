
#include "miniOs_task.h"


void DOS_StoreTask(OS_task_t *task);

OS_task_t *Task_running;
OS_task_t *Task_temp;
OS_task_t *Task_next;

MemList *Tasks_Ready;
MemList *Tasks_Block;


void switchToPSP(void) {
    __asm volatile (
        "mrs r0, control \n"   // Đọc thanh ghi CONTROL
        "orr r0, r0, #0x02 \n" // Đặt bit 1 để chọn PSP (Process Stack Pointer)
        "msr control, r0 \n"   // Ghi giá trị mới vào CONTROL để chuyển sang PSP
        "isb \n"               // Đồng bộ hóa lệnh
    );
}

void Init_os(void)
{
	DOS_stop_kernel();
	switchToPSP();  // Chuyển CPU sang sử dụng PSP
	Tasks_Ready = newMList();
	Tasks_Block = newMList();

	task_create(MiniOS_TaskIdle, "task_idle", 64, 0);
	List_GetObj(Tasks_Ready, 0xFF, (void*)&Task_running);
	Task_running->state = NONE;

}


OS_task_t * task_create(Task_Function funct, const char *name, uint16_t stack_size, uint8_t piority)
{
	OS_task_t *newTask = (OS_task_t*)malloc(sizeof(OS_task_t));
	newTask->function = funct;
	newTask->name = name;
	newTask->piority = piority;
	newTask->stack_size = stack_size;
	newTask->stack_size = (newTask->stack_size / 4) * 4;
	newTask->state = READY;
	newTask->timeSleep = 0;


	newTask->stackBase = (uint32_t *)malloc(newTask->stack_size * 4);

	newTask->stackBase[newTask->stack_size - 1] = 0x01000000;          // xPSR: Thiết lập bit Thumb (T = 1)
	newTask->stackBase[newTask->stack_size - 2] = (uint32_t)funct; // PC: Địa chỉ hàm taskFunction
	newTask->stackBase[newTask->stack_size - 3] = 0xFFFFFFFD;          // LR: Trở về với chế độ Thread
	// Các thanh ghi R12, R3, R2, R1, R0 (chúng ta đặt giá trị mặc định)
	newTask->stackBase[newTask->stack_size - 4] = 0x00000012;          // R12
	newTask->stackBase[newTask->stack_size - 5] = 0x00000003;          // R3
	newTask->stackBase[newTask->stack_size - 6] = 0x00000002;          // R2
	newTask->stackBase[newTask->stack_size - 7] = 0x00000001;          // R1
	newTask->stackBase[newTask->stack_size - 8] = 0x00000000;          // R0

	// Thanh ghi R4-R11 (được lưu thủ công trong context switch)
	newTask->stackBase[newTask->stack_size - 9] = 0x00000004;          // R4
	newTask->stackBase[newTask->stack_size - 10] = 0x00000005;         // R5
	newTask->stackBase[newTask->stack_size - 11] = 0x00000006;         // R6
	newTask->stackBase[newTask->stack_size - 12] = 0x00000007;         // R7
	newTask->stackBase[newTask->stack_size - 13] = 0x00000008;         // R8
	newTask->stackBase[newTask->stack_size - 14] = 0x00000009;         // R9
	newTask->stackBase[newTask->stack_size - 15] = 0x0000000A;         // R10
	newTask->stackBase[newTask->stack_size - 16] = 0x0000000B;         // R11

	// Thiết lập con trỏ ngăn xếp cho task
//	DOS_stop_kernel();
	newTask->StackPointer = &newTask->stackBase[newTask->stack_size - 8];       // Con
	DOS_StoreTask(newTask);
//	DOS_start_kernel();
	return newTask;
}

void saveContext(void) {
    __asm volatile (
        "mrs r0, psp \n"        // Lấy giá trị của PSP (Process Stack Pointer)
        "stmdb r0!, {r4-r11} \n"// Lưu R4-R11 vào ngăn xếp
    );
}

__attribute__((optimize("O3"))) void ContextSwitch(OS_task_t * Current_task, OS_task_t *nextask)
{
	if(Current_task != NULL && Current_task->state != NONE)
	{
		saveContext();
		Current_task->StackPointer = (uint32_t*)__get_PSP();
	}
	__set_PSP((uint32_t)nextask->StackPointer);
	
}

void switch_task(void)
{
	// __asm("svc #0");
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Đặt bit PendSVSET trong ICSR
}

void Task_handle(void)
{
	OS_task_t *Task_ptr = NULL;
	for(uint8_t count = 0; count < Tasks_Block->Numlist; count ++)
	{
		Task_ptr = (OS_task_t *)List_PtrObj(Tasks_Block, count);
		if(Task_ptr == NULL)
		{
			break;
		}
		if(Task_ptr->state!= SURPEND && (-- Task_ptr->timeSleep <= 0))
		{
			List_GetObj(Tasks_Block, count, (void *)&Task_ptr);
			DOS_StoreTask(Task_ptr);
			Task_ptr->timeSleep = 0;
			count --;
			continue;
		}
	}
	if(List_GetObj(Tasks_Ready, 0xFF, (void *)&Task_next) == 1)
	{
		Task_next->state = READY;
		if(Task_running->state != NONE)
		{
			if(Task_running->state == RUNNING)
			{
				if(Task_next->piority >= Task_running->piority)
				{
					Task_running->state = READY;
					Task_next->state = RUNNING;
				}
				else
				{
					DOS_StoreTask(Task_next);
					return;
				}
			}
			if(Task_running->state != RUNNING)
			{
				Task_temp = Task_running;
				Task_running = Task_next;
				Task_running->state = RUNNING;
				DOS_StoreTask(Task_temp);
			}
		}
		else
		{
			Task_temp = Task_running;
			Task_running = Task_next;
			Task_running->state = RUNNING;
			DOS_StoreTask(Task_temp);
		}
	}
	else
	{

		return;
	}

	switch_task();
}

OS_task_t * DOS_GetTaskRunnung(void)
{
	return Task_running;
}


void DOS_start_kernel(void) {
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;  // Set bit để enable ngắt SysTick
}

void DOS_stop_kernel(void) {
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;  // Clear bit để disable ngắt SysTick
}
void DOS_TaskSuspend(OS_task_t *task)
{
	DOS_stop_kernel();
	if(task->state != SURPEND)
	{
		task->state = SURPEND;
		if(task == Task_running)
		{
			if(List_GetObj(Tasks_Ready, 0xFF, (void *)&Task_next) == 1)
			{
				Task_next->state = READY;
				Task_temp = Task_running;
				Task_running = Task_next;
				Task_running->state = RUNNING;
				DOS_StoreTask( Task_temp);
			}
			DOS_start_kernel();
			switch_task();
		}
		else if(task->timeSleep <= 0)
		{
			List_deleteObj(Tasks_Ready, task);
			DOS_StoreTask(task);
		}
	}

}



void DOS_TaskResume(OS_task_t *task)
{
	if(task->state == SURPEND)
	{
		if(task->timeSleep <= 0)
		{
			task->state = READY;
			List_deleteObj(Tasks_Block, task);
			DOS_StoreTask(task);
		}

	}
}

void DOS_StoreTask(OS_task_t *task)
{
	OS_task_t *pTask = NULL;
	uint8_t pos = 0xFF;
	if(task->timeSleep <= 0 && task->state != SURPEND)
	{
		for(uint8_t count = 0; count < Tasks_Ready->Numlist; count ++)
		{
			pTask = List_PtrObj(Tasks_Ready, count);
			if(task->piority <= pTask->piority)
			{
				pos = count;
				break;
			}
		}
		List_AddObj(Tasks_Ready, pos, task);
	}
	else
	{
		for(uint8_t count = 0; count < Tasks_Block->Numlist; count ++)
		{
			pTask = List_PtrObj(Tasks_Block, count);
			if(task->piority < pTask->piority)
			{
				pos = count + 1;
				break;
			}
		}

		List_AddObj(Tasks_Block, pos, task);
	}
}


void task_sleep(uint32_t time)
{
	DOS_stop_kernel();
	Task_running->timeSleep = time;
	Task_running->state = BLOCK;
	if(List_GetObj(Tasks_Ready, 0xFF, (void *)&Task_next) == 1)
	{
		Task_next->state = READY;
		Task_temp = Task_running;
		Task_running = Task_next;
		Task_running->state = RUNNING;
		DOS_StoreTask( Task_temp);
	}
	DOS_start_kernel();
	switch_task();

}

void MiniOS_TaskIdle(void) {

    while(1)
    {

    }
}

//void SysTick_Handler(void)
//{
//  /* USER CODE BEGIN SysTick_IRQn 0 */
//	HAL_GetTick();
//  /* USER CODE END SysTick_IRQn 0 */
////  HAL_IncTick();
//  /* USER CODE BEGIN SysTick_IRQn 1 */
////	tasks[currentTask].stack = __get_PSP();
////	__set_PSP(tasks[nextTask].stack);
//
//	Task_handle();
//
//  /* USER CODE END SysTick_IRQn 1 */
//}
//
//void PendSV_Handler(void)
//{
//  /* USER CODE BEGIN PendSV_IRQn 0 */
//
//  /* USER CODE END PendSV_IRQn 0 */
//  /* USER CODE BEGIN PendSV_IRQn 1 */
//	ContextSwitch(Task_temp, Task_running);
//
//
//  /* USER CODE END PendSV_IRQn 1 */
//}
