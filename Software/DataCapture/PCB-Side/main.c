/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lsm6dsox_reg.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include "u8g2.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */
#define LSM6DSOX_I2C_ADDR (0x6A << 1)
#define DISPLAY_ADDR (0x3C << 1)

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t i2c_tx_buf[256];
static uint8_t i2c_tx_buf_idx = 0;
uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	uint8_t *data;
	    switch(msg)
	    {
	        case U8X8_MSG_BYTE_INIT:
	            break; // HAL I2C already initialized in MX_I2C2_Init

	        case U8X8_MSG_BYTE_START_TRANSFER:
	            i2c_tx_buf_idx = 0;
	            break;

	        case U8X8_MSG_BYTE_SEND:
	            data = (uint8_t *)arg_ptr;
	            while(arg_int > 0) {
	                i2c_tx_buf[i2c_tx_buf_idx++] = *data++;
	                arg_int--;
	            }
	            break;

	        case U8X8_MSG_BYTE_END_TRANSFER:
	        	HAL_I2C_Master_Transmit(
	                &hi2c2,
	                DISPLAY_ADDR,
	                i2c_tx_buf,
	                i2c_tx_buf_idx,
	                1000    
				);
	            break;

	        default:
	            return 0;
	    }
	    return 1;
}

uint8_t u8x8_delay_and_gpio(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
switch(msg)
    {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break; 

        case U8X8_MSG_GPIO_RESET:
            break; 

        case U8X8_MSG_DELAY_MILLI:
            HAL_Delay(arg_int);
            break;

        case U8X8_MSG_DELAY_10MICRO:
            // 72MHz → ~720 cycles per 10µs; loop overhead ~5 cyc/iter → 144 iters
            for(uint8_t d = 0; d < arg_int; d++)
                for(volatile uint32_t i = 0; i < 144; i++) __NOP();
            break;

        case U8X8_MSG_DELAY_100NANO:
            for(uint8_t d = 0; d < arg_int; d++) {
                __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            }
            break;

        default:
            return 0;
    }
    return 1;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  stmdev_ctx_t dev_ctx;

  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hi2c1;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t whoami;
  lsm6dsox_device_id_get(&dev_ctx, &whoami);
  lsm6dsox_reset_set(&dev_ctx, PROPERTY_ENABLE);

  uint8_t rst;
  do {
      lsm6dsox_reset_get(&dev_ctx, &rst);
  } while (rst);

  lsm6dsox_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  lsm6dsox_xl_data_rate_set(
      &dev_ctx,
      LSM6DSOX_XL_ODR_104Hz);

  lsm6dsox_xl_full_scale_set(
      &dev_ctx,
      LSM6DSOX_2g);

  lsm6dsox_gy_data_rate_set(
      &dev_ctx,
      LSM6DSOX_GY_ODR_104Hz);

  lsm6dsox_gy_full_scale_set(
      &dev_ctx,
      LSM6DSOX_250dps);



  u8g2_t u8g2; 
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_stm32_hw_i2c, u8x8_delay_and_gpio);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
  u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);   
  u8g2_ClearBuffer(&u8g2);

  HAL_Delay(500);

  volatile int16_t gyro_raw[3];
  volatile int16_t accel_raw[3];

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  //Check if ready to receive data
      uint8_t xl_ready;
      uint8_t gy_ready;
      lsm6dsox_xl_flag_data_ready_get(&dev_ctx, &xl_ready);
      lsm6dsox_gy_flag_data_ready_get(&dev_ctx, &gy_ready);

      if (xl_ready && gy_ready)
      {
    	  //Poll values, pass into USB
          lsm6dsox_acceleration_raw_get(&dev_ctx, (int16_t *)accel_raw);
          lsm6dsox_angular_rate_raw_get(&dev_ctx, (int16_t *)gyro_raw);

          int32_t ax = ((int32_t)accel_raw[0] * 61) / 1000;
          int32_t ay = ((int32_t)accel_raw[1] * 61) / 1000;
          int32_t az = ((int32_t)accel_raw[2] * 61) / 1000;

          char line[24];
          u8g2_ClearBuffer(&u8g2);

          snprintf(line, sizeof(line), "AX: %4d mg", (int)ax);
          u8g2_DrawStr(&u8g2, 0, 12, line);

          snprintf(line, sizeof(line), "AY: %4d mg", (int)ay);
          u8g2_DrawStr(&u8g2, 0, 26, line);

          snprintf(line, sizeof(line), "AZ: %4d mg", (int)az);
          u8g2_DrawStr(&u8g2, 0, 40, line);

          u8g2_SendBuffer(&u8g2);
          char tx_buf[64];
          int len = snprintf(tx_buf, sizeof(tx_buf),
                    "%d,%d,%d,%d,%d,%d\r\n",
                    (int)accel_raw[0], (int)accel_raw[1], (int)accel_raw[2],
                    (int)gyro_raw[0],  (int)gyro_raw[1],  (int)gyro_raw[2]);

          CDC_Transmit_FS((uint8_t *)tx_buf, (uint16_t)len);



      }

      HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int32_t platform_write(
    void *handle,
    uint8_t reg,
    const uint8_t *bufp,
    uint16_t len)
{
    HAL_I2C_Mem_Write(
        (I2C_HandleTypeDef*)handle,
        LSM6DSOX_I2C_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t*)bufp,
        len,
        1000);

    return 0;
}

int32_t platform_read(
    void *handle,
    uint8_t reg,
    uint8_t *bufp,
    uint16_t len)
{
    HAL_I2C_Mem_Read(
        (I2C_HandleTypeDef*)handle,
        LSM6DSOX_I2C_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        bufp,
        len,
        1000);

    return 0;
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
