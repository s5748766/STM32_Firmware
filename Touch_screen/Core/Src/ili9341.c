#include "ili9341.h"
#include <math.h>

// Complete 5x7 font data (ASCII 32-126)
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space (32)
    {0x00, 0x00, 0x4F, 0x00, 0x00}, // ! (33)
    {0x00, 0x07, 0x00, 0x07, 0x00}, // " (34)
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // # (35)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $ (36)
    {0x23, 0x13, 0x08, 0x64, 0x62}, // % (37)
    {0x36, 0x49, 0x55, 0x22, 0x50}, // & (38)
    {0x00, 0x05, 0x03, 0x00, 0x00}, // ' (39)
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // ( (40)
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ) (41)
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // * (42)
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // + (43)
    {0x00, 0x50, 0x30, 0x00, 0x00}, // , (44)
    {0x08, 0x08, 0x08, 0x08, 0x08}, // - (45)
    {0x00, 0x60, 0x60, 0x00, 0x00}, // . (46)
    {0x20, 0x10, 0x08, 0x04, 0x02}, // / (47)
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0 (48)
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1 (49)
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2 (50)
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3 (51)
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4 (52)
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5 (53)
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6 (54)
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7 (55)
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8 (56)
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9 (57)
    {0x00, 0x36, 0x36, 0x00, 0x00}, // : (58)
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ; (59)
    {0x08, 0x14, 0x22, 0x41, 0x00}, // < (60)
    {0x14, 0x14, 0x14, 0x14, 0x14}, // = (61)
    {0x00, 0x41, 0x22, 0x14, 0x08}, // > (62)
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ? (63)
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @ (64)
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A (65)
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B (66)
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C (67)
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D (68)
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E (69)
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F (70)
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G (71)
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H (72)
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I (73)
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J (74)
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K (75)
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L (76)
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M (77)
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N (78)
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O (79)
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P (80)
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q (81)
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R (82)
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S (83)
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T (84)
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U (85)
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V (86)
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W (87)
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X (88)
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y (89)
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z (90)
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [ (91)
    {0x02, 0x04, 0x08, 0x10, 0x20}, // \ (92)
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ] (93)
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^ (94)
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _ (95)
    {0x00, 0x01, 0x02, 0x04, 0x00}, // ` (96)
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a (97)
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b (98)
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c (99)
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d (100)
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e (101)
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f (102)
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g (103)
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h (104)
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i (105)
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j (106)
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k (107)
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l (108)
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m (109)
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n (110)
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o (111)
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p (112)
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q (113)
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r (114)
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s (115)
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t (116)
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u (117)
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v (118)
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w (119)
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x (120)
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y (121)
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z (122)
    {0x00, 0x08, 0x36, 0x41, 0x00}, // { (123)
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // | (124)
    {0x00, 0x41, 0x36, 0x08, 0x00}, // } (125)
    {0x10, 0x08, 0x08, 0x10, 0x08}  // ~ (126)
};

// Control pin macros
#define CS_LOW()    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH()   HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define RS_LOW()    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET)  // Command
#define RS_HIGH()   HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET)   // Data
#define WR_LOW()    HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_RESET)
#define WR_HIGH()   HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_SET)
#define RD_LOW()    HAL_GPIO_WritePin(LCD_RD_PORT, LCD_RD_PIN, GPIO_PIN_RESET)
#define RD_HIGH()   HAL_GPIO_WritePin(LCD_RD_PORT, LCD_RD_PIN, GPIO_PIN_SET)
#define RST_LOW()   HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET)
#define RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET)

