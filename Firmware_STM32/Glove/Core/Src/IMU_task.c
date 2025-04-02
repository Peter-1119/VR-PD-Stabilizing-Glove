/* Library include */
#include "main.h"
#include "i2c.h"
#include "ICM20948_WE.h"
#include "gpio.h"
#include "IMU_task.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

/* Variable define */
#define TCA_ADDR 0x70
#define SAMPLES 64
#define DELTA_T 1 / 64
#define PI 3.14159265358979323846
//#define count 2


/* Private function prototype */
bool init();
void switchBank(uint8_t newBank);
bool initMagnetometer();
uint8_t readAllDataAndStatus(uint8_t* data);
void readAllData(uint8_t* data);
void setAccRange(ICM20948_accRange accRange);
void setAccDLPF(ICM20948_dlpf dlpf);
void setGyrDLPF(ICM20948_dlpf dlpf);
void setTempDLPF(ICM20948_dlpf dlpf);
void setMagOpMode(AK09916_opMode opMode);


/* Private variable */
int SampleRate = 50, maxFreq = 24, SampleCount = 0;  // IMU sample rate, max freq and current sample count
//int SampleRate = 25, maxFreq = 12, SampleCount = 0;  // IMU sample rate, max freq and current sample count
int freq_n;
float B = (4 / 3.1415926);
float C = (-4 / (3.1415926 * 3.1415926));
float pi = 3.1415926, Delta_t = 0.02;  // Duration between two IMU data
float Sin_signal = 0;
float Cos_signal = 0;

float a1[24], b1[24], c1[24];
float a2[24], b2[24], c2[24];
float a3[24], b3[24], c3[24];

uint8_t IMU_buf[20];
float intensity[24], *intensity_mem;
float threshold = 0.3;
bool Exceed_Threshold = false;
uint8_t Intensity_Record;

int count = 7;  // Amount of IMU
int TCA_Select_Index[7] = {0, 1, 2, 3, 4, 5, 6};  // IMU order for TCA select
//uint8_t init_status[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // IMU initial status
//uint8_t read_status[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // IMU reading status
uint8_t init_status = 0x00;
uint8_t read_status = 0x00;
uint8_t cmd = 0x01;  // TCA select command
uint8_t counter = 0;  // Data collect times
uint8_t resetCounter = 0;

float CosList[SAMPLES], SinList[SAMPLES];
float Result_R[3][SAMPLES], Result_I[3][SAMPLES];

int IndexList[SAMPLES];
int ExpIndex;
int container_temp = 0;
int P = 0;
float temp = 0;


/* Code begin */
void TCA_Select(int num){
	cmd = (0x01 << num);
	HAL_I2C_Master_Transmit(&hi2c1, (TCA_ADDR << 1), &cmd, 1, 0x00000005);
}

void IMU_All_Init(){
	for(int index = 0; index < count; index ++){
		// Select I2C Mux
		TCA_Select(TCA_Select_Index[index]);

		// Initialize IMU
		if (init() == true)
			init_status = init_status | (0x01 << index);

//		if (initMagnetometer() == true)
//			init_status = init_status & (0x01 << index);
	}
}

void IMU_All_Bank_0(){
	for(int index = 0; index < count; index ++){
		if ((init_status & (0x01 << index)) > 0){
			// Select I2C Mux
			TCA_Select(TCA_Select_Index[index]);

			// Switch IMU bank 0 to get data
			switchBank(0);
		}
	}
}

void IMU_All_Read_Data(uint8_t* buf){
	counter++;
	read_status = 0x00;
	*(buf + 0) = counter;

	for(int index = 0; index < count; index++){
		if ((init_status & (0x01 << index)) > 0){
			for(int buf_index = 0; buf_index < 20; buf_index++)
				IMU_buf[buf_index] = 0x00;

			// Select I2C Mux
			TCA_Select(TCA_Select_Index[index]);

			// Read data from IMU
			if (readAllDataAndStatus(IMU_buf))
				read_status = read_status | (0x01 << index);

			// Copy data from IMU buffer to external buffer
			memcpy((buf + 3 + index * 20), IMU_buf, 20);
		}
	}
	// Copy IMU status to external buffer
	*(buf + 1) = init_status;
	*(buf + 2) = read_status;
}

