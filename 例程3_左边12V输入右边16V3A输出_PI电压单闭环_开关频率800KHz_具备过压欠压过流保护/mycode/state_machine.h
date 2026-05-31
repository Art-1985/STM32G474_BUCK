/****************************************************************************************
  * @file    state_machine.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/


#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "common.h"

void state_machine(void);
void Set_duty(uint16_t Time_width);         //设定占空比
void Voltage_Loop(void);
void MODS_01H(void);
void MODS_06H(void);
#define PWM_PERIOD  6800
typedef enum {STOP = 0, Run= !STOP} DP_FlagStatus;
typedef enum {Error = 0, Normal= !Error} Error_FlagStatus;
typedef enum {ON = 0, Miss= !ON} State_FlagStatus;

#endif
