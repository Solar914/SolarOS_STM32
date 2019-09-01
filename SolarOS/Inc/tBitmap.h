/*
*********************************************************************************************************
*
*	ģ������   :   λͼ���ݽṹͷ�ļ�
*	�ļ�����   :   tBitmap.h
*	��    ��   :   V1.0
*	˵    ��   :   ����λͼ���ݽṹ���ṩλͼ������ز���API
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

#ifndef _TLIB_H
#define _TLIB_H

#include <stdint.h>

/*------λͼ���ݽṹ------*/
typedef struct
{
	uint32_t tbitmap;
}tBitmap;

/*------λͼ������ʼ��------*/
void tBitmapInit(tBitmap *tbitmap);

/*------��λ��������ȡ��λΪ1������API------*/
void tBitmapSet(tBitmap *tbitmap, uint8_t pos);
void tBitmapReset(tBitmap *tbitmap, uint8_t pos);
uint8_t tBitmapGetFirstSet(tBitmap *tbitmap);
uint8_t tBitmapPosLen(void);

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
