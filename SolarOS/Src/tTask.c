/*
*********************************************************************************************************
*
*	ģ������   :   �������APIʵ��ģ�顣
*	�ļ�����   :   tTask.c
*	��    ��   :   V1.0
*	˵    ��   :   �����������ݽṹ�������ʼ��������������ĳ�ʼ�����������
*                ��ʵ�֣�������ʱ�����𡢾���״̬��ʵ��
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

#include "SOLAROS.h"
#include <string.h>

/*���������*/
uint8_t schedLockCount;

/*
  ����һ��λͼ���ݽṹ�����������������ȼ�����0λ���ȼ���ߣ�31λ���ȼ���ͣ�
	ĳλΪ1��ʾ�����ȼ���Ӧ������Ϊ����״̬��Ϊ0��ʾ�����ȼ���Ӧ������Ϊ����״̬
*/
tBitmap tbitmap;

/*����һ��tList���͵����飬����ÿһ����Ա����һ��˫��ѭ���������ڴ�Ų�ͬ���ȼ����������*/
tList tTaskList[TASK_MAX_PRIO_COUNT];

/*����������ʱ����*/
tList tTaskDelayedList;

/*��������ָ�룬�ֱ�ָ��ǰ�������һ����*/
tTask *currentTask;
tTask *nextTask;

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskInit
*	����˵��  :   �����ʼ��
*	��    ��  :   task����Ҫ����ʼ��������
*               entry������������ڵ�ַ
*               pararm�����ݸ��������ı���
*               prio����������ȼ�
*               stackStart�������ջ����ʼ��ַ
*               stackSize�������ջ�Ĵ�С
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskInit(tTask *task, void(*entry)(void *), void *param, uint8_t prio, tTaskStack *stackStart, uint32_t stackSize)
{
	/*��ջ�Ľ�����ַ��cortex-M3�Ķ�ջ�ɸߵ�ַ��͵�ַ����*/
	tTaskStack *stackEnd;
	
	/*��task�Ķ�ջ��ʼ��ַ��ֵ*/
	task->stackStart = stackStart;
	
	/*��task�Ķ�ջ��С��ֵ*/
	task->stackSize = stackSize;
	
	/*����ջ�ռ�ȫ����λΪ0����ʼ��*/
	memset(stackStart, 0, stackSize);
	
	/*��ջ�Ľ�����ַ = ��ʼ��ַ+��ջ��С/��ջ��������(4096/4)��0-1023������ʹ��--stackEnd���ȼ�����*/
	stackEnd = stackStart + stackSize/sizeof(tTaskStack);
	
	/*
	  ��ʼ��CPU�Ĵ������ݣ�Ϊ��������һ������١����ֳ������ڵ�CPU�л���������ʱ���Ӹ�����Ķ�ջ�лָ�
	  CPU�Ĵ�����ֵ��δ�õ��ļĴ�����ֱ����д�˼Ĵ����š��Ĵ����ĸ�ֵ˳���ܱ�
	*/
	*(--stackEnd) = (unsigned long)(1<<24);                /*XPSR������CPUΪThumb״̬*/
	*(--stackEnd) = (unsigned long)entry;                  /*����������ڵ�ַ*/ 
	*(--stackEnd) = (unsigned long)0x14;                   /*R14 LR�Ĵ���*/
	*(--stackEnd) = (unsigned long)0x12;                   /*R12*/
	*(--stackEnd) = (unsigned long)0x3;                    /*R3*/
	*(--stackEnd) = (unsigned long)0x2;                    /*R2*/
	*(--stackEnd) = (unsigned long)0x1;                    /*R1*/
	*(--stackEnd) = (unsigned long)param;                  /*R0 = param���������Ĳ���*/
	*(--stackEnd) = (unsigned long)0x11;                   /*R11*/
	*(--stackEnd) = (unsigned long)0x10;                   /*R10*/
	*(--stackEnd) = (unsigned long)0x9;                    /*R9*/
	*(--stackEnd) = (unsigned long)0x8;                    /*R8*/
	*(--stackEnd) = (unsigned long)0x7;                    /*R7*/
	*(--stackEnd) = (unsigned long)0x6;                    /*R6*/
	*(--stackEnd) = (unsigned long)0x5;                    /*R5*/
	*(--stackEnd) = (unsigned long)0x4;                    /*R4*/
	
	/*ִ��--�����󣬽����յ�ֵ���浽task->stack���洢�û�����*/
	task->stack = stackEnd;
	
	/*��ʼ����ʱ������*/
	task->tDelayTick = 0;
	
	/*�������ȼ���ʼ��*/
	task->prio = prio;
	
	/*����ͬ���ȼ�����ʱ��Ƭ*/
	task->slice = TASK_MAX_SLICE_COUNT;
	
	/*��ʼ������״̬*/
	task->state = SOLAROS_TASK_STATE_READY;
	
	/*��ʼ��������������*/
	task->suspendCount = 0;
	
	/*��ʼ������������*/
	task->clean = (void(*)(void *))0;
	
	/*��ʼ�������������Ĵ������*/
	task->cleanParam = (void *)0;
	
	/*��ʼ����������ɾ����־λ��0��ʾû�б�����ɾ��*/
	task->requestDeleteFlag = 0;
	
	/*��ʼ����ʱ���*/
	tNodeInit(&(task->delayNode));
	
	/*��ʼ�����ȼ����н��*/
	tNodeInit(&(task->linkNode));	
	
	/*�������Ӧ�Ľ����뵽�������ȼ�������ȥ*/
	tListInsertAfterLast(&(tTaskList[prio]), &(task->linkNode));
	
	/*��������Ϊ����̬*/
	tBitmapSet(&tbitmap, prio);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskSched
*	����˵��  :   �������
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskSched(void)
{
	/*��ȡ������ȼ��Ĵ��ھ���״̬������*/
	tTask *tempTask;
	
	/*��������Ϊ������Դ������ǰ�Ƚ����ٽ���*/
	uint32_t status = tTaskEnterCritical();
	
	/*���������>0�����˳��ٽ���������return���������������*/
	if(schedLockCount > 0)
	{
		tTaskExitCritical(status);
		return;
	}

	/*��ȡ������ȼ�������*/
  tempTask = tTaskHighestReady();
	
	/*����бȵ�ǰ�������ȼ����ߵ�����*/
	if(currentTask != tempTask)
	{
		nextTask = tempTask;
		
		/*����ָ���л���ɺ󣬴���PendSV�ж�ִ�������л�*/
	  tTaskSwitch();
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tSchedLockInit
*	����˵��  :   ��������ʼ��
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tSchedLockInit(void)
{
	schedLockCount = 0;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskSchedDisable
*	����˵��  :   ʧ���������
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*���������������255��������м�1����*/
	if (schedLockCount < 255)
	{
		schedLockCount++;
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskSchedEnable
*	����˵��  :   ʹ���������
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskSchedEnable(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*���������Ϊ0�������������*/
	if (--schedLockCount == 0)
	{
		tTaskSched();
	}
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskHighestReady
*	����˵��  :   ����������ȼ�������
*	��    ��  :   ��
*	�� �� ֵ  :   task��������ȼ�������
*********************************************************************************************************
*/
tTask *tTaskHighestReady(void)
{
	/*��ȡλͼ���ݽṹ�е�һ����1����������ȼ�*/
	uint8_t prio = tBitmapGetFirstSet(&tbitmap);
	
	/*�ӵ�ǰ���ȼ���Ӧ���������ȼ�������ȡ����һ�����firstNode*/
	tNode *node = tListFirstNode(&(tTaskList[prio]));
	
	/*���ݴ˽�㣬���ش˽���Ӧ������*/
	return tNodeParent(node, tTask, linkNode);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskDelayedListInit
*	����˵��  :   ��ʼ����ʱ����
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskDelayedListInit(void)
{
	tListInit(&tTaskDelayedList);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskPrioListInit
*	����˵��  :   ��ʼ�����ȼ�����
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskPrioListInit(void)
{
	uint32_t i = 0;
	for(i = 0; i < TASK_MAX_PRIO_COUNT; i++)
	{
		tListInit(&(tTaskList[i]));
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskStateUnReady
*	����˵��  :   ��������ʱ����
*	��    ��  :   task������ʱ���������
*               delayticks����ʱʱ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskStateUnReady(tTask *task, uint32_t delayticks)
{
	/*��������Ӷ�Ӧ�����ȼ�������ɾ��*/
	tListRemoveNode(&(tTaskList[task->prio]), &(task->linkNode));
	
	/*����Ӧ�����ȼ�������û������ʱ���ٽ���Ӧ��λͼ���ݽṹ��λ*/
	if(tTaskList[task->prio].nodeCount == 0)
	{
		tBitmapReset(&tbitmap, task->prio);
	}
	
	/*������ʱʱ��*/
	task->tDelayTick = delayticks;
	
	/*��������״̬*/
	task->state |= SOLAROS_TASK_STATE_DELAYED;
	
	/*����������뵽������ʱ���е�ĩβ*/
	tListInsertAfterLast(&tTaskDelayedList, &(task->delayNode));
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskStateReady
*	����˵��  :   ������ʱ��ɣ���������Ϊ����
*	��    ��  :   task����ʱ��ɵ�����
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskStateReady(tTask *task)
{
	/*����������뵽��Ӧ���������ȼ�������*/
	tListInsertAfterLast(&(tTaskList[task->prio]), &(task->linkNode));
	
	/*��λͼ���ݽṹ�ж�Ӧ��������1������*/
	tBitmapSet(&tbitmap, task->prio);
	
	/*��������״̬*/
	task->state &= ~(SOLAROS_TASK_STATE_DELAYED);
	
	/*���������������ʱ������ɾ��*/
	tListRemoveNode(&tTaskDelayedList, &(task->delayNode));
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskSuspend
*	����˵��  :   ����ĳ������
*	��    ��  :   task����Ҫ�����������
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskSuspend(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*��ֹ����ʱ״̬������ִ�й������*/
	if(!(task->state & SOLAROS_TASK_STATE_DELAYED))
	{
		/*���ӹ�������������������һ�ι���ʱ����ִ�������л�����*/
		if(++task->suspendCount <= 1)
		{
			/*��������������״̬*/
			task->state |= SOLAROS_TASK_STATE_SUSPEND;
			
			/*��������Ϊδ����״̬*/
			tListRemoveNode(&(tTaskList[task->prio]), &(task->linkNode));
			if(tTaskList[task->prio].nodeCount == 0)
			{
				tBitmapReset(&tbitmap, task->prio);
			}
			
			/*�������������ǵ�ǰ�������е����񣬾�ִ�������л�����*/
			if(task == currentTask)
			{
				tTaskSched();
			}
		}
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskWakeUp
*	����˵��  :   ����ĳ������
*	��    ��  :   task����Ҫ�����ѵ�����
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskWakeUp(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*�ж������Ƿ��ǹ���״̬*/
	if(task->state & SOLAROS_TASK_STATE_SUSPEND)
	{
		/*�ݼ���������������Ϊ0����ִ�������Ѳ���*/
		if(--task->suspendCount == 0)
		{
			/*��������־*/
			task->state &= ~SOLAROS_TASK_STATE_SUSPEND;
			
			/*��������*/
			tListInsertAfterLast(&(tTaskList[task->prio]), &(task->linkNode));
			tBitmapSet(&tbitmap, task->prio);
			
			/*���ѹ����п����и������ȼ������������ִ��һ���������*/
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskSetDeleteCallFunc
*	����˵��  :   ��������ɾ��ʱҪ���õ�������
*	��    ��  :   task����ɾ��������
*               clean��������
*               cleanParam�����������д��ݵı���
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskSetDeleteCallFunc(tTask *task, void(*clean)(void *cleanParam), void *cleanParam)
{
	task->clean = clean;
	task->cleanParam = cleanParam;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskForceDelete
*	����˵��  :   ǿ��ɾ������
*	��    ��  :   task����ɾ��������
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskForceDelete(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*�ж������Ƿ�����ʱ״̬*/
	if(task->state & SOLAROS_TASK_STATE_DELAYED)
	{
		tListRemoveNode(&tTaskDelayedList, &task->delayNode);
	}
	
	/*�ж������Ƿ��ǹ���״̬�������ǣ���˵�����������л��߾���״̬*/
	else if(!(task->state & SOLAROS_TASK_STATE_SUSPEND))
	{
		tListRemoveNode(&tTaskList[task->prio], &task->linkNode);
		if(tTaskList[task->prio].nodeCount == 0)
		{
			tBitmapReset(&tbitmap, task->prio);
		}
	}
	
	/*ִ��������*/
	if(task->clean)
	{
		task->clean(task->cleanParam);
	}
	
	/*��ɾ�����������е�����ִ�������л�*/
	if(currentTask == task)
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskRequestDelete
*	����˵��  :   ����ɾ��ָ������
*	��    ��  :   task����ɾ��������
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskRequestDelete(tTask *task)
{
	uint32_t status = tTaskEnterCritical();
	
	/*��λ����ɾ�������־λ*/
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskIsRequestedFlag
*	����˵��  :   �жϵ�ǰ�����Ƿ�����ɾ��
*	��    ��  :   ��
*	�� �� ֵ  :   deleteFlag������ɾ����־λ
*********************************************************************************************************
*/
uint8_t tTaskIsRequestedFlag(void)
{
	uint8_t deleteFlag;
	uint32_t status = tTaskEnterCritical();
	
	/*��ѯ��ǰ���������ɾ����־λ�Ƿ���λ*/
	deleteFlag = currentTask->requestDeleteFlag;
	
	tTaskExitCritical(status);
	
	/*���ر�־λ*/
	return deleteFlag;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskDeleteSelf
*	����˵��  :   ����ɾ������
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskDeleteSelf(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*ɾ���Լ�ʱ�������Ѿ��������У�����Ҫ���Լ������ȼ�������ɾ��*/
	tListRemoveNode(&tTaskList[currentTask->prio], &currentTask->linkNode);
	if(tTaskList[currentTask->prio].nodeCount == 0)
	{
		tBitmapReset(&tbitmap, currentTask->prio);
	}
	
	/*ִ��������*/
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);
	}
	
	/*ִ���������*/
	tTaskSched();
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskGetInfo
*	����˵��  :   ��ȡ����������Ϣ
*	��    ��  :   task������
*               info�������������Ϣ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskGetInfo(tTask *task, tTaskInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	/*
	  ���ڶ�ջ�Ӹߵ�ַ��͵�ַ���������ԴӶ�ջ��ʼλ�ü��͵�ַ
	  ��ʼ��ͳ��Ϊ0�Ŀռ�ĸ����������ܵĶ�ջ��С�����ɵõ���ջ�Ŀ�����
	*/
	tTaskStack *stackStart = task->stackStart;
	
	info->prio = task->prio;
	info->slice =task->slice;
	info->state = task->state;
	info->suspendCount = task->suspendCount;
	info->tDelayTick = task->tDelayTick;
	info->stackSize = task->stackSize;
	
	/*��ʼ��freeStack*/
	info->freeStack = 0;
	
	/*�Ӷ�ջ��ʼλ�ý���++��ֱ��������0�����ݣ�����û�г�����ջ�ķ�Χ*/
	while((*stackStart++ == 0)&&(stackStart < (task->stackStart + task->stackSize/sizeof(tTaskStack))))
	{
		/*���ж�ջ�ĸ���++*/
		info->freeStack++;
	}
	
	/*������ֽ���*/
	info->freeStack *= sizeof(tTaskStack);
	
	tTaskExitCritical(status);
}

/***************************** SOLAROS (END OF FILE) *********************************/
