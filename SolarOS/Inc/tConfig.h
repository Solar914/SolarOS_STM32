/*
*********************************************************************************************************
*
*	ģ������   :   ϵͳ��������ͷ�ļ�
*	�ļ�����   :   tConfig.h
*	��    ��   :   V1.0
*	˵    ��   :   ����ϵͳ֧�ֵ����ȼ������������ջ��С���ں˹��ܵĲü�����
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

#ifndef __TCONFIG_H
#define __TCONFIG_H

/*------ϵͳ���ȼ��������ջ��С����------*/
#define TASK_MAX_PRIO_COUNT                    32            /* ����ϵͳ֧�ֵ�������ȼ�����              */
#define SOLAROS_TASK_STACK_SIZE                1024          /* ������ͨ�����ջ��С                      */
#define SOLAROS_IDLETASK_STACK_SIZE            1024          /* ������������ջ��С                      */
#define TASK_MAX_SLICE_COUNT                   10            /* ����ͬ���ȼ���������ʱ��Ƭ              */
#define SOLAROS_SOFTTIMER_TASK_PRIO            1             /* ��ʱsoft��ʱ�������ȼ�                    */
#define SOLAROS_SOFTTIMER_TASK_STACK_SIZE      1024          /* ����soft��ʱ������Ķ�ջ��С              */

/*------����������ƿ���------*/
#define SOLAROS_ENABLE_SEM                     1             /* �ź���ʹ�ܿ���                            */
#define SOLAROS_ENABLE_MBOX                    1             /* ����ʹ�ܿ���                              */
#define SOLAROS_ENABLE_MEMBLOCK                1             /* �洢��ʹ�ܿ���                            */
#define SOALROS_ENABLE_FLAGGROUP               1             /* �¼���־��ʹ�ܿ���                        */
#define SOLAROS_ENABLE_MUTEX                   1             /* �����ź���ʹ�ܿ���                        */
#define SOLAROS_ENABLE_TIMER                   1             /* ��/Ӳ��ʱ��ʹ�ܿ���                       */
#define SOLAROS_ENABLE_THOOKS                  1             /* hooks����ʹ�ܿ���                         */

#endif

/***************************** SOLAROS (END OF FILE) *********************************/
