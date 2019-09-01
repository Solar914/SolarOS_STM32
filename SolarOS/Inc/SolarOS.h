/*
*********************************************************************************************************
*
*	ģ������   :   ϵͳ���Ĺ���ͷ�ļ�
*	�ļ�����   :   SOLAROS.h
*	��    ��   :   V1.0
*	˵    ��   :   ����ϵͳ���Ĺ��ܣ�������ʱ�����ȼ����г�ʼ��������
*                ϵͳ�������ٽ���������������ز�����
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

#ifndef __SOLAROS_H
#define __SOLAROS_H

#include <stdint.h>
#include "tTask.h"

/*------��չ����������------*/
extern uint8_t sysTickSyncFlag;
extern uint32_t idleCount;

/*------������ʱ���к��������ȼ����г�ʼ��------*/
void tTaskDelayedListInit(void);
void tTaskPrioListInit(void);

/*------����Ĺ�����ָ�------*/
void tTaskStateUnReady(tTask *task, uint32_t delayticks);
void tTaskStateReady(tTask *task);

/*------����OS�����л���ʱ�ӽ��ģ�������ʱ��ʱ�ӽ����жϴ���------*/
void tSetSysTickPeriod(uint32_t ms);
void tTaskDelay(uint32_t delay);
void tDelayTickHandler(void);

/*------�ٽ������API------*/
uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

/*------���������API------*/
void tSchedLockInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);

/*------��ȡ������ȼ�����------*/
tTask *tTaskHighestReady(void);

/*------��������뻽��------*/
void tTaskSuspend(tTask *task);
void tTaskWakeUp(tTask *task);

/*------����ɾ������------*/
void tTaskSetDeleteCallFunc(tTask *task, void(*clean)(void *cleanParam), void *cleanParam);
void tTaskForceDelete(tTask *task);
void tTaskRequestDelete(tTask *task);
uint8_t tTaskIsRequestedFlag(void);
void tTaskDeleteSelf(void);

/*------��ȡ������Ϣ------*/
void tTaskGetInfo(tTask *task, tTaskInfo *info);

/*------�������������л�------*/
void tTaskRunFirst(void);
void tTaskSwitch(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
