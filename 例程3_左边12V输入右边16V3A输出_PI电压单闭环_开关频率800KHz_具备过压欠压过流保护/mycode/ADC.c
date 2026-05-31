/****************************************************************************************
  * @file    ADC.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    12-Jan-2023
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "ADC.h"

/*
ADC检测IO配置
              
 PA0   ADC1_IN1      模拟    Iin_detc
 PA1   ADC1_IN2      模拟    Vin_detc
 PA2   ADC1_IN3      模拟    IL_detc
 PA5   ADC2_IN13     模拟    Iout_detc
 PA6   ADC2_IN3      模拟    Vb_detc
 
*/

ADC_HandleTypeDef  ADC1_Handler;      //ADC1句柄
DMA_HandleTypeDef  ADC1_DMA_Handler;  //ADC1的DMA句柄
ADC_HandleTypeDef  ADC2_Handler;      //ADC1句柄
DMA_HandleTypeDef  ADC2_DMA_Handler;  //ADC1的DMA句柄

__IO uint16_t ADC1_RESULT[3]={0};
__IO uint16_t ADC2_RESULT[2]={0};


void ADC1_Init(void)
{
	GPIO_InitTypeDef       GPIO_Initure;                                       //定义IO口相关结构体
	ADC_ChannelConfTypeDef ADC1_ChanConf;                                      //定义ADC2相关结构体
	ADC_MultiModeTypeDef   multimode ;                                         //定义ADC模式相关结构体
	
  __HAL_RCC_ADC12_CLK_ENABLE();                                              //使能ADC1时钟
  __HAL_RCC_GPIOA_CLK_ENABLE(); 																						 //开启GPIOA时钟
	__HAL_RCC_DMA1_CLK_ENABLE();                                               //DMA1时钟使能
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	
  GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2; 												 //PA0/PA1/PA2
  GPIO_Initure.Mode=GPIO_MODE_ANALOG; 																		   //模拟
  GPIO_Initure.Pull=GPIO_NOPULL; 																						 //不带上下拉
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);                                        //GPIOA初始化
	
	ADC1_Handler.Instance=ADC1;                                                //选中ADC2
	ADC1_Handler.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV4;                 //4分频
	ADC1_Handler.Init.Resolution=ADC_RESOLUTION_12B; 													 //12位模式
	ADC1_Handler.Init.DataAlign=ADC_DATAALIGN_RIGHT;													 //右对齐
	ADC1_Handler.Init.ScanConvMode=ENABLE; 															       //非扫描模式
	ADC1_Handler.Init.EOCSelection=DISABLE; 																	 //关闭EOC中断
	ADC1_Handler.Init.ContinuousConvMode=ENABLE; 														   //关闭连续转换
	ADC1_Handler.Init.NbrOfConversion=3; 																       //1个转换在规则序列中
	ADC1_Handler.Init.DiscontinuousConvMode=DISABLE; 													 //禁止不连续采样模式
	ADC1_Handler.Init.NbrOfDiscConversion=0; 																	 //不连续采样通道数为0
	ADC1_Handler.Init.ExternalTrigConv=ADC_EXTERNALTRIG_HRTIM_TRG1;            //ADC触发源选择，参考手册895页
	ADC1_Handler.Init.ExternalTrigConvEdge=ADC_EXTERNALTRIGCONVEDGE_RISING;		 //上升沿触发
	ADC1_Handler.Init.DMAContinuousRequests=ENABLE; 													 //使能DMA请求
	ADC1_Handler.Init.SamplingMode=ADC_SAMPLING_MODE_NORMAL;                   //ADC轮询采样模式
	ADC1_Handler.Init.GainCompensation = 0;                                    //ADC增益设置为0
	ADC1_Handler.Init.LowPowerAutoWait = DISABLE;                              //关闭低功耗模式
  ADC1_Handler.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                      //数据溢出覆盖
  ADC1_Handler.Init.OversamplingMode = DISABLE;                              //不使用数据溢出覆盖功能
	HAL_ADC_Init(&ADC1_Handler);																							 //初始化
	
	multimode.Mode = ADC_MODE_INDEPENDENT;                                     //ADC相互独立运行
  HAL_ADCEx_MultiModeConfigChannel(&ADC1_Handler, &multimode);               //多个ADC运行配置
	
	ADC1_ChanConf.Channel=ADC_CHANNEL_1;                                       //通道
	ADC1_ChanConf.Rank=ADC_REGULAR_RANK_1;                                     //第1个序列
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_12CYCLES_5;                       //采样时间
	ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC1_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);                       //通道配置
	
	ADC1_ChanConf.Channel=ADC_CHANNEL_2;                                       //通道
	ADC1_ChanConf.Rank=ADC_REGULAR_RANK_2;                                     //第2个序列
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_12CYCLES_5;                      //采样时间
	ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC1_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);                       //通道配置
	
	ADC1_ChanConf.Channel=ADC_CHANNEL_3;                                       //通道
	ADC1_ChanConf.Rank=ADC_REGULAR_RANK_3;                                     //第3个序列
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_12CYCLES_5;                      //采样时间
	ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC1_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);                       //通道配置
	
	HAL_ADCEx_Calibration_Start(&ADC1_Handler, ADC_SINGLE_ENDED);              //ADC矫正
	
	ADC1_DMA_Handler.Instance = DMA1_Channel1;                                 //使用DMA1的通道3
  ADC1_DMA_Handler.Init.Request = DMA_REQUEST_ADC1;                          //DMA需求来源ADC2
  ADC1_DMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;                    //数据传输方向：内存到外设
  ADC1_DMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;                        //外设地址不增加
  ADC1_DMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                            //内存地址增加
  ADC1_DMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;       //外设数据宽度
  ADC1_DMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;          //外设数据对齐方式
  ADC1_DMA_Handler.Init.Mode = DMA_CIRCULAR;                                 //循环传输
  ADC1_DMA_Handler.Init.Priority = DMA_PRIORITY_VERY_HIGH ;									 //DMA抢占优先级最高
	HAL_DMA_Init(&ADC1_DMA_Handler);								                           //初始化
	__HAL_LINKDMA(&ADC1_Handler,DMA_Handle,ADC1_DMA_Handler);                  //把结构体的参数赋给DMA
	
	HAL_ADC_Start_DMA(&ADC1_Handler,(uint32_t*)ADC1_RESULT,3);                 //初始化ADC采样
	
	HAL_ADC_Start(&ADC1_Handler);                                              //开启AD
}
void ADC2_Init(void)
{
	GPIO_InitTypeDef       GPIO_Initure;                                       //定义IO口相关结构体
	ADC_ChannelConfTypeDef ADC2_ChanConf;                                      //定义ADC2相关结构体
	ADC_MultiModeTypeDef   multimode ;                                         //定义ADC模式相关结构体
	
  __HAL_RCC_ADC12_CLK_ENABLE();                                              //使能ADC1时钟
  __HAL_RCC_GPIOA_CLK_ENABLE(); 																						 //开启GPIOA时钟
	__HAL_RCC_DMA1_CLK_ENABLE();                                               //DMA1时钟使能
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	
  GPIO_Initure.Pin=GPIO_PIN_5|GPIO_PIN_6;                                    //PA5/PA6
  GPIO_Initure.Mode=GPIO_MODE_ANALOG; 																		   //模拟
  GPIO_Initure.Pull=GPIO_NOPULL; 																						 //不带上下拉
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);                                        //GPIOA初始化
	
	ADC2_Handler.Instance=ADC2;                                                //选中ADC2
	ADC2_Handler.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV4;                 //4分频
	ADC2_Handler.Init.Resolution=ADC_RESOLUTION_12B; 													 //12位模式
	ADC2_Handler.Init.DataAlign=ADC_DATAALIGN_RIGHT;													 //右对齐
	ADC2_Handler.Init.ScanConvMode=ENABLE; 															       //非扫描模式
	ADC2_Handler.Init.EOCSelection=DISABLE; 																	 //关闭EOC中断
	ADC2_Handler.Init.ContinuousConvMode=ENABLE; 														   //关闭连续转换
	ADC2_Handler.Init.NbrOfConversion=2; 																       //1个转换在规则序列中
	ADC2_Handler.Init.DiscontinuousConvMode=DISABLE; 													 //禁止不连续采样模式
	ADC2_Handler.Init.NbrOfDiscConversion=0; 																	 //不连续采样通道数为0
	ADC2_Handler.Init.ExternalTrigConv=ADC_EXTERNALTRIG_HRTIM_TRG1;            //ADC触发源选择，参考手册895页
	ADC2_Handler.Init.ExternalTrigConvEdge=ADC_EXTERNALTRIGCONVEDGE_RISING;		 //上升沿触发
	ADC2_Handler.Init.DMAContinuousRequests=ENABLE; 													 //使能DMA请求
	ADC2_Handler.Init.SamplingMode=ADC_SAMPLING_MODE_NORMAL;                   //ADC轮询采样模式
	ADC2_Handler.Init.GainCompensation = 0;                                    //ADC增益设置为0
	ADC2_Handler.Init.LowPowerAutoWait = DISABLE;                              //关闭低功耗模式
  ADC2_Handler.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                      //数据溢出覆盖
  ADC2_Handler.Init.OversamplingMode = DISABLE;                              //不使用数据溢出覆盖功能
	HAL_ADC_Init(&ADC2_Handler);																							 //初始化
	
	multimode.Mode = ADC_MODE_INDEPENDENT;                                     //ADC相互独立运行
  HAL_ADCEx_MultiModeConfigChannel(&ADC2_Handler, &multimode);               //多个ADC运行配置
	
	ADC2_ChanConf.Channel=ADC_CHANNEL_13;                                       //通道
	ADC2_ChanConf.Rank=ADC_REGULAR_RANK_1;                                     //第1个序列
	ADC2_ChanConf.SamplingTime=ADC_SAMPLETIME_12CYCLES_5;                       //采样时间
	ADC2_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC2_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC2_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC2_Handler,&ADC2_ChanConf);                       //通道配置
	
	ADC2_ChanConf.Channel=ADC_CHANNEL_3;                                       //通道
	ADC2_ChanConf.Rank=ADC_REGULAR_RANK_2;                                     //第2个序列
	ADC2_ChanConf.SamplingTime=ADC_SAMPLETIME_12CYCLES_5;                      //采样时间
	ADC2_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC2_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC2_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC2_Handler,&ADC2_ChanConf);                       //通道配置
	
	HAL_ADCEx_Calibration_Start(&ADC2_Handler, ADC_SINGLE_ENDED);              //ADC矫正
	
	ADC2_DMA_Handler.Instance = DMA1_Channel2;                                 //使用DMA1的通道3
  ADC2_DMA_Handler.Init.Request = DMA_REQUEST_ADC2;                          //DMA需求来源ADC2
  ADC2_DMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;                    //数据传输方向：内存到外设
  ADC2_DMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;                        //外设地址不增加
  ADC2_DMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                            //内存地址增加
  ADC2_DMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;       //外设数据宽度
  ADC2_DMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;          //外设数据对齐方式
  ADC2_DMA_Handler.Init.Mode = DMA_CIRCULAR;                                 //循环传输
  ADC2_DMA_Handler.Init.Priority = DMA_PRIORITY_VERY_HIGH ;									 //DMA抢占优先级最高
	HAL_DMA_Init(&ADC2_DMA_Handler);								                           //初始化
	__HAL_LINKDMA(&ADC2_Handler,DMA_Handle,ADC2_DMA_Handler);                  //把结构体的参数赋给DMA
	
	HAL_ADC_Start_DMA(&ADC2_Handler,(uint32_t*)ADC2_RESULT,2);                 //初始化ADC采样
	
	HAL_ADC_Start(&ADC2_Handler);                                              //开启AD
}
