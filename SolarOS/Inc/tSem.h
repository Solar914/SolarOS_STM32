/*
*********************************************************************************************************
*
*	模块名称   :   信号量相关功能头文件
*	文件名称   :   tSem.h
*	版    本   :   V1.0
*	说    明   :   定义信号量数据结构，信号量信息数据结构，提供信号量初始化，
*                等待信号量，释放信号量相关操作API
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

#ifndef _TSEM_H
#define _TSEM_H

#include "tEvent.h"

/*------sem数据结构------*/
typedef struct _tSem
{
	tEvent event;                                    /* ECB                                                 */    
	uint32_t count;                                  /* 计数器                                              */
	uint32_t maxCount;                               /* 计数器最大值                                        */
}tSem;

/*------信号量信息数据结构------*/
typedef struct _tSemInfo
{
	uint32_t count;                                  /* 信号量计数器的初始值                                */ 
	uint32_t maxCount;                               /* 信号量计数器的最大值                                */ 
	uint32_t taskCount;                              /* ECB等待队列中的任务数量                             */ 
}tSemInfo;

/*------sem初始化、等待、唤醒等API------*/
void tSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount);
uint32_t tSemWait(tSem *sem, uint32_t timeOut);
uint32_t tSemNoWaitGet(tSem *sem);
void tSemNotify(tSem *sem);
void tSemGetInfo(tSem *sem, tSemInfo *info);
uint32_t tSemDelete(tSem *sem);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
