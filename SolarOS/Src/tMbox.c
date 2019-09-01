/*
*********************************************************************************************************
*
*	模块名称   :   邮箱实现模块。
*	文件名称   :   tMbox.c
*	版    本   :   V1.0
*	说    明   :   定义邮箱数据结构，邮箱初始化，从邮箱中等待消息，
*                向邮箱中写入消息，清空邮箱的消息缓冲区，获取邮箱
*                的状态，删除邮箱等API的实现
*
*	修改记录   :
*		             版本号     日期         作者            说明
*		              V1.0    2019-4-16      Solar           第一次版本发布
*		              
*
*	版权       ：  仅限学习交流使用，禁止用于商业用途
*
*********************************************************************************************************
*/

#include "SOLAROS.h"
#include "tMbox.h"

#if SOLAROS_ENABLE_MBOX == 1

/*
*********************************************************************************************************
*	函 数 名  :   tMboxInit
*	功能说明  :   邮箱初始化
*	形    参  :   mbox：邮箱
*               msgBuffer：消息指针缓冲区
*               maxCount：最大计数值
*	返 回 值  :   无
*********************************************************************************************************
*/
void tMboxInit(tMbox *mbox, void **msgBuffer, uint32_t maxCount)
{
	/*ECB初始化*/
	tEventInit(&mbox->event, tEventTypeMbox);
	
	/*初始化消息计数器、读索引、写索引*/
	mbox->count = 0;
	mbox->read = 0;
	mbox->write = 0;
	
	/*初始化消息计数器的最大值*/
	mbox->maxCount = maxCount;
	
	/*
	  初始化消息缓冲区，指向msgBuffer的首地址
	  eg：调用本函数时，传入了参数mbox1MsgBuffer，即void **msgBuffer = mbox1MsgBuffer，
	  mbox1MsgBuffer是一个指针数组，每个元素存放的都是一个指针，指针指向某一个地址，
	  mbox1MsgBuffer是数组的首地址，即存放地址的地址，所以需要二级指针msgBuffer来接收
	  它的值，即msgBuffer的值就是mbox1MsgBuffer的首地址。
	*/
	mbox->msgBuffer = msgBuffer;
}

