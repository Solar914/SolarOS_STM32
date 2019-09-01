/*
*********************************************************************************************************
*
*	模块名称   :   软/硬定时器相关功能头文件
*	文件名称   :   tTimer.h
*	版    本   :   V1.0
*	说    明   :   定义定时器状态，定时器数据结构，提供定时器初始化，定时
*                器的启动/停止，遍历定时器相关操作API     
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

#ifndef _TTIMER_H
#define _TTIMER_H

#include "tEvent.h"

/*------定时器类型宏定义------*/
#define  TIMER_CONFIG_TYPE_HARD       (1<<0)          /* Hard为1                                       */
#define  TIMER_CONFIG_TYPE_SOFT       (0<<0)          /* Soft为0                                       */

/*------定时器状态数据结构------*/
typedef enum _tTimerState
{
	tTimerCreated,                                      /* 创建                                          */
	tTimerStarted,                                      /* 起始                                          */
	tTimerRunning,                                      /* 运行                                          */
	tTimerStopped,                                      /* 停止                                          */
	tTimerDestroyed,                                    /* 销毁                                          */
}tTimerState;

/*------定时器数据结构------*/
typedef struct _tTimer
{
	tNode linkNode;                                     /* 链表结点                                      */
	uint32_t startDelayTicks;                           /* 初始延时时间                                  */
	uint32_t durationTicks;                             /* 运行周期                                      */
	uint32_t delayTicks;                                /* 递减计数器                                    */
	void (*timerFunc)(void *arg);                       /* 回调函数                                      */
	void *arg;                                          /* 回调函数的参数                                */
	uint8_t config;                                     /* hardTimer or softTimer                        */
	tTimerState state;                                  /* 定时器状态                                    */
}tTimer;

/*------定时器初始化、启动、停止等API------*/
void tTimerInit(tTimer *timer, uint32_t startDelayTicks, uint32_t durationTicks,
	              void (*timerFunc)(void *arg), void *arg, uint32_t config);
void tTimerStart(tTimer *timer);
void tTimerStop(tTimer *timer);
void tTimerTraverseList(tList *tTimerList);
void tTimerHardTickProc(void);
void tTimerModuleInit(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
