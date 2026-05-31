#ifndef __RGB_H_
#define __RGB_H_
/****************************************************************************************
  * @file    RGB.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
#include "common.h"

void LED_GPIO_CONFIG(void);

void Red_ON(void);
void Green_ON(void);
void POWER_BOARD_RED_ON(void);
void POWER_BOARD_Green_ON(void);
#define ENdriver  GPIOA->BSRR = GPIO_PIN_11
#define DISdriver GPIOA->BRR = GPIO_PIN_11


#endif
