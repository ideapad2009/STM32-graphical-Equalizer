/*
 * body_DFSDM_I2S.h
 *
 *  Created on: Apr 18, 2020
 *      Author: Hanna Nabil
 */

#ifndef INC_BODY_DFSDM_I2S_GREQ_H_
#define INC_BODY_DFSDM_I2S_GREQ_H_

/*hdfsdm1_filter0.Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
  hdfsdm1_filter0.Init.InjectedParam.ScanMode       = ENABLE;
  hdfsdm1_filter0.Init.InjectedParam.DmaMode        = DISABLE;
  hdfsdm1_filter0.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
  hdfsdm1_filter0.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_FALLING_EDGE;*/
#include "codec_WM8731.h"
#include <math.h>
#include "song_16.h"
#include "song_24_48k.h"
#include "song_24_43k.h"
#include <song_u8_43k.h>
#include "iirFilter.h"
#include "Equalizer.h"
#define W8731_ADDR_0 			     0x1A // this is the address of the CODEC when CSB is low
#define CODEC_ADDRESS               (W8731_ADDR_0<<1)
#define SaturaLH(N, L, H)           (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define GREQ_ERROR_NONE                            0
#define AUDIO_REC      	                   10


uint32_t i,j;
uint32_t x= 0 ;
int32_t RightRecBuff[AUDIO_REC]={0};
//int32_t LeftRecBuff[AUDIO_REC]={0};
int32_t PlayBuff[AUDIO_REC*4]={0}; // playBuff =2* AUDIO_REC coming from DFSDM (as we are duplicating the input signal to stereo)
uint16_t txBuf[AUDIO_REC*4]={0};
int32_t  Sample32 =0;
uint32_t uSample32 =0;
uint16_t uSample16 =0;
int16_t  sample16=0;

uint8_t DmaLeftRecHalfBuffCplt  = 0;
uint8_t DmaLeftRecBuffCplt      = 0;
uint8_t DmaRightRecHalfBuffCplt = 0;
uint8_t DmaRightRecBuffCplt     = 0;
uint8_t PlaybackStarted         = 0;
int8_t error = GREQ_ERROR_NONE;

void TestBlinking(void);
void playSong(void){
	if (x < (SONG_SIZE_24 -4 )){
		for(i = 0,j=x; i < AUDIO_REC; i++,j++)
		{
			PlayBuff[i*2]=    song_24[i];
			PlayBuff[(i*2)+1]=PlayBuff[i*2];
			uSample32        = (uint32_t) song_24[j] ;
			txBuf[i*4]     = (uSample32>>16) & 0xFFFF;
			txBuf[(i*4)+1] = (uSample32 )& 0xFFFF;
			txBuf[(i*4)+2] = txBuf[i*4];
			txBuf[(i*4)+3] = txBuf[(i*4)+1];
		}
		x = x+AUDIO_REC;
	}
	else {x=0;playSong();
	}
	}
int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_I2C1_Init();
	MX_I2S3_Init();
	MX_DFSDM1_Init();
	/*MX_DAC_Init();
	MX_TIM6_Init();
	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);*/

	//__HAL_UNLOCK(&hi2s3);     // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!
	//__HAL_I2S_ENABLE(&hi2s3);
	HAL_Delay(1000);
	if (HAL_I2C_IsDeviceReady(&hi2c1, CODEC_ADDRESS, 1, 10) == HAL_OK){
		TestBlinking();
	}
	Codec_Reset(&hi2c1);

	error =Equalizer_Init(AUDIO_REC*2);
	if (error != GREQ_ERROR_NONE)
	{
		Error_Handler();
	}
	if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, RightRecBuff, AUDIO_REC))
	{
		Error_Handler();
	}
	//playSong();

	//playSong();
	//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2);
	while (1)
	{

		if((DmaRightRecHalfBuffCplt == 1))
		{
			for(i = 0; i < AUDIO_REC/2; i++)
			{
				uSample32=RightRecBuff[i]*3;
				PlayBuff[i*2]=RightRecBuff[i]*2;
				PlayBuff[(i*2)+1]=PlayBuff[i*2];
//				PlayBuff[i*2]=uSample32 <<16 | uSample32 >>16;
//				PlayBuff[(i*2)+1]= PlayBuff[i*2];
//				txBuf[i*4] = RightRecBuff[i]>>16 ;
//				txBuf[(i*4)+1] = (uint16_t)(RightRecBuff[i]<<1);//&0xFF00 //to emit the least 8 bit
//				txBuf[(i*4)+2] = txBuf[i*4];
//				txBuf[(i*4)+3] = txBuf[(i*4)+1];
			}

			//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC);

			/*if(PlaybackStarted == 0)
			{
				if(HAL_OK != HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2))
				{
					Error_Handler();
				}
				PlaybackStarted = 1;
			}*/
			DmaLeftRecHalfBuffCplt  = 0;
			DmaRightRecHalfBuffCplt = 0;
		}
		if( (DmaRightRecBuffCplt == 1))
		{
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				//uSample32=RightRecBuff[i]*3;
				PlayBuff[i*2]=RightRecBuff[i]*2;
				PlayBuff[(i*2)+1]=PlayBuff[i*2];
//				PlayBuff[i*2]=uSample32 <<16 | uSample32 >>16;
//				PlayBuff[(i*2)+1]= PlayBuff[i*2];
//				txBuf[i*4] = RightRecBuff[i]>>16 ;
//				txBuf[(i*4)+1] = (uint16_t)(RightRecBuff[i]) ;//&0xFF00 //to emit the least 8 bit
//		        txBuf[(i*4)+2] = txBuf[i*4];
//				txBuf[(i*4)+3] = txBuf[(i*4)+1];
			}
			/*error=equalizerProcess((int32_t*)&PlayBuff[0],(int32_t*)&PlayBuff[0]);
			if (error != GREQ_ERROR_NONE)
			{
				Error_Handler();
			}*/
			/*for(i = 0; i < AUDIO_REC*2; i++)
			{
				uSample32=PlayBuff[i];
				PlayBuff[i]=  uSample32 <<16 | uSample32 >>16;
			}*/
			HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)&PlayBuff[0], AUDIO_REC*2);
			DmaLeftRecBuffCplt  = 0;
			DmaRightRecBuffCplt = 0;
		}

	}
}


/*void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	playSong();
}*/
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
		DmaRightRecHalfBuffCplt = 1;

	//DmaLeftRecHalfBuffCplt=1;
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{

	DmaRightRecBuffCplt = 1;
	//DmaLeftRecHalfBuffCplt=1;
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	//DAC_FLAG=1;
}
void TestBlinking(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_Delay(1000);
}


#endif /* INC_BODY_DFSDM_I2S_GREQ_H_ */
