
/**
 ******************************************************************************
 * @file    app_x-cube-ai.c
 * @author  X-CUBE-AI C code generator
 * @brief   AI program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/*
 * Description
 *   v1.0 - Minimum template to show how to use the Embedded Client API
 *          model. Only one input and one output is supported. All
 *          memory resources are allocated statically (AI_NETWORK_XX, defines
 *          are used).
 *          Re-target of the printf function is out-of-scope.
 *   v2.0 - add multiple IO and/or multiple heap support
 *
 *   For more information, see the embeded documentation:
 *
 *       [1] %X_CUBE_AI_DIR%/Documentation/index.html
 *
 *   X_CUBE_AI_DIR indicates the location where the X-CUBE-AI pack is installed
 *   typical : C:\Users\[user_name]\STM32Cube\Repository\STMicroelectronics\X-CUBE-AI\7.1.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#if defined ( __ICCARM__ )
#elif defined ( __CC_ARM ) || ( __GNUC__ )
#endif

/* System headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "main.h"
#include "ai_datatypes_defines.h"
#include "network.h"
#include "network_data.h"

/* USER CODE BEGIN includes */
#define TEMP_BUF_SIZE 17
#define ADC_VREF_VOLTS     3.3f
#define ADC_MAX_COUNT      4095.0f
#define LM35_MV_PER_DEG_C  10.0f
char temp_buffer[TEMP_BUF_SIZE];
float temperature_c=0;
volatile float dbg_temperature;
volatile float dbg_normalized;
volatile int8_t dbg_quantized_input;

volatile int8_t dbg_output0;
volatile int8_t dbg_output1;
volatile int8_t dbg_output2;

volatile int dbg_predicted_class;
extern ADC_HandleTypeDef hadc1;

/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/

#if !defined(AI_NETWORK_INPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_in_1[AI_NETWORK_IN_1_SIZE_BYTES];
ai_i8* data_ins[AI_NETWORK_IN_NUM] = {
		data_in_1
};
#else
ai_i8* data_ins[AI_NETWORK_IN_NUM] = {
		NULL
};
#endif

#if !defined(AI_NETWORK_OUTPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_out_1[AI_NETWORK_OUT_1_SIZE_BYTES];
ai_i8* data_outs[AI_NETWORK_OUT_NUM] = {
		data_out_1
};
#else
ai_i8* data_outs[AI_NETWORK_OUT_NUM] = {
		NULL
};
#endif

/* Activations buffers -------------------------------------------------------*/

AI_ALIGNED(32)
static uint8_t pool0[AI_NETWORK_DATA_ACTIVATION_1_SIZE];

ai_handle data_activations0[] = {pool0};

/* AI objects ----------------------------------------------------------------*/

static ai_handle network = AI_HANDLE_NULL;

static ai_buffer* ai_input;
static ai_buffer* ai_output;

static void ai_log_err(const ai_error err, const char *fct)
{
	/* USER CODE BEGIN log */
	if (fct)
		printf("TEMPLATE - Error (%s) - type=0x%02x code=0x%02x\r\n", fct,
				err.type, err.code);
	else
		printf("TEMPLATE - Error - type=0x%02x code=0x%02x\r\n", err.type, err.code);

	do {} while (1);
	/* USER CODE END log */
}

static int ai_boostrap(ai_handle *act_addr)
{
	ai_error err;

	/* Create and initialize an instance of the model */
	err = ai_network_create_and_init(&network, act_addr, NULL);
	if (err.type != AI_ERROR_NONE) {
		ai_log_err(err, "ai_network_create_and_init");
		return -1;
	}

	ai_input = ai_network_inputs_get(network, NULL);
	ai_output = ai_network_outputs_get(network, NULL);

#if defined(AI_NETWORK_INPUTS_IN_ACTIVATIONS)
	/*  In the case where "--allocate-inputs" option is used, memory buffer can be
	 *  used from the activations buffer. This is not mandatory.
	 */
	for (int idx=0; idx < AI_NETWORK_IN_NUM; idx++) {
		data_ins[idx] = ai_input[idx].data;
	}
#else
	for (int idx=0; idx < AI_NETWORK_IN_NUM; idx++) {
		ai_input[idx].data = data_ins[idx];
	}
#endif

#if defined(AI_NETWORK_OUTPUTS_IN_ACTIVATIONS)
	/*  In the case where "--allocate-outputs" option is used, memory buffer can be
	 *  used from the activations buffer. This is no mandatory.
	 */
	for (int idx=0; idx < AI_NETWORK_OUT_NUM; idx++) {
		data_outs[idx] = ai_output[idx].data;
	}
#else
	for (int idx=0; idx < AI_NETWORK_OUT_NUM; idx++) {
		ai_output[idx].data = data_outs[idx];
	}
#endif

	return 0;
}

