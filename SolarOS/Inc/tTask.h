/*
*********************************************************************************************************
*
*	ģ������   :   ������ع���ͷ�ļ�
*	�ļ�����   :   tTask.h
*	��    ��   :   V1.0
*	˵    ��   :   �����������ݽṹ������״̬���ݽṹ���ṩ�����ʼ����������ȣ�������
*                ʹ��/ʧ����ز���API
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

#ifndef _TTASK_H
#define _TTASK_H

#include <stdint.h>
#include "tList.h"
#include "tBitmap.h"
#include "tConfig.h"

/*------����״̬�궨��------*/
#define SOLAROS_TASK_STATE_READY        0                    /* ���λ���������Ƿ���READY״̬��0��ʾ����            */
#define SOLAROS_TASK_STATE_DELAYED      (1<<1)               /* ����1λ���������Ƿ���DELAY״̬��1��ʾ��ʱ״̬       */
#define SOLAROS_TASK_STATE_SUSPEND      (1<<2)               /* ����2λ���������Ƿ��ǹ���״̬��1��ʾ����״̬        */
#define SOLAROS_TASK_STATE_DESTROYED    (1<<3)               /* ����3λ��ʾ�����Ƿ����٣�1��ʾ������              */
#define SOLAROS_TASK_WAIT_MASK          (0XFF << 16)         /* ����״̬�ĸ�16λ������                              */

/*------�����ջ����------*/
typedef uint32_t tTaskStack;

/*------ǰ�����õķ�ʽ����tEvent�ṹ��------*/
struct _tEvent;

/*------�������ݽṹ------*/
typedef struct _tTask
{
	tTaskStack *stack;                                         /* �����ջ����������ʼλ�ã����������ͬ��ַ          */ 
	tTaskStack *stackStart;                                    /* ��ջ����ʼ��ַ                                      */ 
	uint32_t stackSize;                                        /* ��ջ�Ĵ�С�����ֽ�Ϊ��λ                            */ 
	tTaskStack tDelayTick;                                     /* ������ʱʱ��                                        */ 
	uint8_t prio;                                              /* �������ȼ�                                          */ 
	tNode delayNode;                                           /* ������ʱ���н��                                    */ 
	tNode linkNode;                                            /* �������ȼ����н��                                  */ 
	uint32_t slice;                                            /* ͬ���ȼ������ʱ��Ƭ                                */ 
	uint32_t state;                                            /* ����״̬                                            */ 
	uint32_t suspendCount;                                     /* ����������������                                  */ 
	void(*clean)(void *cleanParam);                            /* ����ɾ��ʱ���õ�������                          */ 
	void *cleanParam;                                          /* ���ݸ��������Ĳ���                                */ 
	uint8_t requestDeleteFlag;                                 /* ����ɾ����־����0��ʾ����ɾ��                       */ 
	struct _tEvent *waitEvent;                                 /* ����¼����ƿ飬��ʾ���������ڵȴ���һ���¼����ƿ�  */ 
	void *eventMsg;                                            /* ����¼���Ϣ                                        */ 
	uint32_t eventWaitResult;                                  /* �ȴ��Ľ��������Ӧ�Ĵ�������֮��Ӧ                  */ 
}tTask;

/*------������Ϣ���ݽṹ------*/
typedef struct _tTaskInfo
{
	tTaskStack tDelayTick;
	uint8_t prio;
	uint32_t slice;
	uint32_t state;
	uint32_t suspendCount;
	uint32_t stackSize;
	uint32_t freeStack;
}tTaskInfo;

/*------��չ����������------*/ 
extern tTask *currentTask;                                   /* ָ��ǰ�������е������ָ��                        */ 
extern tTask *nextTask;                                      /* ָ����һ�������е������ָ��                        */
extern uint32_t tickCount;                                   /* ������������ͳ��CPU������                           */
extern uint8_t schedLockCount;                               /* ������������                                        */
extern tBitmap tbitmap;                                      /* λͼ����                                            */
extern tList tTaskList[TASK_MAX_PRIO_COUNT];                 /* �������ȼ�����                                      */
extern tList tTaskDelayedList;                               /* ������ʱ����                                        */

/*
  ��֪node��㣬��֪�ṹ��parent�Լ��ṹ���Աname�������name��parent
  ��ƫ����offset��node��ȥƫ����offset�����ɵõ�node��Ӧ��parent���͵ı���
*/
#define tNodeParent(node, parent, name)   (parent*)((uint32_t)node - (uint32_t)&((parent*)0)->name)

/*------����&APP��ʼ�����API------*/
void tTaskInit(tTask *task, void(*entry)(void *), void *param, uint8_t prio, tTaskStack *stackStart, uint32_t stackSize);
void tTaskSched(void);
void tInitApp(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
