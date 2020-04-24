/*
 * iirFilter.h
 *
 *  Created on: Apr 23, 2020
 *      Author: Hanna Nabil
 */

#ifndef INC_IIRFILTER_H_
#define INC_IIRFILTER_H_

float l_a0, l_a1, l_a2, l_b1, l_b2, lin_z1, lin_z2, lout_z1, lout_z2;
float r_a0, r_a1, r_a2, r_b1, r_b2, rin_z1, rin_z2, rout_z1, rout_z2;
  //left-channel, High-Pass, 1kHz, fs=96kHz, q=0.7
 /* l_a0 = 0.9543457485325094f;
  l_a1 = -1.9086914970650188f;
  l_a2 = 0.9543457485325094f;
  l_b1 = -1.9066459797557103f;
  l_b2 = 0.9107370143743273f;

  //right-channel, Low-Pass, 1kHz, fs)96 kHz, q=0.7
  r_a0 = 0.0010227586546542474f;
  r_a1 = 0.002045517309308495f;
  r_a2 = 0.0010227586546542474f;
  r_b1 = -1.9066459797557103f;
  r_b2 = 0.9107370143743273f;*/

#endif /* INC_IIRFILTER_H_ */
