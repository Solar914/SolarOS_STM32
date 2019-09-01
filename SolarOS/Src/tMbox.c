/*
*********************************************************************************************************
*
*	ģ������   :   ����ʵ��ģ�顣
*	�ļ�����   :   tMbox.c
*	��    ��   :   V1.0
*	˵    ��   :   �����������ݽṹ�������ʼ�����������еȴ���Ϣ��
*                ��������д����Ϣ������������Ϣ����������ȡ����
*                ��״̬��ɾ�������API��ʵ��
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
#include "tMbox.h"

#if SOLAROS_ENABLE_MBOX == 1

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxInit
*	����˵��  :   �����ʼ��
*	��    ��  :   mbox������
*               msgBuffer����Ϣָ�뻺����
*               maxCount��������ֵ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tMboxInit(tMbox *mbox, void **msgBuffer, uint32_t maxCount)
{
	/*ECB��ʼ��*/
	tEventInit(&mbox->event, tEventTypeMbox);
	
	/*��ʼ����Ϣ����������������д����*/
	mbox->count = 0;
	mbox->read = 0;
	mbox->write = 0;
	
	/*��ʼ����Ϣ�����������ֵ*/
	mbox->maxCount = maxCount;
	
	/*
	  ��ʼ����Ϣ��������ָ��msgBuffer���׵�ַ
	  eg�����ñ�����ʱ�������˲���mbox1MsgBuffer����void **msgBuffer = mbox1MsgBuffer��
	  mbox1MsgBuffer��һ��ָ�����飬ÿ��Ԫ�ش�ŵĶ���һ��ָ�룬ָ��ָ��ĳһ����ַ��
	  mbox1MsgBuffer��������׵�ַ������ŵ�ַ�ĵ�ַ��������Ҫ����ָ��msgBuffer������
	  ����ֵ����msgBuffer��ֵ����mbox1MsgBuffer���׵�ַ��
	*/
	mbox->msgBuffer = msgBuffer;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxWait
