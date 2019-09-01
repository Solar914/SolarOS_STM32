/*
*********************************************************************************************************
*
*	ģ������   :   ������ع���ͷ�ļ�
*	�ļ�����   :   tMbox.h
*	��    ��   :   V1.0
*	˵    ��   :   �����������ݽṹ��������Ϣ���ݽṹ���ṩ�����ʼ����
*                �ȴ������е���Ϣ����������д����Ϣ��ز���API
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

#ifndef _TMBOX_H
#define _TMBOX_H

#include "tEvent.h"

/*------��������д�����ݵķ�ʽ------*/
#define  tMBOXSendNormal   0x00                         /* ������ʽ����Ϣ������д����Ϣ                        */
#define  tMBOXSendFront    0x01                         /* ��д�����Ϣ�ȱ���ȡ                                */

/*------�������ݽṹ------*/
typedef struct _tMbox
{
	tEvent event;                                         /* ECB                                                 */
	uint32_t count;                                       /* ��ǰ����Ϣ����                                      */
	uint32_t maxCount;                                    /* ������Ϣ����                                      */
	uint32_t read;                                        /* ������                                              */
	uint32_t write;                                       /* д����                                              */
	void **msgBuffer;                                     /* ��Ϣ������                                          */
}tMbox;

/*------������Ϣ���ݽṹ------*/
typedef struct _tMboxInfo
{
	uint32_t count;                                       /* ��Ϣ����������Ϣ������                              */
	uint32_t maxCount;                                    /* ��Ϣ������������Ϣ                                */
	uint32_t taskCount;                                   /* �ȴ���Ϣ�����������                                */
}tMboxInfo;

/*------�����ʼ�����ȴ������ѵ�API------*/
void tMboxInit(tMbox *Mbox, void **msgBuffer, uint32_t maxCount);
uint32_t tMboxWait(tMbox *mbox, void **msg, uint32_t waitTicks);
uint32_t tMboxNoWait(tMbox *mbox, void **msg);
uint32_t tMboxNotify(tMbox *mbox, void *msg, uint32_t notifyOption);
void tMboxClear(tMbox *mbox);
uint32_t tMboxDelete(tMbox *mbox);
void tMboxGetInfo(tMbox *mbox, tMboxInfo *info);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
