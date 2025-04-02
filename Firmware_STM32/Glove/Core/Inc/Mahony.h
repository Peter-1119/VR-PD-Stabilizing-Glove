/*
 * Mahony.h
 *
 *  Created on: Apr 24, 2024
 *      Author: Peter
 */

#ifndef INC_MAHONY_H_
#define INC_MAHONY_H_

void q_init();
void Record_Gyro_bias(uint8_t *IMU_data);
void statistic_gyro_bias();
void qpq(float *q, float *p, int inverse);
void qGq(float *q, float *p, int inverse);
void q_prod(float *q, float *p);
void vectorCross(float *p1, float *p2);
void mahony(uint8_t* IMU_data, int index, float *float_mem);

#endif /* INC_MAHONY_H_ */
