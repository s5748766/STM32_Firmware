/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body (Pac-Man Start Screen)
 ******************************************************************************
 * Note:
 * - 모든 사용자 코드는 USER CODE 블록 안에만 위치함.
 * - CubeMX 재생성 시에도 본 파일의 사용자 코드는 보존됨.
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_WIDTH   240
#define LCD_HEIGHT  320

/* RGB565 */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_YELLOW  0xFFE0
#define COLOR_BLUE    0x001F
#define COLOR_GRAY    0xC618
#define COLOR_RED     0xF800
#define COLOR_PINK    0xF81F
#define COLOR_CYAN    0x07FF
#define COLOR_ORANGE  0xFD20
#define COLOR_BROWN   0xA145

#define BTN_PRESSED(gpio, pin) (HAL_GPIO_ReadPin((gpio),(pin)) == GPIO_PIN_SET)

/* Buzzer timers are set to PSC=63 -> 1 MHz timer tick (64 MHz / 64) */
#define TIMER_TICK_HZ   1000000UL
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

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
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
/* -------- ILI9341 + Drawing (모두 USER 영역 전용) -------- */
static void ILI9341_Init(void);
static void ILI9341_SetRotation(uint8_t rot);
static void ILI9341_FillScreen(uint16_t color);
static void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t color);
static void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/* primitives */
static void DrawLineH(int x, int y, int w, int t, uint16_t c);
static void DrawLineV(int x, int y, int h, int t, uint16_t c);
static void DrawRoundRect(int x, int y, int w, int h, int r, int t, uint16_t c);
static void DrawFilledCircle(int xc, int yc, int r, uint16_t color);

/* sprites */
static void DrawPacman(int xc, int yc, int r, float mouth_deg, uint16_t color,
		uint16_t bg);
static void DrawPacman_FlipX(int xc, int yc, int r, float mouth_deg,
		uint16_t color, uint16_t bg);
static void DrawGhost(int x, int y, int w, int h, uint16_t body, uint16_t eye);

/* font */
static void DrawChar5x7(int x, int y, char c, uint16_t fg, uint16_t bg,
		int scale);
static void DrawText(int x, int y, const char *s, uint16_t fg, uint16_t bg,
		int scale);

/* scene */
static void StartScreen_Draw(void);
static void wait_for_any_button_press(void);

/* Buzzer */
void BUZZ_Start(void);
void BUZZ_Stop(void);
void BUZZ_SetFreq_TIM1(uint32_t hz); /* PA11 TIM1_CH4 */
void BUZZ_SetFreq_TIM3(uint32_t hz); /* PB1  TIM3_CH4 */
void PLAY_PacmanIntro_Blocking(void); /* 2성부 */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ---------- LCD low-level (핀은 CubeMX에서 설정된 그대로 사용) ---------- */
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

