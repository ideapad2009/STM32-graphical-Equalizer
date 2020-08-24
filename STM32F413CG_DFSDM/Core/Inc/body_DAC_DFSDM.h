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
#include "arm_math.h"
#include "biquad_band1.h"
#include "biquad_band2.h"
#include "biquad_band3.h"
#include "biquad_band4.h"
#include "biquad_band5.h"

#define W8731_ADDR_0 			     0x1A // this is the address of the CODEC when CSB is low
#define CODEC_ADDRESS               (W8731_ADDR_0<<1)
#define SaturaLH(N, L, H)           (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define AUDIO_REC      	            200
#define NUMSTAGES                     3       // Number of 2nd order Biquad stages per filter
#define MAX_GAIN                      100


int32_t RightRecBuff[AUDIO_REC*4]={0};
uint16_t txBuf[AUDIO_REC*8]={0};
float buf_in [AUDIO_REC];
float buf_out [AUDIO_REC];
uint8_t Uart_array[2];

int32_t  Sample32 =0;
uint32_t uSample32 =0;
int16_t  sample16=0;
uint16_t uSample16 =0;
int32_t i =0;


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
	//MX_I2S3_Init();
	MX_DFSDM1_Init();
	MX_DAC_Init();
	MX_TIM6_Init();
	MX_UART5_Init();
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

	//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)txBuf, AUDIO_REC, DAC_ALIGN_12B_R));

	HAL_UART_Receive_DMA(&huart5, Uart_array, 2);
	while (1)
	{

		if((DmaRightRecHalfBuffCplt == 1))
		{

			for(i = 0; i < AUDIO_REC/2; i++)
			{
				buf_in[i]= (float)((int32_t)RightRecBuff[i] >> 8);
			}
			arm_biquad_cascade_df2T_f32(&S1, (float32_t *)&buf_in[0], &buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S2, &buf_out[0],&buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S3, &buf_out[0],&buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S4, &buf_out[0],&buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S5, &buf_out[0],&buf_out[0],AUDIO_REC/2);
			for(i = 0; i < AUDIO_REC/2; i++)
			{
				sample16 = ((int)buf_out[i]);
				//sample16 =  RightRecBuff[i] >> 8;
				uSample16 = ((int)sample16) + 4096;
				txBuf[i] = (uSample16>>1) ;
			}
			//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t* )&txBuf[0], AUDIO_REC/2, DAC_ALIGN_12B_R);
			if(PlaybackStarted == 0)
			{
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, txBuf, AUDIO_REC, DAC_ALIGN_12B_R);
				PlaybackStarted = 1;
			}
			DmaRightRecHalfBuffCplt = 0;
		}
		if( (DmaRightRecBuffCplt == 1))
		{
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				buf_in[i]= (float)((int32_t)RightRecBuff[i] >> 8);
			}
			arm_biquad_cascade_df2T_f32(&S1, (float32_t *)&buf_in[AUDIO_REC/2], &buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S2, &buf_out[AUDIO_REC/2],&buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S3, &buf_out[AUDIO_REC/2],&buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S4, &buf_out[AUDIO_REC/2],&buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_df2T_f32(&S5, &buf_out[AUDIO_REC/2],&buf_out[AUDIO_REC/2],AUDIO_REC/2);
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				sample16 = ((int)buf_out[i]);
				//sample16 =  RightRecBuff[i] >> 8;
				uSample16 = ((int)sample16) + 4096;
				txBuf[i] = (uSample16>>1) ;
			}
			//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t* )&txBuf[AUDIO_REC/2], AUDIO_REC/2, DAC_ALIGN_12B_R);

			DmaRightRecBuffCplt = 0;
		}
	}
}


/*void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
}
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
}*/
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	DmaRightRecHalfBuffCplt = 1;
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	DmaRightRecBuffCplt = 1;
}
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
void TestBlinking(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_Delay(1000);
}

#endif /* INC_BODY_DAC_DFSDM_H_ */
