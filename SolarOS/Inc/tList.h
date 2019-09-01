/*
*********************************************************************************************************
*
*	ģ������   :   ˫��ѭ������ͷ�ļ�
*	�ļ�����   :   tList.h
*	��    ��   :   V1.0
*	˵    ��   :   ���������ݽṹ���������ݽṹ���ṩ�����ʼ����
*                ����Ĳ��롢ɾ����������ز���API                 
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

#ifndef _T_LIST_H
#define _T_LIST_H

#include <stdint.h>

/*------������ݽṹ------*/
typedef struct _tNode
{
	struct _tNode *preNode;
	struct _tNode *nextNode;
}tNode;

/*------�������ݽṹ------*/
typedef struct _tList
{
	tNode headNode;
	uint32_t nodeCount;
}tList;

/*------�׽���β���궨��------*/
#define firstNode    headNode.nextNode                   /* �����еĵ�һ�������ͷ���ĺ�����         */
#define lastNode     headNode.preNode                    /* �����е����һ�����ͷ����ǰ����         */

/*------��㡢�����ʼ��------*/
void tNodeInit(tNode *node);
void tListInit(tList *list);

/*------�������ͳ�ơ�����ָ�����------*/
uint32_t tListCount(tList *list);
tNode *tListFirstNode(tList *list);
tNode *tListLastNode(tList *list);
tNode *tListNodePre(tList *list, tNode *node);
tNode *tListNodeNext(tList *list, tNode *node);

/*------����Ĳ��롢ɾ��------*/
void tListRemoveAll(tList *list);
void tListInsertBeforeFirst(tList *list, tNode *node);
void tListInsertAfterLast(tList *list, tNode *node);
tNode *tListRemoveFirstNode(tList *list);
void tListInsertAfterNode(tList *list, tNode *nodeAfter, tNode *nodeToInsert);
void tListRemoveNode(tList *list, tNode *node);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