*	����˵��  :   �������еȴ���Ϣ������mbox--->msg
*	��    ��  :   mbox������
*               msg����Ŵ������л�ȡ����Ϣ
*               waitTicks���ȴ�ʱ��
*	�� �� ֵ  :   tErrorNoError��������
*********************************************************************************************************
*/
uint32_t tMboxWait(tMbox *mbox, void **msg, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();

	/*������������Ϣ����Ϣ������һ������Ϣ���ݸ�msg*/
	if(mbox->count > 0)
	{
		/*��Ϣ������һ*/
		mbox->count--;
		
		/*
		  �����е���Ϣ���ݸ�msg
		  msg�����Ƕ���ָ�룬*msg��ʾmsgָ��ĵ�ַ�д�ŵ�ֵ��
		  mbox->msgBuffer[mbox->read++]�д�ŵ���һ������ַ��
		  *msg = mbox->msgBuffer[mbox->read++]�൱�ڰ������д��
		  �ĵ�ַ���ݸ�msg��ͨ��**msg�������Ϳ���ȡ���õ�ַ��Ӧ��ֵ��
		*/
		*msg = mbox->msgBuffer[mbox->read++];
		
		/*�ж϶������Ƿ���Ҫ����*/
		if(mbox->read >= mbox->maxCount)
		{
			/*�������ƻص���ͷ��*/
			mbox->read = 0;
		}
		tTaskExitCritical(status);
		
		/*�����޴���*/
		return tErrorNoError;
	}	
	
	/*������������Ϣ���ͽ�������뵽ECB�ĵȴ�������*/
	else
	{
		/*���뵽ECB��waitlist*/
		tEventWait(&mbox->event, currentTask, (void*)0, tEventTypeMbox, waitTicks);
		tTaskExitCritical(status);
		
		//�������
		tTaskSched();
		
		/*
		  �������л������������������һ���ǵȴ���ʱ���л���������ʱ��msg����tDelayTickHandler
		  �жϻ���ʱ�����ֵ����һ���Ǳ�tMboxNotify����ʱ��msg���ڻ���ʱ�����ֵ
		*/
		*msg = currentTask->eventMsg;
	
		/*�ȴ����eventWaitResult��ֵͬ����ʱ����tErrorTimeout�����������Ѿ���tErrorNoError*/
		return currentTask->eventWaitResult;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxNoWait
*	����˵��  :   �޵ȴ��������л�ȡ��Ϣ������mbox--->msg
*	��    ��  :   mbox������
*               msg����Ŵ������л�ȡ����Ϣ
*	�� �� ֵ  :   tErrorNoError��������
*********************************************************************************************************
*/
uint32_t tMboxNoWait(tMbox *mbox, void **msg)
{
	uint32_t status = tTaskEnterCritical();
	
	if(mbox->count > 0)
	{
		mbox->count--;
		*msg = mbox->msgBuffer[mbox->read++];
		if(mbox->read >= mbox->maxCount)
		{
			mbox->read = 0;
		}
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	/*��û����Ϣ*/
	else
	{
		tTaskExitCritical(status);
		
		/*��������Դ���ô���*/
		return tErrorResourceUnavaliable;
	}
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxNotify
*	����˵��  :   ����Ϣ���뵽�����У�����msg--->mbox
*	��    ��  :   mbox������
*               msg���������������Ϣ
*               notifyOption��notifyOptionΪtMBOXSendFrontʱ����ʾ��д��������ȱ���ȡ
*	�� �� ֵ  :   tErrorNoError��������
*********************************************************************************************************
*/
uint32_t tMboxNotify(tMbox *mbox, void *msg, uint32_t notifyOption)
{
	uint32_t status = tTaskEnterCritical();
	
	/*���ECB�ĵȴ������������񣬽�msg���ݸ����񣬲��������񣬽����������*/
	if(tEventWaitCount(&mbox->event) > 0)
	{
		/*��������*/
		tTask *task = tEventWakeUp(&mbox->event, msg, tErrorNoError);
		
		/*�ж����ȼ�*/
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	
	/*���û�������ڵȴ����Ͱ���Ϣ�ŵ�������*/
	else
	{
		/*����Ϣ����������*/
		if(mbox->count >= mbox->maxCount)
		{
			tTaskExitCritical(status);
			/*������Դ������*/
			return tErrorResourceFull;
		}
		
		/*���Ǻ�д�����Ϣ�ȱ���ȡ*/
		if(notifyOption & tMBOXSendFront)
		{
			/*����������ǰ������ͷ*/
			if(mbox->read-- <= 0)
			{
				/*�����������Ƶ���Ϣ��������ĩβ*/
				mbox->read = mbox->maxCount - 1;
			}
			/*
			  ��msg���뵽��Ϣ�����������ñ�����ʱ�����˲���&msg[i]����void *msg = &msg[i]; 
			  ��msg[i]�ĵ�ַ���������ָ��msg������&msg[i]�ĵ�ַ��0x100����ômsg��ֵ����0x100
			  mbox->msgBuffer[mbox->read] = msg; ��ʾ��0x100�ŵ�����msgBuffer��
			*/
			mbox->msgBuffer[mbox->read] = msg;
		}
		
		/*������������ʽд����Ϣ������*/
		else
		{
			/*д�뵽��Ϣ������*/
			mbox->msgBuffer[mbox->write++] = msg;
			
			/*��д�����ѵ��ﻺ����ĩβ*/
			if(mbox->write >= mbox->maxCount)
			{
				/*���������������ͷ��*/
				mbox->write = 0;
			}
		}
		/*��Ϣ��������1*/
		mbox->count++;
	
	}
	tTaskExitCritical(status);
	return tErrorNoError;

}

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxClear
*	����˵��  :   ���������м���������������д����
*	��    ��  :   mbox������
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tMboxClear(tMbox *mbox)
{
	uint32_t status = tTaskEnterCritical();
	
	/*���û�������ڵȴ���˵�����䲻Ϊ�գ���Ҫ��գ�����������ڵȴ���˵������Ϊ��*/
	if(tEventWaitCount(&mbox->event) == 0)
	{
		mbox->count = 0;
		mbox->read = 0;
		mbox->write = 0;
	}
	
	tTaskExitCritical(status);
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxDelete
*	����˵��  :   �������ECB�еĵȴ�����
*	��    ��  :   mbox������
*	�� �� ֵ  :   count���ȴ������е���������
*********************************************************************************************************
*/
uint32_t tMboxDelete(tMbox *mbox)
{
	uint32_t status = tTaskEnterCritical();
	
	/*ͳ��������ECB��waitlist�е���������*/
	uint32_t count = tEventWaitCount(&mbox->event);
	
	tTaskExitCritical(status);
	
	/*��յȴ����У��������񱻻���*/
	tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);
	
	/*�����������*/
	if(count > 0)
	{
		tTaskSched();
	}
	return count;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tMboxGetInfo
*	����˵��  :   ��ȡ�����е���Ϣ
*	��    ��  :   mbox������
*               info����Ŵ������л�ȡ����Ϣ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tMboxGetInfo(tMbox *mbox, tMboxInfo *info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->count = mbox->count;
	info->maxCount = mbox->maxCount;
	info->taskCount = tEventWaitCount(&mbox->event);
	
	tTaskExitCritical(status);
}

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
