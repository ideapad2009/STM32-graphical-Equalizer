/*
 * beforeMain.h
 *
 *  Created on: Mar 6, 2020
 *      Author: Hanna Nabil
 */

#ifndef INC_BEFOREMAIN_H_
#define INC_BEFOREMAIN_H_

#include "codec_WM8731.h"
#include <math.h>

#define W8731_ADDR_0 			0x1A // this is the address of the CODEC when CSB is low
#define CODEC_ADDRESS           (W8731_ADDR_0<<1)
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))
#define AUDIO_REC      96
#define ENABLE 1
#define DISABLE 0

uint32_t i;
int32_t RightRecBuff[AUDIO_REC]={0};
int32_t LeftRecBuff[AUDIO_REC]={0};
int32_t TestBuff[AUDIO_REC];
uint8_t DmaTestHalfBuffCplt  = 0;
uint8_t DmaTestBuffCplt      = 0;
int32_t PlayBuff[AUDIO_REC*2]={0}; // playBuff =2* AUDIO_REC coming from DFSDM (as we are duplicating the input signal to stereo)

uint8_t DmaLeftRecHalfBuffCplt  = 0;
uint8_t DmaLeftRecBuffCplt      = 0;
uint8_t DmaRightRecHalfBuffCplt = 0;
uint8_t DmaRightRecBuffCplt     = 0;
uint8_t PlaybackStarted         = 0;

#define TEST_CODEC  1
#if TEST_CODEC == ENABLE
	int16_t dataI2S[100];
	extern I2S_HandleTypeDef hi2s3;
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
	//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)dataI2S, sample_N*2);
}
#endif

void TestBlinking(void);
#endif /* INC_BEFOREMAIN_H_ */
