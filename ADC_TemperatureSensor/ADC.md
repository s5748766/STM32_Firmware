# ADC_TemperatureSensor

<img width="995" height="550" alt="temp_013" src="https://github.com/user-attachments/assets/3a757270-4e1d-4f5d-86ef-6cedc029c2cb" />
<br><br><br>

<img width="1392" height="908" alt="temp_001" src="https://github.com/user-attachments/assets/58c7a569-3998-4bae-acc3-452452404976" />
<br>
<img width="1392" height="908" alt="temp_002" src="https://github.com/user-attachments/assets/49a9d813-1ae3-4add-936b-4141b97b4a30" />
<br>
<img width="1392" height="908" alt="temp_003" src="https://github.com/user-attachments/assets/2a355620-8223-4e72-898e-822b94b31039" />
<br>
<img width="1392" height="908" alt="temp_004" src="https://github.com/user-attachments/assets/f1e53d23-bfbb-46f4-bebe-b0b217eb95bf" />
<br>
<img width="1392" height="908" alt="temp_005" src="https://github.com/user-attachments/assets/e9e78278-1f8e-4b6d-a16a-8e694537cffe" />
<br>
<img width="1392" height="908" alt="temp_006" src="https://github.com/user-attachments/assets/63a00f61-63e3-44c3-b00f-e00b2f9d7c5e" />
<br>
<img width="1137" height="545" alt="temp_007" src="https://github.com/user-attachments/assets/20db179f-1824-430f-96c4-343147155d33" />
<br>
<img width="1137" height="545" alt="temp_008" src="https://github.com/user-attachments/assets/ed34f441-cf14-4275-9874-772b4f471082" />
<br>
<img width="1137" height="545" alt="temp_009" src="https://github.com/user-attachments/assets/744c9536-b731-43b3-a620-442fef278865" />
<br>
<img width="1225" height="274" alt="temp_010" src="https://github.com/user-attachments/assets/78be6ebb-bf14-4f3d-aee2-b632df80b4ab" />
<br>
<img width="853" height="832" alt="temp_011" src="https://github.com/user-attachments/assets/8d605061-129a-41e3-8a16-14c6708e42e5" />
<br>
<img width="853" height="832" alt="temp_012" src="https://github.com/user-attachments/assets/e6accd8e-6208-4d61-9460-ccd2de35d717" />
<br>



```c
/* USER CODE BEGIN PV */
const float AVG_SLOPE = 4.3E-03;
const float V25 = 1.43;
const float ADC_TO_VOLT = 3.3 / 4096;
/* USER CODE END PV */
```


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
  /* USER CODE BEGIN 2 */
  uint16_t adc1;

  float vSense;
  float temp;

  if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
	  Error_Handler();
  }

  if(HAL_ADC_Start(&hadc1) != HAL_OK)
  {
	  Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_ADC_PollForConversion(&hadc1, 100);
	  adc1 = HAL_ADC_GetValue(&hadc1);
	  //printf("ADC1_temperature : %d \n",adc1);

	  vSense = adc1 * ADC_TO_VOLT;
	  temp = (V25 - vSense) / AVG_SLOPE + 25.0;
	  printf("temperature : %d, %f \n",adc1, temp);

	  HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
```
