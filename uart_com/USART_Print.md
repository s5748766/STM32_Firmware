# USART_Print : printf 시리얼 디버깅 

   * 터미널 통신 프로그램 설치 : [https://github.com/TeraTermProject/teraterm/releases?authuser=0]


<img width="995" height="550" alt="USART_Print_008" src="https://github.com/user-attachments/assets/dcb67c8e-7796-4dee-9f22-f0b0637c0a7e" />


<img width="492" height="558" alt="USART_Print_001" src="https://github.com/user-attachments/assets/d34b82d1-3857-4a56-8ef8-f7c648daaab7" />
<br>
<img width="1433" height="908" alt="USART_Print_002" src="https://github.com/user-attachments/assets/bdee0660-b056-4837-b4da-2f156bdc31ff" />
<br>
<img width="1433" height="908" alt="USART_Print_003" src="https://github.com/user-attachments/assets/3136549e-c240-4625-bcbe-558c9b17aede" />
<br>
<img width="1137" height="545" alt="USART_Print_004" src="https://github.com/user-attachments/assets/356b2b6a-f610-47f8-8711-b5e8af35d6ca" />
<br>
<img width="1137" height="545" alt="USART_Print_005" src="https://github.com/user-attachments/assets/10aa62af-c7d3-48a6-98d3-5948047a0ddf" />
<br>
<img width="1137" height="545" alt="USART_Print_006" src="https://github.com/user-attachments/assets/a400b2dd-8cf2-4094-b7e6-4633bdb62cd3" />
<br>
<img width="1137" height="545" alt="USART_Print_007" src="https://github.com/user-attachments/assets/2124890a-c43d-4be2-8c53-bc3875f66168" />
<br>

```c
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */
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
```

```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  printf("Hello World!\n");
	  HAL_Delay(1000);
    /* USER CODE END WHILE */
```

