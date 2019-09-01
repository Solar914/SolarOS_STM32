/*
*********************************************************************************************************
*
*	模块名称   :   系统核心功能头文件
*	文件名称   :   SOLAROS.h
*	版    本   :   V1.0
*	说    明   :   定义系统核心功能，包括延时、优先级队列初始化，设置
*                系统心跳，临界区操作，任务相关操作等
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

#ifndef __SOLAROS_H
#define __SOLAROS_H

#include <stdint.h>
#include "tTask.h"

/*------扩展变量作用域------*/
extern uint8_t sysTickSyncFlag;
extern uint32_t idleCount;

/*------任务延时队列和任务优先级队列初始化------*/
void tTaskDelayedListInit(void);
void tTaskPrioListInit(void);

/*------任务的挂起与恢复------*/
void tTaskStateUnReady(tTask *task, uint32_t delayticks);
void tTaskStateReady(tTask *task);

/*------设置OS任务切换的时钟节拍，任务延时，时钟节拍中断处理------*/
void tSetSysTickPeriod(uint32_t ms);
void tTaskDelay(uint32_t delay);
void tDelayTickHandler(void);

/*------临界区相关API------*/
uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

/*------调度锁相关API------*/
void tSchedLockInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);

/*------获取最高优先级任务------*/
tTask *tTaskHighestReady(void);

/*------任务挂起与唤醒------*/
void tTaskSuspend(tTask *task);
void tTaskWakeUp(tTask *task);

/*------任务删除操作------*/
void tTaskSetDeleteCallFunc(tTask *task, void(*clean)(void *cleanParam), void *cleanParam);
void tTaskForceDelete(tTask *task);
void tTaskRequestDelete(tTask *task);
uint8_t tTaskIsRequestedFlag(void);
void tTaskDeleteSelf(void);

/*------获取任务信息------*/
void tTaskGetInfo(tTask *task, tTaskInfo *info);

/*------多任务运行与切换------*/
void tTaskRunFirst(void);
void tTaskSwitch(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
