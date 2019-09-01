/*
*********************************************************************************************************
*
*	ģ������   :   �¼����ƿ�ʵ��ģ�顣
*	�ļ�����   :   tEvent.c
*	��    ��   :   V1.0
*	˵    ��   :   �¼����ƿ����ݽṹ��ʵ�֣�ECB(Event Control Block)��
*                �¼�����eventType���ź��������䡢�洢�飩�ȣ��Լ��ȴ�
*                ����waitList���
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

#include "tEvent.h"
#include "SOLAROS.h"

/*
*********************************************************************************************************
*	�� �� ��  :   tEventInit
*	����˵��  :   ��ʼ���¼����ƿ�
*	��    ��  :   event������ʼ����ECB
*               type��ECB������
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tEventInit(tEvent *event, tEventType type)
{
	event->type = type;
	tListInit(&event->waitList);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tEventWait
*	����˵��  :   ��ָ��������뵽ָ����ECB��
*	��    ��  :   event��Ҫ�����ECB
*               task����Ҫ�����������
*               msg����Ҫ���ݸ��������Ϣ
*               eventType��ʱ�������
*               timeOut���ȴ���ʱʱ�䣬��Ϊ0��ʾ��Զ�ȴ�
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t eventType, uint32_t timeOut)
{
	uint32_t status = tTaskEnterCritical();
	
	uint32_t totalTaskCount;
	
	/*����task����ֶ�*/
	/*��16λ����ʾECB��ص�״̬*/
	task->state = (eventType << 16);
	task->eventMsg = msg;
	task->waitEvent = event;
	task->eventWaitResult = tErrorNoError;
	
	/*�����ȼ�������ɾ��������*/
	tListRemoveNode(&tTaskList[task->prio], &task->linkNode);
	if(tTaskList[task->prio].nodeCount == 0)
	{
		tBitmapReset(&tbitmap, task->prio);
	}
	
	//���뵽ECB�ȴ����е�ĩβ
	//tListInsertAfterLast(&task->waitEvent->waitList, &task->linkNode);
	
	/*
	  �����������ȼ���˳���������ECB��waitlist��
	  ˼·������֮ǰ���ж϶������Ƿ����������ޣ���ֱ�Ӳ��룻��������
	  ��������У���Ҫ���������1�����ȼ�1������е���������ȼ����б�
	  �ϣ������ȼ�1���ߣ�������1���뵽��Ӧ�����ǰ�棬�����������
	  ����1�����ȼ���С�ڻ���ڵ�ǰ��������ȼ���������뵽ĩβ
	*/
	totalTaskCount = tListCount(&event->waitList);
	if(totalTaskCount == 0)
	{
		
		/*���ȴ�������������ֱ�ӽ�������뵽����ĩβ*/
		tListInsertAfterLast(&event->waitList, &task->linkNode);
	}
	
	/*�ȴ�������������*/
	else
	{
		uint8_t i = 0;
		
		/*�����һ������Ӧ������Ƚ�*/
		tNode *currentNode = tListFirstNode(&event->waitList);
		for(i = 0; i < totalTaskCount; i++)
		{
			/*ȡ������Ӧ������*/
			tTask *taskToCompare = tNodeParent(currentNode, tTask, linkNode);
			
			/*����������������ȼ�����*/
			if(task->prio < taskToCompare->prio)
			{
				/*���������������뵽currentNode��ǰ�棬Ҳ��currentNode->preNode�ĺ���*/
				tListInsertAfterNode(&event->waitList, currentNode->preNode, &task->linkNode);
				
				/*�˳�ѭ��*/
				break;
			}
			
			/*if�ж����������㣬�ƶ�����һ�����*/
			currentNode = currentNode->nextNode;
		}
		
		/*�����������if�Բ���������*/
		if(i >= totalTaskCount)
		{
			/*����������뵽�ȴ����е�ĩβ*/
			tListInsertAfterLast(&event->waitList, &task->linkNode);
		}
	
	}
	
	/*��������˳�ʱ�ȴ�*/
	if(timeOut != 0)
	{
		/*������ʱʱ��*/
		task->tDelayTick = timeOut;
	
		/*�ı�����״̬Ϊ��ʱ״̬*/
		task->state |= SOLAROS_TASK_STATE_DELAYED;
	
		/*��������뵽��ʱ����*/
		tListInsertAfterLast(&tTaskDelayedList, &(task->delayNode));
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tEventWakeUp
*	����˵��  :   ��ECB�л�������
*	��    ��  :   event���������ڵ�ECB
*               msg����Ҫ���ݸ��������Ϣ
*               result���ȴ��Ľ��
*	�� �� ֵ  :   task�������ѵ�����
*********************************************************************************************************
*/
tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result)
{
	tTask *task = (tTask *)0;
	tNode *node = (tNode *)0;
	
	uint32_t status = tTaskEnterCritical();
	
	/*��ECB�ĵȴ�������ɾ����һ����㲢���ظý��*/
	if((node = tListRemoveFirstNode(&event->waitList)) != (tNode *)0)
	{
		task = tNodeParent(node, tTask, linkNode);
		
		/*����������Ӧ��ECB*/
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->eventWaitResult = result;
		
		/*����������ECB��״̬*/
		task->state &= ~SOLAROS_TASK_WAIT_MASK;
			
		/*�����������˳�ʱ�ȴ����������ʱ���ж�����ɾ��*/
		if(task->tDelayTick != 0)
		{
			task->state &= ~SOLAROS_TASK_STATE_DELAYED;
			tListRemoveNode(&tTaskDelayedList, &task->delayNode);
		}
		
		/*����������뵽���ȼ�������*/
		tListInsertAfterLast(&tTaskList[task->prio], &task->linkNode);
		tBitmapSet(&tbitmap, task->prio);
	}
	
	tTaskExitCritical(status);
	
	/*���ر����ѵ�����*/
	return task;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tEventRemoveTask
*	����˵��  :   �������ECB��ɾ��
*	��    ��  :   task����Ҫɾ��������
*               msg����Ҫ���ݸ��������Ϣ
*               result���ȴ��Ľ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tEventRemoveTask(tTask *task, void *msg, uint32_t result)
{
	uint32_t status = tTaskEnterCritical();
	
	tListRemoveNode(&task->waitEvent->waitList, &task->linkNode);
	
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;
	task->eventWaitResult = result;
	task->state &= ~SOLAROS_TASK_WAIT_MASK;
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tEventRemoveAll
*	����˵��  :   ɾ��ECB�е���������
*	��    ��  :   event����Ҫ��յ�ECB
*               msg����Ҫ���ݸ��������Ϣ
*               result���ȴ��Ľ��
*	�� �� ֵ  :   count��ECB�����������
*********************************************************************************************************
*/
uint32_t tEventRemoveAll(tEvent *event, void *msg, uint32_t result)
{
	uint32_t count;
	tNode *node = (tNode *)0;
	
	uint32_t status = tTaskEnterCritical();
	
	/*���صȴ������������*/
	count = event->waitList.nodeCount;
	
	/*��ʼɾ��*/
	while((node = tListRemoveFirstNode(&event->waitList)) != (tNode *)0)
	{
		tTask *task = tNodeParent(node, tTask, linkNode);
		
		/*�޸�task��״̬*/
		task->state &= ~SOLAROS_TASK_WAIT_MASK;
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->eventWaitResult = result;
		
		/*���task������ʱ�����У�����ʱ�����л���*/
		if(task->tDelayTick != 0)
		{
			task->state &= ~SOLAROS_TASK_STATE_DELAYED;
			tListRemoveNode(&tTaskDelayedList, &task->delayNode);
		}
		
		/*��task���뵽���ȼ�������*/
		tListInsertAfterLast(&tTaskList[task->prio], &task->linkNode);
		tBitmapSet(&tbitmap, task->prio);
	}
	
	tTaskExitCritical(status);
	
	/*����ECB�����������*/
	return count;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tEventWaitCount
*	����˵��  :   ����ECB���ڵȴ������������
*	��    ��  :   event��ECB
*	�� �� ֵ  :   count��ECB���ڵȴ������������
*********************************************************************************************************
*/
uint32_t tEventWaitCount(tEvent *event)
{
	uint32_t count;
	uint32_t status = tTaskEnterCritical();
	
	count = event->waitList.nodeCount;
	
	tTaskExitCritical(status);
	
	return count;
}

/***************************** SOLAROS (END OF FILE) *********************************/
