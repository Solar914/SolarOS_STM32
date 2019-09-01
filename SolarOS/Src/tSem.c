/*
*********************************************************************************************************
*
*	ģ������   :   �ź���ʵ��ģ�顣
*	�ļ�����   :   tSem.c
*	��    ��   :   V1.0
*	˵    ��   :   �����ź������ݽṹ���ź����ĳ�ʼ�����ȴ��ź�����
*                �ͷ��ź�����ɾ���ź�����API��ʵ��
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

#include "tSem.h"
#include "SOLAROS.h"

#if SOLAROS_ENABLE_SEM == 1

/*
*********************************************************************************************************
*	�� �� ��  :   tSemInit
*	����˵��  :   �����ź�����ʼ��
*	��    ��  :   sem�������ź���
*               startCount����ʼ����ֵ
*               maxCount��������ֵ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount)
{
	/*ECB��ʼ��*/
	tEventInit(&sem->event, tEventTypeSem);
	
	/*������maxCount��ʼ��*/
	sem->maxCount = maxCount;
	
	/*������count��ʼ����maxCount == 0��ʾ��count�����ƣ�count=startCount*/
	if(maxCount == 0)
	{
		sem->count = startCount;
	}
	else
	{
		/*(�������ʽ) ? (��) : (��) */
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
	
}

/*
*********************************************************************************************************
*	�� �� ��  :   tSemWait
*	����˵��  :   �ȴ�ĳ���ź����������ó�ʱʱ��
*	��    ��  :   sem�������ź���
*               timeOut����ʱʱ��
*	�� �� ֵ  :   eventWaitResult���ȴ����
*********************************************************************************************************
*/
uint32_t tSemWait(tSem *sem, uint32_t timeOut)
{
	uint32_t status = tTaskEnterCritical();
	
	/*����ź�������������0���ͽ���--���������ĵ�һ����Դ*/
	if(sem->count > 0)
	{
		--sem->count;
		tTaskExitCritical(status);
		
		/*�����޴���*/
		return tErrorNoError;
	}
	
	/*����ź���Ϊ0����˵������Դ���ã���������뵽ECB�ĵȴ�������*/
	else
	{
		/*��ȴ������в������񣬳�ʱʱ������Ϊ0��ʾ��Զ�ȴ�*/
		tEventWait(&sem->event, currentTask, (void*)0, tEventTypeSem, timeOut);
		
		tTaskExitCritical(status);
		
		/*��������뵽�ȴ����к󣬻����������������ִ���������*/
		tTaskSched();
		
		/*����������ִ�е�����ʱ����������ĵȴ����*/
		return currentTask->eventWaitResult;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tSemNoWaitGet
*	����˵��  :   �޵ȴ��Ļ�ȡĳ���ź���
*	��    ��  :   sem�������ź���
*	�� �� ֵ  :   eventWaitResult���ȴ����
*********************************************************************************************************
*/
uint32_t tSemNoWaitGet(tSem *sem)
{
	uint32_t status = tTaskEnterCritical();
	
	/*����ź���������>0���ͽ���--���������ĵ�һ����Դ*/
	if(sem->count > 0)
	{
		--sem->count;
		tTaskExitCritical(status);
		
		/*�����޴���*/
		return tErrorNoError;
	}
	
	/*����ź���������Ϊ0��˵������Դ���ã�ֱ�ӷ��ؽ��tErrorResourceUnavaliable*/
	else
	{
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tSemNotify
*	����˵��  :   �ͷ�ĳ���ź���
*	��    ��  :   sem�������ź���
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tSemNotify(tSem *sem)
{
	uint32_t status = tTaskEnterCritical();
	
	/*�жϸ��ź�����ECB�ĵȴ��������Ƿ�������*/
	if(sem->event.waitList.nodeCount > 0)
	{
		/*������Ļ�����ֱ�ӻ��Ѹ�ECB�еĵ�һ������*/
		tTask *task = tEventWakeUp(&sem->event, (void*)0, tErrorNoError);
		
		/*���Ѻ�ִ��������ȣ��������ȼ�Խ�ߣ�prio��ֵԽС*/
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}	
		tTaskExitCritical(status);
	}
	
	/*û������Ļ����Ͷ��ź�������������++����������һ����Դ*/
	else
	{
		++sem->count;
		if((sem->maxCount != 0) && (sem->count > sem->maxCount))
		{
			sem->count = sem->maxCount;
		}
		tTaskExitCritical(status);
	}
		
}

/*
*********************************************************************************************************
*	�� �� ��  :   tSemGetInfo
*	����˵��  :   ��ȡĳ���ź�������Ϣ
*	��    ��  :   sem�������ź���
*               info���洢��ȡ������Ϣ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tSemGetInfo(tSem *sem, tSemInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	/*�ź�����ǰ����ֵ*/
	info->count = sem->count;
	
	/*�ź���������ֵ*/
	info->maxCount = sem->maxCount;
	
	/*�ź�����ECB���ڵȴ������������*/
	info->taskCount = sem->event.waitList.nodeCount;
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tSemDelete
*	����˵��  :   ɾ��ĳ���ź���
*	��    ��  :   sem�������ź���
*	�� �� ֵ  :   �����ź�����ECB���ڵȴ������������
*********************************************************************************************************
*/
uint32_t tSemDelete(tSem *sem)
{
	uint32_t count;
	uint32_t status = tTaskEnterCritical();
	
	/*���ź�����ECB�ȴ�������ɾ����������*/
	count = tEventRemoveAll(&sem->event, (void *)0, tErrorDel);
	
	/*�ź�������������*/
	sem->count = 0;
	
	tTaskExitCritical(status);
	
	/*��������񱻻��ѣ�ִ���������*/
	if(count > 0)
	{
		tTaskSched();
	}
	return count;
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
