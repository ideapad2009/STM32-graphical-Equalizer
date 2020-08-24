#ifndef INC_BODY_H_
#define INC_BODY_H_

#include "codec_WM8731.h"
#include <math.h>
#include "song_16.h"
#include "song_24_48k.h"
#include "song_24_43k.h"
#include <song_u8_43k.h>
#define W8731_ADDR_0 			0x1A // this is the address of the CODEC when CSB is low
#define CODEC_ADDRESS           (W8731_ADDR_0<<1)
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define AUDIO_REC      	          48


uint32_t i,j;
uint32_t x= 0 ;
int32_t RightRecBuff[AUDIO_REC*4]={0};
int32_t LeftRecBuff[AUDIO_REC]={0};
int16_t RecBuff[AUDIO_REC*2]={0};

uint16_t PlayBuff[AUDIO_REC*2]={0}; // playBuff =2* AUDIO_REC coming from DFSDM (as we are duplicating the input signal to stereo)
uint16_t txBuf[AUDIO_REC*8]={0};
int32_t  Sample32 =0;
uint32_t uSample32 =0;
uint16_t uSample16 =0;

uint8_t DmaLeftRecHalfBuffCplt  = 0;
uint8_t DmaLeftRecBuffCplt      = 0;
uint8_t DmaRightRecHalfBuffCplt = 0;
uint8_t DmaRightRecBuffCplt     = 0;
uint8_t PlaybackStarted         = 0;
uint8_t I2sFlag                 = 0;

#define PLAY_SONG_8_BIT       0  //DAC
#define PLAY_SONG_16_BIT      1
#define PLAY_SONG_24_BIT      2
#define ENABLE 1
#define DISABLE 0

//configure /////////////////////
#define TEST_CODEC_STATE  DISABLE
#define PLAY_SONG_STATE   ENABLE
#define PLAY_SONG_BIT     PLAY_SONG_8_BIT
////////////////////////////////

#if TEST_CODEC_STATE == ENABLE
int32_t dataI2S[100];

void sine_wave(){
#define PI 3.14159f
	//Sample rate and Output freq
#define F_SAMPLE		48000.0f
#define F_OUT			1000.0f
	float mySinVal;
	float sample_dt;
	uint16_t sample_N;
	sample_dt = F_OUT/F_SAMPLE;
	sample_N = F_SAMPLE/F_OUT;
	//Build Sine wave
	for(uint16_t i=0; i<sample_N; i++)
	{
		mySinVal = sinf(i*2*PI*sample_dt);
		dataI2S[i*2] = (mySinVal )*8000;    //Right data (0 2 4 6 8 10 12)
		dataI2S[(i*2) + 1] =(mySinVal )*8000; //Left data  (1 3 5 7 9 11 13)
	}
	//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)dataI2S, sample_N*4);
}
#endif

#if PLAY_SONG_STATE ==   ENABLE
void playSong(void){
#if PLAY_SONG_BIT == PLAY_SONG_24_BIT
	if (x < (SONG_SIZE_43 -4 )){
		for(i = 0,j=x; i < AUDIO_REC; i++,j++)
		{
			uSample32        = (uint32_t) song_43[j] ;
			txBuf[i*4]     = (uSample32>>16) & 0xFFFF;
			txBuf[(i*4)+1] = (uSample32 )& 0xFFFF;
			txBuf[(i*4)+2] = txBuf[i*4];
			txBuf[(i*4)+3] = txBuf[(i*4)+1];
		}
		x = x+AUDIO_REC;
	}
	else {x=0;playSong();}
#elif PLAY_SONG_BIT == PLAY_SONG_16_BIT
	if (x < (SONG_SIZE_16 -1 )){
		for(i = 0,j=x; i < AUDIO_REC; i++,j++)
		{
			txBuf[i*2] = (uint16_t) song_16[j] ;
			txBuf[(i*2)+1] = txBuf[i*2];
		}
		x = x+AUDIO_REC;
	}
	else {x=0;playSong();}
#elif PLAY_SONG_BIT == PLAY_SONG_8_BIT
	if (x < (SONG_SIZE_u8_43 -1 )){
		for(i = 0,j=x; i < AUDIO_REC; i++,j++)
		{
			uSample16 = (int16_t) song_16[j]+ 32768 ;
			txBuf[i]= (uSample16>>4);
		}
		x = x+AUDIO_REC;
	}
	else {x=0;playSong();}
#endif
}
#endif
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


	//__HAL_UNLOCK(&hi2s3);     // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!
	//__HAL_I2S_ENABLE(&hi2s3);
