# STM32_Firmware
## STM32CUBEIDE_PROJECT

### LED BLINK
- LD2 LED에 HIGH, LOW 신호 보내기
- HAL_GPIO_WritePin(); 사용

### UART Communication
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

### DHT11 SENSOR
- DHT11 센서를 이용하여 온습도 측정

### CLCD Display
- CLCD 디스플레이에 문자 출력 및 DHT11 센서 값 표시

### ADC 가변저항
- ADC 핀을 이용하여 가변저항의 전압 값 측정

### 온습도 WEB Service
- 온습도 데이터 값을 시리얼 통신으로 웹에 전달 

### 온습도 및 소리 WEB Service
- 온습도 데이터 값을 시리얼 통신으로 웹에 전달 및 웹(PC)에서의 소리 측정 후 피크 값 이상의 소리 발생 시 LED ON/OFF제어

### Bluetooth Communication
- 블루투스 모듈을 이용하여 웹에 온습도 센서 데이터 및 웹에서 소리 감지 후 디바이스 led ON/OFF제어

### Multi Task
- timer와 외부 인터럽트를 이용하여 led 토글 중 인터럽트 시 기존 led 토글 stop 후 다른 led on/off 제어

### Free_LED_Blink
- FreeRTOS를 이용하여 Multi Task와 같이 led 토글 중 버튼 클릭 시 기존 led 토글 stop 후 다른 led on/off 제어


