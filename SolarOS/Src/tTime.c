/*
*********************************************************************************************************
*
*	模块名称   :   系统时钟实现模块。
*	文件名称   :   tTime.c
*	版    本   :   V1.0
*	说    明   :   设置系统嘀嗒时钟的心跳为100hz，在嘀嗒中断中主要处理延时递减，
*                同优先级任务时间片轮转，任务调度
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
#include "tEvent.h"
#include <ARMCM3.h>

/*空闲任务运行次数计数器*/
uint32_t idleCount;

/*空闲任务运行最大次数计数器*/
uint32_t idleMaxCount;

/*系统tick计数器*/
uint32_t tickCount;

/*idleTask与系统时钟同步标志*/
uint8_t sysTickSyncFlag;

/*CPU使用率*/
float cpuUsage;

/*
*********************************************************************************************************
*	函 数 名  :   tGetCPUUsage
*	功能说明  :   计算CPU利用率，第一步：与系统时钟同步，第二步：在只运行空闲任务的情况下，统计空闲
                任务运行的总次数，第三步：开启多任务，每秒更新空闲任务运行的次数，从而计算出CPU利用率
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tGetCPUUsage(void)
{
	/*系统时钟第一次启动*/
	if(sysTickSyncFlag == 0)
	{
		/*第一次运行时，将同步标志位置1*/
		sysTickSyncFlag = 1;
		
		/*重新初始化tickCount*/
		tickCount = 0;
		
		/*退出*/
		return;
	}
	
	/*1S钟时间到*/
	if(tickCount == 100)
	{
		/*赋值空闲任务最大计数器*/
		idleMaxCount = idleCount;
		
		/*清零空闲任务计数器*/
		idleCount = 0;
		
		/*最大值已获得，开启任务调度*/
		tTaskSchedEnable();
	}
	
	/*每隔1S钟统计下CPU使用率*/
	else if(tickCount % 100 == 0)
	{
		cpuUsage = 100.0 - (idleCount * 100.0)/idleMaxCount;
		idleCount = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tTaskDelay
*	功能说明  :   设置任务延时时间，并将CPU的使用权交给其它任务
*	形    参  :   delay：延时多少个系统时钟，当前的每个系统时钟为10ms
*	返 回 值  :   无
*********************************************************************************************************
*/
void tTaskDelay(uint32_t delay)
{

	uint32_t status = tTaskEnterCritical();
	
	/*任务延时挂起*/
	tTaskStateUnReady(currentTask, delay);
	
	/*任务挂起之后，执行任务调度*/
	tTaskSched();
	
	tTaskExitCritical(status); 
}

/*
*********************************************************************************************************
*	函 数 名  :   tDelayTickHandler
*	功能说明  :   系统时钟中断处理函数，第一：遍历延时队列，第二：更新同优先级任务的时间片，第三：
                遍历延时队列的过程中有任务被唤醒，执行任务调度
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tDelayTickHandler(void)
{
	tNode *node;
	
	uint32_t status = tTaskEnterCritical();
	
	/*遍历任务延时队列，从firstNode开始，到lastNode结束*/
	for(node=tTaskDelayedList.headNode.nextNode; node!=&(tTaskDelayedList.headNode); node=node->nextNode)
	{
		/*获取任务延时结点对应的任务*/
		tTask *task = tNodeParent(node, tTask, delayNode);
		
		/*若延时结束，将相应任务置为就绪*/
		if(--task->tDelayTick == 0)
		{
			
			/*任务进行wait时设置的等待时间到，如果当前任务对应的ECB还没有被复位，即此任务还没有被wakeup*/
			if(task->waitEvent)
			{
				
				/*直接从ECB中删除该任务，并将等待结果设置为超时错误*/
				tEventRemoveTask(task, (void *)0, tErrorTimeout);
			}
			
			/*延时时间到，唤醒任务*/
			tTaskStateReady(task);
		}
	}
	
	/*如果当前任务的时间片用完，就切换到同优先级的下一任务*/
	if(--currentTask->slice == 0)
	{
		
		/*在当前任务优先级队列中，删除当前时间片用完的结点*/
		tListRemoveNode(&(tTaskList[currentTask->prio]), &(currentTask->linkNode));
		
		/*将此时间片用完的结点插入到任务优先级队列的末尾*/
		tListInsertAfterLast(&(tTaskList[currentTask->prio]), &(currentTask->linkNode));
		
		/*重新赋值当前任务的时间片*/
		currentTask->slice = TASK_MAX_SLICE_COUNT;
	}
	
  tTaskExitCritical(status); 
	
	/*任务调度*/
	tTaskSched();
}

/*
*********************************************************************************************************
*	函 数 名  :   tSetSysTickPeriod
*	功能说明  :   设置系统时钟
*	形    参  :   ms：以ms为单位
*	返 回 值  :   无
*********************************************************************************************************
*/
void tSetSysTickPeriod(uint32_t ms)
{
  SysTick->LOAD  = ms * SystemCoreClock / 1000 - 1; 
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
  SysTick->VAL   = 0;                           
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk; 
}

/*
*********************************************************************************************************
*	函 数 名  :   SysTick_Handler
*	功能说明  :   系统时钟中断服务程序
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void SysTick_Handler(void)
{
	tDelayTickHandler();
}

/***************************** SOLAROS (END OF FILE) *********************************/
