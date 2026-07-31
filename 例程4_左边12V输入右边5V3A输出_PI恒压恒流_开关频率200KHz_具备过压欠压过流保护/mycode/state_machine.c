/****************************************************************************************
  * @file    state_machine.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "state_machine.h"
/**********************定义变量**********************************/
float   Vin             =0.0f;  //输入电压
float   Vout            =0.0f;  //输出电压
float   Iin             =0.0f;  //输入电流
float   Iout            =0.0f;  //输出电流
float   Target_voltage  =0.0f;  //目标电压
float   Target_Current  =0.0f;  //目标电流
float   Vin_ovp         =0.0f;  //输入过压保护点
float   Vin_uvp         =0.0f;  //输入欠压保护点
float   Vout_ovp        =0.0f;  //输出过压保护点
float   Iout_ocp        =0.0f;  //输出电流过流保护点
float   IL_average      =0.0f;  //电感平均电流
float   Vref            =0.0f;  //电压参考值
float   Iref            =0.0f;  //电流参考值
float   Target_IL       =0.0f;  //平均电流目标值
float   ILref           =0.0f;  //平均电流参考值
uint32_t OCP_CNT        =0;     //OCP过流次数计数器
uint32_t OCP_CNT1       =0;     //OCP过流次数计数器
uint32_t OVP_CNT        =0;     //OVP过压次数计数器

/**********************定义标志位*********************************/
DP_FlagStatus    flag_Soft_start =Run;
DP_FlagStatus    flag_start_cnt  =Run;
DP_FlagStatus    ERROR_flag      =STOP;
DP_FlagStatus    Data_update_flag=STOP;
Error_FlagStatus flag_Vin_ovp    =Normal;
Error_FlagStatus flag_Vin_uvp    =Normal;
/**********************外部变量**************************************/   
extern   uint16_t  ADC1_RESULT[3];
extern   uint16_t  ADC2_RESULT[3]; 
extern   uint8_t   PC_command[50];
/********************************************************************/
//外部定义结构体：
extern DMA_HandleTypeDef    ADC2_DMA_Handler;
extern DMA_HandleTypeDef    ADC1_DMA_Handler;
extern HRTIM_HandleTypeDef  HRTIM1_structure;
extern DAC_HandleTypeDef      DAC_1_Handler;

/**********************复位变量**************************************/
 void Reset_VAR(void)
{
	DISdriver;             //关闭MOS驱动
	flag_Soft_start =Run;  //初始化FLAG
	flag_start_cnt  =Run;  //初始化FLAG 
	ERROR_flag      =STOP; //初始化FLAG  
	Data_update_flag=STOP;  //初始化FLAG  
  HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA1;	
  HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA2;
  Vin             =0.0f;  //输入电压
  Vout            =0.0f;  //输出电压
  Iin             =0.0f;  //输入电流
  Iout            =0.0f;  //输出电流
  Target_voltage  =5.0f;  //目标电压
  Target_Current  =3.0f;  //目标电流
  Vin_ovp         =24.0f; //输入过压保护点
  Vin_uvp         =8.0f; //输入欠压保护点
  Vout_ovp        =10.0f; //输出过压保护点
  Iout_ocp        =3.6f;  //输出电流过流保护点
  IL_average      =0.0f;  //电感平均电流
  Vref            =0.0f;  //电压参考值
  Iref            =0.0f;  //电流参考值
  Target_IL       =3.0f;  //平均电流目标值
  ILref           =0.0f;  //平均电流参考值
  OCP_CNT         =0;     //OCP过流次数计数器
  OCP_CNT1        =0;     //OCP过流次数计数器
	OVP_CNT         =0;     //OVP过压次数计数器
}

/**********************系统任务**************************************/
typedef enum
{
 Task_0_Initial_state, //初始状态
 Task_1_Vin_detc,      //检测输入电压
 Task_2_Vout_detc,     //检测输入电压
 Task_3_Iout_detc,     //检测输出电压
 Task_4_PC_command,    //检测输出电压
 Task_5_Soft_start     //开始缓启动
}System_Task;

 System_Task Current_State,Next_State; 

