/*
*********************************************************************************************************
*
*	ģ������   :   �ź�����ع���ͷ�ļ�
*	�ļ�����   :   tSem.h
*	��    ��   :   V1.0
*	˵    ��   :   �����ź������ݽṹ���ź�����Ϣ���ݽṹ���ṩ�ź�����ʼ����
*                �ȴ��ź������ͷ��ź�����ز���API
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

#ifndef _TSEM_H
#define _TSEM_H

#include "tEvent.h"

/*------sem���ݽṹ------*/
typedef struct _tSem
{
	tEvent event;                                    /* ECB                                                 */    
	uint32_t count;                                  /* ������                                              */
	uint32_t maxCount;                               /* ���������ֵ                                        */
}tSem;

/*------�ź�����Ϣ���ݽṹ------*/
typedef struct _tSemInfo
{
	uint32_t count;                                  /* �ź����������ĳ�ʼֵ                                */ 
	uint32_t maxCount;                               /* �ź��������������ֵ                                */ 
	uint32_t taskCount;                              /* ECB�ȴ������е���������                             */ 
}tSemInfo;

/*------sem��ʼ�����ȴ������ѵ�API------*/
void tSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount);
uint32_t tSemWait(tSem *sem, uint32_t timeOut);
uint32_t tSemNoWaitGet(tSem *sem);
void tSemNotify(tSem *sem);
void tSemGetInfo(tSem *sem, tSemInfo *info);
uint32_t tSemDelete(tSem *sem);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
