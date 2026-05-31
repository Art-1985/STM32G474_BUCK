/****************************************************************************************
  * @file    HRTIM.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "HRTIM.h"

HRTIM_HandleTypeDef  HRTIM1_structure;

void HRTIM_INT(void)
{
	GPIO_InitTypeDef           GPIO_InitStruct;                           //GPIO参数描述结构体
	HRTIM_TimeBaseCfgTypeDef   pTimeBaseCfg  = {0};                       //HRTIM定时器基本参数描述结构体
  HRTIM_TimerCfgTypeDef      pTimerCfg     = {0};                       //HRTIM波形参数配置结构体
  HRTIM_TimerCtlTypeDef      pTimerCtl     = {0};                       //HRTIM模式配置
  HRTIM_OutputCfgTypeDef     pOutputCfg    = {0};                       //PWM输出有关参数描述结构体
	HRTIM_CompareCfgTypeDef    pCompareCfg   = {0};                       //PWM脉宽参数描述结构体  
	HRTIM_DeadTimeCfgTypeDef   pDeadTimeCfg  = {0};                       //死区时间描述结构体
	HRTIM_ADCTriggerCfgTypeDef pADCTriggerCfg= {0};                       //ADC触发选项配置
	
	__HAL_RCC_HRTIM1_CLK_ENABLE();                                        //开启HRTIM1时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();                                         //开启GPIOA时钟
	
  HRTIM1_structure.Instance = HRTIM1;                                   //调用HRTIM1
  HRTIM1_structure.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;        //中断需求描述
  HRTIM1_structure.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;            //HRTIM是否和外部定时器同步
  HAL_HRTIM_Init(&HRTIM1_structure);                                    //初始化HRTIM1
   
	//183.8235294117647  pS
  pTimeBaseCfg.Period = PWM_PERIOD;  //27200                            //PWM周期,  200KHz=5uS=5000nS/0.183823529=27200
  pTimeBaseCfg.RepetitionCounter = 0x09;                                //重复多少个周期，发生一次中断
  pTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;             //32倍频   ，170MHz*32=5.44Ghz
  pTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;                            //连续计数，周而复始
  HAL_HRTIM_TimeBaseConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_MASTER, &pTimeBaseCfg); //初始化
	HAL_HRTIM_TimeBaseConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, &pTimeBaseCfg);//初始化
  
  pTimerCfg.InterruptRequests = HRTIM_MASTER_IT_NONE;                   //中断需求描述
  pTimerCfg.DMARequests = HRTIM_MASTER_DMA_NONE;                        //DMA需求 
  pTimerCfg.DMASrcAddress = 0x0000;                                     //数据来源
  pTimerCfg.DMADstAddress = 0x0000;                                     //数据目的地
  pTimerCfg.DMASize = 0x1;                                              //搬运多少个数
  pTimerCfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;                   //HALF模式关闭
  pTimerCfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;          //内部互联关闭
  pTimerCfg.StartOnSync = HRTIM_SYNCSTART_ENABLED;                      //同步开始
  pTimerCfg.ResetOnSync = HRTIM_SYNCRESET_ENABLED;                      //同步复位
  pTimerCfg.DACSynchro = HRTIM_DACSYNC_NONE;                            //同步DAC
  pTimerCfg.PreloadEnable = HRTIM_PRELOAD_ENABLED;                      //预加载功能关闭
  pTimerCfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;              //更新事件，独立发生
  pTimerCfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;             //促发模式关闭
  pTimerCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_DISABLED;       //重复计数更新事件功能关闭
  pTimerCfg.ReSyncUpdate = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;       //更新事件不同步
	pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;              //选择事件用于定时器寄存器更新
 // pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;          //定时器复位信号来源
  pTimerCfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_ENABLED;               //定时器复位时，更新事件
  HAL_HRTIM_WaveformTimerConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_MASTER, &pTimerCfg);//初始化
  
  pTimerCfg.InterruptRequests = HRTIM_TIM_IT_NONE;                      //定时器中断需求描述
  pTimerCfg.DMARequests = HRTIM_TIM_DMA_NONE;                           //DMA需求描述
  pTimerCfg.DMASrcAddress = 0x0000;                                     //数据来源
  pTimerCfg.DMADstAddress = 0x0000;                                     //数据目的地
  pTimerCfg.DMASize = 0x1;                                              //搬运个数
	pTimerCfg.DACSynchro=HRTIM_DACSYNC_DACTRIGOUT_1;                      //DAC同步事件来源一共有3个，参考手册902页
  pTimerCfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;                  //推挽模式关闭
  pTimerCfg.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;                    //错误模式关闭
  pTimerCfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;                   //错误功能锁定
  pTimerCfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;     //死区事件插入使能
  pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;//定时器延时保护功能关闭
  pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;              //选择事件用于定时器寄存器更新
  pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;            //定时器复位信号来源
  HAL_HRTIM_WaveformTimerConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, &pTimerCfg);//初始化
	
	pTimerCtl.UpDownMode = HRTIM_TIMERUPDOWNMODE_UP;		                  //向上计数
  pTimerCtl.DualChannelDacEnable = HRTIM_TIMER_DCDE_DISABLED;           //双重模式关闭
  HAL_HRTIM_WaveformTimerControl(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, &pTimerCtl);//初始化

  pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;                      //定时器输出极行高
  pOutputCfg.SetSource = HRTIM_OUTPUTSET_TIMPER;                        //定时器计数回到零时，PWM输出拉高
  pOutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1; 									//定时器计数到Compare1时，PWM输出低
  pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;                      //空闲模式不动作
  pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;                //空闲模式输出无效
  pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;                  //错误模式，输出电平状态
  pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;      //斩波模式关闭
  pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;//初始化
	
  HAL_HRTIM_WaveformOutputConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &pOutputCfg);//初始化
  HAL_HRTIM_WaveformOutputConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, &pOutputCfg);//初始化
	
	pCompareCfg.AutoDelayedMode=HRTIM_AUTODELAYEDMODE_REGULAR;             //普通比较模式
	pCompareCfg.CompareValue=5000;                                         //比较值5000
	HAL_HRTIM_WaveformCompareConfig(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_COMPAREUNIT_1,&pCompareCfg);//初始化
	
	//死区时间配置：参考手册810页
	pDeadTimeCfg.FallingLock=HRTIM_TIMDEADTIME_FALLINGLOCK_READONLY;        //死区时间下降沿只读模式
	pDeadTimeCfg.FallingSign=HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;        //死区时间为正值
	pDeadTimeCfg.FallingSignLock=HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_READONLY;//死区下降沿锁定
	pDeadTimeCfg.FallingValue=10;                                            //下降沿死区时间
	pDeadTimeCfg.Prescaler=HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV1;           //fHRTIM=144MHz
	pDeadTimeCfg.RisingLock=HRTIM_TIMDEADTIME_RISINGLOCK_READONLY;          //死区时间上升沿只读模式
	pDeadTimeCfg.RisingSign=HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;         //下降沿为正值
	pDeadTimeCfg.RisingSignLock=HRTIM_TIMDEADTIME_RISINGSIGNLOCK_READONLY;  //锁定
	pDeadTimeCfg.RisingValue=10;                                             //时间
	HAL_HRTIM_DeadTimeConfig(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,&pDeadTimeCfg);//初始化
/************************************************************************************************************************************/	
	pADCTriggerCfg.Trigger=HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP3;             //HRTIM中的定时器TIMA的Compare3作为触发源
	pADCTriggerCfg.UpdateSource=HRTIM_ADCTRIGGERUPDATE_TIMER_A;             //TIMA复位时可以更新触发源
	HAL_HRTIM_ADCTriggerConfig(&HRTIM1_structure,HRTIM_ADCTRIGGER_1,&pADCTriggerCfg);//ADC触发初始化
	
	HAL_HRTIM_SimpleBaseStart(&HRTIM1_structure,HRTIM_TIMERINDEX_MASTER);   //初始化
	HAL_HRTIM_SimpleBaseStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A);  //初始化
	Delay_us(300);
	
	HAL_HRTIM_SimpleOCStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA1);//初始化
	HAL_HRTIM_SimpleOCStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA2);//初始化
	
	HAL_HRTIM_SimplePWMStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA1);
	HAL_HRTIM_SimplePWMStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA2);
	
	__HAL_HRTIM_TIMER_ENABLE_IT(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_TIM_IT_REP);
	HAL_NVIC_SetPriority(HRTIM1_TIMA_IRQn,0,0); //抢占优先级为0，子优先级0
  HAL_NVIC_EnableIRQ(HRTIM1_TIMA_IRQn);        //使能中断线
	
	__HAL_HRTIM_ENABLE(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A);       //使能
	__HAL_HRTIM_ENABLE(&HRTIM1_structure, HRTIM_TIMERINDEX_MASTER);        //使能
	
	HAL_HRTIM_WaveformSetOutputLevel(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, HRTIM_OUTPUTLEVEL_ACTIVE);//输出电平状态
	HAL_HRTIM_WaveformSetOutputLevel(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, HRTIM_OUTPUTLEVEL_ACTIVE);//输出电平状态
	
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = 500;//输出PWM
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP2xR = 100; //触发DAC_STEP
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = 1000;//触发ADC
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP4xR = 100; //Blanking滤波
	DISdriver;
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;                          //管脚描述
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                               //复用推挽模式
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;                                 //下拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;                    //IO口速度设置为最高
	GPIO_InitStruct.Alternate = GPIO_AF13_HRTIM1;                         //复用为HRTIM1，重映射，芯片手册有描述
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                               //IO初始化
}

