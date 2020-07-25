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
#include "song_16.h"
#include "song_16_32k.h"
//#include "song_24_48k.h"
//#include "song_24_43k.h"
//#include <song_u8_43k.h>
#define W8731_ADDR_0 			     0x1A // this is the address of the CODEC when CSB is low
#define CODEC_ADDRESS               (W8731_ADDR_0<<1)
#define SaturaLH(N, L, H)           (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define AUDIO_REC      	            100


uint32_t i,j;
uint32_t x= 0 ;
int DAC_FLAG =0;
int32_t RightRecBuff[AUDIO_REC*4]={0};
int32_t LeftRecBuff[AUDIO_REC]={0};
int16_t RecBuff[AUDIO_REC*2]={0};

uint16_t PlayBuff[AUDIO_REC*2]={0}; // playBuff =2* AUDIO_REC coming from DFSDM (as we are duplicating the input signal to stereo)
uint16_t txBuf[AUDIO_REC*8]={0};
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


void TestBlinking(void);
void delay(uint32_t ms);
void playSong(void){
	if (x < (SONG_SIZE_16_32k -1 )){
		for(i = 0,j=x; i < AUDIO_REC; i++,j++)
		{
			uSample16= (int16_t)song_16_32k[j] + 32768 ;
			txBuf[i] =uSample16 >>4;

		}
		x = x+AUDIO_REC;
	}
	else {x=0;playSong();}
	}

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
	TestBlinking();

	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);


	//__HAL_UNLOCK(&hi2s3);     // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!
	//__HAL_I2S_ENABLE(&hi2s3);
	//	if (HAL_I2C_IsDeviceReady(&hi2c1, CODEC_ADDRESS, 1, 10) == HAL_OK){
//		TestBlinking();
//	}
	//Codec_Reset(&hi2c1);
	if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, RightRecBuff, AUDIO_REC))
	{
		Error_Handler();
	}
	/*if(HAL_OK != HAL_DFSDM_FilterRegularMsbStart_DMA(&hdfsdm1_filter0, RecBuff, AUDIO_REC))
		{
			Error_Handler();
		}*/

	//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)txBuf, AUDIO_REC, DAC_ALIGN_12B_R));


	while (1)
	{

		if((DmaRightRecHalfBuffCplt == 1))
		{

			for(i = 0; i < AUDIO_REC/2; i++)
			{
				//sample16 =  SaturaLH((RightRecBuff[i] >> 8), -4096, 4096);
				sample16 =  RightRecBuff[i] >> 8;
				uSample16 = (int16_t)sample16 + 4096;
				//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, uSample16>>4);
				txBuf[i] = (uSample16>>1) ;
			}
			//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t* )&txBuf[0], AUDIO_REC/2, DAC_ALIGN_12B_R);
			if(PlaybackStarted == 0)
			{
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, txBuf, AUDIO_REC, DAC_ALIGN_12B_R);
				PlaybackStarted = 1;
			}
			DmaLeftRecHalfBuffCplt  = 0;
			DmaRightRecHalfBuffCplt = 0;
		}
		if( (DmaRightRecBuffCplt == 1))
		{
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				sample16 = RightRecBuff[i] >> 8;
				uSample16 = (int16_t)sample16 + 4096;
				//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, uSample16>>4);
				txBuf[i] = (uSample16>>1) ;
			}
			//HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t* )&txBuf[AUDIO_REC/2], AUDIO_REC/2, DAC_ALIGN_12B_R);

			DmaLeftRecBuffCplt  = 0;
			DmaRightRecBuffCplt = 0;
		}
	}
}


/*void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
	static int x =0;
	x++;
	if (x == 48){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);}
	if ( x > 48){
		x=0;
	}

}*/
/*void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
	//playSong();
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}*/
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
		DmaRightRecHalfBuffCplt = 1;
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{

	DmaRightRecBuffCplt = 1;
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	//DAC_FLAG=1;
}

void TestBlinking(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	//delay(1000);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	//delay(1000);
	HAL_Delay(1000);
}

#endif /* INC_BODY_DAC_DFSDM_H_ */
