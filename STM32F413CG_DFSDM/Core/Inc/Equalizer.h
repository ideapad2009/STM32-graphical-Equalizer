/*
 * Equalizer.h
 *
 *  Created on: Jun 23, 2019
 *      Author: Hanna Nabil
 */

#ifndef EQUALIZER_H_
#define EQUALIZER_H_
int8_t Equalizer_Init( uint16_t buffer_size);
int8_t equalizerProcess(int32_t *In_Buffer);
#endif /* EQUALIZER_H_ */
