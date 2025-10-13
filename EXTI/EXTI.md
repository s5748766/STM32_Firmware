# EXTI

<img src="https://github.com/gotree94/STM32/blob/main/NUCLEO_F103RB/03.EXTI/exti.gif?raw=true" width="400" alt="LED Blink Animation">
<br>

   * 파란색 버튼을 외부 인터럽트로 받아서 LD2를 켜는 인터럽트를 구현한다.

<img width="492" height="558" alt="EXTI_001" src="https://github.com/user-attachments/assets/dfd7371c-f08f-42ea-857d-e780099882a3" />
<br>
<img width="1433" height="908" alt="EXTI_002" src="https://github.com/user-attachments/assets/54423830-56b4-46eb-851a-dca00c904d7a" />
<br>
<img width="1392" height="908" alt="EXTI_003" src="https://github.com/user-attachments/assets/786a135b-efe1-4fee-9845-e1c72f1047f3" />
<br>
<img width="1392" height="908" alt="EXTI_004" src="https://github.com/user-attachments/assets/117ecee7-5e96-4a85-808b-9aad045a3c2b" />
<br>
<img width="1392" height="908" alt="EXTI_005" src="https://github.com/user-attachments/assets/7258f289-e6a3-40d7-bcd1-7438c5cd0f3c" />
<br>
<img width="1392" height="908" alt="EXTI_006" src="https://github.com/user-attachments/assets/b18365f2-5478-4887-84c2-7906ccf567d1" />
<br>
<img width="1392" height="908" alt="EXTI_007" src="https://github.com/user-attachments/assets/0fab6887-b373-4676-8c88-0a3b8f43737d" />
<br>
<img width="1392" height="908" alt="EXTI_008" src="https://github.com/user-attachments/assets/d919baf1-93b9-4da5-bdfb-685c48176507" />
<br>
<img width="1137" height="545" alt="EXTI_009" src="https://github.com/user-attachments/assets/8ebfe316-c55b-4925-95a6-598b18393961" />
<br>


```c
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case B1_Pin:
			HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			break;
		default:
			;
	}
}
/* USER CODE END 0 */
```
