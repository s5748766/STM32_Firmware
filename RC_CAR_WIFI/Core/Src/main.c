/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body (RTOS 4-task + ST7735 + HC-SR04 + DHT11)
 ******************************************************************************
 * @attention
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	MOTOR_STOP = 0, MOTOR_FWD, MOTOR_REV, MOTOR_LEFT, MOTOR_RIGHT
} motor_cmd_t;

typedef struct {
	uint16_t left_mm;
	uint16_t right_mm;
	int8_t temp_c;   // DHT11
	int8_t humid;    // DHT11
	uint8_t dht_ok;   // 1=valid
} sensor_snapshot_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// LCD dimensions
#define LCD_WIDTH  160
#define LCD_HEIGHT 120

// Colors (RGB565)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define LCD_CS_LOW()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_RESET)
#define LCD_CS_HIGH()  HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET)
#define LCD_DC_LOW()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET)
#define LCD_DC_HIGH()  HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_SET)
#define LCD_RES_LOW()  HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_RESET)
#define LCD_RES_HIGH() HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_SET)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* RTOS: threads */
osThreadId_t control_taskHandle;
const osThreadAttr_t control_task_attributes = { .name = "control_task",
		.stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };
osThreadId_t wifi_taskHandle;
const osThreadAttr_t wifi_task_attributes = { .name = "wifi_task", .stack_size =
		256 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };
osThreadId_t sensor_taskHandle;
const osThreadAttr_t sensor_task_attributes = { .name = "sensor_task",
		.stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };
osThreadId_t ui_log_taskHandle;
const osThreadAttr_t ui_log_task_attributes = { .name = "ui_log_task",
		.stack_size = 512 * 4, .priority = (osPriority_t) osPriorityLow, };

/* USER CODE BEGIN PV */
/* 8x8 폰트 배열 (생략 없이, 위에서 제공한 font8x8 그대로 사용) */
static const uint8_t font8x8[][8] = {
/* (== 여기에 너가 올린 폰트 테이블을 그대로 둠 ==) */
#include "font8x8_full.inc" // 편집기 분량 제한을 피하려면 프로젝트에 포함.
		};
/* 만약 별도 파일이 싫다면, 너가 준 테이블을 그대로 이 자리(코멘트 지우고) 붙여넣어도 됨. */

/*** 전역 상태 ***/
static volatile motor_cmd_t g_motor_cmd = MOTOR_STOP;
static sensor_snapshot_t g_sensors = { 0 };

/*** printf 재지정 ***/
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

/*** 마이크로초 유틸 (TIM1) ***/
static inline void us_timer_reset(void) {
	__HAL_TIM_SET_COUNTER(&htim1, 0);
}
static inline uint16_t us_timer_get(void) {
	return __HAL_TIM_GET_COUNTER(&htim1);
} // 0.888us/틱(대략)

/*** DHT11 핀 모드 변경 ***/
static inline void DHT11_SetOutput(void) {
	GPIO_InitTypeDef io = { 0 };
	io.Pin = DHT11_Pin;
	io.Mode = GPIO_MODE_OUTPUT_PP;
	io.Speed = GPIO_SPEED_FREQ_LOW;
	io.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DHT11_GPIO_Port, &io);
}
static inline void DHT11_SetInput(void) {
	GPIO_InitTypeDef io = { 0 };
	io.Pin = DHT11_Pin;
	io.Mode = GPIO_MODE_INPUT;
	io.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DHT11_GPIO_Port, &io);
}

/*** 전방 선언(LCD) ***/
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_WriteData16(uint16_t data);
void LCD_Init(void);
void LCD_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_Fill(uint16_t color);
void LCD_DrawChar(uint8_t x, uint8_t y, char ch, uint16_t color,
		uint16_t bg_color);
void LCD_DrawString(uint8_t x, uint8_t y, const char *str, uint16_t color,
		uint16_t bg_color);

/*** 기능 함수 전방 선언 ***/
void motors_apply(motor_cmd_t cmd);
uint16_t hcsr04_read_mm(GPIO_TypeDef *trig_port, uint16_t trig_pin,
		GPIO_TypeDef *echo_port, uint16_t echo_pin);
uint8_t dht11_read(int8_t *temperature, int8_t *humidity);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART3_UART_Init(void);

