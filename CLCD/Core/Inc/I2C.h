/*
 * I2C.h
 *
 *  Created on: Sep 15, 2025
 *      Author: 50
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "main.h"
#include <stdio.h>


#define delay_ms  HAL_Delay

//#define ADDRESS   0x3F << 1
#define ADDRESS   0x27 << 1

#define RS1_EN1   0x05
#define RS1_EN0   0x01
#define RS0_EN1   0x04
#define RS0_EN0   0x00
#define BackLight 0x08

I2C_HandleTypeDef hi2c1;


int delay = 0;
int value = 0;

void I2C_ScanAddresses(void);

void delay_us(int us);
void LCD_DATA(uint8_t data);
void LCD_CMD(uint8_t cmd);
void LCD_CMD_4bit(uint8_t cmd);
void LCD_INIT(void);
void LCD_XY(char x, char y);
void LCD_CLEAR(void);
void LCD_PUTS(char *str);


void I2C_ScanAddresses(void) {
    HAL_StatusTypeDef result;
    uint8_t i;


    printf("Scanning I2C addresses...\r\n");


    for (i = 1; i < 128; i++) {
        /*
         * HAL_I2C_IsDeviceReady: If a device at the specified address exists return HAL_OK.
         * Since I2C devices must have an 8-bit address, the 7-bit address is shifted left by 1 bit.
         */
        result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10);
        if (result == HAL_OK) {
            printf("I2C device found at address 0x%02X\r\n", i);
        }
    }


    printf("Scan complete.\r\n");
}

void delay_us(int us){
	value = 3;
	delay = us * value;
	for(int i=0;i < delay;i++);
}

void LCD_DATA(uint8_t data) {
	uint8_t temp=(data & 0xF0)|RS1_EN1|BackLight;

	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=(data & 0xF0)|RS1_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(4);

	temp=((data << 4) & 0xF0)|RS1_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp = ((data << 4) & 0xF0)|RS1_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(50);
}

void LCD_CMD(uint8_t cmd) {
	uint8_t temp=(cmd & 0xF0)|RS0_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=(cmd & 0xF0)|RS0_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(4);

	temp=((cmd << 4) & 0xF0)|RS0_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=((cmd << 4) & 0xF0)|RS0_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(50);
}

void LCD_CMD_4bit(uint8_t cmd) {
	uint8_t temp=((cmd << 4) & 0xF0)|RS0_EN1|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	temp=((cmd << 4) & 0xF0)|RS0_EN0|BackLight;
	while(HAL_I2C_Master_Transmit(&hi2c1, ADDRESS, &temp, 1, 1000)!=HAL_OK);
	delay_us(50);
}

void LCD_INIT(void) {

	delay_ms(100);

	LCD_CMD_4bit(0x03); delay_ms(5);
	LCD_CMD_4bit(0x03); delay_us(100);
	LCD_CMD_4bit(0x03); delay_us(100);
	LCD_CMD_4bit(0x02); delay_us(100);
	LCD_CMD(0x28);  // 4 bits, 2 line, 5x8 font
	LCD_CMD(0x08);  // display off, cursor off, blink off
	LCD_CMD(0x01);  // clear display
	delay_ms(3);
	LCD_CMD(0x06);  // cursor movint direction
	LCD_CMD(0x0C);  // display on, cursor off, blink off
}

void LCD_XY(char x, char y) {
	if      (y == 0) LCD_CMD(0x80 + x);
	else if (y == 1) LCD_CMD(0xC0 + x);
	else if (y == 2) LCD_CMD(0x94 + x);
	else if (y == 3) LCD_CMD(0xD4 + x);
}

void LCD_CLEAR(void) {
	LCD_CMD(0x01);
	delay_ms(2);
}

void LCD_PUTS(char *str) {
	while (*str) LCD_DATA(*str++);
}

#endif /* INC_I2C_H_ */
