/*
 * Mahony.c
 *
 *  Created on: Apr 24, 2024
 *      Author: Peter
 */

#include "IMU_task.h"
float Accex, Accey, Accez, Acc[3];
float Gyrox, Gyroy, Gyroz, Gyro[3], Past_Gyro[7][3];
float Kp, Ki, delta_t;
float Mahony_q[7][4];
float Gyro_bias[7][3];
float Gyro_counter = 0;
float gravity[3] = {0.0, 0.0, 1.0};
float Past_eInt[7][3];
float eInt[7][3];

float Past_e[7][3];
float e[3];
float Est_Acc[3];
float acce_scale = 1 / 16384.0;
float gyro_scale = 1 / 32768.0 * 250.0 / 180.0 * 3.141592653589793;

float SquareRootFloat(float number);

void q_init(){
	delta_t = 0.02;
	Kp = 1.0;
	Ki = 0.1;

	Accex = 0; Accey = 0; Accez = 0;
	Gyrox = 0; Gyroy = 0; Gyroz = 0;
	Acc[0] = 0; Acc[1] = 0; Acc[2] = 0;

	for (int index = 0; index < 7; index++){
		Mahony_q[index][0] = 1.0;  // Initialize w component
		for (int component = 1; component < 4; component++)
			Mahony_q[index][component] = 0.0;  // Initialize x, y, z component
	}

	for (int index = 0; index < 7; index++){
		for (int component = 0; component < 3; component++){
			eInt[index][component] = 0.0;  // Initialize x, y, z component
			Gyro_bias[index][component] = 0.0;
			Past_e[index][component] = 0.0;
			Past_eInt[index][component] = 0.0;
			Past_Gyro[index][component] = 0.0;
		}
	}
}

void Record_Gyro_bias(uint8_t *IMU_data){
	for(int index = 0; index < 7; index++){
		float gyrox = (float)((int16_t)(IMU_data[8 + index * 20] << 8 | IMU_data[9 + index * 20])) * gyro_scale * -1.0;
		float gyroy = (float)((int16_t)(IMU_data[6 + index * 20] << 8 | IMU_data[7 + index * 20])) * gyro_scale;
		float gyroz = (float)((int16_t)(IMU_data[10 + index * 20] << 8 | IMU_data[11 + index * 20])) * gyro_scale;

		Gyro_bias[index][0] += gyrox;
		Gyro_bias[index][1] += gyroy;
		Gyro_bias[index][2] += gyroz;
	}
	Gyro_counter += 1;
}

void statistic_gyro_bias(){
	for(int index = 0; index < 7; index++){
		Gyro_bias[index][0] /= Gyro_counter;
		Gyro_bias[index][1] /= Gyro_counter;
		Gyro_bias[index][2] /= Gyro_counter;
	}
}

void qpq(float *q, float *p, int inverse){
	float new_x, new_y, new_z;

	if (inverse == 0){
		new_x = (1 - 2*q[2]*q[2] - 2*q[3]*q[3]) * p[0] + (2*q[1]*q[2] - 2*q[0]*q[3]) * p[1] + (2*q[1]*q[3] + 2*q[0]*q[2]) * p[2];
		new_y = (2*q[1]*q[2] + 2*q[0]*q[3]) * p[0] + (1 - 2*q[1]*q[1] - 2*q[3]*q[3]) * p[1] + (2*q[2]*q[3] - 2*q[0]*q[1]) * p[2];
		new_z = (2*q[1]*q[3] - 2*q[0]*q[2]) * p[0] + (2*q[2]*q[3] + 2*q[0]*q[1]) * p[1] + (1 - 2*q[1]*q[1] - 2*q[2]*q[2]) * p[2];
	}
	else{
		new_x = (1 - 2*q[2]*q[2] - 2*q[3]*q[3]) * p[0] + (2*q[1]*q[2] + 2*q[0]*q[3]) * p[1] + (2*q[1]*q[3] - 2*q[0]*q[2]) * p[2];
		new_y = (2*q[1]*q[2] - 2*q[0]*q[3]) * p[0] + (1 - 2*q[1]*q[1] - 2*q[3]*q[3]) * p[1] + (2*q[2]*q[3] + 2*q[0]*q[1]) * p[2];
		new_z = (2*q[1]*q[3] + 2*q[0]*q[2]) * p[0] + (2*q[2]*q[3] - 2*q[0]*q[1]) * p[1] + (1 - 2*q[1]*q[1] - 2*q[2]*q[2]) * p[2];
	}

	*(p + 0) = new_x;
	*(p + 1) = new_y;
	*(p + 2) = new_z;
}