void state_machine(void)//状态机
{
	Next_State=Current_State=Task_0_Initial_state;
	while(1)
	{	 
		 Current_State=Next_State;
		 switch(Current_State)
			{
				case Task_0_Initial_state://初始状态
													{	 
                             Reset_VAR();										 //复位变量
														 Red_ON();
														 PID_INT();                      //PID补偿器初始化										  									                      													
														 Next_State=Task_1_Vin_detc;		 //下一状态：读取ADC值										
													}  
													break;
				case Task_1_Vin_detc://检测输入电压
													{			
                            if(Data_update_flag==STOP){Next_State = Task_1_Vin_detc;break;}	//数据没有更新
                            Data_update_flag=STOP;//HRTIMA中断函数里边中会更新数据Data_update_flag=Run													
														if (Vin > Vin_ovp) //过压
														{
															flag_Vin_ovp = Error;			   
															Next_State = Task_0_Initial_state; 
															break;
														}
														if (flag_Vin_ovp == Error)//过压中
														{
															if (Vin < Vin_ovp-2.0f)flag_Vin_ovp = Normal;
															else{Next_State = Task_0_Initial_state;break;} 
														}
														if (Vin < Vin_uvp)//欠压
														{
															flag_Vin_uvp = Error; 
															Next_State = Task_0_Initial_state;
															break;
														}
														if (flag_Vin_uvp == Error)//欠压中
														{
															if (Vin > Vin_uvp+2.0f)flag_Vin_uvp = Normal;
															else{Next_State = Task_0_Initial_state;break;}
														}
														Next_State = Task_2_Vout_detc;
													}
													break;	
				case Task_2_Vout_detc : //检测输出电压 
													{																	  
													  if(Vout>Vout_ovp)OVP_CNT++;//多次检测到过压，确定以及肯定是过压了
														else OVP_CNT=0;
														if(OVP_CNT>200){Next_State=Task_0_Initial_state;break;}
														Next_State=Task_3_Iout_detc;																																																		 												
													}					
													break;			
				case Task_3_Iout_detc : //检测输出电流
													{																	  
													  if(Iout>Iout_ocp)//多次检测到过流，确定以及肯定是过流了
                            {
															OCP_CNT1++;
															if(OCP_CNT1>2000){Next_State=Task_0_Initial_state;break;}
														}
														else OCP_CNT1=0;
														Next_State=Task_4_PC_command;																																																		 												
													}					
													break;		
				case Task_4_PC_command : //检测上位机指令
													{
                            if(flag_start_cnt!=STOP)	Next_State=Task_5_Soft_start ; 
                            else 		Next_State=Task_1_Vin_detc;																																																														 												
													}					
													break;														
				case Task_5_Soft_start ://缓启动
													{
                            Delay_ms(1000);
														Green_ON();//系统指示	
														ENdriver;//使能驱动
													  HRTIM1->sCommonRegs.OENR |= HRTIM_OUTPUT_TA1;//使能输出
													  HRTIM1->sCommonRegs.OENR |= HRTIM_OUTPUT_TA2;//使能输出
                            flag_start_cnt=STOP;														
													  Next_State=Task_1_Vin_detc;													
													}														
													break;													
				default:          break;
			}
	}
}
  
//ADC1_RESULT[0]保存输入电流
//ADC1_RESULT[1]保存输入电压
//ADC1_RESULT[2]保存电感电流平均值
//ADC2_RESULT[1]保存输出电压
//ADC2_RESULT[0]保存输出电流
void HRTIM1_TIMA_IRQHandler(void)
{
if((HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].TIMxDIER & HRTIM_TIM_IT_REP) == HRTIM_TIM_IT_REP)
{
 HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].TIMxICR = HRTIM_TIM_IT_REP;
 __HAL_DMA_CLEAR_FLAG(&ADC1_DMA_Handler, DMA_FLAG_TC1);
 __HAL_DMA_CLEAR_FLAG(&ADC2_DMA_Handler, DMA_FLAG_TC2);		
 Data_update_flag=Run;
 Vin       =(float)((ADC1_RESULT[1]*3300)>>12)*0.011f;//(电压单位:V)//电阻分压：VA_detc=R22/(R20+R22)=430Ω/(4.3K+430Ω)
 Iin       =(float)((ADC1_RESULT[0]*3300)>>12)*0.00757575f-12.49998f;//(电流单位A)//芯片手册CC6904SO-10A，131mV/A,1mv对应，1mV/132=0.00757575A
 IL_average=(float)((ADC1_RESULT[2]*3300)>>12)*0.00757575f-12.49998f;//(电流单位A)//芯片手册CC6904SO-10A，131mV/A,1mv对应，1mV/132=0.00757575A
 Iout      =(float)((ADC2_RESULT[0]*3300)>>12)*0.00757575f-12.49998f;//(电流单位A)//芯片手册CC6904SO-10A，131mV/A,1mv对应，1mV/132=0.00757575A
 Vout      =(float)((ADC2_RESULT[1]*3300)>>12)*0.011f;//(电压单位:V)//电阻分压：VA_detc=R21/(R19+R21)=430Ω/(4.3K+430Ω)
	if(flag_start_cnt==STOP)
	 {
		PID_loop(Vout,Iout);
		if(flag_Soft_start!=STOP)
		 {		
			 if(Vref>Target_voltage&&Iref>Target_Current){flag_Soft_start=STOP;} 
			 if(Vref<Target_voltage)Vref+=0.001f;	
			 if(Iref<Target_Current)Iref+=0.001f; 
		 }
	 }
	}
}