static inline void LCD_SET_DATA(uint8_t d) {
	HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin,
			(d & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin,
			(d & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin,
			(d & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin,
			(d & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin,
			(d & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin,
			(d & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin,
			(d & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin,
			(d & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static inline void LCD_WRITE_STROBE(void) {
	__NOP();
	__NOP();
	LCD_WR_LOW();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	LCD_WR_HIGH();
	__NOP();
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
static void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t c) {
	if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
		return;
	ILI9341_SetWindow(x, y, x, y);
	ILI9341_WriteData16(c);
}
static void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
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
static void ILI9341_FillScreen(uint16_t c) {
	ILI9341_FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, c);
}

/* ---- ILI9341 init & rotation (USER 영역 전용) ---- */
static void ILI9341_SetRotation(uint8_t rot) {
	/* 0: 세로 정상, 1: 가로(시계), 2: 180도, 3: 가로(반시계) */
	uint8_t madctl;
	switch (rot) {
	case 0:
		madctl = 0x48;
		break; /* MX|BGR */
	case 1:
		madctl = 0x28;
		break; /* MV|BGR */
	case 2:
		madctl = 0xC8;
		break; /* MX|MY|BGR  -> 180° */
	default:
		madctl = 0xE8;
		break;/* MX|MY|MV|BGR */
	}
	ILI9341_WriteCmd(0x36);
	ILI9341_WriteData8(madctl);
}
static void ILI9341_Init(void) {
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

	/* 모듈을 뒤집어 장착한 상황: 180° */
	ILI9341_SetRotation(2);
}

/* ----------------------------- Font (5x7) ----------------------------- */
static const uint8_t font5x7[][5] = { { 0, 0, 0, 0, 0 }, /* ' ' */
{ 0x7F, 0x09, 0x09, 0x09, 0x06 }, /* P */
{ 0x7C, 0x12, 0x11, 0x12, 0x7C }, /* A */
{ 0x3E, 0x41, 0x41, 0x41, 0x22 }, /* C */
{ 0x7F, 0x02, 0x0C, 0x02, 0x7F }, /* M */
{ 0x7F, 0x04, 0x08, 0x10, 0x7F }, /* N */
};
static int fidx(char c) {
	switch (c) {
	case ' ':
		return 0;
	case 'P':
		return 1;
	case 'A':
		return 2;
	case 'C':
		return 3;
	case 'M':
		return 4;
	case 'N':
		return 5;
	default:
		return 0;
	}
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

/* ===== ★ 상하(위↔아래) 미러 전용 렌더러 ===== */
static void DrawChar5x7_FlipY(int x, int y, char c, uint16_t fg, uint16_t bg,
		int s) {
	int i = fidx(c);
	for (int col = 0; col < 5; col++) {
		uint8_t bits = font5x7[i][col];
		for (int row = 0; row < 7; row++) {
			uint16_t color = (bits & (1 << (6 - row))) ? fg : bg;
			int tx = x + col * s; /* 좌우는 그대로 */
			int ty = y + (6 - row) * s; /* 상하만 반전 */
			ILI9341_FillRect(tx, ty, s, s, color);
		}
	}
	ILI9341_FillRect(x + 5 * s, y, s, 7 * s, bg);
}

static void DrawText_FlipY(int x, int y, const char *s, uint16_t fg,
		uint16_t bg, int scale) {
	int cx = x;
	while (*s) {
		DrawChar5x7_FlipY(cx, y, *s++, fg, bg, scale);
		cx += 6 * scale;
	}
}
/* -------------------------- Primitives --------------------------- */
static void DrawFilledCircle(int xc, int yc, int r, uint16_t color) {
	for (int y = -r; y <= r; y++) {
		int dx = (int) (sqrtf((float) r * r - (float) y * y) + 0.5f);
		ILI9341_FillRect(xc - dx, yc + y, 2 * dx + 1, 1, color);
	}
}
static void DrawLineH(int x, int y, int w, int t, uint16_t c) {
	ILI9341_FillRect(x, y - t / 2, w, t, c);
}
static void DrawLineV(int x, int y, int h, int t, uint16_t c) {
	ILI9341_FillRect(x - t / 2, y, t, h, c);
}
static void DrawRoundRect(int x, int y, int w, int h, int r, int t, uint16_t c) {
	/* 외곽선을 두께 t로 근사 */
	for (int i = 0; i < t; i++) {
		int ri = r - i;
		if (ri < 0)
			ri = 0;
		/* 네 귀퉁이 호 */
		for (int yy = -ri; yy <= ri; yy++) {
			int dx = (int) (sqrtf((float) ri * ri - (float) yy * yy) + 0.5f);
			/* 상단 좌/우 */
			ILI9341_FillRect(x + r - dx, y + i, 2 * dx, 1, c);
			ILI9341_FillRect(x + w - r - dx, y + i, 2 * dx, 1, c);
			/* 하단 좌/우 */
			ILI9341_FillRect(x + r - dx, y + h - 1 - i, 2 * dx, 1, c);
			ILI9341_FillRect(x + w - r - dx, y + h - 1 - i, 2 * dx, 1, c);
		}
		/* 직선 부분 */
		DrawLineH(x + r, y + i, w - 2 * r, 1, c);
		DrawLineH(x + r, y + h - 1 - i, w - 2 * r, 1, c);
		DrawLineV(x + i, y + r, h - 2 * r, 1, c);
		DrawLineV(x + w - 1 - i, y + r, h - 2 * r, 1, c);
	}
}

/* -------------------------- Sprites --------------------------- */
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
			ILI9341_FillRect(xs[0], y, xs[1] - xs[0] + 1, 1, COLOR_BLACK);
		}
	}
	/* 눈 */
	ILI9341_FillRect(xc + r / 5, yc - r / 2, r / 6, r / 6, COLOR_BLACK);
}

/* 오른쪽↔왼쪽 좌우 반전 팩맨 */
static void DrawPacman_FlipX(int xc, int yc, int r, float mouth_deg,
		uint16_t color, uint16_t bg) {
	/* 몸통 채우기 */
	DrawFilledCircle(xc, yc, r, color);

	/* 입(왼쪽 방향으로 벌어지게): 중심각을 π(180°) 기준으로 ±a/2 */
	float a = mouth_deg * 3.1415926f / 180.f;
	int x1 = xc, y1 = yc;
	int x2 = xc - (int) (r * cosf(a / 2)), y2 = yc - (int) (r * sinf(a / 2));
	int x3 = xc - (int) (r * cosf(a / 2)), y3 = yc + (int) (r * sinf(a / 2));

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
#define EDGE(xa,ya,xb,yb) \
            if (!((ya<y && yb<y) || (ya>y && yb>y) || (ya==yb))) { \
                float t = (ya==yb) ? 0.f : ((float)(y - ya) / (float)(yb - ya)); \
                xs[n++] = xa + (int)((xb - xa) * t); \
            }
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
			ILI9341_FillRect(xs[0], y, xs[1] - xs[0] + 1, 1, COLOR_BLACK);
		}
	}

	/* 눈: 왼쪽에 위치하도록 이동 */
	ILI9341_FillRect(xc - r / 5 - r / 6, yc - r / 2, r / 6, r / 6, COLOR_BLACK);
}

static void DrawGhost(int x, int y, int w, int h, uint16_t body, uint16_t eye) {
	ILI9341_FillRect(x, y + h / 4, w, 3 * h / 4, body);
	int r = w / 2;
	for (int yy = 0; yy < h / 2; yy++) {
		int dx = (int) sqrtf((float) r * r - (float) (r - yy) * (r - yy));
		ILI9341_FillRect(x + r - dx, y + yy, 2 * dx, 1, body);
	}
	int tooth = w / 5;
	for (int i = 0; i < 5; i++)
		if (i % 2 == 0)
			ILI9341_FillRect(x + i * tooth, y + h - (h / 8), tooth, h / 8,
			COLOR_BLACK);
	int ex = x + w / 4, ey = y + h / 3;
	ILI9341_FillRect(ex, ey, w / 6, h / 6, COLOR_WHITE);
	ILI9341_FillRect(ex + w / 2, ey, w / 6, h / 6, COLOR_WHITE);
	ILI9341_FillRect(ex + w / 12, ey + h / 12, w / 12, w / 12, eye);
	ILI9341_FillRect(ex + w / 2 + w / 12, ey + h / 12, w / 12, w / 12, eye);
}

/* --------------------------- Start Screen ------------------------------ */
/* 원본 참고 이미지를 가상의 기준 캔버스(가로 360, 세로 210)로 설계한 뒤
 LCD(240x320)에 비율 유지로 축소하여 중앙 배치. 위/아래 레터박스는 자동으로 검정. */
static void StartScreen_Draw(void) {
	ILI9341_FillScreen(COLOR_BLACK);

	/* 기준 사이즈(원본 비율 유사) */
	const float W_REF = 360.0f;
	const float H_REF = 210.0f;

	/* 스케일 및 오프셋 (가로를 기준으로 맞추면 오른쪽이 잘리지 않음) */
	float s = (float) LCD_WIDTH / W_REF;
	int scene_w = (int) (W_REF * s + 0.5f); /* == LCD_WIDTH */
	int scene_h = (int) (H_REF * s + 0.5f);
	int x0 = (LCD_WIDTH - scene_w) / 2; /* 보통 0 */
	int y0 = (LCD_HEIGHT - scene_h) / 2; /* 위/아래 검정 여백 */

	/* ----- 상단 미로 라인/캡슐들 ----- */
	int stroke = (int) (3 * s);
	if (stroke < 2)
		stroke = 2;
	/* 좌/우 큰 둥근 사각 프레임 */
	DrawRoundRect(x0 + (20 * s), y0 + (15 * s), (70 * s), (70 * s), (18 * s),
			stroke, COLOR_BLUE);
	DrawRoundRect(x0 + (270 * s), y0 + (135 * s), (70 * s), (70 * s), (18 * s),
			stroke, COLOR_BLUE);
	/* 세로 T자 */
	DrawRoundRect(x0 + (328 * s), y0 + (20 * s), (28 * s), (110 * s), (14 * s),
			stroke, COLOR_BLUE);
	DrawRoundRect(x0 + (20 * s), y0 + (120 * s), (28 * s), (110 * s), (14 * s),
			stroke, COLOR_BLUE);
	/* 상단 가로 캡슐 2개 */
	DrawRoundRect(x0 + (120 * s), y0 + (38 * s), (85 * s), (22 * s), (11 * s),
			stroke, COLOR_BLUE);
	DrawRoundRect(x0 + (215 * s), y0 + (38 * s), (85 * s), (22 * s), (11 * s),
			stroke, COLOR_BLUE);

	/* ----- 과일 아이콘(단순 원) ----- */
	DrawFilledCircle(x0 + (210 * s), y0 + (30 * s), (5 * s), COLOR_RED); /* 사과 */
	DrawFilledCircle(x0 + (165 * s), y0 + (30 * s), (5 * s), COLOR_RED); /* 딸기 */
	DrawFilledCircle(x0 + (58 * s), y0 + (172 * s), (6 * s), COLOR_RED); /* 체리 */

	/* ----- 중앙 로고 "PAC MAN" (노란색) ----- */
	int logo_scale = (int) (5 * s);
	if (logo_scale < 2)
		logo_scale = 2;
	int text_y = y0 + (80 * s);
	/* "PAC" */
	DrawText_FlipY(x0 + (110 * s), text_y, "PAC", COLOR_YELLOW, COLOR_BLACK,
			logo_scale);

	/* 공백 조금 */
	DrawText_FlipY(x0 + (110 * s) + 3 * (6 * logo_scale) + (8 * s), text_y,
			"MAN", COLOR_YELLOW, COLOR_BLACK, logo_scale);

	/* ----- 하단 경로 점(펠릿) & 팩맨 ----- */
	int dot_r = (int) (3 * s);
	if (dot_r < 2)
		dot_r = 2;
	int big_r = (int) (7 * s);
	int base_y = y0 + (140 * s);
	int base_x = x0 + (100 * s);

	/* L자 도트 */
	for (int i = 0; i < 7; i++)
		DrawFilledCircle(base_x + i * (12 * s), base_y, dot_r, COLOR_ORANGE);
	for (int i = 1; i < 8; i++)
		DrawFilledCircle(base_x, base_y + i * (12 * s), dot_r, COLOR_ORANGE);

	/* 굵은 파워 펠릿 2개 + 오렌지 */
	DrawFilledCircle(base_x + (8 * 12 * s), base_y, big_r, COLOR_ORANGE);
	DrawFilledCircle(base_x + (4 * 12 * s), base_y - (12 * s), (int) (5 * s),
	COLOR_ORANGE);

	/* 오른쪽으로 이어지는 도트 열 */
	for (int i = 1; i <= 10; i++)
		DrawFilledCircle(base_x + (8 * 12 * s) + i * (11 * s),
				base_y + (12 * s), dot_r, COLOR_ORANGE);

	/* 팩맨 (오른쪽 끝) */
	DrawPacman_FlipX(base_x + (8 * 12 * s) + (11 * 11 * s), base_y + (12 * s),
			(int) (10 * s), 45.f, COLOR_YELLOW, COLOR_BLACK);

	/* ----- 유령 3개 ----- */
	DrawGhost(x0 + (75 * s), y0 + (120 * s), (24 * s), (18 * s), COLOR_CYAN,
	COLOR_BLACK);
	DrawGhost(x0 + (180 * s), y0 + (185 * s), (24 * s), (18 * s), COLOR_RED,
	COLOR_BLACK);
	DrawGhost(x0 + (315 * s), y0 + (115 * s), (24 * s), (18 * s), COLOR_PINK,
	COLOR_BLACK);
}

/* 버튼 50ms 디바운스 */
static int any_button_now(void) {
	return BTN_PRESSED(UP_GPIO_Port, UP_Pin)
			|| BTN_PRESSED(LEFT_GPIO_Port, LEFT_Pin)
			|| BTN_PRESSED(RIGHT_GPIO_Port, RIGHT_Pin)
			|| BTN_PRESSED(DOWN_GPIO_Port, DOWN_Pin);
}
static void wait_for_any_button_press(void) {
	while (!any_button_now())
		HAL_Delay(1);
	HAL_Delay(50);
	while (any_button_now())
		HAL_Delay(1);
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

/* Pac-Man intro (간략 2성부) */
#define QN(ms)  (ms)
#define EN(ms)  ((ms)/2)
static const uint16_t TEMPO_MS = 430;
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
static const uint16_t voice1_freq[] = { B5, B6, B5, G5, E5, C5, D5, G4, 0, E5,
		C5, D5, G4 };
static const uint16_t voice2_freq[] = { G4, D5, G4, E5, C5, G4, B4, D4, G3, C4,
		G4, B4, D4 };
static const uint16_t voice_dur[] = { EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS),
		EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), QN(TEMPO_MS),
		EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), EN(TEMPO_MS), QN(TEMPO_MS) };
static const size_t VOICE_LEN = (sizeof(voice1_freq) / sizeof(voice1_freq[0]));
void PLAY_PacmanIntro_Blocking(void) {
	BUZZ_Start();
	for (size_t i = 0; i < VOICE_LEN; ++i) {
		BUZZ_SetFreq_TIM1(voice1_freq[i]);
		BUZZ_SetFreq_TIM3(voice2_freq[i]);
		HAL_Delay(voice_dur[i] - 10);
		BUZZ_SetFreq_TIM1(0);
		BUZZ_SetFreq_TIM3(0);
		HAL_Delay(10);
	}
	BUZZ_Stop();
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_USART2_UART_Init();
	MX_TIM1_Init();
	MX_TIM3_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	ILI9341_Init();
	StartScreen_Draw();
	PLAY_PacmanIntro_Blocking();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		wait_for_any_button_press();
		StartScreen_Draw();  // 버튼 누르면 다시 그리기
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
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

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

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
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 64 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 65535;
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
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 64 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65535;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
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
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin | LCD_D1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,
			LCD_RD_Pin | LCD_WR_Pin | LCD_RS_Pin | LD2_Pin | LCD_D7_Pin
					| LCD_D0_Pin | LCD_D2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
	LCD_CS_Pin | LCD_D6_Pin | LCD_D3_Pin | LCD_D5_Pin | LCD_D4_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
	GPIO_InitStruct.Pin = LCD_RST_Pin | LCD_D1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : UP_Pin LEFT_Pin */
	GPIO_InitStruct.Pin = UP_Pin | LEFT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LD2_Pin
	 LCD_D7_Pin LCD_D0_Pin LCD_D2_Pin */
	GPIO_InitStruct.Pin = LCD_RD_Pin | LCD_WR_Pin | LCD_RS_Pin | LD2_Pin
			| LCD_D7_Pin | LCD_D0_Pin | LCD_D2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
	 LCD_D4_Pin */
	GPIO_InitStruct.Pin = LCD_CS_Pin | LCD_D6_Pin | LCD_D3_Pin | LCD_D5_Pin
			| LCD_D4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : DOWN_Pin RIGHT_Pin */
	GPIO_InitStruct.Pin = DOWN_Pin | RIGHT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* (필요 시 콜백 등 여기에) */
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
  (void)file; (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
