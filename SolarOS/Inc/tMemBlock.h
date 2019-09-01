/*
*********************************************************************************************************
*
*	模块名称   :   存储块相关功能头文件
*	文件名称   :   tMemBlock.h
*	版    本   :   V1.0
*	说    明   :   定义存储块数据结构，存储块信息数据结构，提供存储块初始化，
*                等待空闲存储区，将空闲存储区放入存储块相关操作API
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

#ifndef _TMEMBLOCK_H
#define _TMEMBLOCK_H

#include "SolarOS.h"
#include "tList.h"
#include "tEvent.h"

/*------定长存储块数据结构------*/
typedef struct _tMemBlock
{
	tEvent event;                                    /* ECB,用于管理任务队列                                     */
	void *memStart;                                  /* 内存块的起始地址                                         */
	uint32_t blockSize;                              /* 单个内存块的大小                                         */
	uint32_t blockCnt;                               /* 内存块的数量                                             */
	tList blockList;                                 /* 将各个内存块用链表链接起来进行管理                       */
}tMemBlock;

/*------存储块信息数据结构------*/
typedef struct _tMemBlockInfo
{
	uint32_t BlockCnt;                               /* 可用存储块的数量                                         */
  uint32_t maxBlockCnt;                            /* 最大的存储块的数量                                       */
	uint32_t blockSize;                              /* 单个存储块的大小                                         */
	uint32_t taskCount;                              /* 等待存储块的任务的数量                                   */
}tMemBlockInfo;

/*------定长存储块初始化、等待、唤醒等API------*/
void tMemBlockInit(tMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait(tMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet(tMemBlock *memBlock, uint8_t **mem);
void tMemBlockNotify(tMemBlock *memBlock, uint8_t *mem);
void tMemBlockGetInfo(tMemBlock *memBlock, tMemBlockInfo *info);
uint32_t tMemBlockDelete(tMemBlock *memBlock);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