/* RTOS task entry points */
void StartControlTask(void *argument);
void StartWifiTask(void *argument);
void StartSensorTask(void *argument);
void StartUiLogTask(void *argument);

/* LCD low-level -------------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_WriteCommand(uint8_t cmd) {
	LCD_CS_LOW();
	LCD_DC_LOW();
	HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
	LCD_CS_HIGH();
}
void LCD_WriteData(uint8_t data) {
	LCD_CS_LOW();
	LCD_DC_HIGH();
	HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
	LCD_CS_HIGH();
}
void LCD_WriteData16(uint16_t data) {
	uint8_t buffer[2];
	buffer[0] = (data >> 8) & 0xFF;
	buffer[1] = data & 0xFF;
	LCD_CS_LOW();
	LCD_DC_HIGH();
	HAL_SPI_Transmit(&hspi1, buffer, 2, HAL_MAX_DELAY);
	LCD_CS_HIGH();
}
void LCD_Init(void) {
	LCD_RES_LOW();
	HAL_Delay(100);
	LCD_RES_HIGH();
	HAL_Delay(100);
	LCD_WriteCommand(ST7735_SWRESET);
	HAL_Delay(150);
	LCD_WriteCommand(ST7735_SLPOUT);
	HAL_Delay(120);

	LCD_WriteCommand(ST7735_FRMCTR1);
	LCD_WriteData(0x01);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x2D);
	LCD_WriteCommand(ST7735_FRMCTR2);
	LCD_WriteData(0x01);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x2D);
	LCD_WriteCommand(ST7735_FRMCTR3);
	LCD_WriteData(0x01);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x2D);
	LCD_WriteData(0x01);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x2D);
	LCD_WriteCommand(ST7735_INVCTR);
	LCD_WriteData(0x07);

	LCD_WriteCommand(ST7735_PWCTR1);
	LCD_WriteData(0xA2);
	LCD_WriteData(0x02);
	LCD_WriteData(0x84);
	LCD_WriteCommand(ST7735_PWCTR2);
	LCD_WriteData(0xC5);
	LCD_WriteCommand(ST7735_PWCTR3);
	LCD_WriteData(0x0A);
	LCD_WriteData(0x00);
	LCD_WriteCommand(ST7735_PWCTR4);
	LCD_WriteData(0x8A);
	LCD_WriteData(0x2A);
	LCD_WriteCommand(ST7735_PWCTR5);
	LCD_WriteData(0x8A);
	LCD_WriteData(0xEE);
	LCD_WriteCommand(ST7735_VMCTR1);
	LCD_WriteData(0x0E);

	LCD_WriteCommand(ST7735_INVOFF);
	LCD_WriteCommand(ST7735_MADCTL);
	LCD_WriteData(0x60); // 90도 + X 미러 (네가 쓰던 값)

	LCD_WriteCommand(ST7735_COLMOD);
	LCD_WriteData(0x05);

	// Column
	LCD_WriteCommand(ST7735_CASET);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x9F); // 159
	// Row (120px)
	LCD_WriteCommand(ST7735_RASET);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x77); // 79 → 여기선 120에 맞춰 0x77로 바꿀 수 있음

	LCD_WriteCommand(ST7735_NORON);
	HAL_Delay(10);
	LCD_WriteCommand(ST7735_DISPON);
	HAL_Delay(100);
}
void LCD_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	uint8_t x_offset = 0, y_offset = 0;
	LCD_WriteCommand(ST7735_CASET);
	LCD_WriteData(0x00);
	LCD_WriteData(x0 + x_offset);
	LCD_WriteData(0x00);
	LCD_WriteData(x1 + x_offset);
	LCD_WriteCommand(ST7735_RASET);
	LCD_WriteData(0x00);
	LCD_WriteData(y0 + y_offset);
	LCD_WriteData(0x00);
	LCD_WriteData(y1 + y_offset);
	LCD_WriteCommand(ST7735_RAMWR);
}
void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
	if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
		return;
	LCD_SetWindow(x, y, x, y);
	LCD_WriteData16(color);
}
void LCD_Fill(uint16_t color) {
	LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	LCD_CS_LOW();
	LCD_DC_HIGH();
	uint8_t buf[2] = { (color >> 8) & 0xFF, color & 0xFF };
	for (uint32_t i = 0; i < (uint32_t) LCD_WIDTH * LCD_HEIGHT; i++) {
		HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
	}
	LCD_CS_HIGH();
}
void LCD_DrawChar(uint8_t x, uint8_t y, char ch, uint16_t color,
		uint16_t bg_color) {
	if (ch < 32 || ch > 126)
		ch = 32;
	const uint8_t *fc = font8x8[ch - 32];
	for (uint8_t i = 0; i < 8; i++) {
		uint8_t line = fc[i];
		for (uint8_t j = 0; j < 8; j++) {
			if (line & (0x01 << j))
				LCD_DrawPixel(x + j, y + i, color);
			else
				LCD_DrawPixel(x + j, y + i, bg_color);
		}
	}
}
void LCD_DrawString(uint8_t x, uint8_t y, const char *s, uint16_t color,
		uint16_t bg) {
	uint8_t ox = x;
	while (*s) {
		if (*s == '\n') {
			y += 8;
			x = ox;
			s++;
			continue;
		}
		if (x + 8 > LCD_WIDTH) {
			x = ox;
			y += 8;
		}
		if (y + 8 > LCD_HEIGHT)
			break;
		LCD_DrawChar(x, y, *s, color, bg);
		x += 8;
		s++;
	}
}
/* USER CODE END 0 */

