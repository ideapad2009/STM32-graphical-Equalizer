/*
 * body_DAC_DFSDM.h
 *
 *  Created on: Apr 11, 2020
 *      Author: Hanna Nabil
 */

#ifndef INC_BODY_DAC_DFSDM_H_
#define INC_BODY_DAC_DFSDM_H_

/*hdfsdm1_filter0.Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
  hdfsdm1_filter0.Init.InjectedParam.ScanMode       = ENABLE;
  hdfsdm1_filter0.Init.InjectedParam.DmaMode        = DISABLE;
  hdfsdm1_filter0.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
  hdfsdm1_filter0.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_FALLING_EDGE;*/
//#include "codec_WM8731.h"
#include <math.h>
#include "arm_math.h"
#include "biquad_band1.h"
#include "biquad_band2.h"
#include "biquad_band3.h"
#include "biquad_band4.h"
#include "biquad_band5.h"
#include "song_16.h"
#include "song_24_48k.h"
#include "song_24_43k.h"
#include <song_u8_43k.h>

#define SaturaLH(N, L, H)           (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define AUDIO_REC      	            48
#define NUMSTAGES                     3       // Number of 2nd order Biquad stages per filter
#define MAX_GAIN                      100

uint32_t i,j;
uint32_t x= 0 ;
int DAC_FLAG =0;
int32_t RightRecBuff[AUDIO_REC*4]={0};
int32_t LeftRecBuff[AUDIO_REC]={0};
int16_t RecBuff[AUDIO_REC*2]={0};

uint16_t PlayBuff[AUDIO_REC*2]={0}; // playBuff =2* AUDIO_REC coming from DFSDM (as we are duplicating the input signal to stereo)
uint16_t txBuf[AUDIO_REC*8]={0};
float buf_in [AUDIO_REC];
float buf_out [AUDIO_REC];
uint8_t Uart_array[2];
int32_t  Sample32 =0;
int32_t  Sample32_ =0;
uint32_t uSample32 =0;
uint16_t uSample16 =0;
int16_t  sample16=0;

uint8_t DmaLeftRecHalfBuffCplt  = 0;
uint8_t DmaLeftRecBuffCplt      = 0;
uint8_t DmaRightRecHalfBuffCplt = 0;
uint8_t DmaRightRecBuffCplt     = 0;
uint8_t PlaybackStarted         = 0;
arm_biquad_cascade_df2T_instance_f32 S1, S2, S3,S4,S5;
float biquadStateBand1[4 * NUMSTAGES],biquadStateBand2[4 * NUMSTAGES],biquadStateBand3[4 * NUMSTAGES],biquadStateBand4[4 * NUMSTAGES],biquadStateBand5[4 * NUMSTAGES];
int gainDB[5] = {10,10,5,6,7};
void TestBlinking(void);