//	if (HAL_I2C_IsDeviceReady(&hi2c1, CODEC_ADDRESS, 1, 10) == HAL_OK){
//		TestBlinking();
//	}
	//Codec_Reset(&hi2c1);
#if TEST_CODEC_STATE == DISABLE & PLAY_SONG_STATE == DISABLE
	/* if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter1, LeftRecBuff, AUDIO_REC))
     {
   	  Error_Handler();
     }*/
	/*if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, RightRecBuff, AUDIO_REC))
	{
		Error_Handler();
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);*/
//	if(HAL_OK != HAL_DFSDM_FilterRegularMsbStart_DMA(&hdfsdm1_filter0, RecBuff, AUDIO_REC))
//		{
//			Error_Handler();
//		}

#endif
#if TEST_CODEC_STATE == ENABLE
	sine_wave();
#endif
#if ((PLAY_SONG_STATE == ENABLE) & (PLAY_SONG_BIT== PLAY_SONG_24_BIT))
	playSong();
	if(HAL_OK != HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2))
	{
		Error_Handler();
	}
#elif ((PLAY_SONG_STATE  == ENABLE) & (PLAY_SONG_BIT ==PLAY_SONG_16_BIT))
	playSong();
	if(HAL_OK != HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2))
	{
		Error_Handler();
	}
#elif ((PLAY_SONG_STATE  == ENABLE) & (PLAY_SONG_BIT ==PLAY_SONG_8_BIT))
	MX_DAC_Init();
	MX_TIM6_Init();
	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);
	playSong();
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, txBuf, AUDIO_REC, DAC_ALIGN_12B_R);
#endif

	while (1)
	{

#if TEST_CODEC_STATE == DISABLE & PLAY_SONG_STATE == DISABLE

		/*if((DmaRightRecHalfBuffCplt == 1))
		{

			for(i = 0; i < AUDIO_REC/2; i++)
			{
				uSample32 =  (uint32_t) RightRecBuff[i];
				uSample16 = (uSample32>>20) & 0xFFF;
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,uSample16);
				//PlayBuff[2*i]     = SaturaLH((RightRecBuff[i] >> 8), -32768, 32767);
				uSample32 =  (uint32_t) RightRecBuff[i];
				txBuf[i*4] = (uSample32>>16) & 0xFFFF;
				txBuf[(i*4)+1] = uSample32 & 0xFFFF;
				txBuf[(i*4)+2] = txBuf[i*4];
				txBuf[(i*4)+3] = txBuf[(i*4)+1];

			}
			//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2);
//			if(PlaybackStarted == 0)
//			{
//				if(HAL_OK != HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2))
//				{
//					Error_Handler();
//				}
//				PlaybackStarted = 1;
//			}
			DmaLeftRecHalfBuffCplt  = 0;
			DmaRightRecHalfBuffCplt = 0;
		}
		if( (DmaRightRecBuffCplt == 1))
		{
//			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//			HAL_Delay(200);
			for(i = AUDIO_REC/2; i < AUDIO_REC; i++)
			{
				uSample32 =  (uint32_t) RightRecBuff[i];
				uSample16 = (uSample32>>20) & 0xFFF;
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,uSample16);
				uSample32 = (uint32_t) RightRecBuff[i];
				txBuf[i*4] = (uSample32>>16) & 0xFFFF;
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,txBuf[i*4]);
				txBuf[(i*4)+1] = uSample32 & 0xFFFF;
				txBuf[(i*4)+2] = txBuf[i*4];
				txBuf[(i*4)+3] = txBuf[(i*4)+1];
			}
			//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)txBuf, AUDIO_REC*2);
			DmaLeftRecBuffCplt  = 0;
			DmaRightRecBuffCplt = 0;
		}

*/
#endif
	}
}

#if PLAY_SONG_BIT == PLAY_SONG_8_BIT
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
	playSong();
}
#endif
#if PLAY_SONG_STATE == ENABLE
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	playSong();
}
#endif
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	//if(hdfsdm_filter == &hdfsdm1_filter0)
	//{
		DmaRightRecHalfBuffCplt = 1;
	//}
/*
	else if (hdfsdm_filter == &hdfsdm1_filter1)
	{
		DmaRightRecHalfBuffCplt DmaLeftRecHalfBuffCplt = 1;
	}*/
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	//if(hdfsdm_filter == &hdfsdm1_filter0)
	//{

	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);

	DmaRightRecBuffCplt = 1;
	//}
	/*
	else if (hdfsdm_filter == &hdfsdm1_filter1)
	{
		 DmaLeftRecBuffCplt= 1;
	}*/
}

void TestBlinking(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_Delay(1000);
}

#endif /* INC_BODY2_H_ */
