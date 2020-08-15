/**
  ******************************************************************************
  * File Name          : DFSDM.c
  * Description        : This file provides code for the configuration
  *                      of the DFSDM instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "dfsdm.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
DFSDM_Channel_HandleTypeDef hdfsdm1_channel1;
DMA_HandleTypeDef hdma_dfsdm1_flt0;

/* DFSDM1 init function */
void MX_DFSDM1_Init(void)
{

  hdfsdm1_filter0.Instance = DFSDM1_Filter0;
  hdfsdm1_filter0.Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
  hdfsdm1_filter0.Init.RegularParam.FastMode = ENABLE;
  hdfsdm1_filter0.Init.RegularParam.DmaMode = ENABLE;
  hdfsdm1_filter0.Init.FilterParam.SincOrder = DFSDM_FILTER_SINC4_ORDER;
  hdfsdm1_filter0.Init.FilterParam.Oversampling = 64;
  hdfsdm1_filter0.Init.FilterParam.IntOversampling = 1;
  if (HAL_DFSDM_FilterInit(&hdfsdm1_filter0) != HAL_OK)
  {
    Error_Handler();
  }
  hdfsdm1_channel1.Instance = DFSDM1_Channel1;
  hdfsdm1_channel1.Init.OutputClock.Activation = ENABLE;
  hdfsdm1_channel1.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
  hdfsdm1_channel1.Init.OutputClock.Divider = 20;
  hdfsdm1_channel1.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hdfsdm1_channel1.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
  hdfsdm1_channel1.Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
  hdfsdm1_channel1.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_FALLING;
  hdfsdm1_channel1.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hdfsdm1_channel1.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
  hdfsdm1_channel1.Init.Awd.Oversampling = 10;
  hdfsdm1_channel1.Init.Offset = 0;
  hdfsdm1_channel1.Init.RightBitShift = 0;
  if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter0, DFSDM_CHANNEL_1, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK)
  {
    Error_Handler();
  }

}

static uint32_t HAL_RCC_DFSDM1_CLK_ENABLED=0;

static uint32_t DFSDM1_Init = 0;

void HAL_DFSDM_FilterMspInit(DFSDM_Filter_HandleTypeDef* dfsdm_filterHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if((IS_DFSDM1_FILTER_INSTANCE(dfsdm_filterHandle->Instance))&&(DFSDM1_Init == 0))
  {
  /* USER CODE BEGIN DFSDM1_MspInit 0 */

  /* USER CODE END DFSDM1_MspInit 0 */
    /* DFSDM1 clock enable */
    HAL_RCC_DFSDM1_CLK_ENABLED++;
    if(HAL_RCC_DFSDM1_CLK_ENABLED==1){
      __HAL_RCC_DFSDM1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DFSDM1 GPIO Configuration
    PA4     ------> DFSDM1_DATIN1
    PA8     ------> DFSDM1_CKOUT
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_DFSDM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_DFSDM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN DFSDM1_MspInit 1 */

  /* USER CODE END DFSDM1_MspInit 1 */
  DFSDM1_Init++;
  }

    /* DFSDM1 DMA Init */
    /* DFSDM1_FLT0 Init */
  if(dfsdm_filterHandle->Instance == DFSDM1_Filter0){
    hdma_dfsdm1_flt0.Instance = DMA2_Stream0;
    hdma_dfsdm1_flt0.Init.Channel = DMA_CHANNEL_7;
    hdma_dfsdm1_flt0.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dfsdm1_flt0.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dfsdm1_flt0.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dfsdm1_flt0.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dfsdm1_flt0.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dfsdm1_flt0.Init.Mode = DMA_CIRCULAR;
    hdma_dfsdm1_flt0.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dfsdm1_flt0.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dfsdm1_flt0) != HAL_OK)
    {
      Error_Handler();
    }

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one stream to perform all the requested DMAs. */
    __HAL_LINKDMA(dfsdm_filterHandle,hdmaInj,hdma_dfsdm1_flt0);
    __HAL_LINKDMA(dfsdm_filterHandle,hdmaReg,hdma_dfsdm1_flt0);
  }

}

void HAL_DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef* dfsdm_channelHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if((IS_DFSDM1_CHANNEL_INSTANCE(dfsdm_channelHandle->Instance))&&(DFSDM1_Init == 0))
  {
  /* USER CODE BEGIN DFSDM1_MspInit 0 */

  /* USER CODE END DFSDM1_MspInit 0 */
    /* DFSDM1 clock enable */
    HAL_RCC_DFSDM1_CLK_ENABLED++;
    if(HAL_RCC_DFSDM1_CLK_ENABLED==1){
      __HAL_RCC_DFSDM1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DFSDM1 GPIO Configuration
    PA4     ------> DFSDM1_DATIN1
    PA8     ------> DFSDM1_CKOUT
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_DFSDM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_DFSDM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN DFSDM1_MspInit 1 */

  /* USER CODE END DFSDM1_MspInit 1 */
  DFSDM1_Init++;
  }
}

void HAL_DFSDM_FilterMspDeInit(DFSDM_Filter_HandleTypeDef* dfsdm_filterHandle)
{

  if((IS_DFSDM1_FILTER_INSTANCE(dfsdm_filterHandle->Instance)))
  {
    DFSDM1_Init-- ;
    if((DFSDM1_Init == 0))
    {
  /* USER CODE BEGIN DFSDM1_MspDeInit 0 */

  /* USER CODE END DFSDM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DFSDM1_CLK_DISABLE();

    /**DFSDM1 GPIO Configuration
    PA4     ------> DFSDM1_DATIN1
    PA8     ------> DFSDM1_CKOUT
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_8);

    /* DFSDM1 DMA DeInit */
    HAL_DMA_DeInit(dfsdm_filterHandle->hdmaInj);
    HAL_DMA_DeInit(dfsdm_filterHandle->hdmaReg);

  /* USER CODE BEGIN DFSDM1_MspDeInit 1 */

  /* USER CODE END DFSDM1_MspDeInit 1 */
    }
  }
}

void HAL_DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef* dfsdm_channelHandle)
{

  if((IS_DFSDM1_CHANNEL_INSTANCE(dfsdm_channelHandle->Instance)))
  {
    DFSDM1_Init-- ;
    if((DFSDM1_Init == 0))
    {
  /* USER CODE BEGIN DFSDM1_MspDeInit 0 */

  /* USER CODE END DFSDM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DFSDM1_CLK_DISABLE();

    /**DFSDM1 GPIO Configuration
    PA4     ------> DFSDM1_DATIN1
    PA8     ------> DFSDM1_CKOUT
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_8);

  /* USER CODE BEGIN DFSDM1_MspDeInit 1 */

  /* USER CODE END DFSDM1_MspDeInit 1 */
    }
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
