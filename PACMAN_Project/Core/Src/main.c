/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* LCD size */
#define LCD_WIDTH   240
#define LCD_HEIGHT  320

/* RGB565 */
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_YELLOW    0xFFE0
#define COLOR_BLUE      0x001F
#define COLOR_RED       0xF800
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_ORANGE    0xFD20

/* Button helper */
#define BTN_PRESSED(gpio, pin)   (HAL_GPIO_ReadPin((gpio),(pin)) == GPIO_PIN_SET)

/* Buzzer timers are set to PSC=63 -> 1 MHz timer tick (64 MHz / 64) */
#define TIMER_TICK_HZ   1000000UL
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
/* ILI9341 */
void ILI9341_Init(void);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t color);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/* UI */
void Pacman_StartScreen(void);
void Next_Step_Screen(void);

/* Input */
void wait_for_any_button_press(void);

/* Buzzer */
void BUZZ_Start(void);
void BUZZ_Stop(void);
void BUZZ_SetFreq_TIM1(uint32_t hz); /* PA11 TIM1_CH4 */
void BUZZ_SetFreq_TIM3(uint32_t hz); /* PB1  TIM3_CH4 */
void PLAY_PacmanIntro_Blocking(void); /* 2성부 */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ----------------------------- LCD low-level ----------------------------- */
static inline void LCD_CS_LOW(void) {
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}
static inline void LCD_CS_HIGH(void) {
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
static inline void LCD_RS_CMD(void) {
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
}
static inline void LCD_RS_DATA(void) {
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
}
static inline void LCD_WR_LOW(void) {
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
}
static inline void LCD_WR_HIGH(void) {
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
}
static inline void LCD_RD_HIGH(void) {
	HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
}
static inline void LCD_RST_LOW(void) {
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
}
static inline void LCD_RST_HIGH(void) {
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
}

/* 8-bit data bus */
static inline void LCD_SET_DATA(uint8_t d) {
	HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin,
			(d & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin,
			(d & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin,
			(d & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin,
			(d & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin,
			(d & (1 << 4)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin,
			(d & (1 << 5)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin,
			(d & (1 << 6)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin,
			(d & (1 << 7)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static inline void LCD_WRITE_STROBE(void) {
	LCD_WR_LOW();
	__NOP();
	__NOP();
	__NOP();
	LCD_WR_HIGH();
}

static void ILI9341_WriteCmd(uint8_t cmd) {
	LCD_RS_CMD();
	LCD_SET_DATA(cmd);
	LCD_WRITE_STROBE();
}
static void ILI9341_WriteData8(uint8_t d) {
	LCD_RS_DATA();
	LCD_SET_DATA(d);
	LCD_WRITE_STROBE();
}
static void ILI9341_WriteData16(uint16_t d) {
	ILI9341_WriteData8(d >> 8);
	ILI9341_WriteData8(d & 0xFF);
}

static void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1,
		uint16_t y1) {
	ILI9341_WriteCmd(0x2A);
	ILI9341_WriteData16(x0);
	ILI9341_WriteData16(x1);
	ILI9341_WriteCmd(0x2B);
	ILI9341_WriteData16(y0);
	ILI9341_WriteData16(y1);
	ILI9341_WriteCmd(0x2C);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t c) {
	if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
		return;
	ILI9341_SetWindow(x, y, x, y);
	ILI9341_WriteData16(c);
}
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t c) {
	if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
		return;
	uint16_t x1 = (x + w - 1 < LCD_WIDTH) ? (x + w - 1) : (LCD_WIDTH - 1);
	uint16_t y1 = (y + h - 1 < LCD_HEIGHT) ? (y + h - 1) : (LCD_HEIGHT - 1);
	ILI9341_SetWindow(x, y, x1, y1);
	uint32_t n = (uint32_t) (x1 - x + 1) * (uint32_t) (y1 - y + 1);
	while (n--)
		ILI9341_WriteData16(c);
}
void ILI9341_FillScreen(uint16_t c) {
	ILI9341_FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, c);
}

/* tiny 5x7 font subset */
static const uint8_t font5x7[][5] = { { 0, 0, 0, 0, 0 }, /* ' ' */
{ 0x7C, 0x12, 0x11, 0x12, 0x7C }, /* A */
{ 0x7F, 0x49, 0x49, 0x49, 0x41 }, /* E */
{ 0x7F, 0x09, 0x09, 0x09, 0x06 }, /* P */
{ 0x7F, 0x09, 0x19, 0x29, 0x46 }, /* R */
{ 0x46, 0x49, 0x49, 0x49, 0x31 }, /* S */
{ 0x01, 0x01, 0x7F, 0x01, 0x01 }, /* T */
{ 0x3E, 0x41, 0x49, 0x49, 0x3A }, /* G */
{ 0x07, 0x08, 0x70, 0x08, 0x07 }, /* Y */
{ 0x00, 0x00, 0x5F, 0x00, 0x00 }, /* ! */
};
static int fidx(char c) {
	if (c == ' ')
		return 0;
	if (c == 'A')
		return 1;
	if (c == 'E')
		return 2;
	if (c == 'P')
		return 3;
	if (c == 'R')
		return 4;
	if (c == 'S')
		return 5;
	if (c == 'T')
		return 6;
	if (c == 'G')
		return 7;
	if (c == 'Y')
		return 8;
	if (c == '!')
		return 9;
	return 0;
}
static void DrawChar5x7(int x, int y, char c, uint16_t fg, uint16_t bg, int s) {
	int i = fidx(c);
	for (int col = 0; col < 5; col++) {
		uint8_t bits = font5x7[i][col];
		for (int row = 0; row < 7; row++) {
			uint16_t color = (bits & (1 << (6 - row))) ? fg : bg;
			ILI9341_FillRect(x + col * s, y + row * s, s, s, color);
		}
	}
	ILI9341_FillRect(x + 5 * s, y, s, 7 * s, bg);
}
static void DrawText(int x, int y, const char *s, uint16_t fg, uint16_t bg,
		int scale) {
	int cx = x;
	while (*s) {
		DrawChar5x7(cx, y, *s++, fg, bg, scale);
		cx += 6 * scale;
	}
}

/* circles + pacman */
static void DrawFilledCircle(int xc, int yc, int r, uint16_t color) {
	for (int y = -r; y <= r; y++) {
		int dx = (int) (sqrtf((float) r * r - (float) y * y));
		ILI9341_FillRect(xc - dx, yc + y, 2 * dx + 1, 1, color);
	}
}
static void DrawPacman(int xc, int yc, int r, float mouth_deg, uint16_t color,
		uint16_t bg) {
	DrawFilledCircle(xc, yc, r, color);
	float a = mouth_deg * 3.1415926f / 180.f;
	int x1 = xc, y1 = yc;
	int x2 = xc + (int) (r * cosf(a / 2)), y2 = yc - (int) (r * sinf(a / 2));
	int x3 = xc + (int) (r * cosf(-a / 2)), y3 = yc - (int) (r * sinf(-a / 2));
	int miny = y1;
	if (y2 < miny)
		miny = y2;
	if (y3 < miny)
		miny = y3;
	int maxy = y1;
	if (y2 > maxy)
		maxy = y2;
	if (y3 > maxy)
		maxy = y3;
	for (int y = miny; y <= maxy; y++) {
		int xs[3], n = 0;
#define EDGE(xa,ya,xb,yb) if(!((ya<y&&yb<y)||(ya>y&&yb>y)||(ya==yb))){ float t=(ya==yb)?0.f:((float)(y-ya)/(float)(yb-ya)); xs[n++]=xa+(int)((xb-xa)*t); }
		EDGE(x1, y1, x2, y2);
		EDGE(x2, y2, x3, y3);
		EDGE(x3, y3, x1, y1);
#undef EDGE
		if (n >= 2) {
			if (xs[0] > xs[1]) {
				int t = xs[0];
				xs[0] = xs[1];
				xs[1] = t;
			}
			ILI9341_FillRect(xs[0], y, xs[1] - xs[0] + 1, 1, bg);
		}
	}
	ILI9341_FillRect(xc + r / 4, yc - r / 3, r / 6, r / 6, COLOR_WHITE);
}

static void DrawGhost(int x, int y, int w, int h, uint16_t body, uint16_t eye) {

	/* 몸통 */
	ILI9341_FillRect(x, y + h / 4, w, 3 * h / 4, body);
	/* 머리 반원 */
	int r = w / 2;
	for (int yy = 0; yy < h / 2; yy++) {
		int dx = (int) sqrtf((float) r * r - (float) (r - yy) * (r - yy));
		ILI9341_FillRect(x + r - dx, y + yy, 2 * dx, 1, body);
	}
	/* 밑단 톱니 */
	int tooth = w / 5;
	for (int i = 0; i < 5; i++) {
		if (i % 2 == 0)
			ILI9341_FillRect(x + i * tooth, y + h - (h / 8), tooth, h / 8,
			COLOR_BLACK);
	}
	/* 눈 */
	int ex = x + w / 4, ey = y + h / 3;
	ILI9341_FillRect(ex, ey, w / 6, h / 6, COLOR_WHITE);
	ILI9341_FillRect(ex + w / 2, ey, w / 6, h / 6, COLOR_WHITE);
	ILI9341_FillRect(ex + w / 12, ey + h / 12, w / 12, h / 12, eye);
	ILI9341_FillRect(ex + w / 2 + w / 12, ey + h / 12, w / 12, h / 12, eye);
}

/* screens */
void Pacman_StartScreen(void) {
	/* 배경: 검정 */
	ILI9341_FillScreen(COLOR_BLACK);

	/* 미로 프레임 몇 줄만 포인트로 (전체 미로는 무거우니 라인 샘플) */
	ILI9341_FillRect(10, 40, 220, 4, COLOR_BLUE);
	ILI9341_FillRect(10, 80, 220, 4, COLOR_BLUE);
	ILI9341_FillRect(10, 120, 220, 4, COLOR_BLUE);
	ILI9341_FillRect(10, 160, 220, 4, COLOR_BLUE);

	/* PAC-MAN 로고풍 텍스트 (노란색, 3배 확대) */
	DrawText(20, 8, "PAPRESS", COLOR_YELLOW, COLOR_BLACK, 3); // 'PACMAN'을 폰트 제한 때문에 'PA'만 노랑+유령으로 대체
	/* 'C' 대신 팩맨 아이콘으로 로고 느낌 내기 */
	DrawPacman(20 + 6 * 3 * 2 + 18, 20, 12, 60.f, COLOR_YELLOW,
	COLOR_BLACK); // 글자 옆에 팩맨

	/* 유령 4종 (핑크/빨강/하양(눈만)/파랑) */
	int gx = 30, gy = 190, gw = 36, gh = 28, gap = 8;
	DrawGhost(gx + 0 * (gw + gap), gy, gw, gh, COLOR_MAGENTA, COLOR_BLUE); // Pinky
	DrawGhost(gx + 1 * (gw + gap), gy, gw, gh, COLOR_RED, COLOR_BLUE); // Blinky
	DrawGhost(gx + 2 * (gw + gap), gy, gw, gh, COLOR_CYAN, COLOR_BLUE); // Inky (몸은 시안)
	DrawGhost(gx + 3 * (gw + gap), gy, gw, gh, COLOR_ORANGE, COLOR_BLUE); // Clyde

	/* PRESS START */
	DrawText(30, 260, "PRESS START", COLOR_WHITE, COLOR_BLACK, 2);

	/* 데모 도트(점) 몇 개 */
	for (int x = 16; x < 224; x += 12)
		ILI9341_FillRect(x, 100, 3, 3, COLOR_WHITE);
}
void Next_Step_Screen(void) {
	ILI9341_FillScreen(COLOR_BLACK);
	DrawText(32, 80, "GET READY!", COLOR_YELLOW, COLOR_BLACK, 3);
	DrawText(32, 120, "STAGE 1", COLOR_CYAN, COLOR_BLACK, 3);
	DrawPacman(60, 200, 18, 40.f, COLOR_YELLOW, COLOR_BLACK);
	ILI9341_FillRect(10, 240, 220, 4, COLOR_BLUE);
}

/* 50ms debounce for any of UP/LEFT/RIGHT/DOWN */
static int any_button_now(void) {
	return BTN_PRESSED(UP_GPIO_Port, UP_Pin)
			|| BTN_PRESSED(LEFT_GPIO_Port, LEFT_Pin)
			|| BTN_PRESSED(RIGHT_GPIO_Port, RIGHT_Pin)
			|| BTN_PRESSED(DOWN_GPIO_Port, DOWN_Pin);
}
void wait_for_any_button_press(void) {
	while (!any_button_now()) {
		HAL_Delay(1);
	}
	HAL_Delay(50);
	if (!any_button_now())
		return;
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	while (any_button_now()) {
		HAL_Delay(1);
	}
	HAL_Delay(20);
}

/* ----------------------------- ILI9341 init ------------------------------ */
void ILI9341_Init(void) {
	LCD_CS_HIGH();
	LCD_WR_HIGH();
	LCD_RD_HIGH();
	LCD_RST_LOW();
	HAL_Delay(20);
	LCD_RST_HIGH();
	HAL_Delay(120);

	LCD_CS_LOW();

	ILI9341_WriteCmd(0x01);
	HAL_Delay(5);
	ILI9341_WriteCmd(0x28);

	ILI9341_WriteCmd(0xCF);
	ILI9341_WriteData8(0x00);
	ILI9341_WriteData8(0x83);
	ILI9341_WriteData8(0x30);
	ILI9341_WriteCmd(0xED);
	ILI9341_WriteData8(0x64);
	ILI9341_WriteData8(0x03);
	ILI9341_WriteData8(0x12);
	ILI9341_WriteData8(0x81);
	ILI9341_WriteCmd(0xE8);
	ILI9341_WriteData8(0x85);
	ILI9341_WriteData8(0x01);
	ILI9341_WriteData8(0x79);
	ILI9341_WriteCmd(0xCB);
	ILI9341_WriteData8(0x39);
	ILI9341_WriteData8(0x2C);
	ILI9341_WriteData8(0x00);
	ILI9341_WriteData8(0x34);
	ILI9341_WriteData8(0x02);
	ILI9341_WriteCmd(0xF7);
	ILI9341_WriteData8(0x20);
	ILI9341_WriteCmd(0xEA);
	ILI9341_WriteData8(0x00);
	ILI9341_WriteData8(0x00);

	ILI9341_WriteCmd(0xC0);
	ILI9341_WriteData8(0x26);
	ILI9341_WriteCmd(0xC1);
	ILI9341_WriteData8(0x11);
	ILI9341_WriteCmd(0xC5);
	ILI9341_WriteData8(0x35);
	ILI9341_WriteData8(0x3E);
	ILI9341_WriteCmd(0xC7);
	ILI9341_WriteData8(0xBE);

	ILI9341_WriteCmd(0x36);
	ILI9341_WriteData8(0x48); /* MY=1, BGR=1 */
	ILI9341_WriteCmd(0x3A);
	ILI9341_WriteData8(0x55); /* RGB565 */

	ILI9341_WriteCmd(0xB1);
	ILI9341_WriteData8(0x00);
	ILI9341_WriteData8(0x1B);
	ILI9341_WriteCmd(0xB6);
	ILI9341_WriteData8(0x0A);
	ILI9341_WriteData8(0xA2);

	ILI9341_WriteCmd(0xF2);
	ILI9341_WriteData8(0x00);
	ILI9341_WriteCmd(0x26);
	ILI9341_WriteData8(0x01);

	ILI9341_WriteCmd(0x11);
	HAL_Delay(120);
	ILI9341_WriteCmd(0x29);
}

/* ----------------------------- Buzzer (PWM) ------------------------------ */
/* duty 50% square: ARR = TIMER_TICK_HZ/freq - 1 , CCR = (ARR+1)/2 */
static void set_pwm_freq(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t hz) {
	if (hz == 0) {
		__HAL_TIM_SET_COMPARE(htim, channel, 0); /* mute */
		return;
	}
	uint32_t arr = (TIMER_TICK_HZ / hz);
	if (arr < 2)
		arr = 2;
	arr -= 1;
	__HAL_TIM_SET_AUTORELOAD(htim, arr);
	__HAL_TIM_SET_COMPARE(htim, channel, (arr + 1) / 2);
	__HAL_TIM_SET_COUNTER(htim, 0);
}

void BUZZ_Start(void) {
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); /* PA11 */
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4); /* PB1  */
}
void BUZZ_Stop(void) {
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
}
void BUZZ_SetFreq_TIM1(uint32_t hz) {
	set_pwm_freq(&htim1, TIM_CHANNEL_4, hz);
}
void BUZZ_SetFreq_TIM3(uint32_t hz) {
	set_pwm_freq(&htim3, TIM_CHANNEL_4, hz);
}

/* Pac-Man intro (간략 2성부 편곡) — 템포 140bpm 기준 근사 */
#define QN(ms)  (ms)          /* quarter  = 1beat */
#define EN(ms)  ((ms)/2)      /* eighth   = 1/2   */
#define SN(ms)  ((ms)/4)      /* sixteenth= 1/4   */

static const uint16_t TEMPO_MS = 430; /* quarter ≈430ms (@~140bpm) */

/* 음계 주파수(Hz) – 4/5/6옥타브 일부 */
enum {
	/* 3옥타브(추가) */
	C3 = 131,
	Cs3 = 139,
	D3 = 147,
	Ds3 = 156,
	E3 = 165,
	F3 = 175,
	Fs3 = 185,
	G3 = 196,
	Gs3 = 208,
	A3 = 220,
	As3 = 233,
	B3 = 247,

	/* 4옥타브 */
	C4 = 262,
	Cs4 = 277,
	D4 = 294,
	Ds4 = 311,
	E4 = 330,
	F4 = 349,
	Fs4 = 370,
	G4 = 392,
	Gs4 = 415,
	A4 = 440,
	As4 = 466,
	B4 = 494,

	/* 5옥타브 */
	C5 = 523,
	Cs5 = 554,
	D5 = 587,
	Ds5 = 622,
	E5 = 659,
	F5 = 698,
	Fs5 = 740,
	G5 = 784,
	Gs5 = 831,
	A5 = 880,
	As5 = 932,
	B5 = 988,

	/* 6옥타브 */
	C6 = 1047,
	Cs6 = 1109,
	D6 = 1175,
	Ds6 = 1245,
	E6 = 1319,
	F6 = 1397,
	Fs6 = 1480,
	G6 = 1568,
	Gs6 = 1661,
	A6 = 1760,
	As6 = 1865,
	B6 = 1976
};

/* 멜로디/하모니 (클래식 팩맨 인트로의 느낌을 살린 근사치) */
static const uint16_t voice1_freq[] = { B5, B6, B5, G5, E5, C5, D5, G4, 0, E5,
		C5, D5, G4 };
static const uint16_t voice2_freq[] = { G4, D5, G4, E4, C4, G4, B4, D4, G3, C4,
		G4, B4, D4 };
static const uint16_t voice_dur[] = { EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS),
		EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), QN(TEMPO_MS),
		EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), QN(TEMPO_MS) };
static const size_t VOICE_LEN = (sizeof(voice1_freq) / sizeof(voice1_freq[0]));

void PLAY_PacmanIntro_Blocking(void) {
	BUZZ_Start();
	for (size_t i = 0; i < VOICE_LEN; ++i) {
		BUZZ_SetFreq_TIM1(voice1_freq[i]); /* 멜로디 */
		BUZZ_SetFreq_TIM3(voice2_freq[i]); /* 하모니 */
		HAL_Delay(voice_dur[i] - 10);
		/* 짧은 스택카토 */
		BUZZ_SetFreq_TIM1(0);
		BUZZ_SetFreq_TIM3(0);
		HAL_Delay(10);
	}
	BUZZ_Stop();
}
/* USER CODE END 0 */

/* ------------------------------- main ------------------------------------ */
int main(void) {
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_TIM1_Init();
	MX_TIM3_Init();

	/* LCD + Start screen */
	ILI9341_Init();
	Pacman_StartScreen();

	/* 재생: 시작 화면이 뜨자마자 2성부 인트로 */
	PLAY_PacmanIntro_Blocking();

	while (1) {
		/* 아무 방향 버튼을 누르면 다음 화면 */
		wait_for_any_button_press();
		Next_Step_Screen();

		/* 다시 아무 버튼 → 스타트로 복귀 */
		wait_for_any_button_press();
		Pacman_StartScreen();
		PLAY_PacmanIntro_Blocking();
	}
}

/* ------------------------- CubeMX init blocks ---------------------------- */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		Error_Handler();
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; /* TIM3 x2 -> 64MHz */
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1; /* TIM1 = 64MHz     */
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
		Error_Handler();
}

static void MX_TIM1_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreak = { 0 };

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 64 - 1; /* 64MHz/64 = 1MHz tick */
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1000 - 1; /* dummy (will change per note) */
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
		Error_Handler();
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
		Error_Handler();
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
		Error_Handler();
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
		Error_Handler();
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500; /* 50% (will be updated) */
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
		Error_Handler();
	sBreak.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreak.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreak.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreak.DeadTime = 0;
	sBreak.BreakState = TIM_BREAK_DISABLE;
	sBreak.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreak.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreak) != HAL_OK)
		Error_Handler();
	HAL_TIM_MspPostInit(&htim1);
}

