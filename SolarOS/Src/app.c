/*
*********************************************************************************************************
*
*	模块名称   :   用户程序模块
*	文件名称   :   app.c
*	版    本   :   V1.0
*	说    明   :   主要用于用户应用程序的创建，本例程创建了4个应用程序，
*                1个空闲任务及相应的任务入口函数，可在逻辑分析仪中查看
*                运行效果
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

/*定义四个任务*/
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;

/*定义四个任务堆栈*/
tTaskStack tTask1Env[SOLAROS_TASK_STACK_SIZE];
tTaskStack tTask2Env[SOLAROS_TASK_STACK_SIZE];
tTaskStack tTask3Env[SOLAROS_TASK_STACK_SIZE];
tTaskStack tTask4Env[SOLAROS_TASK_STACK_SIZE];

/*定义一个空闲任务和空闲任务的堆栈*/
tTask tIdleTask;
tTaskStack tIdleTaskEnv[SOLAROS_IDLETASK_STACK_SIZE];

/*定义四个标志位，分别用于对应任务函数中标志位的翻转指示*/
uint8_t tTaskFlag1 = 0;
uint8_t tTaskFlag2 = 0;
uint8_t tTaskFlag3 = 0;
uint8_t tTaskFlag4 = 0;

/*
*********************************************************************************************************
*	函 数 名  :   task1Entry
*	功能说明  :   任务1入口函数，任务1为最高优先级，执行任务代码前先执行系统时钟时钟配置
*	形    参  :   param，任务函数的入口参数
*	返 回 值  :   无
*********************************************************************************************************
*/
void task1Entry(void *param)
{
	tSetSysTickPeriod(10);
	for(;;)
	{
		tTaskFlag1 = 1;
		tTaskDelay(1);
		tTaskFlag1 = 0;
		tTaskDelay(1);
		
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   task2Entry
*	功能说明  :   任务2入口函数
*	形    参  :   param，任务函数的入口参数
*	返 回 值  :   无
*********************************************************************************************************
*/
void task2Entry(void *param)
{
	for(;;)
	{
		tTaskFlag2 = 1;
		tTaskDelay(1);
		tTaskFlag2 = 0;
		tTaskDelay(1);
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   task3Entry
*	功能说明  :   任务3入口函数
*	形    参  :   param，任务函数的入口参数
*	返 回 值  :   无
*********************************************************************************************************
*/
void task3Entry(void *param)
{
	for(;;)
	{
		tTaskFlag3 = 1;
		tTaskDelay(1);
		tTaskFlag3 = 0;
		tTaskDelay(1);
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   task4Entry
*	功能说明  :   任务4入口函数
*	形    参  :   param，任务函数的入口参数
*	返 回 值  :   无
*********************************************************************************************************
*/
void task4Entry(void *param)
{
	for(;;)
	{
		tTaskFlag4 = 1;
		tTaskDelay(1);
		tTaskFlag4 = 0;
		tTaskDelay(1);	
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tIdleTaskEntry
*	功能说明  :   空闲任务入口函数
*	形    参  :   param，任务函数的入口参数
*	返 回 值  :   无
*********************************************************************************************************
*/
void tIdleTaskEntry(void *param)
{
	for(;;)
	{

	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tInitApp
*	功能说明  :   初始化各个任务
*	形    参  :   无
*	返 回 值  :   无
*********************************************************************************************************
*/
void tInitApp(void)
{

	/*任务初始化*/
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, tTask1Env, sizeof(tTask1Env));
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, tTask2Env, sizeof(tTask2Env));
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 2, tTask3Env, sizeof(tTask3Env));
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 3, tTask4Env, sizeof(tTask4Env));
	
	tTaskInit(&tIdleTask, tIdleTaskEntry, (void *)0, TASK_MAX_PRIO_COUNT-1, tIdleTaskEnv, sizeof(tIdleTaskEnv));
}

/***************************** SOLAROS (END OF FILE) *********************************/
