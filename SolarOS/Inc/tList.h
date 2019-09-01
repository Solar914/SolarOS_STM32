/*
*********************************************************************************************************
*
*	模块名称   :   双向循环链表头文件
*	文件名称   :   tList.h
*	版    本   :   V1.0
*	说    明   :   定义结点数据结构，链表数据结构，提供链表初始化，
*                链表的插入、删除、查找相关操作API                 
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

#ifndef _T_LIST_H
#define _T_LIST_H

#include <stdint.h>

/*------结点数据结构------*/
typedef struct _tNode
{
	struct _tNode *preNode;
	struct _tNode *nextNode;
}tNode;

/*------链表数据结构------*/
typedef struct _tList
{
	tNode headNode;
	uint32_t nodeCount;
}tList;

/*------首结点和尾结点宏定义------*/
#define firstNode    headNode.nextNode                   /* 链表中的第一个结点是头结点的后向结点         */
#define lastNode     headNode.preNode                    /* 链表中的最后一个结点头结点的前向结点         */

/*------结点、链表初始化------*/
void tNodeInit(tNode *node);
void tListInit(tList *list);

/*------结点数量统计、返回指定结点------*/
uint32_t tListCount(tList *list);
tNode *tListFirstNode(tList *list);
tNode *tListLastNode(tList *list);
tNode *tListNodePre(tList *list, tNode *node);
tNode *tListNodeNext(tList *list, tNode *node);

/*------链表的插入、删除------*/
void tListRemoveAll(tList *list);
void tListInsertBeforeFirst(tList *list, tNode *node);
void tListInsertAfterLast(tList *list, tNode *node);
tNode *tListRemoveFirstNode(tList *list);
void tListInsertAfterNode(tList *list, tNode *nodeAfter, tNode *nodeToInsert);
void tListRemoveNode(tList *list, tNode *node);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
