/*
*********************************************************************************************************
*
*	ģ������   :   �洢��ʵ��ģ�顣
*	�ļ�����   :   tMemBlock.c
*	��    ��   :   V1.0
*	˵    ��   :   ����洢�����ݽṹ���洢���ʼ�����Ӵ洢���л�ȡ���д洢����
*                �����д洢������洢�飬��ȡ�洢����Ϣ��ɾ���洢���API��ʵ��
*
*	�޸ļ�¼   :
*		             �汾��     ����         ����            ˵��
*		              V1.0    2019-4-16      Solar           ��һ�ΰ汾����
*		              
*
*	��Ȩ       ��  ����ѧϰ����ʹ�ã���ֹ������ҵ��;
*
*********************************************************************************************************
*/

#include "tMemBlock.h"

#if SOLAROS_ENABLE_MEMBLOCK == 1

/*
*********************************************************************************************************
*	�� �� ��  :   tMemBlockInit
*	����˵��  :   �洢���ʼ��
*	��    ��  :   memBlock���洢��
*               memStart���洢�����ʼ��ַ
*               blockSize�������洢��Ĵ�С
*               blockCnt���洢�������
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tMemBlockInit(tMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt)
{
	/*�洢�����ʼ��ַ*/
	uint8_t *memBlockStart = memStart;
	
	/*�洢��Ľ�����ַ*/
	uint8_t *memBlockEnd = memStart + blockSize * blockCnt;
	
	/*�����жϵ����洢��Ĵ�С������С��һ��tNode�Ĵ�С����Ϊÿ��С�洢�����ٰ���һ��tNode*/
	if(blockSize < sizeof(tNode))
	{
		return;
	}
	
	/*��ʼ��ECB*/
	tEventInit(&memBlock->event, tEventTypeMemBlock);
	
	/*��ʼ����ʼ��ַ���洢���С���洢������*/
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->blockCnt = blockCnt;
	
	/*��ʼ���洢���������ڰ�ÿ��С�洢����������������*/
	tListInit(&memBlock->blockList);
	
	/*��ÿ��С�洢������뵽������*/
	while(memBlockStart < memBlockEnd)
	{
		/*��ʼ��ÿһ��С�洢���㣬��ÿ���洢�����ʼ��ַǿ��ת����tNode�������*/
		tNodeInit((tNode *)memBlockStart);
		
	  /*��ÿ�������뵽������*/
		tListInsertAfterLast(&memBlock->blockList, (tNode *)memBlockStart);
		memBlockStart += blockSize;
	}

}

/*
*********************************************************************************************************
*	�� �� ��  :   tMemBlockWait
*	����˵��  :   ��memBlock�еȴ����еĴ洢�飬���ݸ�mem�������õȴ���ʱ
*	��    ��  :   memBlock���洢��
*               mem������ȡ�õĴ洢��
*               waitTicks����ʱʱ��
*	�� �� ֵ  :   tErrorNoError��������
*********************************************************************************************************
*/
uint32_t tMemBlockWait(tMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	
	/*�ж�memBlock���Ƿ��п��еĴ洢��*/
	if(memBlock->blockList.nodeCount > 0)
	{
		/*
		  �п��еĴ洢�飬�ʹ�memBlock��ȡ��һ�������ص�ֵ��һ��ָ�������
		  ���䴫�ݸ�һ������ָ����������
		*/
		*mem = (uint8_t *)tListRemoveFirstNode(&memBlock->blockList);
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		/*û�п��еĴ洢�飬�ͽ�������뵽ECB�ĵȴ������еȴ�*/
		tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);		
		tTaskExitCritical(status);
		
		/*�����������*/
		tTaskSched();
		
		/*��ʱ���߱�Notify������õ���Ϣ���ݸ�msg*/
		*mem = currentTask->eventMsg;
		
		/*���صȴ����*/
		return currentTask->eventWaitResult;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMemBlockNoWaitGet
*	����˵��  :   ��memBlock���޵ȴ��Ļ�ȡ���д洢�飬���ݸ�mem
*	��    ��  :   memBlock���洢��
*               mem������ȡ�õĴ洢��
*	�� �� ֵ  :   tErrorNoError��������
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
		
		/*��������Դ���ô���*/
		return tErrorResourceUnavaliable;
	}
	
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMemBlockNotify
*	����˵��  :   ��mem�еı���Ŀ��д洢�����׵�ַ���ݸ�memBlock�еĻ�����
*	��    ��  :   memBlock���洢��
*               mem�����д洢����׵�ַ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tMemBlockNotify(tMemBlock *memBlock, uint8_t *mem)
{
	uint32_t status = tTaskEnterCritical();
	
	/*��memBlock���������ڵȴ��洢��*/
	if(memBlock->event.waitList.nodeCount > 0)
	{
		/*��mem���ݸ��ȴ�����ͷ�������񣬻�������*/
		tTask *task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	else
	{
		/*��û�������ڵȴ������ô洢����뵽������*/
		tListInsertAfterLast(&memBlock->blockList, (tNode *)mem);
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMemBlockGetInfo
*	����˵��  :   ��ȡ�洢��������Ϣ
*	��    ��  :   memBlock���洢��
*               info������洢�����Ϣ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tMemBlockGetInfo(tMemBlock *memBlock, tMemBlockInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	/*���ô洢�������*/
	info->BlockCnt = memBlock->blockList.nodeCount;
	
	/*�洢��Ĵ�С*/
	info->blockSize = memBlock->blockSize;
	
	/*���洢�������*/
	info->maxBlockCnt = memBlock->blockCnt;
	
	/*�ȴ��洢������������*/
	info->taskCount = memBlock->event.waitList.nodeCount;
	
	tTaskExitCritical(status);

}

/*
*********************************************************************************************************
*	�� �� ��  :   tMemBlockDelete
*	����˵��  :   ɾ���洢��
*	��    ��  :   memBlock���洢��
*	�� �� ֵ  :   count���洢��ECB�еȴ�����������
*********************************************************************************************************
*/
uint32_t tMemBlockDelete(tMemBlock *memBlock)
{
	uint32_t count;
	uint32_t status = tTaskEnterCritical();
	
	/*��մ洢��ECB�е�����*/
	count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);
	
	/*��������񱻻��ѣ�ִ���������*/
	if(count > 0)
	{
		tTaskSched();
	}
	tTaskExitCritical(status);
	
	/*���صȴ��洢������������*/
	return count;
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
