# HC-SR04

<img width="600" height="480" alt="shield-001" src="https://github.com/user-attachments/assets/4c7f5dc6-ffe6-4f62-bcb1-376dc55e13a9" />
<br>
<img width="600" height="480" alt="shield-002" src="https://github.com/user-attachments/assets/e67d445b-c3b0-483c-92b6-4100a39a662e" />
<br>
<img width="600" height="480" alt="shield-002" src="https://github.com/user-attachments/assets/48183bb9-3a11-42a8-9ab9-c07975e4e6f8" />
<br><br>

<img width="600" height="520" alt="ultrasonic_001" src="https://github.com/user-attachments/assets/1827803d-4843-4b12-a703-df4b046097b6" />
<br>
<img width="600" height="450" alt="ultrasonic_002" src="https://github.com/user-attachments/assets/4fcc69d9-bb62-4270-856b-af036091733e" />
<br>
<img width="600" height="450" alt="ultrasonic_003" src="https://github.com/user-attachments/assets/3d8242ce-dfee-400f-bc4e-84e069c0e1b2" />
<br>
<img width="600" height="450" alt="ultrasonic_004" src="https://github.com/user-attachments/assets/878c5a3f-8f04-4ad9-b35d-cde7ff035420" />
<br>

<img width="1392" height="908" alt="hc_sr40_001" src="https://github.com/user-attachments/assets/0957fd23-ceb9-459e-9d91-7be4f3750c26" />
<br>
<img width="1392" height="908" alt="hc_sr40_002" src="https://github.com/user-attachments/assets/8e359bc7-de92-41ec-9859-8f212bbca60f" />
<br>
<img width="995" height="550" alt="hc_sr40_003" src="https://github.com/user-attachments/assets/43f8cc02-e8e5-496e-8031-85a921506eee" />
<br>

```c
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
```

```c
/* USER CODE BEGIN PD */
#define HIGH 1
#define LOW 0
long unsigned int echo_time;
int dist;
/* USER CODE END PD */
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

void timer_start(void)
{
   HAL_TIM_Base_Start(&htim1);
}

void delay_us(uint16_t us)
{
   __HAL_TIM_SET_COUNTER(&htim1, 0); // initislize counter to start from 0
   while((__HAL_TIM_GET_COUNTER(&htim1))<us); // wait count until us
}

void trig(void)
{
   HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, HIGH);
   delay_us(10);
   HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, LOW);
}

/**
* @brief echo 신호가 HIGH를 유지하는 시간을 (㎲)단위로 측정하여 리턴한다.
* @param no param(void)
*/
long unsigned int echo(void)
{
   long unsigned int echo = 0;

   while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == LOW){}
        __HAL_TIM_SET_COUNTER(&htim1, 0);
        while(HAL_GPIO_ReadPin(ECHO1_GPIO_Port, ECHO1_Pin) == HIGH);
        echo = __HAL_TIM_GET_COUNTER(&htim1);
   if( echo >= 240 && echo <= 23000 ) return echo;
   else return 0;
}
```

```c
  /* USER CODE BEGIN 2 */
  timer_start();
  printf("Ranging with HC-SR04\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  trig();
	      echo_time = echo();
	      if( echo_time != 0){
	          dist = (int)(17 * echo_time / 100);
	          printf("Distance = %d(mm)\n", dist);
	      }
	      else printf("Out of Range!\n");
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
```