/* Application entry ---------------------------------------------------------*/
int main(void) {
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_SPI1_Init();
	MX_TIM1_Init();
	MX_USART3_UART_Init();

	/* 시작 전 하드웨어 준비 */
	HAL_TIM_Base_Start(&htim1);   // us 타이머

	LCD_Init();
	LCD_Fill(BLACK);
	LCD_DrawString(4, 4, "Booting...", YELLOW, BLACK);

	/* RTOS */
	osKernelInitialize();

	control_taskHandle = osThreadNew(StartControlTask, NULL,
			&control_task_attributes);
	wifi_taskHandle = osThreadNew(StartWifiTask, NULL, &wifi_task_attributes);
	sensor_taskHandle = osThreadNew(StartSensorTask, NULL,
			&sensor_task_attributes);
	ui_log_taskHandle = osThreadNew(StartUiLogTask, NULL,
			&ui_log_task_attributes);

	osKernelStart();

	while (1) {
	}
}

/* Clock ---------------------------------------------------------------------*/
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;  // 64 MHz
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* SPI1 ----------------------------------------------------------------------*/
static void MX_SPI1_Init(void) {
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_1LINE;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; // 빠르면 8/4로 조절
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
}

/* TIM1 (us 타이머) ----------------------------------------------------------*/
static void MX_TIM1_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 64 - 1; // 64MHz/64 ≈ 1MHz (실제로 F103에서 TIM1는 x2 클럭—대략 0.888us)
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 0xFFFF;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
}

/* USART2 (printf) -----------------------------------------------------------*/
static void MX_USART2_UART_Init(void) {
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
}

/* USART3 (Wi-Fi/명령 수신) ---------------------------------------------------*/
static void MX_USART3_UART_Init(void) {
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
}

/* GPIO ----------------------------------------------------------------------*/
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOA,
			LCD_RES_Pin | LCD_DC_Pin | R_TRIG_Pin | LBF_Pin | LFB_Pin,
			GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,
			LBB_Pin | LFF_Pin | RFF_Pin | RFB_Pin | RBF_Pin | RBB_Pin,
			GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, L_TRIG_Pin | LCD_CS_Pin, GPIO_PIN_RESET);

	/* Blue Button */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/* DHT11 + R_ECHO */
	GPIO_InitStruct.Pin = DHT11_Pin | R_ECHO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* L_ECHO */
	GPIO_InitStruct.Pin = L_ECHO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(L_ECHO_GPIO_Port, &GPIO_InitStruct);

	/* LCD / TRIG / 모터 핀들 */
	GPIO_InitStruct.Pin = LCD_RES_Pin | LCD_DC_Pin | R_TRIG_Pin | LBF_Pin
			| LFB_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LBB_Pin | LFF_Pin | RFF_Pin | RFB_Pin | RBF_Pin
			| RBB_Pin;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = L_TRIG_Pin | LCD_CS_Pin;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* ------------------- 기능 구현부 ------------------------------------------*/
