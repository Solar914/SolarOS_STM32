/*
*********************************************************************************************************
*
*	模块名称   :   任务相关功能头文件
*	文件名称   :   tTask.h
*	版    本   :   V1.0
*	说    明   :   定义任务数据结构，任务状态数据结构，提供任务初始化，任务调度，调度锁
*                使能/失能相关操作API
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

#ifndef _TTASK_H
#define _TTASK_H

#include <stdint.h>
#include "tList.h"
#include "tBitmap.h"
#include "tConfig.h"

/*------任务状态宏定义------*/
#define SOLAROS_TASK_STATE_READY        0                    /* 最低位定义任务是否是READY状态，0表示就绪            */
#define SOLAROS_TASK_STATE_DELAYED      (1<<1)               /* 左移1位定义任务是否是DELAY状态，1表示延时状态       */
#define SOLAROS_TASK_STATE_SUSPEND      (1<<2)               /* 左移2位定义任务是否是挂起状态，1表示挂起状态        */
#define SOLAROS_TASK_STATE_DESTROYED    (1<<3)               /* 左移3位表示任务是否被销毁，1表示被销毁              */
#define SOLAROS_TASK_WAIT_MASK          (0XFF << 16)         /* 任务状态的高16位的掩码                              */

/*------任务堆栈定义------*/
typedef uint32_t tTaskStack;

/*------前向引用的方式声明tEvent结构体------*/
struct _tEvent;

/*------任务数据结构------*/
typedef struct _tTask
{
	tTaskStack *stack;                                         /* 任务堆栈，定义在起始位置，和任务变量同地址          */ 
	tTaskStack *stackStart;                                    /* 堆栈的起始地址                                      */ 
	uint32_t stackSize;                                        /* 堆栈的大小，以字节为单位                            */ 
	tTaskStack tDelayTick;                                     /* 任务延时时间                                        */ 
	uint8_t prio;                                              /* 任务优先级                                          */ 
	tNode delayNode;                                           /* 任务延时队列结点                                    */ 
	tNode linkNode;                                            /* 任务优先级队列结点                                  */ 
	uint32_t slice;                                            /* 同优先级任务的时间片                                */ 
	uint32_t state;                                            /* 任务状态                                            */ 
	uint32_t suspendCount;                                     /* 任务挂起次数计数器                                  */ 
	void(*clean)(void *cleanParam);                            /* 任务被删除时调用的清理函数                          */ 
	void *cleanParam;                                          /* 传递给清理函数的参数                                */ 
	uint8_t requestDeleteFlag;                                 /* 请求删除标志，非0表示请求删除                       */ 
	struct _tEvent *waitEvent;                                 /* 添加事件控制块，表示此任务正在等待哪一个事件控制块  */ 
	void *eventMsg;                                            /* 存放事件消息                                        */ 
	uint32_t eventWaitResult;                                  /* 等待的结果，有相应的错误码与之对应                  */ 
}tTask;

/*------任务信息数据结构------*/
typedef struct _tTaskInfo
{
	tTaskStack tDelayTick;
	uint8_t prio;
	uint32_t slice;
	uint32_t state;
	uint32_t suspendCount;
	uint32_t stackSize;
	uint32_t freeStack;
}tTaskInfo;

/*------扩展变量作用域------*/ 
extern tTask *currentTask;                                   /* 指向当前正在运行的任务的指针                        */ 
extern tTask *nextTask;                                      /* 指向下一即将运行的任务的指针                        */
extern uint32_t tickCount;                                   /* 计数器，用于统计CPU利用率                           */
extern uint8_t schedLockCount;                               /* 调度锁计数器                                        */
extern tBitmap tbitmap;                                      /* 位图变量                                            */
extern tList tTaskList[TASK_MAX_PRIO_COUNT];                 /* 任务优先级链表                                      */
extern tList tTaskDelayedList;                               /* 任务延时链表                                        */

/*
  已知node结点，已知结构体parent以及结构体成员name，可求出name到parent
  的偏移量offset，node减去偏移量offset，即可得到node对应的parent类型的变量
*/
#define tNodeParent(node, parent, name)   (parent*)((uint32_t)node - (uint32_t)&((parent*)0)->name)

/*------任务&APP初始化相关API------*/
void tTaskInit(tTask *task, void(*entry)(void *), void *param, uint8_t prio, tTaskStack *stackStart, uint32_t stackSize);
void tTaskSched(void);
void tInitApp(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