void qGq(float *q, float *p, int inverse){
	if (inverse == 0){
		Est_Acc[0] = 2*q[1]*q[3] + 2*q[0]*q[2];
		Est_Acc[1] = 2*q[2]*q[3] - 2*q[0]*q[1];
		Est_Acc[2] = 1 - 2*q[1]*q[1] - 2*q[2]*q[2];
	}
	else{
		Est_Acc[0] = 2*q[1]*q[3] - 2*q[0]*q[2];
		Est_Acc[1] = 2*q[2]*q[3] + 2*q[0]*q[1];
		Est_Acc[2] = 1 - 2*q[1]*q[1] - 2*q[2]*q[2];
	}
}

void q_prod(float *q, float *p){
	float new_qw = q[0] + 0.5 * (-q[1] * p[0] - q[2] * p[1] - q[3] * p[2]) * delta_t;
	float new_qx = q[1] + 0.5 * (q[0] * p[0] + q[2] * p[2] - q[3] * p[1]) * delta_t;
	float new_qy = q[2] + 0.5 * (q[0] * p[1] - q[1] * p[2] + q[3] * p[0]) * delta_t;
	float new_qz = q[3] + 0.5 * (q[0] * p[2] + q[1] * p[1] - q[2] * p[0]) * delta_t;

	*(q + 0) = new_qw;
	*(q + 1) = new_qx;
	*(q + 2) = new_qy;
	*(q + 3) = new_qz;
}

void vectorCross(float *p1, float *p2){
	e[0] = p1[1] * p2[2] - p1[2] * p2[1];
	e[1] = p1[2] * p2[0] - p1[0] * p2[2];
	e[2] = p1[0] * p2[1] - p1[1] * p2[0];
}

void mahony(uint8_t* IMU_data, int index, float *float_mem){
	//	float accex = (float)((int16_t)(IMU_data[0] << 8 | IMU_data[1])) * acce_scale;
	//	float accey = (float)((int16_t)(IMU_data[2] << 8 | IMU_data[3])) * acce_scale;
	// Correct hardware layout design
	Accex = (float)((int16_t)(IMU_data[2] << 8 | IMU_data[3])) * acce_scale * -1.0;
	Accey = (float)((int16_t)(IMU_data[0] << 8 | IMU_data[1])) * acce_scale;
	Accez = (float)((int16_t)(IMU_data[4] << 8 | IMU_data[5])) * acce_scale;
	Acc[0] = Accex;
	Acc[1] = Accey;
	Acc[2] = Accez;

	//	float gyrox = (float)((int16_t)(IMU_data[6] << 8 | IMU_data[7])) * gyro_scale;
	//	float gyroy = (float)((int16_t)(IMU_data[8] << 8 | IMU_data[9])) * gyro_scale;
	// Correct hardware layout design
	Gyrox = (float)((int16_t)(IMU_data[8] << 8 | IMU_data[9])) * gyro_scale * -1.0;
	Gyroy = (float)((int16_t)(IMU_data[6] << 8 | IMU_data[7])) * gyro_scale;
	Gyroz = (float)((int16_t)(IMU_data[10] << 8 | IMU_data[11])) * gyro_scale;
	Gyro[0] = Gyrox - Gyro_bias[index][0];
	Gyro[1] = Gyroy - Gyro_bias[index][1];
	Gyro[2] = Gyroz - Gyro_bias[index][2];

	for(int component = 0; component < 3; component++){
		Est_Acc[component] = 0.0;
		e[component] = 0.0;
	}

	if (Accex != 0 || Accey != 0 || Accez != 0){
		qGq(Mahony_q[index], gravity, 1);
		vectorCross(Est_Acc, Acc);

		for(int component = 0; component < 3; component++){
//			eInt[index][component] = eInt[index][component] + Ki * e[component] * delta_t;
//			Gyro[component] = Gyro[component] - Kp * e[component] - eInt[index][component];
			eInt[index][component] = eInt[index][component] + Ki * (e[component] + Past_e[index][component]) * delta_t / 2;
			Gyro[component] = ((Gyro[component] + Past_Gyro[index][component]) - Kp * (e[component] + Past_e[index][component]) - (eInt[index][component] + Past_eInt[index][component])) / 2;
		}
	}

	q_prod(Mahony_q[index], Gyro);

	float q_scale = 1 / SquareRootFloat(Mahony_q[index][0] * Mahony_q[index][0] + Mahony_q[index][1] * Mahony_q[index][1] + Mahony_q[index][2] * Mahony_q[index][2] + Mahony_q[index][3] * Mahony_q[index][3]);
	for(int component = 0; component < 4; component++){
		Mahony_q[index][component] = Mahony_q[index][component] * q_scale;
	}

	*(float_mem + 0) = Mahony_q[index][0];
	*(float_mem + 1) = Mahony_q[index][1] * -1.0;
	*(float_mem + 2) = Mahony_q[index][2] * -1.0;
	*(float_mem + 3) = Mahony_q[index][3] * -1.0;

	for(int component = 0; component < 3; component++){
		Past_Gyro[index][component] = Gyro[component];
		Past_e[index][component] = e[component];
		Past_eInt[index][component] = eInt[index][component];
	}
}