// Write 8-bit data to parallel bus
void ILI9341_WriteData8(uint8_t data) {
    // Set data pins
    HAL_GPIO_WritePin(LCD_D0_PORT, LCD_D0_PIN, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D1_PORT, LCD_D1_PIN, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D2_PORT, LCD_D2_PIN, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D3_PORT, LCD_D3_PIN, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Write strobe
    WR_LOW();
    __NOP(); // Small delay
    __NOP();
    WR_HIGH();
}

// Read 8-bit data from parallel bus
uint8_t ILI9341_ReadData8(void) {
    uint8_t data = 0;

    // Read strobe
    RD_LOW();
    __NOP(); // Small delay
    __NOP();

    // Read data pins
    if (HAL_GPIO_ReadPin(LCD_D0_PORT, LCD_D0_PIN)) data |= 0x01;
    if (HAL_GPIO_ReadPin(LCD_D1_PORT, LCD_D1_PIN)) data |= 0x02;
    if (HAL_GPIO_ReadPin(LCD_D2_PORT, LCD_D2_PIN)) data |= 0x04;
    if (HAL_GPIO_ReadPin(LCD_D3_PORT, LCD_D3_PIN)) data |= 0x08;
    if (HAL_GPIO_ReadPin(LCD_D4_PORT, LCD_D4_PIN)) data |= 0x10;
    if (HAL_GPIO_ReadPin(LCD_D5_PORT, LCD_D5_PIN)) data |= 0x20;
    if (HAL_GPIO_ReadPin(LCD_D6_PORT, LCD_D6_PIN)) data |= 0x40;
    if (HAL_GPIO_ReadPin(LCD_D7_PORT, LCD_D7_PIN)) data |= 0x80;

    RD_HIGH();
    return data;
}

// Configure data pins for output
void ILI9341_SetDataPinsOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = LCD_D0_PIN;
    HAL_GPIO_Init(LCD_D0_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D1_PIN;
    HAL_GPIO_Init(LCD_D1_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D2_PIN;
    HAL_GPIO_Init(LCD_D2_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D3_PIN;
    HAL_GPIO_Init(LCD_D3_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D4_PIN;
    HAL_GPIO_Init(LCD_D4_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D5_PIN;
    HAL_GPIO_Init(LCD_D5_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D6_PIN;
    HAL_GPIO_Init(LCD_D6_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D7_PIN;
    HAL_GPIO_Init(LCD_D7_PORT, &GPIO_InitStruct);
}

// Configure data pins for input
void ILI9341_SetDataPinsInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = LCD_D0_PIN;
    HAL_GPIO_Init(LCD_D0_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D1_PIN;
    HAL_GPIO_Init(LCD_D1_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D2_PIN;
    HAL_GPIO_Init(LCD_D2_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D3_PIN;
    HAL_GPIO_Init(LCD_D3_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D4_PIN;
    HAL_GPIO_Init(LCD_D4_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D5_PIN;
    HAL_GPIO_Init(LCD_D5_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D6_PIN;
    HAL_GPIO_Init(LCD_D6_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LCD_D7_PIN;
    HAL_GPIO_Init(LCD_D7_PORT, &GPIO_InitStruct);
}

void ILI9341_WriteCommand(uint8_t cmd) {
    CS_LOW();
    RS_LOW();  // Command mode
    ILI9341_WriteData8(cmd);
    CS_HIGH();
}

void ILI9341_WriteData(uint8_t data) {
    CS_LOW();
    RS_HIGH(); // Data mode
    ILI9341_WriteData8(data);
    CS_HIGH();
}

void ILI9341_WriteData16(uint16_t data) {
    CS_LOW();
    RS_HIGH(); // Data mode
    ILI9341_WriteData8(data >> 8);   // High byte
    ILI9341_WriteData8(data & 0xFF); // Low byte
    CS_HIGH();
}

uint8_t ILI9341_ReadData(void) {
    uint8_t data;

    ILI9341_SetDataPinsInput();
    CS_LOW();
    RS_HIGH(); // Data mode
    data = ILI9341_ReadData8();
    CS_HIGH();
    ILI9341_SetDataPinsOutput();

    return data;
}

void ILI9341_Init(void) {
    // Initialize control pins
    RD_HIGH();
    WR_HIGH();
    CS_HIGH();

    // Configure data pins as output initially
    ILI9341_SetDataPinsOutput();

    // Hardware reset
    RST_HIGH();
    HAL_Delay(10);
    RST_LOW();
    HAL_Delay(10);
    RST_HIGH();
    HAL_Delay(120);

    // Software reset
    ILI9341_WriteCommand(ILI9341_SWRESET);
    HAL_Delay(150);

    // Power control A
    ILI9341_WriteCommand(0xCB);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);

    // Power control B
    ILI9341_WriteCommand(0xCF);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);

    // Driver timing control A
    ILI9341_WriteCommand(0xE8);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);

    // Driver timing control B
    ILI9341_WriteCommand(0xEA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);

    // Power on sequence control
    ILI9341_WriteCommand(0xED);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);

    // Pump ratio control
    ILI9341_WriteCommand(0xF7);
    ILI9341_WriteData(0x20);

    // Power control 1
    ILI9341_WriteCommand(ILI9341_PWCTR1);
    ILI9341_WriteData(0x23);

    // Power control 2
    ILI9341_WriteCommand(ILI9341_PWCTR2);
    ILI9341_WriteData(0x10);

    // VCOM control 1
    ILI9341_WriteCommand(ILI9341_VMCTR1);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);

    // VCOM control 2
    ILI9341_WriteCommand(ILI9341_VMCTR2);
    ILI9341_WriteData(0x86);

    // Memory access control
    ILI9341_WriteCommand(ILI9341_MADCTL);
    ILI9341_WriteData(0x48);

    // Pixel format
    ILI9341_WriteCommand(ILI9341_PIXFMT);
    ILI9341_WriteData(0x55);

    // Frame rate control
    ILI9341_WriteCommand(ILI9341_FRMCTR1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x18);

    // Display function control
    ILI9341_WriteCommand(ILI9341_DFUNCTR);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);

    // Gamma function disable
    ILI9341_WriteCommand(0xF2);
    ILI9341_WriteData(0x00);

    // Gamma curve
    ILI9341_WriteCommand(ILI9341_GAMMASET);
    ILI9341_WriteData(0x01);

    // Positive gamma correction
    ILI9341_WriteCommand(ILI9341_GMCTRP1);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);

    // Negative gamma correction
    ILI9341_WriteCommand(ILI9341_GMCTRN1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);

    // Exit sleep mode
    ILI9341_WriteCommand(ILI9341_SLPOUT);
    HAL_Delay(120);

    // Display on
    ILI9341_WriteCommand(ILI9341_DISPON);
    HAL_Delay(50);

    // Fill screen with black
    ILI9341_Fill(BLACK);
}

void ILI9341_SetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ILI9341_WriteCommand(ILI9341_CASET);
    ILI9341_WriteData16(x1);
    ILI9341_WriteData16(x2);

    ILI9341_WriteCommand(ILI9341_PASET);
    ILI9341_WriteData16(y1);
    ILI9341_WriteData16(y2);

    ILI9341_WriteCommand(ILI9341_RAMWR);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;

    ILI9341_SetAddress(x, y, x, y);
    ILI9341_WriteData16(color);
}

void ILI9341_Fill(uint16_t color) {
    ILI9341_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    if ((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if ((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ILI9341_SetAddress(x, y, x + w - 1, y + h - 1);

    CS_LOW();
    RS_HIGH(); // Data mode

    for (uint32_t i = 0; i < w * h; i++) {
        ILI9341_WriteData8(color >> 8);   // High byte
        ILI9341_WriteData8(color & 0xFF); // Low byte
    }

    CS_HIGH();
}

void ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        ILI9341_DrawPixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ILI9341_DrawLine(x, y, x + w - 1, y, color);
    ILI9341_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    ILI9341_DrawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    ILI9341_DrawLine(x, y + h - 1, x, y, color);
}

void ILI9341_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    ILI9341_DrawPixel(x0, y0 + r, color);
    ILI9341_DrawPixel(x0, y0 - r, color);
    ILI9341_DrawPixel(x0 + r, y0, color);
    ILI9341_DrawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ILI9341_DrawPixel(x0 + x, y0 + y, color);
        ILI9341_DrawPixel(x0 - x, y0 + y, color);
        ILI9341_DrawPixel(x0 + x, y0 - y, color);
        ILI9341_DrawPixel(x0 - x, y0 - y, color);
        ILI9341_DrawPixel(x0 + y, y0 + x, color);
        ILI9341_DrawPixel(x0 - y, y0 + x, color);
        ILI9341_DrawPixel(x0 + y, y0 - x, color);
        ILI9341_DrawPixel(x0 - y, y0 - x, color);
    }
}

void ILI9341_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bgcolor) {
    if (ch < 32 || ch > 126) ch = 32; // Space for unsupported characters

    const uint8_t *font_data = font5x7[ch - 32];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = font_data[col];
        for (uint8_t row = 0; row < 8; row++) {  // Changed from 7 to 8
            if (line & (1 << row)) {  // Changed bit checking method
                ILI9341_DrawPixel(x + col, y + row, color);
            } else if (bgcolor != color) {
                ILI9341_DrawPixel(x + col, y + row, bgcolor);
            }
        }
    }
}

void ILI9341_DrawString(uint16_t x, uint16_t y, char* str, uint16_t color, uint16_t bgcolor) {
    uint16_t start_x = x;

    while (*str) {
        if (*str == '\n') {
            y += 9;  // Changed from 8 to 9 for better line spacing
            x = start_x;
        } else {
            ILI9341_DrawChar(x, y, *str, color, bgcolor);
            x += 6;  // 5 pixels + 1 pixel spacing
        }
        str++;
    }
}