static void MX_TIM3_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 64 - 1; /* 1MHz tick */
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 1000 - 1; /* dummy */
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
		Error_Handler();
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
		Error_Handler();
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
		Error_Handler();
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
		Error_Handler();
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
		Error_Handler();
	HAL_TIM_MspPostInit(&htim3);
}

static void MX_USART2_UART_Init(void) {
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
		Error_Handler();
}

static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin | LCD_D1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,
			LCD_RD_Pin | LCD_WR_Pin | LCD_RS_Pin | LD2_Pin | LCD_D7_Pin
					| LCD_D0_Pin | LCD_D2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,
	LCD_CS_Pin | LCD_D6_Pin | LCD_D3_Pin | LCD_D5_Pin | LCD_D4_Pin,
			GPIO_PIN_RESET);

	/* B1 for EXTI (optional) */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/* LCD outputs */
	GPIO_InitStruct.Pin = LCD_RST_Pin | LCD_D1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LCD_RD_Pin | LCD_WR_Pin | LCD_RS_Pin | LD2_Pin
			| LCD_D7_Pin | LCD_D0_Pin | LCD_D2_Pin;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LCD_CS_Pin | LCD_D6_Pin | LCD_D3_Pin | LCD_D5_Pin
			| LCD_D4_Pin;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Direction buttons: PC2/PC3, PB15/PB13 (NOPULL) */
	GPIO_InitStruct.Pin = UP_Pin | LEFT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RIGHT_Pin | DOWN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI prio (B1) */
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* Error handler */
void Error_Handler(void) {
	__disable_irq();
	while (1) {
	}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line){ (void)file; (void)line; }
#endif
