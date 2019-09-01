/*
*********************************************************************************************************
*
*	模块名称   :   任务相关API实现模块。
*	文件名称   :   tTask.c
*	版    本   :   V1.0
*	说    明   :   定义任务数据结构，任务初始化，任务调度锁的初始化，任务调度
*                的实现，任务延时、挂起、就绪状态的实现
*
*	修改记录   :
*		             版本号     日期         作者            说明
*		              V1.0    2019-4-16      Solar           第一次版本发布
*		              
*
*	版权       ：  仅限学习交流使用，禁止用于商业用途
*
*********************************************************************************************************
*/

#include "SOLAROS.h"
#include <string.h>

/*定义调度锁*/
uint8_t schedLockCount;

/*
  定义一个位图数据结构，用来存放任务的优先级，第0位优先级最高，31位优先级最低，
	某位为1表示此优先级对应的任务为就绪状态，为0表示此优先级对应的任务为挂起状态
*/
tBitmap tbitmap;

/*定义一个tList类型的数组，它的每一个成员都是一个双向循环链表，用于存放不同优先级的任务队列*/
tList tTaskList[TASK_MAX_PRIO_COUNT];

/*定义任务延时队列*/
tList tTaskDelayedList;

/*定义任务指针，分别指向当前任务和下一任务*/
tTask *currentTask;
tTask *nextTask;

/*
*********************************************************************************************************
*	函 数 名  :   tTaskInit
*	功能说明  :   任务初始化
*	形    参  :   task：需要被初始化的任务
*               entry：任务函数的入口地址
*               pararm：传递给任务函数的变量
*               prio：任务的优先级
*               stackStart：任务堆栈的起始地址
*               stackSize：任务堆栈的大小
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskInit(tTask *task, void(*entry)(void *), void *param, uint8_t prio, tTaskStack *stackStart, uint32_t stackSize)
{
	/*堆栈的结束地址，cortex-M3的堆栈由高地址向低地址增长*/
	tTaskStack *stackEnd;
	
	/*给task的堆栈起始地址赋值*/
	task->stackStart = stackStart;
	
	/*给task的堆栈大小赋值*/
	task->stackSize = stackSize;
	
	/*将堆栈空间全部置位为0，初始化*/
	memset(stackStart, 0, stackSize);
	
	/*堆栈的结束地址 = 起始地址+堆栈大小/堆栈数据类型(4096/4)，0-1023，所以使用--stackEnd，先减后用*/
	stackEnd = stackStart + stackSize/sizeof(tTaskStack);
	
	/*
	  初始化CPU寄存器数据，为任务配置一个“虚假”的现场，用于当CPU切换至该任务时，从该任务的堆栈中恢复
	  CPU寄存器的值。未用到的寄存器，直接填写了寄存器号。寄存器的赋值顺序不能变
	*/
	*(--stackEnd) = (unsigned long)(1<<24);                /*XPSR，设置CPU为Thumb状态*/
	*(--stackEnd) = (unsigned long)entry;                  /*任务函数的入口地址*/ 
	*(--stackEnd) = (unsigned long)0x14;                   /*R14 LR寄存器*/
	*(--stackEnd) = (unsigned long)0x12;                   /*R12*/
	*(--stackEnd) = (unsigned long)0x3;                    /*R3*/
	*(--stackEnd) = (unsigned long)0x2;                    /*R2*/
	*(--stackEnd) = (unsigned long)0x1;                    /*R1*/
	*(--stackEnd) = (unsigned long)param;                  /*R0 = param，任务函数的参数*/
	*(--stackEnd) = (unsigned long)0x11;                   /*R11*/
	*(--stackEnd) = (unsigned long)0x10;                   /*R10*/
	*(--stackEnd) = (unsigned long)0x9;                    /*R9*/
	*(--stackEnd) = (unsigned long)0x8;                    /*R8*/
	*(--stackEnd) = (unsigned long)0x7;                    /*R7*/
	*(--stackEnd) = (unsigned long)0x6;                    /*R6*/
	*(--stackEnd) = (unsigned long)0x5;                    /*R5*/
	*(--stackEnd) = (unsigned long)0x4;                    /*R4*/
	
	/*执行--操作后，将最终的值保存到task->stack，存储用户数据*/
	task->stack = stackEnd;
	
	/*初始化延时计数器*/
	task->tDelayTick = 0;
	
	/*任务优先级初始化*/
	task->prio = prio;
	
	/*设置同优先级任务时间片*/
	task->slice = TASK_MAX_SLICE_COUNT;
	
	/*初始化任务状态*/
	task->state = SOLAROS_TASK_STATE_READY;
	
	/*初始化任务挂起计数器*/
	task->suspendCount = 0;
	
	/*初始化任务清理函数*/
	task->clean = (void(*)(void *))0;
	
	/*初始化任务清理函数的传入参数*/
	task->cleanParam = (void *)0;
	
	/*初始化任务请求删除标志位，0表示没有被请求删除*/
	task->requestDeleteFlag = 0;
	
	/*初始化延时结点*/
	tNodeInit(&(task->delayNode));
	
	/*初始化优先级队列结点*/
	tNodeInit(&(task->linkNode));	
	
	/*将任务对应的结点插入到任务优先级队列中去*/
	tListInsertAfterLast(&(tTaskList[prio]), &(task->linkNode));
	
	/*将任务置为就绪态*/
	tBitmapSet(&tbitmap, prio);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskSched
*	功能说明  :   任务调度
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskSched(void)
{
	/*获取最高优先级的处于就绪状态的任务*/
	tTask *tempTask;
	
	/*调度锁作为共享资源，访问前先进入临界区*/
	uint32_t status = tTaskEnterCritical();
	
	/*如果调度锁>0，则退出临界区，并且return，不进行任务调度*/
	if(schedLockCount > 0)
	{
		tTaskExitCritical(status);
		return;
	}

	/*获取最高优先级的任务*/
  tempTask = tTaskHighestReady();
	
	/*如果有比当前任务优先级更高的任务*/
	if(currentTask != tempTask)
	{
		nextTask = tempTask;
		
		/*任务指针切换完成后，触发PendSV中断执行任务切换*/
	  tTaskSwitch();
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tSchedLockInit
*	功能说明  :   调度锁初始化
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tSchedLockInit(void)
{
	schedLockCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskSchedDisable
*	功能说明  :   失能任务调度
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*如果调度锁不大于255，对其进行加1操作*/
	if (schedLockCount < 255)
	{
		schedLockCount++;
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskSchedEnable
*	功能说明  :   使能任务调度
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskSchedEnable(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*如果调度锁为0，进行任务调度*/
	if (--schedLockCount == 0)
	{
		tTaskSched();
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskHighestReady
*	功能说明  :   返回最高优先级的任务
*	形    参  :   无
*	返 回 值  :   task：最高优先级的任务
*********************************************************************************************************
*/
tTask *tTaskHighestReady(void)
{
	/*获取位图数据结构中第一个置1的任务的优先级*/
	uint8_t prio = tBitmapGetFirstSet(&tbitmap);
	
	/*从当前优先级对应的任务优先级队列中取出第一个结点firstNode*/
	tNode *node = tListFirstNode(&(tTaskList[prio]));
	
	/*根据此结点，返回此结点对应的任务*/
	return tNodeParent(node, tTask, linkNode);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskDelayedListInit
*	功能说明  :   初始化延时队列
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskDelayedListInit(void)
{
	tListInit(&tTaskDelayedList);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskPrioListInit
*	功能说明  :   初始化优先级队列
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskPrioListInit(void)
{
	uint32_t i = 0;
	for(i = 0; i < TASK_MAX_PRIO_COUNT; i++)
	{
		tListInit(&(tTaskList[i]));
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskStateUnReady
*	功能说明  :   将任务延时挂起
*	形    参  :   task：被延时挂起的任务
*               delayticks：延时时间
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskStateUnReady(tTask *task, uint32_t delayticks)
{
	/*将任务结点从对应的优先级队列中删除*/
	tListRemoveNode(&(tTaskList[task->prio]), &(task->linkNode));
	
	/*当对应的优先级队列中没有任务时，再将对应的位图数据结构复位*/
	if(tTaskList[task->prio].nodeCount == 0)
	{
		tBitmapReset(&tbitmap, task->prio);
	}
	
	/*设置延时时间*/
	task->tDelayTick = delayticks;
	
	/*设置任务状态*/
	task->state |= SOLAROS_TASK_STATE_DELAYED;
	
	/*将该任务插入到任务延时队列的末尾*/
	tListInsertAfterLast(&tTaskDelayedList, &(task->delayNode));
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskStateReady
*	功能说明  :   任务延时完成，将任务置为就绪
*	形    参  :   task：延时完成的任务
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskStateReady(tTask *task)
{
	/*将任务结点插入到对应的任务优先级队列中*/
	tListInsertAfterLast(&(tTaskList[task->prio]), &(task->linkNode));
	
	/*将位图数据结构中对应的任务置1，就绪*/
	tBitmapSet(&tbitmap, task->prio);
	
	/*设置任务状态*/
	task->state &= ~(SOLAROS_TASK_STATE_DELAYED);
	
	/*将该任务从任务延时队列中删除*/
	tListRemoveNode(&tTaskDelayedList, &(task->delayNode));
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskSuspend
*	功能说明  :   挂起某个任务
*	形    参  :   task：将要被挂起的任务
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskSuspend(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*禁止对延时状态的任务执行挂起操作*/
	if(!(task->state & SOLAROS_TASK_STATE_DELAYED))
	{
		/*增加挂起计数器，仅当任务第一次挂起时，才执行任务切换操作*/
		if(++task->suspendCount <= 1)
		{
			/*设置任务进入挂起状态*/
			task->state |= SOLAROS_TASK_STATE_SUSPEND;
			
			/*设置任务为未就绪状态*/
			tListRemoveNode(&(tTaskList[task->prio]), &(task->linkNode));
			if(tTaskList[task->prio].nodeCount == 0)
			{
				tBitmapReset(&tbitmap, task->prio);
			}
			
			/*如果挂起的任务是当前正在运行的任务，就执行任务切换操作*/
			if(task == currentTask)
			{
				tTaskSched();
			}
		}
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskWakeUp
*	功能说明  :   唤醒某个任务
*	形    参  :   task：将要被唤醒的任务
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskWakeUp(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*判断任务是否是挂起状态*/
	if(task->state & SOLAROS_TASK_STATE_SUSPEND)
	{
		/*递减挂起计数器，如果为0，就执行任务唤醒操作*/
		if(--task->suspendCount == 0)
		{
			/*清除挂起标志*/
			task->state &= ~SOLAROS_TASK_STATE_SUSPEND;
			
			/*唤醒任务*/
			tListInsertAfterLast(&(tTaskList[task->prio]), &(task->linkNode));
			tBitmapSet(&tbitmap, task->prio);
			
			/*唤醒过程中可能有更高优先级的任务就绪，执行一次任务调度*/
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskSetDeleteCallFunc
*	功能说明  :   设置任务被删除时要调用的清理函数
*	形    参  :   task：被删除的任务
*               clean：清理函数
*               cleanParam：往清理函数中传递的变量
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskSetDeleteCallFunc(tTask *task, void(*clean)(void *cleanParam), void *cleanParam)
{
	task->clean = clean;
	task->cleanParam = cleanParam;
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskForceDelete
*	功能说明  :   强制删除任务
*	形    参  :   task：被删除的任务
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskForceDelete(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*判断任务是否处于延时状态*/
	if(task->state & SOLAROS_TASK_STATE_DELAYED)
	{
		tListRemoveNode(&tTaskDelayedList, &task->delayNode);
	}
	
	/*判断任务是否是挂起状态，若不是，就说明任务在运行或者就绪状态*/
	else if(!(task->state & SOLAROS_TASK_STATE_SUSPEND))
	{
		tListRemoveNode(&tTaskList[task->prio], &task->linkNode);
		if(tTaskList[task->prio].nodeCount == 0)
		{
			tBitmapReset(&tbitmap, task->prio);
		}
	}
	
	/*执行清理函数*/
	if(task->clean)
	{
		task->clean(task->cleanParam);
	}
	
	/*被删除的正在运行的任务，执行任务切换*/
	if(currentTask == task)
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskRequestDelete
*	功能说明  :   请求删除指定任务
*	形    参  :   task：被删除的任务
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskRequestDelete(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*置位请求删除任务标志位*/
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskIsRequestedFlag
*	功能说明  :   判断当前任务是否被请求删除
*	形    参  :   无
*	返 回 值  :   deleteFlag：请求删除标志位
*********************************************************************************************************
*/
uint8_t tTaskIsRequestedFlag(void)
{
	uint8_t deleteFlag;
	uint32_t status = tTaskEnterCritical();
	
	/*查询当前任务的请求删除标志位是否被置位*/
	deleteFlag = currentTask->requestDeleteFlag;
	
	tTaskExitCritical(status);
	
	/*返回标志位*/
	return deleteFlag;
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskDeleteSelf
*	功能说明  :   任务删除自身
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskDeleteSelf(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*删除自己时，任务已经正在运行，所以要把自己从优先级队列中删除*/
	tListRemoveNode(&tTaskList[currentTask->prio], &currentTask->linkNode);
	if(tTaskList[currentTask->prio].nodeCount == 0)
	{
		tBitmapReset(&tbitmap, currentTask->prio);
	}
	
	/*执行清理函数*/
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);
	}
	
	/*执行任务调度*/
	tTaskSched();
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskGetInfo
*	功能说明  :   获取任务的相关信息
*	形    参  :   task：任务
*               info：保存任务的信息
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskGetInfo(tTask *task, tTaskInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	/*
	  由于堆栈从高地址向低地址增长，所以从堆栈起始位置即低地址
	  开始，统计为0的空间的个数，除以总的堆栈大小，即可得到堆栈的空闲率
	*/
	tTaskStack *stackStart = task->stackStart;
	
	info->prio = task->prio;
	info->slice =task->slice;
	info->state = task->state;
	info->suspendCount = task->suspendCount;
	info->tDelayTick = task->tDelayTick;
	info->stackSize = task->stackSize;
	
	/*初始化freeStack*/
	info->freeStack = 0;
	
	/*从堆栈起始位置进行++，直到遇到非0的数据，并且没有超出堆栈的范围*/
	while((*stackStart++ == 0)&&(stackStart < (task->stackStart + task->stackSize/sizeof(tTaskStack))))
	{
		/*空闲堆栈的个数++*/
		info->freeStack++;
	}
	
	/*换算成字节数*/
	info->freeStack *= sizeof(tTaskStack);
	
	tTaskExitCritical(status);
}

/***************************** SOLAROS (END OF FILE) *********************************/