/* 모터 구동: TB6612/L298 등 드라이버에 맞게 수정 */
void motors_apply(motor_cmd_t cmd) {
	switch (cmd) {
	case MOTOR_FWD:
		HAL_GPIO_WritePin(LFF_GPIO_Port, LFF_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LFB_GPIO_Port, LFB_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RFF_GPIO_Port, RFF_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RFB_GPIO_Port, RFB_Pin, GPIO_PIN_RESET);
		break;
	case MOTOR_REV:
		HAL_GPIO_WritePin(LFF_GPIO_Port, LFF_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LFB_GPIO_Port, LFB_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RFF_GPIO_Port, RFF_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RFB_GPIO_Port, RFB_Pin, GPIO_PIN_SET);
		break;
	case MOTOR_LEFT:
		HAL_GPIO_WritePin(LFF_GPIO_Port, LFF_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LFB_GPIO_Port, LFB_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RFF_GPIO_Port, RFF_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RFB_GPIO_Port, RFB_Pin, GPIO_PIN_RESET);
		break;
	case MOTOR_RIGHT:
		HAL_GPIO_WritePin(LFF_GPIO_Port, LFF_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LFB_GPIO_Port, LFB_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RFF_GPIO_Port, RFF_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RFB_GPIO_Port, RFB_Pin, GPIO_PIN_SET);
		break;
	default:
	case MOTOR_STOP:
		HAL_GPIO_WritePin(LFF_GPIO_Port, LFF_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LFB_GPIO_Port, LFB_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RFF_GPIO_Port, RFF_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RFB_GPIO_Port, RFB_Pin, GPIO_PIN_RESET);
		break;
	}
}

/* 초음파 1회 측정 (폴링 버전: 간단/안정) */
uint16_t hcsr04_read_mm(GPIO_TypeDef *trig_port, uint16_t trig_pin,
		GPIO_TypeDef *echo_port, uint16_t echo_pin) {
	// TRIG: 10us High
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
	us_timer_reset();
	while (us_timer_get() < 12)
		; // ≈10us
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

	// ECHO: HIGH 시작 대기 (타임아웃 20ms)
	us_timer_reset();
	while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_RESET) {
		if (us_timer_get() > 20000)
			return 0xFFFF; // timeout
	}
	// HIGH 폭 계측
	us_timer_reset();
	while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET) {
		if (us_timer_get() > 40000)
			break; // 40ms cap (~6.8m)
	}
	uint16_t us = us_timer_get();           // ~0.888us 단위
	float usec = us * 0.888f;               // 보정 (근사)
	uint16_t dist = (uint16_t) (usec * 0.343f / 2.0f); // mm (음속 343 m/s)
	return dist;
}

/* DHT11 읽기(간단 파서, 타이밍 여유 있음) */
uint8_t dht11_read(int8_t *temperature, int8_t *humidity) {
	uint8_t data[5] = { 0 };

	// 시작 신호
	DHT11_SetOutput();
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
	HAL_Delay(18); // 18ms Low
	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
	us_timer_reset();
	while (us_timer_get() < 40)
		; // 20~40us
	DHT11_SetInput();

	// 응답: LOW 80us, HIGH 80us
	us_timer_reset();
	while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
		if (us_timer_get() > 120)
			return 0; // no response
	}
	us_timer_reset();
	while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET) {
		if (us_timer_get() > 120)
			return 0;
	}
	us_timer_reset();
	while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
		if (us_timer_get() > 120)
			return 0;
	}

	// 40비트 수신
	for (int i = 0; i < 40; i++) {
		// LOW 구간
		us_timer_reset();
		while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET) {
			if (us_timer_get() > 120)
				return 0;
		}
		// HIGH 길이 측정
		us_timer_reset();
		while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
			if (us_timer_get() > 200)
				break;
		}
		uint16_t high_us = us_timer_get(); // 26~28us=0, ~70us=1 (보정필요)
		uint8_t bit = (high_us > 60) ? 1 : 0;  // 임계 60us 근사
		data[i / 8] <<= 1;
		data[i / 8] |= bit;
	}

	uint8_t sum = data[0] + data[1] + data[2] + data[3];
	if (sum != data[4])
		return 0;

	*humidity = data[0];
	*temperature = data[2];
	return 1;
}