int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_I2C1_Init();
	MX_I2S3_Init();
	MX_DFSDM1_Init();
	MX_DAC_Init();
	MX_TIM6_Init();


	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);
	TestBlinking();

	//10*(gainDB[0]+10)  ==> first 10 is for (5*NUMSTAGES)  , second 10 is for -10 to 10 gainDB
	arm_biquad_cascade_df2T_init_f32(&S1, NUMSTAGES,&band1_coeff[ (5*NUMSTAGES)*(gainDB[0]+MAX_GAIN)],&biquadStateBand1[0]);
	arm_biquad_cascade_df2T_init_f32(&S2, NUMSTAGES,&band2_coeff[ (5*NUMSTAGES)*(gainDB[1]+MAX_GAIN)],&biquadStateBand2[0]);
	arm_biquad_cascade_df2T_init_f32(&S3, NUMSTAGES,&band3_coeff[ (5*NUMSTAGES)*(gainDB[2]+MAX_GAIN)],&biquadStateBand3[0]);
	arm_biquad_cascade_df2T_init_f32(&S4, NUMSTAGES,&band4_coeff[ (5*NUMSTAGES)*(gainDB[3]+MAX_GAIN)],&biquadStateBand4[0]);
	arm_biquad_cascade_df2T_init_f32(&S5, NUMSTAGES,&band5_coeff[ (5*NUMSTAGES)*(gainDB[4]+MAX_GAIN)],&biquadStateBand5[0]);

	if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, RightRecBuff, AUDIO_REC))
	{
		Error_Handler();
	}
	/*if(HAL_OK != HAL_DFSDM_FilterRegularMsbStart_DMA(&hdfsdm1_filter0, RecBuff, AUDIO_REC))
		{
			Error_Handler();
		}*/
	HAL_UART_Receive_DMA(&huart5, Uart_array, 2);
	while (1)
	{

		if((DmaRightRecHalfBuffCplt == 1))
		{

			for(i = 0; i < AUDIO_REC/2; i++)
			{
				sample16 =  SaturaLH((RightRecBuff[i] >> 8), -32768, 32767);
				uSample16 = (int16_t)sample16 + 32767;
				//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, uSample16>>4);
				txBuf[i] = (uSample16>>4) ;
			}
			HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, txBuf, AUDIO_REC, DAC_ALIGN_12B_R);
			/*if(PlaybackStarted == 0)
			{
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, txBuf, AUDIO_REC, DAC_ALIGN_12B_R);
				PlaybackStarted = 1;
			}*/
			DmaLeftRecHalfBuffCplt  = 0;
			DmaRightRecHalfBuffCplt = 0;
		}
		if( (DmaRightRecBuffCplt == 1))
		{
			for(i = 0; i < AUDIO_REC/2; i++)
			{
				sample16 =  SaturaLH((RightRecBuff[i] >> 8), -32768, 32767);
				uSample16 = (int16_t)sample16 + 32767;
				//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, uSample16>>4);
				txBuf[i] = (uSample16>>4) ;
			}
			//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, &txBuf[AUDIO_REC/2], AUDIO_REC/2, DAC_ALIGN_12B_R);

			DmaLeftRecBuffCplt  = 0;
			DmaRightRecBuffCplt = 0;
		}
	}
}


/*void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{

}*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	switch((int8_t)Uart_array[0])
	{
	case 1:
		gainDB[0] =(int8_t)(Uart_array[1]);
		//S1.pCoeffs=&band1_coeff[(5*NUMSTAGES)*(gainDB[0]+10)];
		arm_biquad_cascade_df2T_init_f32(&S1, NUMSTAGES,&band1_coeff[ (5*NUMSTAGES)*(gainDB[0]+MAX_GAIN)],&biquadStateBand1[0]);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		break;
	case 2:
		gainDB[1] =(int8_t)(Uart_array[1]);
		//S2.pCoeffs=&band2_coeff[(5*NUMSTAGES)*(gainDB[1]+10)];
		arm_biquad_cascade_df2T_init_f32(&S2, NUMSTAGES,&band2_coeff[ (5*NUMSTAGES)*(gainDB[1]+MAX_GAIN)],&biquadStateBand2[0]);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		break;
	case 3:
		gainDB[2] =(int8_t)(Uart_array[1]);
		arm_biquad_cascade_df2T_init_f32(&S3, NUMSTAGES,&band3_coeff[ (5*NUMSTAGES)*(gainDB[2]+MAX_GAIN)],&biquadStateBand3[0]);
		break;
	case 4:
		gainDB[3] =(int8_t)(Uart_array[1]);
		arm_biquad_cascade_df2T_init_f32(&S4, NUMSTAGES,&band4_coeff[ (5*NUMSTAGES)*(gainDB[3]+MAX_GAIN)],&biquadStateBand4[0]);
		break;
	case 5:
		gainDB[4] =(int8_t)(Uart_array[1]);
		arm_biquad_cascade_df2T_init_f32(&S5, NUMSTAGES,&band5_coeff[ (5*NUMSTAGES)*(gainDB[4]+MAX_GAIN)],&biquadStateBand5[0]);
		break;
	}
}
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	DmaRightRecHalfBuffCplt = 1;
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	DmaRightRecBuffCplt = 1;
}
void TestBlinking(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_Delay(1000);
}

#endif /* INC_BODY_DAC_DFSDM_H_ */
