/*
*********************************************************************************************************
*
*	ģ������   :   �洢����ع���ͷ�ļ�
*	�ļ�����   :   tMemBlock.h
*	��    ��   :   V1.0
*	˵    ��   :   ����洢�����ݽṹ���洢����Ϣ���ݽṹ���ṩ�洢���ʼ����
*                �ȴ����д洢���������д洢������洢����ز���API
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

#ifndef _TMEMBLOCK_H
#define _TMEMBLOCK_H

#include "SolarOS.h"
#include "tList.h"
#include "tEvent.h"

/*------�����洢�����ݽṹ------*/
typedef struct _tMemBlock
{
	tEvent event;                                    /* ECB,���ڹ����������                                     */
	void *memStart;                                  /* �ڴ�����ʼ��ַ                                         */
	uint32_t blockSize;                              /* �����ڴ��Ĵ�С                                         */
	uint32_t blockCnt;                               /* �ڴ�������                                             */
	tList blockList;                                 /* �������ڴ�������������������й���                       */
}tMemBlock;

/*------�洢����Ϣ���ݽṹ------*/
typedef struct _tMemBlockInfo
{
	uint32_t BlockCnt;                               /* ���ô洢�������                                         */
  uint32_t maxBlockCnt;                            /* ���Ĵ洢�������                                       */
	uint32_t blockSize;                              /* �����洢��Ĵ�С                                         */
	uint32_t taskCount;                              /* �ȴ��洢������������                                   */
}tMemBlockInfo;

/*------�����洢���ʼ�����ȴ������ѵ�API------*/
void tMemBlockInit(tMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait(tMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet(tMemBlock *memBlock, uint8_t **mem);
void tMemBlockNotify(tMemBlock *memBlock, uint8_t *mem);
void tMemBlockGetInfo(tMemBlock *memBlock, tMemBlockInfo *info);
uint32_t tMemBlockDelete(tMemBlock *memBlock);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
