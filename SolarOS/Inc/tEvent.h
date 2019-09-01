/*
*********************************************************************************************************
*
*	ģ������   :   �¼����ƿ���ع���ͷ�ļ�
*	�ļ�����   :   tEvent.h
*	��    ��   :   V1.0
*	˵    ��   :   �����¼����ƿ����ݽṹ���¼��������ݽṹ���ṩECB��ʼ����ECB
*                �ȴ���ECB������ز���API
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

#ifndef _TEVENT_H
#define _TEVENT_H

#include "tList.h"
#include "tTask.h"

/*------���������ݽṹ------*/
typedef enum _tError
{
	tErrorNoError = 0,                                     /* û�д���                                          */
	tErrorTimeout,                                         /* ��ʱ����                                          */
	tErrorResourceUnavaliable,                             /* ����Դ����                                        */
	tErrorDel,                                             /* ��ɾ��                                            */
	tErrorResourceFull,                                    /* ��Դ��                                            */
}tError;

/*------�¼��������ݽṹ------*/
typedef enum _tEventType
{ 
	tEventTypeUnknown = 0,                                 /* δ֪����                                          */
	tEventTypeSem,                                         /* �ź���                                            */
	tEventTypeMbox,                                        /* ����                                              */
	tEventTypeMemBlock,                                    /* �洢��                                            */
	tEventTypeFlagGroup,                                   /* �¼���־��                                        */
	tEventTypeMutex,                                       /* �����ź���                                        */
}tEventType;

/*------ECB���ݽṹ------*/
typedef struct _tEvent
{
	tEventType type;                                       /* �¼�����                                          */
	tList waitList;                                        /* ����ȴ�����                                      */
}tEvent;

/*------ECB��ʼ�����ȴ������ѵ�API------*/
void tEventInit(tEvent *event, tEventType type);
void tEventWait(tEvent *event, tTask *task, void *msg, uint32_t eventType, uint32_t timeOut);
tTask *tEventWakeUp(tEvent *event, void *msg, uint32_t result);
void tEventRemoveTask(tTask *task, void *msg, uint32_t result);
uint32_t tEventRemoveAll(tEvent *event,void *msg, uint32_t result);
uint32_t tEventWaitCount(tEvent *event);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
