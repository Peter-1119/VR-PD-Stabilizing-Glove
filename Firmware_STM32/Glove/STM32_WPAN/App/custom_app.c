/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_app.c
  * @author  MCD Application Team
  * @brief   Custom Example Application (Server)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "custom_app.h"
#include "custom_stm.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "IMU_task.h"
#include "ICM20948_WE.h"
#include "Mahony.h"
#include "usbd_cdc_if.h"
#include "gpio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  /* IMU_Data */
  uint8_t               Imu_Notification_Status;
  /* USER CODE BEGIN CUSTOM_APP_Context_t */

  uint8_t Update_IMU_Task_Timer_Id;

  /* USER CODE END CUSTOM_APP_Context_t */

  uint16_t              ConnectionHandle;
} Custom_App_Context_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define Update_IMU_Task_TIMER_PERIOD (20000 / CFG_TS_TICK_VAL)  // 20ms
#define SAMPLERATE 64  // for FFT sample rate (not real IMU sample rate)

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[247];
uint8_t NotifyCharData[247];

/* USER CODE BEGIN PV */
uint8_t IMU_Status[143];  // Get IMU raw data
uint8_t IMU_q[112];  // Estimate IMU orientation
uint8_t PWM_Status[15];
uint8_t Freq_Intensity[256];

int *PWM_Status_mem;
int Clocks_Raise_Per_Value = 1000000 / (5 * 20000);  // (1s / (Raise_Speed * clock_count_time))
int Clocks_Count = 0;  // Counter for PWM raise times per value
int PWM_KEEP_TIME = 300 * 1000000 / 20000;  // (300s * 1,000,000us) / 20,000us
int PWM_KEEP_TIME_COUNT = 0;   // Counter for PWM keep time
int PWM_Raise_Status = false;  // Current PWM Open status
int FFT_Counter = 0;

uint8_t Button_Cur_State = true;
uint8_t Button_Pre_State = true;
uint8_t Change_State_Force = false;

int PWM_MAX_ACTIVE_TIM = 1030;  // PWM max duty cycle
int PWM_MIN_ACTIVE_TIM = 980;   // PWM min duty cycle

int PWM_CC1_ACTIVE_TIM = 980;     // Current PWM 1 duty cycle
int PWM_CC2_ACTIVE_TIM = 980;     // Current PWM 2 duty cycle

float accex, accey, accez, Past_Acc[3][64];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* IMU_Data */
static void Custom_Imu_Update_Char(void);
static void Custom_Imu_Send_Notification(void);

/* USER CODE BEGIN PFP */

void Button_Press_Detect();
static void IMU_Update_Timer_Callback();
static void IMU_task();
void delta_t_m();

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_1 */

  /* USER CODE END CUSTOM_STM_App_Notification_1 */
  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* USER CODE END CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* IMU_Data */
    case CUSTOM_STM_IMU_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_IMU_READ_EVT */

      /* USER CODE END CUSTOM_STM_IMU_READ_EVT */
      break;

    case CUSTOM_STM_IMU_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_IMU_NOTIFY_ENABLED_EVT */

      /* USER CODE END CUSTOM_STM_IMU_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_IMU_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_IMU_NOTIFY_DISABLED_EVT */

      /* USER CODE END CUSTOM_STM_IMU_NOTIFY_DISABLED_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_STM_App_Notification_default */

      /* USER CODE END CUSTOM_STM_App_Notification_default */
      break;
  }
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_2 */

  /* USER CODE END CUSTOM_STM_App_Notification_2 */
  return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_APP_Notification_1 */

  /* USER CODE END CUSTOM_APP_Notification_1 */

  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

    /* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
    case CUSTOM_CONN_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */

    	HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);

      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */

    	HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);

      /* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_APP_Notification_default */

      /* USER CODE END CUSTOM_APP_Notification_default */
      break;
  }

  /* USER CODE BEGIN CUSTOM_APP_Notification_2 */

  /* USER CODE END CUSTOM_APP_Notification_2 */

  return;
}

void Custom_APP_Init(void)
{
  /* USER CODE BEGIN CUSTOM_APP_Init */
	// Add pivot
	for(int index = 0; index < 4; index++)
		UpdateCharData[index] = 0xff - (uint8_t)index;

	// Initial mahony q, IMU setting
	q_init();
	IMU_All_Init();
	IMU_All_Bank_0();
	FFT_Init(1);

	// Record gyro drift
	for(int count = 0; count < 50; count++){
		IMU_All_Read_Data(IMU_Status);
		Record_Gyro_bias(IMU_Status + 3);
		HAL_Delay(20);
	}
	statistic_gyro_bias();

	UTIL_SEQ_RegTask(1 << CFG_IMU_TASK_UPDATE, UTIL_SEQ_RFU, IMU_task);

	HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(Custom_App_Context.Update_IMU_Task_Timer_Id),
				hw_ts_Repeated, IMU_Update_Timer_Callback);

	HW_TS_Start(Custom_App_Context.Update_IMU_Task_Timer_Id, Update_IMU_Task_TIMER_PERIOD);
  /* USER CODE END CUSTOM_APP_Init */
  return;
}

/* USER CODE BEGIN FD */

void Button_Press_Detect(){
	Button_Cur_State = HAL_GPIO_ReadPin(Btn_GPIO_Port, Btn_Pin);

	if ((Button_Cur_State != Button_Pre_State) && (Button_Cur_State == 1)){
		Change_State_Force = 1;
	}

	Button_Pre_State = Button_Cur_State;
}