/* ------------------- RTOS TASKS -------------------------------------------*/
void StartControlTask(void *argument) {
	TickType_t t0 = xTaskGetTickCount();
	for (;;) {
		motors_apply(g_motor_cmd);
		vTaskDelayUntil(&t0, pdMS_TO_TICKS(10)); // 100 Hz
	}
}
void StartWifiTask(void *argument) {
	/* 간단 텍스트 명령 수신: "FWD", "REV", "LEFT", "RIGHT", "STOP" + \n */
	char rx[32];
	uint8_t idx = 0;
	char ch;
	for (;;) {
		if (HAL_UART_Receive(&huart3, (uint8_t*) &ch, 1, 10) == HAL_OK) {
			if (ch == '\r')
				continue;
			if (ch == '\n') {
				rx[idx] = 0;
				idx = 0;
				if (!strcmp(rx, "FWD"))
					g_motor_cmd = MOTOR_FWD;
				else if (!strcmp(rx, "REV"))
					g_motor_cmd = MOTOR_REV;
				else if (!strcmp(rx, "LEFT"))
					g_motor_cmd = MOTOR_LEFT;
				else if (!strcmp(rx, "RIGHT"))
					g_motor_cmd = MOTOR_RIGHT;
				else
					g_motor_cmd = MOTOR_STOP;
				printf("[CMD] %s -> %d\r\n", rx, (int) g_motor_cmd);
			} else {
				if (idx < sizeof(rx) - 1)
					rx[idx++] = ch;
			}
		} else {
			osDelay(1);
		}
	}
}
void StartSensorTask(void *argument) {
	TickType_t t_ultra = xTaskGetTickCount();
	TickType_t t_dht = xTaskGetTickCount();
	for (;;) {
		/* 50ms마다 초음파(좌/우 순차) */
		if (xTaskGetTickCount() - t_ultra >= pdMS_TO_TICKS(50)) {
			uint16_t l = hcsr04_read_mm(L_TRIG_GPIO_Port, L_TRIG_Pin,
					L_ECHO_GPIO_Port, L_ECHO_Pin);
			osDelay(30); // 크로스토크 방지
			uint16_t r = hcsr04_read_mm(R_TRIG_GPIO_Port, R_TRIG_Pin,
					R_ECHO_GPIO_Port, R_ECHO_Pin);
			if (l != 0xFFFF)
				g_sensors.left_mm = l;
			if (r != 0xFFFF)
				g_sensors.right_mm = r;
			t_ultra = xTaskGetTickCount();
		}
		/* 1초마다 DHT11 */
		if (xTaskGetTickCount() - t_dht >= pdMS_TO_TICKS(1000)) {
			int8_t t = 0, h = 0;
			uint8_t ok = dht11_read(&t, &h);
			if (ok) {
				g_sensors.temp_c = t;
				g_sensors.humid = h;
				g_sensors.dht_ok = 1;
			} else {
				g_sensors.dht_ok = 0;
			}
			t_dht = xTaskGetTickCount();
		}
		osDelay(1);
	}
}
void StartUiLogTask(void *argument) {
	char line[64];
	LCD_Fill(BLACK);
	LCD_DrawString(4, 4, "STM32F103 RTOS Demo", CYAN, BLACK);
	for (;;) {
		/* 화면 갱신 */
		snprintf(line, sizeof(line), "CMD: %d", (int) g_motor_cmd);
		LCD_DrawString(4, 20, line, YELLOW, BLACK);
		snprintf(line, sizeof(line), "UL: %4u mm", g_sensors.left_mm);
		LCD_DrawString(4, 32, line, GREEN, BLACK);
		snprintf(line, sizeof(line), "UR: %4u mm", g_sensors.right_mm);
		LCD_DrawString(4, 44, line, GREEN, BLACK);
		if (g_sensors.dht_ok) {
			snprintf(line, sizeof(line), "T:%2dC H:%2d%%", g_sensors.temp_c,
					g_sensors.humid);
			LCD_DrawString(4, 56, line, WHITE, BLACK);
		} else {
			LCD_DrawString(4, 56, "DHT: --", WHITE, BLACK);
		}
		osDelay(200);
	}
}

/* EXTI (Blue Button: 모터 STOP 토글 예시) -----------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == B1_Pin) {
		g_motor_cmd = MOTOR_STOP;
	}
}

/* Error Handler --------------------------------------------------------------*/
void Error_Handler(void) {
	__disable_irq();
	while (1) {
	}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line){ (void)file; (void)line; }
#endif
