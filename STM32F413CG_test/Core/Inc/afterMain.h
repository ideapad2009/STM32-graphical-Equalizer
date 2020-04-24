/*
 * afterMain.h
 *
 *  Created on: Mar 6, 2020
 *      Author: Hanna Nabil
 */

#ifndef INC_AFTERMAIN_H_
#define INC_AFTERMAIN_H_

void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	if(hdfsdm_filter == &hdfsdm1_filter0)
	{
		DmaRightRecHalfBuffCplt = 1;
	}/*
	else if (hdfsdm_filter == &hdfsdm1_filter1)
	{
		DmaRightRecHalfBuffCplt DmaLeftRecHalfBuffCplt = 1;
	}*/
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	//if(hdfsdm_filter == &hdfsdm1_filter0)
	//{
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
#endif /* INC_AFTERMAIN_H_ */