void FFT_Init(int All){
	if (All == 1){
		for(int index = 0; index < SAMPLES; index++){
			CosList[index] = cos(2 * PI * index / SAMPLES);
			SinList[index] = -sin(2 * PI * index / SAMPLES);

			IndexList[index] = index;
			IndexList[index] = ((IndexList[index] & 0b11110000) >> 4) | ((IndexList[index] & 0b00001111) << 4);
			IndexList[index] = ((IndexList[index] & 0b11001100) >> 2) | ((IndexList[index] & 0b00110011) << 2);
			IndexList[index] = ((IndexList[index] & 0b10101010) >> 1) | ((IndexList[index] & 0b01010101) << 1);
			IndexList[index] = IndexList[index] >> 2;
		}
	}
	for(int component = 0; component < SAMPLES; component++){
		for(int axis = 0; axis < 3; axis++){
			Result_R[axis][component] = 0;
			Result_I[axis][component] = 0;
		}
	}
}

void FFT(float *signal, int axis){
	int Container_Num = SAMPLES >> 2;
    for(int index = 0; index < SAMPLES; index += 2){
        Result_R[axis][index] = signal[IndexList[index]] + signal[IndexList[index + 1]];
        Result_R[axis][index + 1] = signal[IndexList[index]] - signal[IndexList[index + 1]];
    }

    for(int container = 4; container <= SAMPLES; container <<= 1){
        container_temp = container >> 1;
        for(int component = 0; component < container_temp; component++){
            ExpIndex = SAMPLES * component / container;
            for(int index = 0; index < Container_Num; index++){
                P = (index * container) + component;
                Result_R[axis][P] = Result_R[axis][P] + CosList[ExpIndex] * Result_R[axis][P + container_temp] - SinList[ExpIndex] * Result_I[axis][P + container_temp];
                Result_I[axis][P] = Result_I[axis][P] + CosList[ExpIndex] * Result_I[axis][P + container_temp] + SinList[ExpIndex] * Result_R[axis][P + container_temp];
            }
        }
        for(int component = container_temp; component < container; component++){
            ExpIndex = SAMPLES * component / container;
            for(int index = 0; index < Container_Num; index++){
                P = (index * container) + component;
                temp = Result_R[axis][P];
                Result_R[axis][P] = Result_R[axis][P - container_temp] + (CosList[ExpIndex] * Result_R[axis][P] - SinList[ExpIndex] * Result_I[axis][P]) * 2;
                Result_I[axis][P] = Result_I[axis][P - container_temp] + (CosList[ExpIndex] * Result_I[axis][P] + SinList[ExpIndex] * temp) * 2;
            }
        }
        Container_Num = Container_Num >> 1;
    }
}

// Calculate freq strength
void Frequency_Energy(uint8_t* buf, uint8_t* freq_content){
	Exceed_Threshold = false;
	intensity_mem = (float *)freq_content;  // For record into buffer to send

	for(int freqIndex = 4; freqIndex < 16; freqIndex++){
		// DFT frequency intensity
		c1[freqIndex] = Result_R[0][freqIndex] * Result_R[0][freqIndex] + Result_I[0][freqIndex] * Result_I[0][freqIndex];
		c2[freqIndex] = Result_R[1][freqIndex] * Result_R[1][freqIndex] + Result_I[1][freqIndex] * Result_I[1][freqIndex];
		c3[freqIndex] = Result_R[2][freqIndex] * Result_R[2][freqIndex] + Result_I[2][freqIndex] * Result_I[2][freqIndex];

		// Combine x, y, z-axis intensity and check whether exceed threshold
		intensity[freqIndex] = SquareRootFloat(c1[freqIndex] + c2[freqIndex] + c3[freqIndex]) * DELTA_T;
		*(intensity_mem + freqIndex - 4) = intensity[freqIndex];
//		 *(intensity_mem + freqIndex) = intensity[freqIndex];  // For record into buffer to send
		if (intensity[freqIndex] > threshold)
			Exceed_Threshold = true;
	}

	// Record if intensity exceed threshold
	Intensity_Record = Intensity_Record << 1;
	if (Exceed_Threshold == true)
		Intensity_Record = Intensity_Record + 0x01;

	// Reset sample count and record fourier transform calculate times
	SampleCount = 0;
	resetCounter += 1;

	buf[0] = resetCounter;
	buf[1] = Intensity_Record;
}


