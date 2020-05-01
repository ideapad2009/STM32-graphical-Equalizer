/*
 * equalizer.c
 *
 *  Created on: Jun 23, 2019
 *      Author: Hanna Nabil
 */

#include "main.h"
#include "audio_fw_glo.h"
#include "greq_glo.h"
#include "Equalizer.h"
//#include <stdint.h>
//#include <stdlib.h>



#define SELECTED_BANDS                     GREQ_NB_BANDS_5
static int16_t UserGaindB[SELECTED_BANDS] = {-12,-12,-12,-12,-12};    /* Gain for each band*/

/* Graphic Equalizer */
#define NULL ((void *)0)
void *pGreqPersistentMem = NULL;
void *pGreqScratchMem = NULL;
static  buffer_t EqualizerIn;
static  buffer_t *pEqualizerIn = &EqualizerIn;
static  buffer_t EqualizerOut;
static  buffer_t *pEqualizerOut = &EqualizerOut;

static  greq_static_param_t greq_static_param;
static  greq_dynamic_param_t greq_dynamic_param;

static int16_t GrEqPos = GREQ_NO_PRESET;
static int16_t GrEqEn = 1;



int8_t Equalizer_Init( uint16_t buffer_size)
{

	int32_t error = GREQ_ERROR_NONE;
	/* Allocat mem for GrEq */
	pGreqPersistentMem = malloc(  greq_persistent_mem_size); /* greq_persistent_mem_size  0x224 */
	pGreqScratchMem = malloc(greq_scratch_mem_size);       /* greq_scratch_mem_size  0xF00 */

	/* GREQ effect reset */
	error = greq_reset(pGreqPersistentMem, pGreqScratchMem);
	if (error != GREQ_ERROR_NONE)
	{
		return (error);
	}
	/* GREQ effect static parameters setting */
	greq_static_param.nb_bands = SELECTED_BANDS;  /* 10-bands equalizer */
	error = greq_setParam(&greq_static_param, pGreqPersistentMem);
	if (error != GREQ_ERROR_NONE)
	{
		return (error);
	}
	/* GREQ dynamic parameters that can be updated here every frame if required */
	greq_dynamic_param.enable = 1;            /* Enables the effect */
	for ( int i =0 ; i< SELECTED_BANDS ;i++ ){
		greq_dynamic_param.user_gain_per_band_dB[i] = UserGaindB[i]; /* Gain for band 1 */
	}
	greq_dynamic_param.gain_preset_idx = GREQ_NO_PRESET;         /* User preset defined above */

	error = greq_setConfig(&greq_dynamic_param, pGreqPersistentMem);
	if (error != GREQ_ERROR_NONE)
	{
		return (error);
	}
	/* I/O buffers settings */
	// it will process buffer_size *2
	EqualizerIn.nb_bytes_per_Sample = 4; /* 8 bits in 0ne byte */
	EqualizerIn.nb_channels = 2; /* stereo */
	EqualizerIn.buffer_size = buffer_size/2; /* just half buffer is process (size per channel) */
	EqualizerIn.mode = INTERLEAVED;

	EqualizerOut.nb_bytes_per_Sample = 4; /* 8 bits in 0ne byte */
	EqualizerOut.nb_channels = 2; /* stereo */
	EqualizerOut.buffer_size = buffer_size/2; /* just half buffer is process (size per channel) */
	EqualizerOut.mode = INTERLEAVED;

	return GREQ_ERROR_NONE;


}
int8_t equalizerProcess(int32_t *In_Buffer,int32_t * Out_Buffer){
	int32_t error = GREQ_ERROR_NONE;
	EqualizerIn.data_ptr = &In_Buffer[0];
	EqualizerOut.data_ptr = &Out_Buffer[0];
	error = greq_process(pEqualizerIn, pEqualizerIn, pGreqPersistentMem);
		return error;


}
