# STM32_Firmware
## STM32CUBEIDE_PROJECT

### LED BLINK
- LD2 LED에 HIGH, LOW 신호 보내기
- HAL_GPIO_WritePin(); 사용

### UART COMMUNICTION
- Teraterm 터미널에 특정 문자, 숫자 보내기
- HAL_UART_Receive(); 사용
- HAL_UART_Transmit(); 사용

### INTERRUPT
- PC13(BLUE BUTTON) 누를 시 LD2 LED 인터럽트(ON/OFF)

### TIMER CONTROL
- gTimerCnt 변수의 값이 0~100까지 카운트 되었을 때 LD2 LED TOGGLE

### ADC TEMPERATURE SENSOR
- STM32F103CBT6 내부 온도 센서 측정 데이터 Teraterm 터미널로 전송

### MOTOR CONTROL
- 시리얼 터미널로 키 입력
- 전진, 후진, 좌, 우 방향 전환
- 상태 확인

### ULTRASONIC SENCOR
- 시리얼 터미널에 초음파 센서 값 출력
- 출력 시간 조정 가능

### BUZZOR CONTROL
- 부저를 이용한 특정 소리 및 음악 재생

### SERVO MOTOR CONTROL
- 서보 모터를 제어

### DHT11_SENSOR
- DHT11 센서를 이용하여 온습도 측정
