/*
 * codec_WM8731.c
 *
 *  Created on: Feb 19, 2020
 *      Author: Hanna Nabil
 */


/*
 * codec.c - Discovery board codec interface routines
 *
 * Cut from stm32f4_discovery_audio_codec.c
 *
 */

/*========================

                CS43L22 Audio Codec Control Functions
                                                ==============================*/
/**
  * @brief  Initializes the audio codec and all related interfaces (control
  *         interface: I2C and audio interface: I2S)
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */

#include "codec_WM8731.h"
extern I2S_HandleTypeDef hi2s3;
//extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef i2cx;
static uint8_t iData[2];

#define W8731_ADDR_0 0x1A // this is the address of the CODEC when CSB is low
#define W8731_ADDR_1 0x1B // this is the address of the CODEC when CSB is high
#define W8731_NUM_REGS 10
/* The 7 bits Codec address (sent through I2C interface) */
#define CODEC_ADDRESS           (W8731_ADDR_0<<1)

const uint16_t w8731_init_data[] =
{
	0x097,	// Reg 00: Left Line In (0dB, mute off)
	0x097,	// Reg 01: Right Line In (0dB, mute off)
	0x07F,	// Reg 02: Left Headphone out (0dB)
	0x07F,	// Reg 03: Right Headphone out (0dB)
	0x012,	// Reg 04: Analog Audio Path Control (DAC sel, Mute Mic) //0x012 in old version
	0x000,	// Reg 05: Digital Audio Path Control
	0x067,	// Reg 06: Power Down Control (Clkout, Osc, Mic Off) //0x062 in old version
//	0x00E,	// Reg 07: Digital Audio Interface Format (i2s, 32-bit, slave)
//  0x002,	// Reg 07: Digital Audio Interface Format (i2s, 16-bit, slave)
	0x00A,	// Reg 07: Digital Audio Interface Format (i2s, 24-bit, slave)
	//0x000,	// Reg 08: Sampling Control (Normal,BOSR=256fs, 48k ADC/DAC)
	0x020,	// Reg 08: Sampling Control (Normal,BOSR=256fs, 44.1k ADC/DAC)
//	0x002,	// Reg 08: Sampling Control (Normal,BOSR=384fs, 48k ADC/DAC)
	0x001	// Reg 09: Active Control
};


void Codec_Reset(I2C_HandleTypeDef *i2c_handle)
{
	uint8_t i;
	uint8_t status =0;
	i2cx = *i2c_handle;
	Codec_WriteRegister(0x0F, 0x00);

	/* Load default values */
	for(i=0;i<W8731_NUM_REGS;i++)
	{
		HAL_Delay(100);

		status=Codec_WriteRegister(i, w8731_init_data[i]);
		while (status ==0){
			status=Codec_WriteRegister(i, w8731_init_data[i]);
		}

	}
}

/**
  * @brief  Writes a Byte to a given register into the audio codec through the
            control interface (I2C)
  * @param  RegisterAddr: The address (location) of the register to be written.
  * @param  RegisterValue: the Byte value to be written into destination register.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue)
{
	uint32_t result = 0;
	// RegisterValue is 9-bit register
	/* Assemble 2-byte data in WM8731 format */
	uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x1);
	uint8_t Byte2 = RegisterValue&0xFF;
	iData[0] = Byte1;
	iData[1] = Byte2;
	/* Transmit the slave address and enable writing operation */
	// take the most significant 7 bit address in the address byte
	if (HAL_I2C_Master_Transmit(&i2cx, CODEC_ADDRESS, iData, 2, 1000) != HAL_OK)
		{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

		result = 0;
		}
	else {

		result =1;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	}

	return result;
}

