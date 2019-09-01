/*
*********************************************************************************************************
*
*	ģ������   :   ��ʱ��ʵ��ģ�顣
*	�ļ�����   :   tTimer.c
*	��    ��   :   V1.0
*	˵    ��   :   ���嶨ʱ�����ݽṹ����ʱ���ڶ�Ӧ�����н���ɨ�裬ˢ��ʱ��
*                ���ܻ����Ӻ�Ӳ��ʱ������ж��н���ɨ�裬Ҫ����ص���������
*                ̫����
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

#include "tTimer.h"
#include "SOLAROS.h"
#include "tSem.h"

#if SOLAROS_ENABLE_TIMER == 1

/*����hard��ʱ������*/
tList tTimerHardList;

/*����soft��ʱ������*/
tList tTimerSoftList;

/*����soft��ʱ����Դ�����ź���*/
tSem tTimerSoftProtectSem;

/*����hard��ʱ����Դ�����ź���*/
tSem tTimerHardProtectSem;

/*����soft��ʱ������*/
tTask tTimerSoftTask;

/*����soft��ʱ������Ķ�ջ*/
tTaskStack tTimerSoftTaskStack[SOLAROS_SOFTTIMER_TASK_STACK_SIZE];

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerInit
*	����˵��  :   ��ʱ����ʼ����soft��ʱ��ͨ��soft��ʱ������ȥ���£�hard��ʱ��ͨ��ϵͳʱ���ж�ȥ����
*	��    ��  :   timer��           ��ʱ��
*               startDelayTicks�� ��ʼ��ʱʱ��
*               durationTicks��   ��������ʱ��
*               timerFunc��       ��ʱ���ص�����
*               arg��             �ص������Ĳ���
*               config��          ��ʱ�����ã�soft or hard
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerInit(tTimer *timer, uint32_t startDelayTicks, uint32_t durationTicks,
	              void (*timerFunc)(void *arg), void *arg, uint32_t config)
{
	/*��ʼ�����*/
	tNodeInit(&timer->linkNode);
	
	/*��ʼ����ʼ��ʱʱ��*/
	timer->startDelayTicks = startDelayTicks;
	
	/*��ʼ����������ʱ��*/
	timer->durationTicks = durationTicks;
	
	/*��ʼ���ص�����*/
	timer->timerFunc = timerFunc;
	
	/*��ʼ���ص������Ĳ���*/
	timer->arg = arg;
	
	/*���ö�ʱ��ģʽ��Soft or Hard*/
	timer->config = config;
	
	/*���ö�ʱ��״̬������*/
	timer->state = tTimerCreated;
	
	/*�жϳ�ʼ��ʱʱ���Ƿ�Ϊ0*/
	if(startDelayTicks == 0)
	{
		/*�ݼ��������ĳ�ʼֵΪ��������ʱ���ֵ*/
		timer->delayTicks = durationTicks;
	}
	
	/*��Ϊ0*/
	else
	{
		/*�ݼ��������ĳ�ʼֵΪ��ʼ��ʱʱ���ֵ*/
		timer->delayTicks = startDelayTicks;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerStart
*	����˵��  :   ������ʱ��������ʱ�����뵽��ʱ��������
*	��    ��  :   timer����ʱ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerStart(tTimer *timer)
{
	switch(timer->state)
	{
		/*��ʱ�����ڴ������/ֹͣ״̬*/
		case tTimerCreated:
		case tTimerStopped:
			
			/*����״̬Ϊ����״̬*/
			timer->state = tTimerStarted;
		
			/*��������������ʱ�������¶�������ʱ���и�ֵ*/
			timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
		
			/*�ж��Ƿ�Ϊhard��ʱ��*/
			if(timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				/*hard��ʱ����ϵͳ�����ж��е��ã����ٽ����ķ�ʽ��hard��ʱ����������б���*/
				uint32_t status = tTaskEnterCritical();
				
				/*���ö�ʱ�����뵽hard��ʱ��������*/
				tListInsertAfterLast(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			
			/*soft��ʱ��*/
			else
			{
				#if SOLAROS_ENABLE_SEM == 1
				
				/*�ȴ�soft��ʱ���ı����ź���*/
				tSemWait(&tTimerSoftProtectSem, 0);	
				#endif
				
				/*���ö�ʱ�����뵽soft��ʱ��������*/
				tListInsertAfterLast(&tTimerSoftList, &timer->linkNode);
				#if SOLAROS_ENABLE_SEM == 1
				tSemNotify(&tTimerSoftProtectSem);
				#endif
			}
		break;
		
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerStop
*	����˵��  :   ֹͣ��ʱ��������ʱ���Ӷ�ʱ��������ɾ��
*	��    ��  :   timer����ʱ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerStop(tTimer *timer)
{
	switch(timer->state)
	{
		case tTimerStarted:
		case tTimerRunning:
			
			/*�ı䶨ʱ��״̬*/
			timer->state = tTimerStopped;
			
			/*�Ƿ�Ϊhard��ʱ��*/
			if(timer->config&TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = tTaskEnterCritical();
				
				/*��hard��ʱ��������ɾ���˶�ʱ��*/
				tListRemoveNode(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			
			/*soft��ʱ��*/
			else
			{
				#if SOLAROS_ENABLE_SEM == 1
				tSemWait(&tTimerSoftProtectSem, 0);
				#endif
				/*��soft��ʱ��������ɾ���˶�ʱ��*/
				tListRemoveNode(&tTimerSoftList, &timer->linkNode);
				#if SOLAROS_ENABLE_SEM == 1
				tSemNotify(&tTimerSoftProtectSem);
				#endif
			}
		break;
		
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerTraverseList
*	����˵��  :   ������ʱ������
*	��    ��  :   tTimerList����ʱ������soft or hard
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerTraverseList(tList *tTimerList)
{
	tNode *node;
	tTimer *timer;
	
	/*������ʱ�������е�ÿ��node*/
	for(node = tTimerList->headNode.nextNode; node != &(tTimerList->headNode); node = node->nextNode)
	{
		/*����node��ȡ��Ӧ��timer*/
		timer = tNodeParent(node, tTimer, linkNode);
		
		/*--�ݼ�����������Ϊ0��ִ����Ӧ����*/
		if(--timer->delayTicks == 0)
		{
			/*�ı䶨ʱ��״̬*/
			timer->state = tTimerRunning;
		
			/*ִ�ж�ʱ���ص�����*/
			timer->timerFunc(timer->arg);
			
			/*����ʱ��Ϊ���������У����¸�ֵ�ݼ�������*/
			if(timer->durationTicks)
			{
				timer->delayTicks = timer->durationTicks;
			}
			
			/*����ʱ�������������У�ֻ������ʱ����һ��*/
			else
			{
				/*���˶�ʱ����������ɾ��*/
				tListRemoveNode(tTimerList, &timer->linkNode);
				
				/*��Ϊֹͣ״̬*/
				timer->state = tTimerStopped;
				
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerSoftTaskEntry
*	����˵��  :   soft��ʱ��������ں���
*	��    ��  :   param����ڲ���
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerSoftTaskEntry(void *param)
{
	for(;;)
	{
		/*�ȴ�ϵͳʱ���ж��ȱ���hard��ʱ����Ȼ��hard��ʱ�������ź���*/
		#if SOLAROS_ENABLE_SEM == 1
		tSemWait(&tTimerHardProtectSem, 0);
		#endif
		
		/*�ȴ�soft��ʱ�������ʼ����ɺ�����һ���ź���*/
		#if SOLAROS_ENABLE_SEM == 1
		tSemWait(&tTimerSoftProtectSem, 0);
		#endif
		
		/*����soft��ʱ��������*/
		tTimerTraverseList(&tTimerSoftList);
		
		/*�ͷ��ź���*/
		#if SOLAROS_ENABLE_SEM == 1
		tSemNotify(&tTimerSoftProtectSem);
		#endif
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerHardTickProc
*	����˵��  :   hard��ʱ����ϵͳʱ���ж��е���
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerHardTickProc(void)
{
	uint32_t status = tTaskEnterCritical();
	
	/*����hard��ʱ������*/
	#if SOLAROS_ENABLE_SEM == 1
	tTimerTraverseList(&tTimerHardList);
	#endif
	
	tTaskExitCritical(status);
	
	/*�ͷŽ����жϱ����ź�����֪ͨsoft��ʱ����������*/
	#if SOLAROS_ENABLE_SEM == 1
	tSemNotify(&tTimerHardProtectSem);
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTimerModuleInit
*	����˵��  :   �����ʱ����ʼ��������soft��hard��ʱ��
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTimerModuleInit(void)
{
	tListInit(&tTimerSoftList);
	tListInit(&tTimerHardList);
	
	/*soft��ʱ���ź�����ʼ��ֵΪ1�����ڼ���soft��ʱ��������*/
	#if SOLAROS_ENABLE_SEM == 1
	tSemInit(&tTimerSoftProtectSem, 1, 1);
	tSemInit(&tTimerHardProtectSem, 0, 0);
	#endif
	
	/*��ʼ��soft��ʱ��������*/
	tTaskInit(&tTimerSoftTask, tTimerSoftTaskEntry, (void *)0, SOLAROS_SOFTTIMER_TASK_PRIO, 
		        tTimerSoftTaskStack, sizeof(tTimerSoftTaskStack));
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
