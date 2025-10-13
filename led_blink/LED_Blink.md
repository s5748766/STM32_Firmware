## 1. LED_Blink

<img src="https://github.com/gotree94/STM32/blob/main/NUCLEO_F103RB/01.LED_Blink/LED_Blinks.gif?raw=true" width="400" alt="LED Blink Animation">
<br>

<img width="520" height="350" alt="LED_Blinks_001" src="https://github.com/user-attachments/assets/3aa18d2c-5bc8-4d24-ae1f-898272f96bf9" />
<br>
<img width="1359" height="805" alt="LED_Blinks_002" src="https://github.com/user-attachments/assets/d2634652-6e5b-46be-bbb8-027a9d0f3c9b" />
<br>
<img width="402" height="51" alt="LED_Blinks_004" src="https://github.com/user-attachments/assets/6cf1078d-0d6a-4d3f-88b3-b3e46151b983" />
<br>
<img width="492" height="558" alt="LED_Blinks_005" src="https://github.com/user-attachments/assets/3197a10b-59cd-43e6-baa2-4d5f46a0e8a1" />
<br>
<img width="486" height="129" alt="LED_Blinks_006" src="https://github.com/user-attachments/assets/4d967a98-f423-4f2f-ac96-e863f1115848" />
<br>
<img width="1433" height="908" alt="LED_Blinks_007" src="https://github.com/user-attachments/assets/3df11327-4bef-41f6-8e77-d99d4d80d467" />
<br>
<img width="1433" height="908" alt="LED_Blinks_008" src="https://github.com/user-attachments/assets/d3133bc5-09d9-4b51-baec-290a8c229c6f" />
<br>
<img width="1433" height="908" alt="LED_Blinks_009" src="https://github.com/user-attachments/assets/401ce922-6baf-4321-a3eb-87b8bffff721" />
<br>
<img width="1433" height="908" alt="LED_Blinks_010" src="https://github.com/user-attachments/assets/f4b312b8-fa49-4413-bb46-4125ad84b26b" />
<br>
<img width="242" height="297" alt="LED_Blinks_011" src="https://github.com/user-attachments/assets/7bf0e956-97db-48b5-9856-d4a119fc7538" />
<br>
<img width="486" height="165" alt="LED_Blinks_012" src="https://github.com/user-attachments/assets/ca3e7a4a-8acc-4cfd-9905-e01ccbeb556c" />
<br>
<img width="1137" height="545" alt="LED_Blinks_013" src="https://github.com/user-attachments/assets/c4d741b9-39ba-41f5-9f5a-405e0d6c1157" />
<br>
<img width="1137" height="545" alt="LED_Blinks_014" src="https://github.com/user-attachments/assets/e0c09683-07a4-4a1b-8dd1-3e2f91cba259" /><br>


* main.c
```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	  HAL_Delay(1000);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
	  HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
```
<img src="https://github.com/gotree94/STM32/blob/main/NUCLEO_F103RB/01.LED_Blink/led2.gif?raw=true" width="400" alt="LED Blink Animation">
<br>

<img width="583" height="360" alt="shield-101" src="https://github.com/user-attachments/assets/b614372a-7016-45c8-9bda-873a4b3c9944" />
<br>

* main.c
```c
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
	  HAL_Delay(1000);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
	  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
	  HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
```