// Calculate fourier transform (a & b)
int Calculate_ab(float accex, float accey, float accez){
	for(int freqIndex = 0; freqIndex < maxFreq; freqIndex++){
		freq_n = ((freqIndex + 1) * SampleCount) % SampleRate;
		Sin_signal = FastTrigonometric(2.0F * pi * (float)freq_n / SampleRate, SinWave);
		Cos_signal = FastTrigonometric(2.0F * pi * (float)freq_n / SampleRate, CosWave);

		a1[freqIndex] += (accex * Sin_signal);
		b1[freqIndex] += (accex * Cos_signal);

		a2[freqIndex] += (accey * Sin_signal);
		b2[freqIndex] += (accey * Cos_signal);

		a3[freqIndex] += (accez * Sin_signal);
		b3[freqIndex] += (accez * Cos_signal);
	}

	// record fourier transform n times
	SampleCount++;

	if (SampleCount == SampleRate)
		return 1;
	else
		return 0;
}

// Calculate freq strength
void Calculate_Freq_Energy(uint8_t* buf){
	Exceed_Threshold = false;
	//intensity_mem = (float *)(buf + 30);  // For record into buffer to send

	for(int freqIndex = 0; freqIndex < maxFreq; freqIndex++){
		// DFT frequency intensity
		c1[freqIndex] = SquareRootFloat(a1[freqIndex] * a1[freqIndex] + b1[freqIndex] * b1[freqIndex]) * Delta_t;
		c2[freqIndex] = SquareRootFloat(a2[freqIndex] * a2[freqIndex] + b2[freqIndex] * b2[freqIndex]) * Delta_t;
		c3[freqIndex] = SquareRootFloat(a3[freqIndex] * a3[freqIndex] + b3[freqIndex] * b3[freqIndex]) * Delta_t;

		// Combine x, y, z-axis intensity and check whether exceed threshold
		intensity[freqIndex] = SquareRootFloat(c1[freqIndex] * c1[freqIndex] + c2[freqIndex] * c2[freqIndex] + c3[freqIndex] * c3[freqIndex]);
		// *(intensity_mem + freqIndex) = intensity[freqIndex];  // For record into buffer to send
		if (intensity[freqIndex] > threshold)
			Exceed_Threshold = true;

		// Reset the fourier parameters(a & b)
		a1[freqIndex] = 0.0; b1[freqIndex] = 0.0;
		a2[freqIndex] = 0.0; b2[freqIndex] = 0.0;
		a3[freqIndex] = 0.0; b3[freqIndex] = 0.0;
	}

	// Record if intensity exceed threshold
	Intensity_Record = Intensity_Record << 1;
	if (Exceed_Threshold == true)
		Intensity_Record = Intensity_Record + 0x01;

	// Reset sample count and record fourier transform calculate times
	SampleCount = 0;
	resetCounter += 1;

	buf[0] = resetCounter;
	buf[1] = Intensity_Record;
}

// Fast sin method Reference Website: https://gamedev.stackexchange.com/questions/4779/is-there-a-faster-sine-function
float FastTrigonometric(float x, trigonometric wave){
	if (wave == SinWave){
		if (x > pi) x -= (2 * pi);
		return (B * x + C * x * ((x < 0) ? -x : x));
	}
	else if(wave == CosWave){
		x += (pi / 2);
		if (x > pi) x -= (2 * pi);
		return (B * x + C * x * ((x < 0) ? -x : x));
	}
	return 0.0f;
}

// STM32-72M sqrt method spend 8us
// Detail reference: https://www.youtube.com/watch?v=g1r3iLejTw0
float SquareRootFloat(float number){
	long i;
	float x, y;
	const float f = 1.5F;

	x = number * 0.5F;
	y = number;
	i = * ( long * ) &y;
	i = 0x5f3759df - ( i >> 1 );
	y = * ( float * ) &i;
	y = y * ( f - ( x * y * y ) );
	y = y * ( f - ( x * y * y ) );
	return number * y;
}
