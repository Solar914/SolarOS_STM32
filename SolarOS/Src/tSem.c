/*
*********************************************************************************************************
*
*	模块名称   :   信号量实现模块。
*	文件名称   :   tSem.c
*	版    本   :   V1.0
*	说    明   :   定义信号量数据结构，信号量的初始化，等待信号量、
*                释放信号量、删除信号量的API的实现
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

#include "tSem.h"
#include "SOLAROS.h"

#if SOLAROS_ENABLE_SEM == 1

/*
*********************************************************************************************************
*	函 数 名  :   tSemInit
*	功能说明  :   计数信号量初始化
*	形    参  :   sem：计数信号量
*               startCount：初始计数值
*               maxCount：最大计数值
*	返 回 值  :   无
*********************************************************************************************************
*/
void tSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount)
{
	/*ECB初始化*/
	tEventInit(&sem->event, tEventTypeSem);
	
	/*计数器maxCount初始化*/
	sem->maxCount = maxCount;
	
	/*计数器count初始化，maxCount == 0表示对count无限制，count=startCount*/
	if(maxCount == 0)
	{
		sem->count = startCount;
	}
	else
	{
		/*(条件表达式) ? (真) : (假) */
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
	
}

/*
*********************************************************************************************************
*	函 数 名  :   tSemWait
*	功能说明  :   等待某个信号量，可设置超时时间
*	形    参  :   sem：计数信号量
*               timeOut：超时时间
*	返 回 值  :   eventWaitResult：等待结果
*********************************************************************************************************
*/
uint32_t tSemWait(tSem *sem, uint32_t timeOut)
{
	uint32_t status = tTaskEnterCritical();
	
	/*如果信号量计数器大于0，就进行--操作，消耗掉一个资源*/
	if(sem->count > 0)
	{
		--sem->count;
		tTaskExitCritical(status);
		
		/*返回无错误*/
		return tErrorNoError;
	}
	
	/*如果信号量为0，就说明无资源可用，将任务插入到ECB的等待队列中*/
	else
	{
		/*向等待队列中插入任务，超时时间设置为0表示永远等待*/
		tEventWait(&sem->event, currentTask, (void*)0, tEventTypeSem, timeOut);
		
		tTaskExitCritical(status);
		
		/*将任务插入到等待队列后，会有其它任务就绪，执行任务调度*/
		tTaskSched();
		
		/*当任务重新执行到这里时，返回任务的等待结果*/
		return currentTask->eventWaitResult;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tSemNoWaitGet
*	功能说明  :   无等待的获取某个信号量
*	形    参  :   sem：计数信号量
*	返 回 值  :   eventWaitResult：等待结果
*********************************************************************************************************
*/
uint32_t tSemNoWaitGet(tSem *sem)
{
	uint32_t status = tTaskEnterCritical();
	
	/*如果信号量计数器>0，就进行--操作，消耗掉一个资源*/
	if(sem->count > 0)
	{
		--sem->count;
		tTaskExitCritical(status);
		
		/*返回无错误*/
		return tErrorNoError;
	}
	
	/*如果信号量计数器为0，说明无资源可用，直接返回结果tErrorResourceUnavaliable*/
	else
	{
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tSemNotify
*	功能说明  :   释放某个信号量
*	形    参  :   sem：计数信号量
*	返 回 值  :   无
*********************************************************************************************************
*/
void tSemNotify(tSem *sem)
{
	uint32_t status = tTaskEnterCritical();
	
	/*判断该信号量的ECB的等待对列中是否有任务*/
	if(sem->event.waitList.nodeCount > 0)
	{
		/*有任务的话，就直接唤醒该ECB中的第一个任务*/
		tTask *task = tEventWakeUp(&sem->event, (void*)0, tErrorNoError);
		
		/*唤醒后执行任务调度，任务优先级越高，prio数值越小*/
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}	
		tTaskExitCritical(status);
	}
	
	/*没有任务的话，就对信号量计数器进行++操作，新增一个资源*/
	else
	{
		++sem->count;
		if((sem->maxCount != 0) && (sem->count > sem->maxCount))
		{
			sem->count = sem->maxCount;
		}
		tTaskExitCritical(status);
	}
		
}

/*
*********************************************************************************************************
*	函 数 名  :   tSemGetInfo
*	功能说明  :   获取某个信号量的信息
*	形    参  :   sem：计数信号量
*               info：存储获取到的信息
*	返 回 值  :   无
*********************************************************************************************************
*/
void tSemGetInfo(tSem *sem, tSemInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	/*信号量当前计数值*/
	info->count = sem->count;
	
	/*信号量最大计数值*/
	info->maxCount = sem->maxCount;
	
	/*信号量的ECB中在等待的任务的数量*/
	info->taskCount = sem->event.waitList.nodeCount;
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tSemDelete
*	功能说明  :   删除某个信号量
*	形    参  :   sem：计数信号量
*	返 回 值  :   返回信号量的ECB中在等待的任务的数量
*********************************************************************************************************
*/
uint32_t tSemDelete(tSem *sem)
{
	uint32_t count;
	uint32_t status = tTaskEnterCritical();
	
	/*从信号量的ECB等待队列中删除所有任务*/
	count = tEventRemoveAll(&sem->event, (void *)0, tErrorDel);
	
	/*信号量计数器清零*/
	sem->count = 0;
	
	tTaskExitCritical(status);
	
	/*如果有任务被唤醒，执行任务调度*/
	if(count > 0)
	{
		tTaskSched();
	}
	return count;
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
