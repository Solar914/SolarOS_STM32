/*
*********************************************************************************************************
*
*	模块名称   :   系统参数设置头文件
*	文件名称   :   tConfig.h
*	版    本   :   V1.0
*	说    明   :   定义系统支持的优先级数量，任务堆栈大小，内核功能的裁剪设置
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

#ifndef __TCONFIG_H
#define __TCONFIG_H

/*------系统优先级、任务堆栈大小配置------*/
#define TASK_MAX_PRIO_COUNT                    32            /* 定义系统支持的最大优先级数量              */
#define SOLAROS_TASK_STACK_SIZE                1024          /* 定义普通任务堆栈大小                      */
#define SOLAROS_IDLETASK_STACK_SIZE            1024          /* 定义空闲任务堆栈大小                      */
#define TASK_MAX_SLICE_COUNT                   10            /* 定义同优先级任务的最大时间片              */
#define SOLAROS_SOFTTIMER_TASK_PRIO            1             /* 定时soft定时器的优先级                    */
#define SOLAROS_SOFTTIMER_TASK_STACK_SIZE      1024          /* 定义soft定时器任务的堆栈大小              */

/*------条件编译控制开关------*/
#define SOLAROS_ENABLE_SEM                     1             /* 信号量使能开关                            */
#define SOLAROS_ENABLE_MBOX                    1             /* 邮箱使能开关                              */
#define SOLAROS_ENABLE_MEMBLOCK                1             /* 存储块使能开关                            */
#define SOALROS_ENABLE_FLAGGROUP               1             /* 事件标志组使能开关                        */
#define SOLAROS_ENABLE_MUTEX                   1             /* 互斥信号量使能开关                        */
#define SOLAROS_ENABLE_TIMER                   1             /* 软/硬定时器使能开关                       */
#define SOLAROS_ENABLE_THOOKS                  1             /* hooks函数使能开关                         */

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
