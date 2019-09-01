/*
*********************************************************************************************************
*
*	ģ������   :   ��/Ӳ��ʱ����ع���ͷ�ļ�
*	�ļ�����   :   tTimer.h
*	��    ��   :   V1.0
*	˵    ��   :   ���嶨ʱ��״̬����ʱ�����ݽṹ���ṩ��ʱ����ʼ������ʱ
*                ��������/ֹͣ��������ʱ����ز���API     
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

#ifndef _TTIMER_H
#define _TTIMER_H

#include "tEvent.h"

/*------��ʱ�����ͺ궨��------*/
#define  TIMER_CONFIG_TYPE_HARD       (1<<0)          /* HardΪ1                                       */
#define  TIMER_CONFIG_TYPE_SOFT       (0<<0)          /* SoftΪ0                                       */

/*------��ʱ��״̬���ݽṹ------*/
typedef enum _tTimerState
{
	tTimerCreated,                                      /* ����                                          */
	tTimerStarted,                                      /* ��ʼ                                          */
	tTimerRunning,                                      /* ����                                          */
	tTimerStopped,                                      /* ֹͣ                                          */
	tTimerDestroyed,                                    /* ����                                          */
}tTimerState;

/*------��ʱ�����ݽṹ------*/
typedef struct _tTimer
{
	tNode linkNode;                                     /* ������                                      */
	uint32_t startDelayTicks;                           /* ��ʼ��ʱʱ��                                  */
	uint32_t durationTicks;                             /* ��������                                      */
	uint32_t delayTicks;                                /* �ݼ�������                                    */
	void (*timerFunc)(void *arg);                       /* �ص�����                                      */
	void *arg;                                          /* �ص������Ĳ���                                */
	uint8_t config;                                     /* hardTimer or softTimer                        */
	tTimerState state;                                  /* ��ʱ��״̬                                    */
}tTimer;

/*------��ʱ����ʼ����������ֹͣ��API------*/
void tTimerInit(tTimer *timer, uint32_t startDelayTicks, uint32_t durationTicks,
	              void (*timerFunc)(void *arg), void *arg, uint32_t config);
void tTimerStart(tTimer *timer);
void tTimerStop(tTimer *timer);
void tTimerTraverseList(tList *tTimerList);
void tTimerHardTickProc(void);
void tTimerModuleInit(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
