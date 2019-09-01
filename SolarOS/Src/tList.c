/*
*********************************************************************************************************
*
*	模块名称   :   双向循环链表实现模块。
*	文件名称   :   tList.c
*	版    本   :   V1.0
*	说    明   :   定义双向循环链表数据结构，链表初始化，链表头部、尾部
*                插入结点，删除结点
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

#include "tList.h"

/*
*********************************************************************************************************
*	函 数 名  :   tNodeInit
*	功能说明  :   结点初始化
*	形    参  :   node：需要被初始化的结点
*	返 回 值  :   无
*********************************************************************************************************
*/
void tNodeInit(tNode *node)
{
	/*node的后向结点是node结点本身*/
	node->nextNode = node;
	
	/*node的前向结点是node结点本身*/
	node->preNode = node;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListInit
*	功能说明  :   链表初始化
*	形    参  :   list：需要被初始化的链表
*	返 回 值  :   无
*********************************************************************************************************
*/
void tListInit(tList *list)
{
	/*头结点的后向结点是头结点本身*/
	list->firstNode = &(list->headNode);
	
	/*头结点的前向结点是头结点本身*/
	list->lastNode = &(list->headNode);
	
	/*列表中的结点数量是0*/
	list->nodeCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListCount
*	功能说明  :   返回链表中结点的数量
*	形    参  :   list：链表
*	返 回 值  :   nodeCount：链表中结点的数量
*********************************************************************************************************
*/
uint32_t tListCount(tList *list)
{
	return list->nodeCount;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListFirstNode
*	功能说明  :   返回链表中的第一个结点
*	形    参  :   list：链表
*	返 回 值  :   node：链表中的第一个结点
*********************************************************************************************************
*/
tNode *tListFirstNode(tList *list)
{
	/*定义一个结点node*/
	tNode *node = (tNode*)0;
	
	/*如果链表不为空*/
	if(list->nodeCount != 0)
	{
		/*将链表中的第一个结点赋值给node*/
		node = list->firstNode;
	}
	
	return node;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListLastNode
*	功能说明  :   返回链表中的最后一个结点
*	形    参  :   list：链表
*	返 回 值  :   node：链表中的最后一个结点
*********************************************************************************************************
*/
tNode *tListLastNode(tList *list)
{
	tNode *node = (tNode*)0;
	
	if(list->nodeCount != 0)
	{
		node = list->lastNode;
	}
	
	return node;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListNodePre
*	功能说明  :   返回链表中的指定结点的前向结点
*	形    参  :   list：链表
*               node: 链表中的指定结点
*	返 回 值  :   preNode：指定结点的前向结点
*********************************************************************************************************
*/
tNode *tListNodePre(tList *list, tNode *node)
{
	/*如果该结点的前向结点是其本身，说明链表中只有头结点，nodeCount=0*/
	if(node->preNode == node)
	{
		/*返回0*/
		return (tNode*)0;
	}
	
	/*若正常*/
	else
	{
		/*返回前向结点*/
		return node->preNode;
	}
}

/*
*********************************************************************************************************
*	函 数 名  :   tListNodeNext
*	功能说明  :   返回链表中的指定结点的后向结点
*	形    参  :   list：链表
*               node: 链表中的指定结点
*	返 回 值  :   nextNode：指定结点的后向结点
*********************************************************************************************************
*/
tNode *tListNodeNext(tList *list, tNode *node)
{
	/*如果该结点的后向结点是其本身，说明该链表中只有头结点，nodeCount=0*/
	if(node->nextNode == node)
	{
		/*返回0*/
		return (tNode*)0;
	}
	
	/*若正常*/
	else
	{
		/*返回后向结点*/
		return node->nextNode;
	}
}
	
/*
*********************************************************************************************************
*	函 数 名  :   tListRemoveAll
*	功能说明  :   删除链表中的所有结点
*	形    参  :   list：链表
*	返 回 值  :   无
*********************************************************************************************************
*/
void tListRemoveAll(tList *list)
{
	uint32_t count;
	tNode *tempNode;
	
	/*定义一个临时结点，用来存放需要遍历的结点，先从第一个结点开始*/
	tempNode = list->firstNode;
	for(count = list->nodeCount; count != 0; count--)
	{
		/*将临时结点赋值给currentNode*/
		tNode *currentNode = tempNode;
		
		/*临时结点指向下一个结点*/
		tempNode = tempNode->nextNode;
		
		/*当前结点的后向结点指向其本身*/
		currentNode->nextNode = currentNode;
		
		/*当前结点的前向结点指向其本身*/
		currentNode->preNode = currentNode;
	}
	
	/*链表中头结点的后向结点指向头结点*/
	list->firstNode = &(list->headNode);
	
  /*链表中头结点的前向结点指向头结点*/
	list->lastNode = &(list->headNode);
	
	/*链表中的结点数量为0*/
	list->nodeCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListInsertBeforeFirst
*	功能说明  :   在链表中的headNode头结点和firstNode结点之间插入node结点
*               1、node的后向结点为头结点的后向结点
*               2、node的前向结点为头结点的后向结点的前向结点
*               3、头结点的后向结点的前向结点为node
*               4、头结点的后向结点为node
*               5、链表中结点的数量加1
*	形    参  :   list：链表
*               node: 待插入的结点
*	返 回 值  :   无
*********************************************************************************************************
*/
void tListInsertBeforeFirst(tList *list, tNode *node)
{
	
	node->preNode = list->headNode.nextNode->preNode;
	node->nextNode = list->headNode.nextNode;
	
	list->headNode.nextNode->preNode = node;
	list->headNode.nextNode = node;

	list->nodeCount++;
	
}

/*
*********************************************************************************************************
*	函 数 名  :   tListInsertAfterLast
*	功能说明  :   在链表中的lastNode结点和头结点之间插入node结点
*               1、node的后向结点为头结点的前向结点的后向结点
*               2、node的前向结点为头结点的前向结点
*               3、头结点的前向结点的后向结点为node
*               4、头结点的前向结点为node
*               5、链表中的结点数量加1
*	形    参  :   list：链表
*               node: 待插入的结点
*	返 回 值  :   无
*********************************************************************************************************
*/
void tListInsertAfterLast(tList *list, tNode *node)
{
	node->nextNode = list->headNode.preNode->nextNode;
	node->preNode = list->headNode.preNode;

	list->headNode.preNode->nextNode = node;	
	list->headNode.preNode = node;

	list->nodeCount++;

}

/*
*********************************************************************************************************
*	函 数 名  :   tListRemoveFirstNode
*	功能说明  :   删除firstNode并返回该结点
*               1、判断链表是否为空，为空返回0
*               2、若链表不为空，将firstNode赋值给node
*               3、firstNode的后向结点的前向结点为头结点
*               4、头结点的后向结点为firstNode的后向结点
*               5、结点数量减1
*	形    参  :   list：链表
*	返 回 值  :   node: 被删除的结点
*********************************************************************************************************
*/
tNode *tListRemoveFirstNode(tList *list)
{
	tNode *node = (tNode*)0;
	if(list->nodeCount != 0)
	{
		/*将firstNode赋值给node*/
		node = list->headNode.nextNode;
		
		/*firstNode结点的后向结点的前向结点为first的前向结点*/
		node->nextNode->preNode = node->preNode;
		
		/*头结点的后向结点为firstNode的后向结点*/
		list->headNode.nextNode = node->nextNode;
		
		/*结点数量减1*/
		list->nodeCount--;
		
	}
	return node;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListInsertAfterNode
*	功能说明  :   在链表中指定结点的后部插入结点
*	形    参  :   list：链表
*               nodeAfter：指定的结点
*               nodeToInsert：待插入的结点
*	返 回 值  :   无
*********************************************************************************************************
*/
void tListInsertAfterNode(tList *list, tNode *nodeAfter, tNode *nodeToInsert)
{
	/*修改nodeToInsert的前向结点和后向结点*/
	nodeToInsert->nextNode = nodeAfter->nextNode;
	nodeToInsert->preNode = nodeAfter->nextNode->preNode;
	
	/*首先修改nodeAfter结点的后向结点的前向结点，再修改nodeAfter结点的后向结点，注意修改顺序*/
	nodeAfter->nextNode->preNode = nodeToInsert;
	nodeAfter->nextNode = nodeToInsert;

	/*结点数量加1*/
	list->nodeCount++;
}

/*
*********************************************************************************************************
*	函 数 名  :   tListRemoveNode
*	功能说明  :   在链表中删除指定的结点
*	形    参  :   list：链表
*               node：待删除的结点
*	返 回 值  :   无
*********************************************************************************************************
*/
void tListRemoveNode(tList *list, tNode *node)
{
	/*修改node结点的前向结点的后向结点指向，指向node结点的后向结点*/
	node->preNode->nextNode = node->nextNode;
	
	/*修改node结点的后向结点的前向结点指向，指向node结点的前向结点*/
	node->nextNode->preNode = node->preNode;
	
	/*结点数量减1*/
	list->nodeCount --;
}

/***************************** SOLAROS (END OF FILE) *********************************/
