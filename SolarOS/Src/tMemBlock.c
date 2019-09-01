/*
*********************************************************************************************************
*
*	模块名称   :   存储块实现模块。
*	文件名称   :   tMemBlock.c
*	版    本   :   V1.0
*	说    明   :   定义存储块数据结构，存储块初始化，从存储块中获取空闲存储区，
*                将空闲存储区放入存储块，获取存储块信息，删除存储块等API的实现
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

#include "tMemBlock.h"

#if SOLAROS_ENABLE_MEMBLOCK == 1

/*
*********************************************************************************************************
*	函 数 名  :   tMemBlockInit
*	功能说明  :   存储块初始化
*	形    参  :   memBlock：存储块
*               memStart：存储块的起始地址
*               blockSize：单个存储块的大小
*               blockCnt：存储块的数量
*	返 回 值  :   无
*********************************************************************************************************
*/
void tMemBlockInit(tMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt)
{
	/*存储块的起始地址*/
	uint8_t *memBlockStart = memStart;
	
	/*存储块的结束地址*/
	uint8_t *memBlockEnd = memStart + blockSize * blockCnt;
	
	/*首先判断单个存储块的大小，不可小于一个tNode的大小，因为每个小存储块至少包含一个tNode*/
	if(blockSize < sizeof(tNode))
	{
		return;
	}
	
	/*初始化ECB*/
	tEventInit(&memBlock->event, tEventTypeMemBlock);
	
	/*初始化起始地址，存储块大小，存储块数量*/
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->blockCnt = blockCnt;
	
	/*初始化存储块链表，用于把每个小存储块用链表连接起来*/
	tListInit(&memBlock->blockList);
	
	/*将每个小存储块结点插入到链表中*/
	while(memBlockStart < memBlockEnd)
	{
		/*初始化每一个小存储块结点，把每个存储块的起始地址强制转换成tNode结点类型*/
		tNodeInit((tNode *)memBlockStart);
		
	  /*将每个结点插入到链表中*/
		tListInsertAfterLast(&memBlock->blockList, (tNode *)memBlockStart);
		memBlockStart += blockSize;
	}

}

/*
*********************************************************************************************************
*	函 数 名  :   tMemBlockWait
*	功能说明  :   从memBlock中等待空闲的存储块，传递给mem，可设置等待超时
*	形    参  :   memBlock：存储块
*               mem：保存取得的存储块
*               waitTicks：超时时间
*	返 回 值  :   tErrorNoError：错误码
*********************************************************************************************************
*/
uint32_t tMemBlockWait(tMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	
	/*判断memBlock中是否有空闲的存储块*/
	if(memBlock->blockList.nodeCount > 0)
	{
		/*
		  有空闲的存储块，就从memBlock中取出一个，返回的值是一个指针变量，
		  将其传递给一个二级指针用来保存
		*/
		*mem = (uint8_t *)tListRemoveFirstNode(&memBlock->blockList);
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		/*没有空闲的存储块，就将任务插入到ECB的等待队列中等待*/
		tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);		
		tTaskExitCritical(status);
		
		/*进行任务调度*/
		tTaskSched();
		
		/*超时或者被Notify，将获得的消息传递给msg*/
		*mem = currentTask->eventMsg;
		
		/*返回等待结果*/
		return currentTask->eventWaitResult;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tMemBlockNoWaitGet
*	功能说明  :   从memBlock中无等待的获取空闲存储块，传递给mem
*	形    参  :   memBlock：存储块
*               mem：保存取得的存储块
*	返 回 值  :   tErrorNoError：错误码
*********************************************************************************************************
*/
uint32_t tMemBlockNoWaitGet(tMemBlock *memBlock, uint8_t **mem)
{
	uint32_t status = tTaskEnterCritical();
	
	if(memBlock->blockList.nodeCount > 0)
	{
		*mem = (uint8_t *)tListRemoveFirstNode(&memBlock->blockList);
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		tTaskExitCritical(status);
		
		/*返回无资源可用错误*/
		return tErrorResourceUnavaliable;
	}
	
}

/*
*********************************************************************************************************
*	函 数 名  :   tMemBlockNotify
*	功能说明  :   将mem中的保存的空闲存储区的首地址传递给memBlock中的缓冲区
*	形    参  :   memBlock：存储块
*               mem：空闲存储块的首地址
*	返 回 值  :   无
*********************************************************************************************************
*/
void tMemBlockNotify(tMemBlock *memBlock, uint8_t *mem)
{
	uint32_t status = tTaskEnterCritical();
	
	/*若memBlock中有任务在等待存储块*/
	if(memBlock->event.waitList.nodeCount > 0)
	{
		/*将mem传递给等待队列头部的任务，唤醒任务*/
		tTask *task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	else
	{
		/*若没有任务在等待，将该存储块插入到链表当中*/
		tListInsertAfterLast(&memBlock->blockList, (tNode *)mem);
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	函 数 名  :   tMemBlockGetInfo
*	功能说明  :   获取存储块的相关信息
*	形    参  :   memBlock：存储块
*               info：保存存储块的信息
*	返 回 值  :   无
*********************************************************************************************************
*/
void tMemBlockGetInfo(tMemBlock *memBlock, tMemBlockInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	/*可用存储块的数量*/
	info->BlockCnt = memBlock->blockList.nodeCount;
	
	/*存储块的大小*/
	info->blockSize = memBlock->blockSize;
	
	/*最大存储块的数量*/
	info->maxBlockCnt = memBlock->blockCnt;
	
	/*等待存储块的任务的数量*/
	info->taskCount = memBlock->event.waitList.nodeCount;
	
	tTaskExitCritical(status);

}

/*
*********************************************************************************************************
*	函 数 名  :   tMemBlockDelete
*	功能说明  :   删除存储块
*	形    参  :   memBlock：存储块
*	返 回 值  :   count：存储块ECB中等待的任务数量
*********************************************************************************************************
*/
uint32_t tMemBlockDelete(tMemBlock *memBlock)
{
	uint32_t count;
	uint32_t status = tTaskEnterCritical();
	
	/*清空存储块ECB中的任务*/
	count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);
	
	/*如果有任务被唤醒，执行任务调度*/
	if(count > 0)
	{
		tTaskSched();
	}
	tTaskExitCritical(status);
	
	/*返回等待存储块的任务的数量*/
	return count;
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
