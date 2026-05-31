#include "RGB.h"

/****************************************************************************************
  * @file    RGB.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
/***********************************************************************************/
//控制LED的IO口可以参考原理图或者Config.h文件
// PA15     RED         高电平有效   
// PB13     GREEN       高电平有效    
// PB12     Power_board       高电平亮绿灯，低电平亮红灯 
// PA11     使能Driver
/***********************************************************************************/
void LED_GPIO_CONFIG(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;                         //定义结构体
	
	__HAL_RCC_GPIOA_CLK_ENABLE();			                        //开启GPIOB时钟
	__HAL_RCC_GPIOB_CLK_ENABLE();			                        //开启GPIOB时钟
	
	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_15;                        //选择IO管脚
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;              //选择推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;               //IO口速度设置为低速
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                   //初始化
	GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_12;            //选择IO管脚
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                   //初始化	
  Red_ON();     Delay_ms(500);                              //点亮红色LED
}

void Red_ON(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);       //PA15输出低电平3.3V
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);     //PB15输出低电平0V
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);       //PB12输出低电平0.0V
}


void Green_ON(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);       //PA15输出低电平0.0V
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);         //PB15输出低电平3.3V
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);         //PB12输出低电平3.3V
}

void POWER_BOARD_RED_ON(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);       //PB12输出低电平0.0V
}
void POWER_BOARD_Green_ON(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);         //PB12输出低电平3.3V
}



