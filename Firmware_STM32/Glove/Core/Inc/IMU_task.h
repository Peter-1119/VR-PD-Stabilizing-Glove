/*
 * IMU_task.h
 *
 *  Created on: Feb 21, 2024
 *      Author: Peter
 */

#ifndef INC_IMU_TASK_H_
#define INC_IMU_TASK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "main.h"

typedef enum Trigonometric{
	SinWave,
	CosWave
}trigonometric;

void TCA_Select(int num);
void IMU_All_Init();
void IMU_All_Bank_0();
void IMU_All_Read_Data(uint8_t* buf);
void FFT_Init(int All);
void FFT(float *signal, int axis);
void Frequency_Energy(uint8_t* buf, uint8_t* freq_content);
int Calculate_ab(float accex, float accey, float accez);
void Calculate_Freq_Energy(uint8_t* buf);
float FastTrigonometric(float x, trigonometric wave);
float SquareRootFloat(float number);

#ifdef __cplusplus
}
#endif

#endif /* INC_IMU_TASK_H_ */
