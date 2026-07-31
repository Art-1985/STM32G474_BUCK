/****************************************************************************************
  * @file    PID.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#ifndef __type3__H__
#define __type3__H__

#include "common.h"


void  PID_INT(void);//PID初始化
void  PID_loop(float voltage,float Current);

typedef struct 
{
float Kp;
float Ki;
float Kp_i;
float error1;
float error2;
float error_add;	
} type_pid;
#endif
