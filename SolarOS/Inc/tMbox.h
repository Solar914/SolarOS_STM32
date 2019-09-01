/*
*********************************************************************************************************
*
*	模块名称   :   邮箱相关功能头文件
*	文件名称   :   tMbox.h
*	版    本   :   V1.0
*	说    明   :   定义邮箱数据结构，邮箱信息数据结构，提供邮箱初始化，
*                等待邮箱中的消息，向邮箱中写入消息相关操作API
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

#ifndef _TMBOX_H
#define _TMBOX_H

#include "tEvent.h"

/*------向邮箱中写入数据的方式------*/
#define  tMBOXSendNormal   0x00                         /* 正常方式向消息缓冲区写入消息                        */
#define  tMBOXSendFront    0x01                         /* 后写入的消息先被读取                                */

/*------邮箱数据结构------*/
typedef struct _tMbox
{
	tEvent event;                                         /* ECB                                                 */
	uint32_t count;                                       /* 当前的消息数量                                      */
	uint32_t maxCount;                                    /* 最大的消息数量                                      */
	uint32_t read;                                        /* 读索引                                              */
	uint32_t write;                                       /* 写索引                                              */
	void **msgBuffer;                                     /* 消息缓冲区                                          */
}tMbox;

/*------邮箱消息数据结构------*/
typedef struct _tMboxInfo
{
	uint32_t count;                                       /* 消息缓冲区中消息的数量                              */
	uint32_t maxCount;                                    /* 消息缓冲区最大的消息                                */
	uint32_t taskCount;                                   /* 等待消息的任务的数量                                */
}tMboxInfo;

/*------邮箱初始化、等待、唤醒等API------*/
void tMboxInit(tMbox *Mbox, void **msgBuffer, uint32_t maxCount);
uint32_t tMboxWait(tMbox *mbox, void **msg, uint32_t waitTicks);
uint32_t tMboxNoWait(tMbox *mbox, void **msg);
uint32_t tMboxNotify(tMbox *mbox, void *msg, uint32_t notifyOption);
void tMboxClear(tMbox *mbox);
uint32_t tMboxDelete(tMbox *mbox);
void tMboxGetInfo(tMbox *mbox, tMboxInfo *info);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
