/*
*********************************************************************************************************
*
*	模块名称   :   事件控制块实现模块。
*	文件名称   :   tEvent.c
*	版    本   :   V1.0
*	说    明   :   事件控制块数据结构的实现，ECB(Event Control Block)由
*                事件类型eventType（信号量、邮箱、存储块）等，以及等待
*                队列waitList组成
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

#include "tEvent.h"
#include "SOLAROS.h"

/*
*********************************************************************************************************
*	函 数 名  :   tEventInit
*	功能说明  :   初始化事件控制块
*	形    参  :   event：待初始化的ECB
*               type：ECB的类型
*	返 回 值  :   无
*********************************************************************************************************
*/
void tEventInit(tEvent *event, tEventType type)
{
	event->type = type;
	tListInit(&event->waitList);
}

/*
*********************************************************************************************************
*	函 数 名  :   tEventWait
*	功能说明  :   将指定任务插入到指定的ECB中
*	形    参  :   event：要插入的ECB
*               task：将要被插入的任务
*               msg：将要传递给任务的消息
*               eventType：时间的类型
*               timeOut：等待超时时间，若为0表示永远等待
*	返 回 值  :   无
*********************************************************************************************************
*/
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t eventType, uint32_t timeOut)
{
	uint32_t status = tTaskEnterCritical();
	
	uint32_t totalTaskCount;
	
	/*设置task相关字段*/
	/*高16位，表示ECB相关的状态*/
	task->state = (eventType << 16);
	task->eventMsg = msg;
	task->waitEvent = event;
	task->eventWaitResult = tErrorNoError;
	
	/*从优先级队列中删除该任务*/
	tListRemoveNode(&tTaskList[task->prio], &task->linkNode);
	if(tTaskList[task->prio].nodeCount == 0)
	{
		tBitmapReset(&tbitmap, task->prio);
	}
	
	//插入到ECB等待队列的末尾
	//tListInsertAfterLast(&task->waitEvent->waitList, &task->linkNode);
	
	/*
	  按照任务优先级的顺序插入任务到ECB的waitlist中
	  思路：插入之前先判断队列中是否有任务，若无，则直接插入；如有任务
	  则遍历队列，将要插入的任务1的优先级1与队列中的任务的优先级进行比
	  较，如优先级1更高，则将任务1插入到相应任务的前面，若遍历到最后，
	  任务1的优先级均小于或等于当前任务的优先级，则将其插入到末尾
	*/
	totalTaskCount = tListCount(&event->waitList);
	if(totalTaskCount == 0)
	{
		
		/*若等待队列中无任务，直接将任务插入到队列末尾*/
		tListInsertAfterLast(&event->waitList, &task->linkNode);
	}
	
	/*等待队列中有任务*/
	else
	{
		uint8_t i = 0;
		
		/*先与第一个结点对应的任务比较*/
		tNode *currentNode = tListFirstNode(&event->waitList);
		for(i = 0; i < totalTaskCount; i++)
		{
			/*取出结点对应的任务*/
			tTask *taskToCompare = tNodeParent(currentNode, tTask, linkNode);
			
			/*若待插入的任务优先级更高*/
			if(task->prio < taskToCompare->prio)
			{
				/*将待插入的任务插入到currentNode的前面，也即currentNode->preNode的后面*/
				tListInsertAfterNode(&event->waitList, currentNode->preNode, &task->linkNode);
				
				/*退出循环*/
				break;
			}
			
			/*if判断条件不满足，移动到下一个结点*/
			currentNode = currentNode->nextNode;
		}
		
		/*若遍历到最后，if仍不满足条件*/
		if(i >= totalTaskCount)
		{
			/*将该任务插入到等待队列的末尾*/
			tListInsertAfterLast(&event->waitList, &task->linkNode);
		}
	
	}
	
	/*如果设置了超时等待*/
	if(timeOut != 0)
	{
		/*设置延时时间*/
		task->tDelayTick = timeOut;
	
		/*改变任务状态为延时状态*/
		task->state |= SOLAROS_TASK_STATE_DELAYED;
	
		/*将任务插入到延时队列*/
		tListInsertAfterLast(&tTaskDelayedList, &(task->delayNode));
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tEventWakeUp
*	功能说明  :   从ECB中唤醒任务
*	形    参  :   event：任务所在的ECB
*               msg：将要传递给任务的消息
*               result：等待的结果
*	返 回 值  :   task：被唤醒的任务
*********************************************************************************************************
*/
tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result)
{
	tTask *task = (tTask *)0;
	tNode *node = (tNode *)0;
	
	uint32_t status = tTaskEnterCritical();
	
	/*从ECB的等待队列中删除第一个结点并返回该结点*/
	if((node = tListRemoveFirstNode(&event->waitList)) != (tNode *)0)
	{
		task = tNodeParent(node, tTask, linkNode);
		
		/*清除该任务对应的ECB*/
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->eventWaitResult = result;
		
		/*清除任务关于ECB的状态*/
		task->state &= ~SOLAROS_TASK_WAIT_MASK;
			
		/*若任务设置了超时等待，将其从延时队列队列中删除*/
		if(task->tDelayTick != 0)
		{
			task->state &= ~SOLAROS_TASK_STATE_DELAYED;
			tListRemoveNode(&tTaskDelayedList, &task->delayNode);
		}
		
		/*将该任务插入到优先级对列中*/
		tListInsertAfterLast(&tTaskList[task->prio], &task->linkNode);
		tBitmapSet(&tbitmap, task->prio);
	}
	
	tTaskExitCritical(status);
	
	/*返回被唤醒的任务*/
	return task;
}

/*
*********************************************************************************************************
*	函 数 名  :   tEventRemoveTask
*	功能说明  :   将任务从ECB中删除
*	形    参  :   task：需要删除的任务
*               msg：将要传递给任务的消息
*               result：等待的结果
*	返 回 值  :   无
*********************************************************************************************************
*/
void tEventRemoveTask(tTask *task, void *msg, uint32_t result)
{
	uint32_t status = tTaskEnterCritical();
	
	tListRemoveNode(&task->waitEvent->waitList, &task->linkNode);
	
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;
	task->eventWaitResult = result;
	task->state &= ~SOLAROS_TASK_WAIT_MASK;
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tEventRemoveAll
*	功能说明  :   删除ECB中的所有任务
*	形    参  :   event：将要清空的ECB
*               msg：将要传递给任务的消息
*               result：等待的结果
*	返 回 值  :   count：ECB中任务的数量
*********************************************************************************************************
*/
uint32_t tEventRemoveAll(tEvent *event, void *msg, uint32_t result)
{
	uint32_t count;
	tNode *node = (tNode *)0;
	
	uint32_t status = tTaskEnterCritical();
	
	/*返回等待的任务的数量*/
	count = event->waitList.nodeCount;
	
	/*开始删除*/
	while((node = tListRemoveFirstNode(&event->waitList)) != (tNode *)0)
	{
		tTask *task = tNodeParent(node, tTask, linkNode);
		
		/*修改task的状态*/
		task->state &= ~SOLAROS_TASK_WAIT_MASK;
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->eventWaitResult = result;
		
		/*如果task还在延时队列中，从延时队列中唤醒*/
		if(task->tDelayTick != 0)
		{
			task->state &= ~SOLAROS_TASK_STATE_DELAYED;
			tListRemoveNode(&tTaskDelayedList, &task->delayNode);
		}
		
		/*将task插入到优先级队列中*/
		tListInsertAfterLast(&tTaskList[task->prio], &task->linkNode);
		tBitmapSet(&tbitmap, task->prio);
	}
	
	tTaskExitCritical(status);
	
	/*返回ECB中任务的数量*/
	return count;
}

/*
*********************************************************************************************************
*	函 数 名  :   tEventWaitCount
*	功能说明  :   返回ECB中在等待的任务的数量
*	形    参  :   event：ECB
*	返 回 值  :   count：ECB中在等待的任务的数量
*********************************************************************************************************
*/
uint32_t tEventWaitCount(tEvent *event)
{
	uint32_t count;
	uint32_t status = tTaskEnterCritical();
	
	count = event->waitList.nodeCount;
	
	tTaskExitCritical(status);
	
	return count;
}

/***************************** SOLAROS (END OF FILE) *********************************/
