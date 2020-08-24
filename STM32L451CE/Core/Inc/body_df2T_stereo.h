/*
 * body_df2T_stereo.h
 *
 *  Created on: May 16, 2020
 *      Author: Hanna Nabil
 */

#ifndef INC_BODY_DF2T_STEREO_H_
#define INC_BODY_DF2T_STEREO_H_

/*   DFSDM--> I2S  CMSIS  df2T Direct Form II transposed structure  mono    */
#ifndef INC_BODY_DF2T_MONO_H_
#define INC_BODY_DF2T_MONO_H_

#include "codec_WM8731.h"
#include "arm_math.h"
#include "biquad_band1.h"
#include "biquad_band2.h"
#include "biquad_band3.h"
#include "biquad_band4.h"
#include "biquad_band5.h"
#define W8731_ADDR_0 			     0x1A // this is the address of the CODEC when CSB is low
#define CODEC_ADDRESS               (W8731_ADDR_0<<1)
#define SaturaLH(N, L, H)           (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define AUDIO_REC      	             200
#define NUMSTAGES                     3       // Number of 2nd order Biquad stages per filter
#define MAX_GAIN                      100

uint32_t i,j;
uint32_t x= 0 ;
int32_t RightRecBuff[AUDIO_REC]={0};
//int32_t LeftRecBuff[AUDIO_REC]={0};
int32_t PlayBuff[AUDIO_REC*4]={0}; // playBuff =2* AUDIO_REC coming from DFSDM (as we are duplicating the input signal to stereo)
uint16_t txBuf[AUDIO_REC*4]={0};
float r_buf_in [AUDIO_REC];
float r_buf_out [AUDIO_REC];
uint8_t Uart_array[2];

uint32_t uSample32 =0;
uint16_t uSample16 =0;
int16_t  sample16=0;
int32_t  Sample32 =0;
uint8_t DmaLeftRecHalfBuffCplt  = 0;
uint8_t DmaLeftRecBuffCplt      = 0;
uint8_t DmaRightRecHalfBuffCplt = 0;
uint8_t DmaRightRecBuffCplt     = 0;
uint8_t PlaybackStarted         = 0;
arm_biquad_cascade_stereo_df2T_instance_f32  S1, S2, S3,S4,S5;
float biquadStateBand1[4 * NUMSTAGES],biquadStateBand2[4 * NUMSTAGES],biquadStateBand3[4 * NUMSTAGES],biquadStateBand4[4 * NUMSTAGES],biquadStateBand5[4 * NUMSTAGES];
//lowPass filter f_cut =1000
float coeffTable[5*NUMSTAGES] =
{0.000015551721780892f, 0.000031103443561783f, 0.000015551721780892f, 1.769504348512840108f, -0.784773331782564032f,
		1.f, 2.f, 1.f, 1.888555953889039962f, -0.904852228768565969f};

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
	MX_UART5_Init();
	HAL_Delay(1000);
	if (HAL_I2C_IsDeviceReady(&hi2c1, CODEC_ADDRESS, 1, 10) == HAL_OK){
		TestBlinking();
	}
	Codec_Reset(&hi2c1);
	//10*(gainDB[0]+10)  ==> first 10 is for (5*NUMSTAGES)  , second 10 is for -10 to 10 gainDB
	arm_biquad_cascade_stereo_df2T_init_f32(&S1, NUMSTAGES,&band1_coeff[ (5*NUMSTAGES)*(gainDB[0]+MAX_GAIN)],&biquadStateBand1[0]);
	arm_biquad_cascade_stereo_df2T_init_f32(&S2, NUMSTAGES,&band2_coeff[ (5*NUMSTAGES)*(gainDB[1]+MAX_GAIN)],&biquadStateBand2[0]);
	arm_biquad_cascade_stereo_df2T_init_f32(&S3, NUMSTAGES,&band3_coeff[ (5*NUMSTAGES)*(gainDB[2]+MAX_GAIN)],&biquadStateBand3[0]);
	arm_biquad_cascade_stereo_df2T_init_f32(&S4, NUMSTAGES,&band4_coeff[ (5*NUMSTAGES)*(gainDB[3]+MAX_GAIN)],&biquadStateBand4[0]);
	arm_biquad_cascade_stereo_df2T_init_f32(&S5, NUMSTAGES,&band5_coeff[ (5*NUMSTAGES)*(gainDB[4]+MAX_GAIN)],&biquadStateBand5[0]);

	if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, RightRecBuff, AUDIO_REC))
	{
		Error_Handler();
	}
	//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2);
	HAL_UART_Receive_DMA(&huart5, Uart_array, 2);
	while (1)
	{
		if((DmaRightRecHalfBuffCplt == 1))
		{
			for(i = 0; i < AUDIO_REC/2; i++)
			{
				r_buf_in[i]= (float) RightRecBuff[i];
			}
			arm_biquad_cascade_stereo_df2T_f32(&S1, (float32_t *)&r_buf_in[0], &r_buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S2, &r_buf_out[0],&r_buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S3, &r_buf_out[0],&r_buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S4, &r_buf_out[0],&r_buf_out[0],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S5, &r_buf_out[0],&r_buf_out[0],AUDIO_REC/2);
			for(i = 0; i < AUDIO_REC/2; i++)
			{
				txBuf[i*4]  =  ((int)r_buf_out[i])>>16;
				txBuf[(i*4)+1]= ((int)r_buf_out[i])&0xFFFF;
				txBuf[(i*4)+2] = txBuf[i*4];
				txBuf[(i*4)+3] = txBuf[(i*4)+1];
			}

			if(PlaybackStarted == 0)
			{
				if(HAL_OK != HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2))
				{
					Error_Handler();
				}
				PlaybackStarted = 1;
			}
			DmaLeftRecHalfBuffCplt  = 0;
			DmaRightRecHalfBuffCplt = 0;
		}
		if( (DmaRightRecBuffCplt == 1))
		{
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				r_buf_in[i]= (float) RightRecBuff[i];
			}
			arm_biquad_cascade_stereo_df2T_f32(&S1,(float32_t *)&r_buf_in[AUDIO_REC/2] , &r_buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S2, &r_buf_out[AUDIO_REC/2],&r_buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S3, &r_buf_out[AUDIO_REC/2],&r_buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S4, &r_buf_out[AUDIO_REC/2],&r_buf_out[AUDIO_REC/2],AUDIO_REC/2);
			arm_biquad_cascade_stereo_df2T_f32(&S5, &r_buf_out[AUDIO_REC/2],&r_buf_out[AUDIO_REC/2],AUDIO_REC/2);
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				txBuf[i*4]  =  ((int)r_buf_out[i])>>16;
				txBuf[(i*4)+1]= ((int)r_buf_out[i])&0xFFFF;
				txBuf[(i*4)+2] = txBuf[i*4];
				txBuf[(i*4)+3] = txBuf[(i*4)+1];
			}
			/*for(i = 0; i < AUDIO_REC*2; i++)
			{
				uSample32=PlayBuff[i];
				PlayBuff[i]=  uSample32 <<16 | uSample32 >>16;
			}*/
			//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)&txBuf[0], AUDIO_REC*2);
			DmaLeftRecBuffCplt  = 0;
			DmaRightRecBuffCplt = 0;
		}

	}
}


/*void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	playSong();
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

#endif /* INC_BODY_DF2T_STEREO_H_ */
