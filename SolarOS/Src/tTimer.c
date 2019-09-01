/*
*********************************************************************************************************
*
*	模块名称   :   定时器实现模块。
*	文件名称   :   tTimer.c
*	版    本   :   V1.0
*	说    明   :   定义定时器数据结构，软定时器在对应任务中进行扫描，刷新时间
*                可能会有延后，硬定时器嘀嗒中断中进行扫描，要求其回调函数不能
*                太复杂
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

#include "tTimer.h"
#include "SOLAROS.h"
#include "tSem.h"

#if SOLAROS_ENABLE_TIMER == 1

/*定义hard定时器链表*/
tList tTimerHardList;

/*定义soft定时器链表*/
tList tTimerSoftList;

/*创建soft定时器资源保护信号量*/
tSem tTimerSoftProtectSem;

/*创建hard定时器资源保护信号量*/
tSem tTimerHardProtectSem;

/*定义soft定时器任务*/
tTask tTimerSoftTask;

/*定义soft定时器任务的堆栈*/
tTaskStack tTimerSoftTaskStack[SOLAROS_SOFTTIMER_TASK_STACK_SIZE];

/*
*********************************************************************************************************
*	函 数 名  :   tTimerInit
*	功能说明  :   定时器初始化，soft定时器通过soft定时器任务去更新，hard定时器通过系统时钟中断去更新
*	形    参  :   timer：           定时器
*               startDelayTicks： 初始延时时间
*               durationTicks：   周期运行时间
*               timerFunc：       定时器回调函数
*               arg：             回调函数的参数
*               config：          定时器配置：soft or hard
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerInit(tTimer *timer, uint32_t startDelayTicks, uint32_t durationTicks,
	              void (*timerFunc)(void *arg), void *arg, uint32_t config)
{
	/*初始化结点*/
	tNodeInit(&timer->linkNode);
	
	/*初始化初始延时时间*/
	timer->startDelayTicks = startDelayTicks;
	
	/*初始化周期运行时间*/
	timer->durationTicks = durationTicks;
	
	/*初始化回调函数*/
	timer->timerFunc = timerFunc;
	
	/*初始化回调函数的参数*/
	timer->arg = arg;
	
	/*配置定时器模式：Soft or Hard*/
	timer->config = config;
	
	/*设置定时器状态：创建*/
	timer->state = tTimerCreated;
	
	/*判断初始延时时间是否为0*/
	if(startDelayTicks == 0)
	{
		/*递减计数器的初始值为周期运行时间的值*/
		timer->delayTicks = durationTicks;
	}
	
	/*不为0*/
	else
	{
		/*递减计数器的初始值为初始延时时间的值*/
		timer->delayTicks = startDelayTicks;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTimerStart
*	功能说明  :   启动定时器，将定时器插入到定时器链表中
*	形    参  :   timer：定时器
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerStart(tTimer *timer)
{
	switch(timer->state)
	{
		/*定时器处于创建完成/停止状态*/
		case tTimerCreated:
		case tTimerStopped:
			
			/*设置状态为启动状态*/
			timer->state = tTimerStarted;
		
			/*若设置有启动延时，则重新对启动延时进行赋值*/
			timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
		
			/*判断是否为hard定时器*/
			if(timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				/*hard定时器在系统节拍中断中调用，用临界区的方式对hard定时器的链表进行保护*/
				uint32_t status = tTaskEnterCritical();
				
				/*将该定时器插入到hard定时器链表中*/
				tListInsertAfterLast(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			
			/*soft定时器*/
			else
			{
				#if SOLAROS_ENABLE_SEM == 1
				
				/*等待soft定时器的保护信号量*/
				tSemWait(&tTimerSoftProtectSem, 0);	
				#endif
				
				/*将该定时器插入到soft定时器链表中*/
				tListInsertAfterLast(&tTimerSoftList, &timer->linkNode);
				#if SOLAROS_ENABLE_SEM == 1
				tSemNotify(&tTimerSoftProtectSem);
				#endif
			}
		break;
		
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTimerStop
*	功能说明  :   停止定时器，将定时器从定时器链表中删除
*	形    参  :   timer：定时器
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerStop(tTimer *timer)
{
	switch(timer->state)
	{
		case tTimerStarted:
		case tTimerRunning:
			
			/*改变定时器状态*/
			timer->state = tTimerStopped;
			
			/*是否为hard定时器*/
			if(timer->config&TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = tTaskEnterCritical();
				
				/*从hard定时器链表中删除此定时器*/
				tListRemoveNode(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			
			/*soft定时器*/
			else
			{
				#if SOLAROS_ENABLE_SEM == 1
				tSemWait(&tTimerSoftProtectSem, 0);
				#endif
				/*从soft定时器链表中删除此定时器*/
				tListRemoveNode(&tTimerSoftList, &timer->linkNode);
				#if SOLAROS_ENABLE_SEM == 1
				tSemNotify(&tTimerSoftProtectSem);
				#endif
			}
		break;
		
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTimerTraverseList
*	功能说明  :   遍历定时器链表
*	形    参  :   tTimerList：定时器链表，soft or hard
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerTraverseList(tList *tTimerList)
{
	tNode *node;
	tTimer *timer;
	
	/*遍历定时器链表中的每个node*/
	for(node = tTimerList->headNode.nextNode; node != &(tTimerList->headNode); node = node->nextNode)
	{
		/*根据node获取对应的timer*/
		timer = tNodeParent(node, tTimer, linkNode);
		
		/*--递减计数器，若为0则执行相应操作*/
		if(--timer->delayTicks == 0)
		{
			/*改变定时器状态*/
			timer->state = tTimerRunning;
		
			/*执行定时器回调函数*/
			timer->timerFunc(timer->arg);
			
			/*若定时器为周期性运行，重新赋值递减计数器*/
			if(timer->durationTicks)
			{
				timer->delayTicks = timer->durationTicks;
			}
			
			/*若定时器无周期性运行，只在启动时运行一次*/
			else
			{
				/*将此定时器从链表中删除*/
				tListRemoveNode(tTimerList, &timer->linkNode);
				
				/*置为停止状态*/
				timer->state = tTimerStopped;
				
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTimerSoftTaskEntry
*	功能说明  :   soft定时器任务入口函数
*	形    参  :   param：入口参数
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerSoftTaskEntry(void *param)
{
	for(;;)
	{
		/*等待系统时钟中断先遍历hard定时器，然后hard定时器发来信号量*/
		#if SOLAROS_ENABLE_SEM == 1
		tSemWait(&tTimerHardProtectSem, 0);
		#endif
		
		/*等待soft定时器任务初始化完成后发来第一个信号量*/
		#if SOLAROS_ENABLE_SEM == 1
		tSemWait(&tTimerSoftProtectSem, 0);
		#endif
		
		/*遍历soft定时器的链表*/
		tTimerTraverseList(&tTimerSoftList);
		
		/*释放信号量*/
		#if SOLAROS_ENABLE_SEM == 1
		tSemNotify(&tTimerSoftProtectSem);
		#endif
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTimerHardTickProc
*	功能说明  :   hard定时器在系统时钟中断中调用
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerHardTickProc(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*遍历hard定时器链表*/
	#if SOLAROS_ENABLE_SEM == 1
	tTimerTraverseList(&tTimerHardList);
	#endif
	
	tTaskExitCritical(status);
	
	/*释放节拍中断保护信号量，通知soft定时器任务运行*/
	#if SOLAROS_ENABLE_SEM == 1
	tSemNotify(&tTimerHardProtectSem);
	#endif
}

/*
*********************************************************************************************************
*	函 数 名  :   tTimerModuleInit
*	功能说明  :   软件定时器初始化，包含soft和hard定时器
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTimerModuleInit(void)
{
	tListInit(&tTimerSoftList);
	tListInit(&tTimerHardList);
	
	/*soft定时器信号量初始化值为1，用于激活soft定时器的任务*/
	#if SOLAROS_ENABLE_SEM == 1
	tSemInit(&tTimerSoftProtectSem, 1, 1);
	tSemInit(&tTimerHardProtectSem, 0, 0);
	#endif
	
	/*初始化soft定时器的任务*/
	tTaskInit(&tTimerSoftTask, tTimerSoftTaskEntry, (void *)0, SOLAROS_SOFTTIMER_TASK_PRIO, 
		        tTimerSoftTaskStack, sizeof(tTimerSoftTaskStack));
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
