/*
*********************************************************************************************************
*
*	模块名称   :   事件控制块相关功能头文件
*	文件名称   :   tEvent.h
*	版    本   :   V1.0
*	说    明   :   定义事件控制块数据结构、事件类型数据结构，提供ECB初始化，ECB
*                等待，ECB唤醒相关操作API
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

#ifndef _TEVENT_H
#define _TEVENT_H

#include "tList.h"
#include "tTask.h"

/*------错误码数据结构------*/
typedef enum _tError
{
	tErrorNoError = 0,                                     /* 没有错误                                          */
	tErrorTimeout,                                         /* 超时错误                                          */
	tErrorResourceUnavaliable,                             /* 无资源可用                                        */
	tErrorDel,                                             /* 被删除                                            */
	tErrorResourceFull,                                    /* 资源满                                            */
}tError;

/*------事件类型数据结构------*/
typedef enum _tEventType
{ 
	tEventTypeUnknown = 0,                                 /* 未知类型                                          */
	tEventTypeSem,                                         /* 信号量                                            */
	tEventTypeMbox,                                        /* 邮箱                                              */
	tEventTypeMemBlock,                                    /* 存储块                                            */
	tEventTypeFlagGroup,                                   /* 事件标志组                                        */
	tEventTypeMutex,                                       /* 互斥信号量                                        */
}tEventType;

/*------ECB数据结构------*/
typedef struct _tEvent
{
	tEventType type;                                       /* 事件类型                                          */
	tList waitList;                                        /* 任务等待队列                                      */
}tEvent;

/*------ECB初始化、等待、唤醒等API------*/
void tEventInit(tEvent *event, tEventType type);
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t eventType, uint32_t timeOut);
tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result);
void tEventRemoveTask(tTask *task, void *msg, uint32_t result);
uint32_t tEventRemoveAll(tEvent *event,void *msg, uint32_t result);
uint32_t tEventWaitCount(tEvent *event);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
