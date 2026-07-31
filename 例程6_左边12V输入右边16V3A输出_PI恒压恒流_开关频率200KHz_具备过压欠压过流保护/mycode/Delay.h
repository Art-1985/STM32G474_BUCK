#ifndef __DELAY_H__
#define __DELAY_H__

#include "common.h"
/****************************************************************************************
  * @file    delay.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
void Delay_us(uint16_t time);
void Delay_ms(__IO uint32_t nCount);

void Init_TIM_Basic(TIM_TypeDef *TIM_X);  //初始化定时器6做延时函数使用
#endif