static void IMU_Update_Timer_Callback(){
	// Initialize IMU status buffer and send buffer
	for (int index = 0; index < 140; index++) IMU_Status[index] = 0;
	for (int index = 0; index < 112; index ++) IMU_q[index] = 0;

	// Get all peripheral IMU data
	IMU_All_Read_Data(IMU_Status);

	for(int index = 0; index < 7; index++)
		mahony((IMU_Status + 3 + 20 * index), index, (float *)(IMU_q + 16 * index));

	// Get glove board IMU data
	accex = (int16_t)((IMU_Status[3] << 8) | IMU_Status[4]) / 16384.0;
	accey = (int16_t)((IMU_Status[5] << 8) | IMU_Status[6]) / 16384.0;
	accez = (int16_t)((IMU_Status[7] << 8) | IMU_Status[8]) / 16384.0;

	FFT_Counter++;
	Past_Acc[0][50 - FFT_Counter] = accex;
	Past_Acc[1][50 - FFT_Counter] = accey;
	Past_Acc[2][50 - FFT_Counter] = accez;

	// Calculate frequency energy
	if (FFT_Counter == 50){
		FFT_Init(0);
		FFT(Past_Acc[0], 0);
		FFT(Past_Acc[1], 1);
		FFT(Past_Acc[2], 2);
		Frequency_Energy(PWM_Status, Freq_Intensity);
		FFT_Counter = 0;

		for(int index = 0; index < SAMPLERATE - 50; index++){
			Past_Acc[0][index + 50] = Past_Acc[0][index];
			Past_Acc[1][index + 50] = Past_Acc[1][index];
			Past_Acc[2][index + 50] = Past_Acc[2][index];
		}
	}
//	if (Calculate_ab(accex, accey, accez) == 1)
//		Calculate_Freq_Energy(PWM_Status);

	// Control PWM open
	if (((PWM_Status[1] & 0x07) == 0x07) || (Change_State_Force == 1 && PWM_Raise_Status == false)){
		PWM_Raise_Status = true;
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	}
	else if ((PWM_KEEP_TIME_COUNT >= PWM_KEEP_TIME) || (Change_State_Force == 1 && PWM_Raise_Status == true)){
		PWM_Raise_Status = false;
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	}

	// Count PWM raise times
	Clocks_Count += 1;

	// Count PWM keep time
	if (PWM_Raise_Status == true && ((PWM_Status[1] & 7) != 7))
		PWM_KEEP_TIME_COUNT += 1;
	else
		PWM_KEEP_TIME_COUNT = 0;

	// Gradually change PWM duty cycle
	if (Clocks_Count == Clocks_Raise_Per_Value){
		if ((PWM_Raise_Status == true) && (PWM_CC1_ACTIVE_TIM < PWM_MAX_ACTIVE_TIM))
			PWM_CC1_ACTIVE_TIM += 1;
		else if ((PWM_Raise_Status == false) && (PWM_CC1_ACTIVE_TIM > PWM_MIN_ACTIVE_TIM))
			PWM_CC1_ACTIVE_TIM -= 1;

		if ((PWM_Raise_Status == true) && (PWM_CC2_ACTIVE_TIM < PWM_MAX_ACTIVE_TIM))
			PWM_CC2_ACTIVE_TIM += 1;
		else if ((PWM_Raise_Status == false) && (PWM_CC2_ACTIVE_TIM > PWM_MIN_ACTIVE_TIM))
			PWM_CC2_ACTIVE_TIM -= 1;

		Clocks_Count = 0;
	}

	// Test Code
	*(PWM_Status + 2) = PWM_Raise_Status;;
	PWM_Status_mem = (int *)(PWM_Status + 3);
	*(PWM_Status_mem + 0) = PWM_CC1_ACTIVE_TIM;  // 980 ~ 1300
	*(PWM_Status_mem + 1) = PWM_CC2_ACTIVE_TIM;  // 980 ~ 1300
	*(PWM_Status_mem + 2) = PWM_KEEP_TIME_COUNT;  // PWM_Keep_Time_Count

	TIM1->CCR1 = PWM_CC1_ACTIVE_TIM;
	TIM1->CCR2 = PWM_CC2_ACTIVE_TIM;
	Change_State_Force = 0;

	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	UTIL_SEQ_SetTask(1 << CFG_IMU_TASK_UPDATE, CFG_SCH_PRIO_0);
}

static void IMU_task(){
	memcpy((UpdateCharData + 4), IMU_Status, 15);

//	memcpy((UpdateCharData + 7), (IMU_Status + 3), 6);
//	memcpy((UpdateCharData + 13), (IMU_Status + 23), 6);

//	memcpy((UpdateCharData + 7), (IMU_Status + 9), 6);
//	memcpy((UpdateCharData + 13), (IMU_Status + 109), 6);

//	memcpy((UpdateCharData + 4), IMU_Status, 15);
//	memcpy((UpdateCharData + 4), IMU_Status, 23);  // Read include magnetometer data
	memcpy((UpdateCharData + 23), (IMU_q + 16), 96);
	memcpy((UpdateCharData + 119), PWM_Status, 15);
	Custom_Imu_Update_Char();
	CDC_Transmit_FS(UpdateCharData, sizeof(uint8_t) * 140);
}

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* IMU_Data */
void Custom_Imu_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Imu_UC_1*/

  updateflag = 1;

  /* USER CODE END Imu_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_IMU, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Imu_UC_Last*/

  /* USER CODE END Imu_UC_Last*/
  return;
}

void Custom_Imu_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Imu_NS_1*/

  /* USER CODE END Imu_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_IMU, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Imu_NS_Last*/

  /* USER CODE END Imu_NS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/* USER CODE END FD_LOCAL_FUNCTIONS*/