static int ai_run(void)
{
	ai_i32 batch;

	batch = ai_network_run(network, ai_input, ai_output);
	if (batch != 1) {
		ai_log_err(ai_network_get_error(network),
				"ai_network_run");
		return -1;
	}

	return 0;
}

/* USER CODE BEGIN 2 */
static ai_i8 quantize_int8(float value, float scale, int32_t zero_point)
{
	float qf = (value / scale) + (float)zero_point;

	int32_t q;

	/*
	 * Manual rounding.
	 * Avoiding roundf() for now.
	 */
	if (qf >= 0.0f)
	{
		q = (int32_t)(qf + 0.5f);
	}
	else
	{
		q = (int32_t)(qf - 0.5f);
	}

	/*
	 * INT8 saturation
	 */
	if (q > 127)
	{
		q = 127;
	}

	if (q < -128)
	{
		q = -128;
	}

	return (ai_i8)q;
}
int acquire_and_process_data(ai_i8* data[])
{
	/* fill the inputs of the c-model */
	if (HAL_ADC_Start(&hadc1) != HAL_OK)
	{
	    return -1;
	}

	if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
	{
	    HAL_ADC_Stop(&hadc1);
	    return -1;
	}

	uint32_t adc_raw = HAL_ADC_GetValue(&hadc1);

	HAL_ADC_Stop(&hadc1);

	/*Converting Raw Temperature into °C */
	temperature_c=(adc_raw * ADC_VREF_VOLTS * 100.0f) / ADC_MAX_COUNT;

	float normalized = (temperature_c - TEMP_MEAN) / TEMP_STD;

	float input_scale = AI_BUFFER_META_INFO_INTQ_GET_SCALE(ai_input[0].meta_info,0);

	int32_t input_zero_point = AI_BUFFER_META_INFO_INTQ_GET_ZEROPOINT(ai_input[0].meta_info,0);

	data[0][0] = quantize_int8(normalized,input_scale,input_zero_point);

	/* Below variables are for debugging */
	dbg_temperature     = temperature_c;
	dbg_normalized      = normalized;
	//dbg_quantized_input = quantized_input;

	return 0;
}

int post_process(ai_i8* data[])
{
	/* process the predictions*/
	ai_i8 *output = data[0];

	int max_index = 0;

	/* Find the largest output */
	if (output[1] > output[max_index])
	{
		max_index = 1;
	}

	if (output[2] > output[max_index])
	{
		max_index = 2;
	}

	/* Save for debugger also */
	dbg_output0 = output[0];
	dbg_output1 = output[1];
	dbg_output2 = output[2];

	dbg_predicted_class = max_index;

	/* LCD Row 1*/
	snprintf(temp_buffer,TEMP_BUF_SIZE,"T: %d %cC",(int)temperature_c,223);
	KM_LCD_Write_Cmd(0x80);

	KM_LCD_Write_Str((char *)temp_buffer);

	/* LCD Row 2*/
	KM_LCD_Write_Cmd(0xC0);

	switch (max_index)
	{
	case 0:
		KM_LCD_Write_Str("CLASS: COLD      ");
		break;

	case 1:
		KM_LCD_Write_Str("CLASS: NORMAL    ");
		break;

	case 2:
		KM_LCD_Write_Str("CLASS: HOT       ");
		break;

	default:
		KM_LCD_Write_Str("CLASS: ERROR     ");
		break;
	}

	return 0;
}
/* USER CODE END 2 */

/* Entry points --------------------------------------------------------------*/

void MX_X_CUBE_AI_Init(void)
{
	/* USER CODE BEGIN 5 */
	KM_LCD_Write_Cmd(0x82);
	KM_LCD_Write_Str("INITIALIZING");
	KM_LCD_Write_Cmd(0xC4);
	KM_LCD_Write_Str("AI MODEL");
	HAL_Delay(2000);
	KM_LCD_Write_Cmd(0x01);
	ai_boostrap(data_activations0);
	/* USER CODE END 5 */
}

void MX_X_CUBE_AI_Process(void)
{
	/* USER CODE BEGIN 6 */
	int res = -1;

	/* MENTIONING 'P' TO INDICATE PREDICTING STARTS */
	KM_LCD_Write_Cmd(0x8F);
	KM_LCD_Write_Data('P');

	if (network) {

		/* 1. Sensor acquisition + preprocessing */
		res = acquire_and_process_data(data_ins);

		/* 2. Neural-network inference */
		if (res == 0)
		{
			res = ai_run();
		}

		/* CLEARING P AS PREDICTION DONE */
		HAL_Delay(500);
		KM_LCD_Write_Cmd(0x8F);
		KM_LCD_Write_Data(' ');

		/* 3. Output interpretation */
		if (res == 0)
		{
			res = post_process(data_outs);
		}
	}

	if (res) {
		ai_error err = {AI_ERROR_INVALID_STATE, AI_ERROR_CODE_NETWORK};
		ai_log_err(err, "Process has FAILED");
	}

	/* USER CODE END 6 */
}
#ifdef __cplusplus
}
#endif