//void mahony(uint8_t* IMU_data, int index, float *float_mem){
//	//	float accex = (float)((int16_t)(IMU_data[0] << 8 | IMU_data[1])) * acce_scale;
//	//	float accey = (float)((int16_t)(IMU_data[2] << 8 | IMU_data[3])) * acce_scale;
//	// Correct hardware layout design
//	float accex = (float)((int16_t)(IMU_data[2] << 8 | IMU_data[3])) * acce_scale * -1.0;
//	float accey = (float)((int16_t)(IMU_data[0] << 8 | IMU_data[1])) * acce_scale;
//	float accez = (float)((int16_t)(IMU_data[4] << 8 | IMU_data[5])) * acce_scale;
//	float Acc[3] = {accex, accey, accez};
//
//	//	float gyrox = (float)((int16_t)(IMU_data[6] << 8 | IMU_data[7])) * gyro_scale;
//	//	float gyroy = (float)((int16_t)(IMU_data[8] << 8 | IMU_data[9])) * gyro_scale;
//	// Correct hardware layout design
//	float gyrox = (float)((int16_t)(IMU_data[8] << 8 | IMU_data[9])) * gyro_scale * -1.0;
//	float gyroy = (float)((int16_t)(IMU_data[6] << 8 | IMU_data[7])) * gyro_scale;
//	float gyroz = (float)((int16_t)(IMU_data[10] << 8 | IMU_data[11])) * gyro_scale;
//	float Gyro[3] = {gyrox - Gyro_bias[index][0], gyroy - Gyro_bias[index][1], gyroz - Gyro_bias[index][2]};
//
//	for(int component = 0; component < 3; component++){
//		Est_Acc[component] = 0.0;
//		e[component] = 0.0;
//	}
//
//	if (accex != 0 || accey != 0 || accez != 0){
//		qGq(Mahony_q[index], gravity, 1);
//		vectorCross(Est_Acc, Acc);
//
//		for(int component = 0; component < 3; component++){
//			eInt[index][component] = eInt[index][component] + Ki * e[component] * delta_t;
//			Gyro[component] = Gyro[component] - Kp * e[component] - eInt[index][component];
//		}
//	}
//
//	q_prod(Mahony_q[index], Gyro);
//
//	float q_scale = 1 / SquareRootFloat(Mahony_q[index][0] * Mahony_q[index][0] + Mahony_q[index][1] * Mahony_q[index][1] + Mahony_q[index][2] * Mahony_q[index][2] + Mahony_q[index][3] * Mahony_q[index][3]);
//	for(int component = 0; component < 4; component++){
//		Mahony_q[index][component] = Mahony_q[index][component] * q_scale;
//	}
//
//	*(float_mem + 0) = Mahony_q[index][0];
//	*(float_mem + 1) = Mahony_q[index][1] * -1.0;
//	*(float_mem + 2) = Mahony_q[index][2] * -1.0;
//	*(float_mem + 3) = Mahony_q[index][3] * -1.0;
//}
