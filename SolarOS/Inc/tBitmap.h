/*
*********************************************************************************************************
*
*	模块名称   :   位图数据结构头文件
*	文件名称   :   tBitmap.h
*	版    本   :   V1.0
*	说    明   :   定义位图数据结构，提供位图数据相关操作API
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

#ifndef _TLIB_H
#define _TLIB_H

#include <stdint.h>

/*------位图数据结构------*/
typedef struct
{
	uint32_t tbitmap;
}tBitmap;

/*------位图变量初始化------*/
void tBitmapInit(tBitmap *tbitmap);

/*------置位操作，获取首位为1操作等API------*/
void tBitmapSet(tBitmap *tbitmap, uint8_t pos);
void tBitmapReset(tBitmap *tbitmap, uint8_t pos);
uint8_t tBitmapGetFirstSet(tBitmap *tbitmap);
uint8_t tBitmapPosLen(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
