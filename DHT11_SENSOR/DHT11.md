# DHT11 : 온도 습도 센서

   * PA0 - DATA

<img width="644" height="586" alt="F103RB-pin" src="https://github.com/user-attachments/assets/8e00380d-6d85-49b3-b845-88558092ffc0" />
<br>

<img width="995" height="550" alt="DHT11_001" src="https://github.com/user-attachments/assets/a487c821-8c1c-4ec8-a241-ed420251dc1e" />
<br>

<img width="1592" height="935" alt="DHT11_002" src="https://github.com/user-attachments/assets/cccbea1d-c0d8-4c90-9eba-7ab735dfb138" />
<br>

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PTD */
typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    uint8_t temp_decimal;
    uint8_t hum_decimal;
    uint8_t checksum;
} DHT11_Data;
/* USER CODE END PTD */
```

```c
/* USER CODE BEGIN PD */
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_0
/* USER CODE END PD */
```

```c
/* USER CODE BEGIN PV */
DHT11_Data dht11_data;
char uart_buffer[100];  // uart_buffer 변수 선언 추가
/* USER CODE END PV */
```

```c
/* USER CODE BEGIN PFP */
void DHT11_SetPinOutput(void);
void DHT11_SetPinInput(void);
void DHT11_SetPin(GPIO_PinState state);
GPIO_PinState DHT11_ReadPin(void);
void DHT11_DelayUs(uint32_t us);
uint8_t DHT11_Start(void);
uint8_t DHT11_ReadBit(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadData(DHT11_Data *data);
/* USER CODE END PFP */
```

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  if (ch == '\n')
    HAL_UART_Transmit (&huart2, (uint8_t*) "\r", 1, 0xFFFF);
  HAL_UART_Transmit (&huart2, (uint8_t*) &ch, 1, 0xFFFF);

  return ch;
}

// DHT11 함수 구현
void DHT11_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

void DHT11_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

void DHT11_SetPin(GPIO_PinState state) {
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, state);
}

GPIO_PinState DHT11_ReadPin(void) {
    return HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN);
}

void DHT11_DelayUs(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

uint8_t DHT11_Start(void) {
    uint8_t response = 0;

    // 출력 모드로 설정
    DHT11_SetPinOutput();

    // 시작 신호 전송 (18ms LOW)
    DHT11_SetPin(GPIO_PIN_RESET);
    HAL_Delay(20);  // 18ms -> 20ms로 변경 (더 안정적)

    // HIGH로 변경 후 20-40us 대기
    DHT11_SetPin(GPIO_PIN_SET);
    DHT11_DelayUs(30);

    // 입력 모드로 변경
    DHT11_SetPinInput();

    // DHT11 응답 확인 (80us LOW + 80us HIGH)
    DHT11_DelayUs(40);

    if (!(DHT11_ReadPin())) {
        DHT11_DelayUs(80);
        if (DHT11_ReadPin()) {
            response = 1;
        } else {
            response = 0;
        }
    }

    // HIGH가 끝날 때까지 대기
    while (DHT11_ReadPin());

    return response;
}

uint8_t DHT11_ReadBit(void) {
    // LOW 신호가 끝날 때까지 대기 (50us)
    while (!(DHT11_ReadPin()));

    // HIGH 신호 시작 후 30us 대기
    DHT11_DelayUs(30);

    // 여전히 HIGH면 1, LOW면 0
    if (DHT11_ReadPin()) {
        // HIGH가 끝날 때까지 대기
        while (DHT11_ReadPin());
        return 1;
    } else {
        return 0;
    }
}

uint8_t DHT11_ReadByte(void) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte = (byte << 1) | DHT11_ReadBit();
    }
    return byte;
}

uint8_t DHT11_ReadData(DHT11_Data *data) {
    if (!DHT11_Start()) {
        return 0; // 시작 신호 실패
    }

    // 5바이트 데이터 읽기
    data->humidity = DHT11_ReadByte();
    data->hum_decimal = DHT11_ReadByte();
    data->temperature = DHT11_ReadByte();
    data->temp_decimal = DHT11_ReadByte();
    data->checksum = DHT11_ReadByte();

    // 체크섬 확인
    uint8_t calculated_checksum = data->humidity + data->hum_decimal +
                                 data->temperature + data->temp_decimal;

    if (calculated_checksum == data->checksum) {
        return 1; // 성공
    } else {
        return 0; // 체크섬 오류
    }
}
/* USER CODE END 0 */
```

```c
  /* USER CODE BEGIN 2 */

  // 타이머 시작 (마이크로초 단위 지연용)
  HAL_TIM_Base_Start(&htim2);

  // UART 초기화 메시지
  sprintf(uart_buffer, "DHT11 Temperature & Humidity Sensor Test\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);

  /* USER CODE END 2 */
```

```c
	    /* USER CODE BEGIN 3 */

	    if (DHT11_ReadData(&dht11_data)) {
	      // 데이터 읽기 성공
	      sprintf(uart_buffer, "Temperature: %d°C, Humidity: %d%%\r\n",
	              dht11_data.temperature, dht11_data.humidity);
	      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
	    } else {
	      // 데이터 읽기 실패
	      sprintf(uart_buffer, "DHT11 Read Error!\r\n");
	      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
	    }

	    // 2초 대기 (DHT11은 최소 2초 간격으로 읽어야 함)
	    HAL_Delay(2000);

	  }
	  /* USER CODE END 3 */
```
