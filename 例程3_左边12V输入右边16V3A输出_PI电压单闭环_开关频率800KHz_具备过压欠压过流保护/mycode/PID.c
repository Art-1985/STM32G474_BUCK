/****************************************************************************************
  * @file    PID.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "PID.h"
/****************************电压环参数**********************************************/
type_pid V_PI={0};                   //电压环PI
uint16_t Pulse_width=0.0f;           //占空比
float    PWM_K=PWM_PERIOD*0.303030f; //脉宽常数
extern float   Vref;                 //电压参考值
void PID_INT(void)//PID初始化
{
	V_PI.error1=0.0f;
	V_PI.error2=0.0f;
	V_PI.error_add=0.0f;
	V_PI.Ki=0.002f;
	V_PI.Kp=0.015f;
	V_PI.Kp_i=V_PI.Ki+V_PI.Kp;

	Pulse_width=0.015f*PWM_PERIOD*0.303030f;//最小脉宽
}
/*****************************************************
如果对PI闭环控制不理解，可以哔哩哔哩搜索：尖叫的变压器，观看通用视频教程
*****************************************************/
void PID_loop(float voltage)
{
	//电压环
	V_PI.error1=Vref-voltage;//获取误差差值
	V_PI.error_add+=V_PI.Kp_i*V_PI.error1-V_PI.Kp*V_PI.error2;//PI增量
	V_PI.error2=V_PI.error1;//移位
	if(V_PI.error_add>2.145f)V_PI.error_add=2.145f;//限制最大占空比:2.145f/3.3f=65%
	else if(V_PI.error_add<0.05f)V_PI.error_add=0.05f;//限制最小占空比:0.05f/3.3f=1.5%
  //双环竞争，占空比=(误差电压/3.3V)，脉宽=占空比*周期
	Pulse_width=PWM_K*V_PI.error_add;//脉宽计算可以到哔哩哔哩观看我们的视频教程
	//更新寄存器
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR=Pulse_width;//更新占空比
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP3xR=Pulse_width>>1;//在脉宽中点采集数据，避免噪声   
}