/*
*********************************************************************************************************
*	函 数 名  :   tMboxWait
*	功能说明  :   从邮箱中等待消息，方向：mbox--->msg
*	形    参  :   mbox：邮箱
*               msg：存放从邮箱中获取的消息
*               waitTicks：等待时长
*	返 回 值  :   tErrorNoError：错误码
*********************************************************************************************************
*/
uint32_t tMboxWait(tMbox *mbox, void **msg, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();

	/*若邮箱中有消息，消息数量减一，将消息传递给msg*/
	if(mbox->count > 0)
	{
		/*消息数量减一*/
		mbox->count--;
		
		/*
		  邮箱中的消息传递给msg
		  msg本身是二级指针，*msg表示msg指向的地址中存放的值，
		  mbox->msgBuffer[mbox->read++]中存放的是一个个地址，
		  *msg = mbox->msgBuffer[mbox->read++]相当于把数组中存放
		  的地址传递给msg，通过**msg操作，就可以取出该地址对应的值了
		*/
		*msg = mbox->msgBuffer[mbox->read++];
		
		/*判断读索引是否需要回绕*/
		if(mbox->read >= mbox->maxCount)
		{
			/*读索引绕回到开头处*/
			mbox->read = 0;
		}
		tTaskExitCritical(status);
		
		/*返回无错误*/
		return tErrorNoError;
	}	
	
	/*若邮箱中无消息，就将任务插入到ECB的等待队列中*/
	else
	{
		/*插入到ECB的waitlist*/
		tEventWait(&mbox->event, currentTask, (void*)0, tEventTypeMbox, waitTicks);
		tTaskExitCritical(status);
		
		//任务调度
		tTaskSched();
		
		/*
		  当任务切换回来后，有两种情况：一种是等待超时后切换回来，此时的msg是在tDelayTickHandler
		  中断唤醒时赋予的值，另一种是被tMboxNotify，此时的msg是在唤醒时赋予的值
		*/
		*msg = currentTask->eventMsg;
	
		/*等待结果eventWaitResult的值同理，超时就是tErrorTimeout，被正常唤醒就是tErrorNoError*/
		return currentTask->eventWaitResult;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tMboxNoWait
*	功能说明  :   无等待从邮箱中获取消息，方向：mbox--->msg
*	形    参  :   mbox：邮箱
*               msg：存放从邮箱中获取的消息
*	返 回 值  :   tErrorNoError：错误码
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
	
	/*若没有消息*/
	else
	{
		tTaskExitCritical(status);
		
		/*返回无资源可用错误*/
		return tErrorResourceUnavaliable;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tMboxNotify
*	功能说明  :   将消息放入到邮箱中，方向：msg--->mbox
*	形    参  :   mbox：邮箱
*               msg：待放入邮箱的消息
*               notifyOption：notifyOption为tMBOXSendFront时，表示后写入的数据先被读取
*	返 回 值  :   tErrorNoError：错误码
*********************************************************************************************************
*/
uint32_t tMboxNotify(tMbox *mbox, void *msg, uint32_t notifyOption)
{
	uint32_t status = tTaskEnterCritical();
	
	/*如果ECB的等待队列中有任务，将msg传递给任务，并唤醒任务，进行任务调度*/
	if(tEventWaitCount(&mbox->event) > 0)
	{
		/*唤醒任务*/
		tTask *task = tEventWakeUp(&mbox->event, msg, tErrorNoError);
		
		/*判断优先级*/
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	
	/*如果没有任务在等待，就把消息放到缓冲区*/
	else
	{
		/*若消息缓冲区已满*/
		if(mbox->count >= mbox->maxCount)
		{
			tTaskExitCritical(status);
			/*返回资源满错误*/
			return tErrorResourceFull;
		}
		
		/*若是后写入的消息先被读取*/
		if(notifyOption & tMBOXSendFront)
		{
			/*若读索引被前移至开头*/
			if(mbox->read-- <= 0)
			{
				/*将读索引回绕到消息缓冲区的末尾*/
				mbox->read = mbox->maxCount - 1;
			}
			/*
			  将msg放入到消息缓冲区，调用本函数时传入了参数&msg[i]，即void *msg = &msg[i]; 
			  将msg[i]的地址赋予空类型指针msg，假设&msg[i]的地址是0x100，那么msg的值就是0x100
			  mbox->msgBuffer[mbox->read] = msg; 表示将0x100放到数组msgBuffer中
			*/
			mbox->msgBuffer[mbox->read] = msg;
		}
		
		/*若采用正常方式写入消息缓冲区*/
		else
		{
			/*写入到消息缓冲区*/
			mbox->msgBuffer[mbox->write++] = msg;
			
			/*若写索引已到达缓冲区末尾*/
			if(mbox->write >= mbox->maxCount)
			{
				/*将其回绕至缓冲区头部*/
				mbox->write = 0;
			}
		}
		/*消息计数器加1*/
		mbox->count++;
	
	}
	tTaskExitCritical(status);
	return tErrorNoError;

}

/*
*********************************************************************************************************
*	函 数 名  :   tMboxClear
*	功能说明  :   清理邮箱中计数器、读索引、写索引
*	形    参  :   mbox：邮箱
*	返 回 值  :   无
*********************************************************************************************************
*/
void tMboxClear(tMbox *mbox)
{
	uint32_t status = tTaskEnterCritical();
	
	/*如果没有任务在等待，说明邮箱不为空，需要清空，如果有任务在等待，说明邮箱为空*/
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
*	函 数 名  :   tMboxDelete
*	功能说明  :   清空邮箱ECB中的等待队列
*	形    参  :   mbox：邮箱
*	返 回 值  :   count：等待队列中的任务数量
*********************************************************************************************************
*/
uint32_t tMboxDelete(tMbox *mbox)
{
	uint32_t status = tTaskEnterCritical();
	
	/*统计邮箱中ECB的waitlist中的任务数量*/
	uint32_t count = tEventWaitCount(&mbox->event);
	
	tTaskExitCritical(status);
	
	/*清空等待队列，会有任务被唤醒*/
	tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);
	
	/*进行任务调度*/
	if(count > 0)
	{
		tTaskSched();
	}
	return count;
}

/*
*********************************************************************************************************
*	函 数 名  :   tMboxGetInfo
*	功能说明  :   获取邮箱中的信息
*	形    参  :   mbox：邮箱
*               info：存放从邮箱中获取的消息
*	返 回 值  :   无
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
