# DXL Bare Metal Base Module - V2  


작성자 : 이은상

베어메탈 환경에서 DXL(다이나믹셀)을 제어할 수 있는 가장 기초가 되는 시스템.

싱글톤 패턴이 적용되었고, V1 에서는 사용자가 직접 접하는 함수는 **일반화** / **특수화** 방식 중 특수화 방식을, 보이지 않는 부분에 있어서는 일반화 방식을 채택하였다.

새롭게 추가된 V2에서는 사용자 인터페이스도 일반화 방식을 채택하여, 좀더 코드 사이즈를 경량화 하였다.

```
일반화 : 함수를 정의함에 있어서, 공통 부분을 캡슐화 하여 인자를 통해 다양한 케이스를 처리하도록 설계한 방식 
특수화 : 함수를 정의함에 있어서, 모든 케이스에 대하여 전부 함수를 작성하여 각각 명확한 1대1 대응이 되도록 설계한 방식
```

해당 시스템의 기본적인 컨셉은 거대한 Task Handler라는 단일 객체에 사용하는 외부 자원들을 연결하고, Task Handler 내부를 사용자가 직접 커스텀하여 원하는 Task를 작성하고 main.c에서 호출하는 방식이다. 

(Task Handler와 DXL Handler는 동기적인 설계로 최대한 사용자 친화적인 인터페이스를 구성하는데 집중하였고, 이하 Handler에서는 비동기적인 설계로 최대한 자원을 효율적으로 사용하도록 작성하려고 노력하였다. 만약, 이하 Handler에서 수정사항이 발생할 경우에는 개별 커스텀 보다는 업데이트를 통한 버전관리를 지향한다. )

이 글에서는 총 4가지 차례로 설명한다.

[**1) 전체 시스템 구성 및 흐름**](#전체-시스템-구성)  

[**2) main.c 세팅 방법**](#mainc-세팅)

[**3) Task Handler 커스텀 방법**](#task-handler-커스텀)

[**4) 각 Class 별 멤버 변수 및 함수**](#각-class-별-멤버-변수-및-함수)



## 전체 시스템 구성

전체 시스템은 그림과 같이 구성되어 있다. 

<img width=400 alt="SystemStructIMG" src="https://www.dropbox.com/scl/fi/nc2amt7l3uc8i13e330r8/.png?rlkey=r69x28xeqdro6jigrxmrboa77&raw=1">

싱글톤 패턴이기 때문에, 모든 객체는 단 한개씩만 존재한다. 가장 외부에 TaskHandler 객체가 존재하며 내부에는 크게 DXLHandler와 PSensorHandler가 있으며, 이들은 각각 DXL 모터와 Photo 센서의 제어를 담당한다. 

DXLHandler 안에는 다시 DXLProtocol, UARTHandler 등이 존재한다.
 
각 객체는 자신의 고유한 기능을 수행하며, 그 목적에 맞는 인터페이스만이 존재한다. 

* **Task Handler** : 크게 Main_Process() 와 GPIO_Process() 2개의 인터페이스를 가지며, Main_Process가 전체 프로세스의 진행을 맡고, GPIO_Process는 각 목적에 맞게 GPIO_Input(), GPIO_PWM() 등 여러 개의 인터페이스로 구성되어 있다.

* **DXL Handler** : 시스템에 연결되어 있는 모든 DXL을 제어할 수 있는 인터페이스를 가지며, 동작의 명확성을 위해 컨트롤 테이블에 해당하는 모든 명령이 각 함수로 작성되어 있다. 

* **DXL Protocol** : DXL Handler 내부 객체로, DXL Handler가 명령을 내리면 해당 Instruction Packet을 만들어 반환하는 역할을 한다. 이후 수신된 Packet의 CRC 체크로 담당한다. 

* **UART Handler** : DXL Handler 내부 객체로, DXL Protocol이 반환한 Packet을 UART 통신을 통해 송신하고, 내부 버퍼에 수신 Packet을 저장하는 역할을 한다. UART 통신 모드 또한 담당한다. 

* **Packet** : Packet 정보를 좀더 효율적으로 담기위해 정의된 객체로, 필요 시 할당하여 사용한 뒤에 즉시 메모리에서 해제되어 소멸한다. 

* **DXLDefine.h** : DXL Handler에서 사용하는 모든 상수 매크로를 정의해둔 파일

* **PSensor Handler** : Homing에 필요한 모든 Photo Sensor들을 관리하는 인터페이스를 가지며, 각 Photo Sensor들은 하나의 DXL과 맵핑된다. 각 Sensor 별로 현재 감지 여부와 Homing 진행 단계를 관리한다. 



## 전체 시스템 흐름

전체 시스템의 흐름은 그림과 같이 진행된다.

<img width=700 alt="SystemFlowIMG" src="https://www.dropbox.com/scl/fi/qv2s8ergv3yupxxwjcj9y/.png?rlkey=ywt9krs1im3dcb1k81jwzhxts&raw=1">

시스템의 큰 주기는 Task Handler의 Main_Process() 함수 내부에서 진행된다. 해당 함수는 _ucMainState를 기준으로 case를 구분하여 진행되는데, 이는 크게 6단계로 구분된다. 

(아래 순서는 기본적인 가이드라인일 뿐이니, 자신의 Task에 맞게 커스텀하면 된다. )

##### MAIN_OFF
시스템의 전원이 꺼져있는 상태이다. 전원이 들어옴과 동시에 DXL Handler 객체와 PSensor Handler 객체를 초기화 및 등록하고, 완료 시 MAIN_INIT 단계로 진입한다. <br>  

##### MAIN_INIT
DXL의 ERROM / RAM 영역을 세팅한다. 이 경우 SyncWriteRead 함수를 통해 한번에 세팅 및 확인하고, 완료 시 MAIN_IDLE 단계로 진입한다. <br>  

##### MAIN_IDLE
GPIO Input에 따라 세워진 Flag를 인식하여, MAIN_HOMING 또는 MAIN_OPERATION 단계로 진입한다. <br> (경우에 따라, MAIN_INIT 이후 자동으로 MAIN_HOMING을 거친 뒤 MAIN_IDLE로 진입하는 경우도 있다. 이는 사용자의 커스텀을 따른다.) <br>  

##### MAIN_HOMING
각 DXL들을 Homimg 시킨다. 모든 DXL들은 대응되는 각 Photo Sensor와 매칭되어 Homing을 하며, 각각 독립적으로 Homimg을 수행한다. Homing은 크게 7단계로 나뉜다. <br>
>***HOMING_START*** : DXL을 속도제어 모드로 세팅하고, 현재 Photo Sensor에 감지되는 물체가 있는지 여부를 판단하여, HOMING_FAST_FAR / HOMING_FAST_CLOSE 단계로 진입한다. <br>  
***HOMING_FAST_FAR*** : Photo Sensor에 물체가 감지되는 경우, 빠르게 멀어지도록 모터를 돌리다가 Sensor에서 물치가 감지되지 않는 경우, HOMING_SLOW_CLOSE 단계로 진입한다. <br>  
***HOMING_FAST_CLOSE*** : Photo Sensor에 물체가 감지되지 않는 경우, 빠르게 가까워지도록 모터를 돌리다가 Sensor에서 물치가 감지되는 경우, HOMING_SLOW_FAR 단계로 진입한다. <br>  
***HOMING_SLOW_FAR*** : 매우 천천히 멀어지도록 모터를 돌리다가 Sensor에서 물치가 감지되지 않는 경우, HOMING_SLOW_CLOSE 단계로 진입한다. <br>  
***HOMING_SLOW_CLOSE*** : 매우 천천히 가까워지도록 모터를 돌리다가 Sensor에서 물치가 감지되는 경우, HOMING_OFFSET_SET 단계로 진입한다. <br>  
***HOMING_OFFSET_SET*** : 현재 모터 위상에서 position 값이 0이 되도록 Homing Offset 값을 설정한 뒤, Present Postion 값이 0으로 확인되는 경우, HOMING_CPLT 단계로 진입한다. <br>  
***HOMING_CPLT*** : DXL을 다시 위상제어 모드로 세팅하고, 1을 반환한다. <br>  

##### MAIN_OPERATION
각 DXL들이 사용자가 입력한 작업을 수행한다. 사용자가 가장 직접적으로 커스텀해야 하는 부분이다. Operation의 단계는 사용자에 의해 자유롭게 결정되나, 기본적으로는 4단계로 나뉜다. <br>  
>***OPERATION_START*** : Operation에서 작업을 수행하기 위한 DXL 세팅을 한다. 완료 시 OPERATION_BUSY로 진입한다. <br>  
***OPERATION_BUSY*** : 작업을 수행한다. 완료 혹은 (외부에 의한)중단 시 OPERATION_STOP로 진입한다. <br>  
***OPERATION_STOP*** : DXL 구동을 멈추고 세팅을 다시 초기 상태로 돌린다. 완료 시 OPERATION_CPLT로 진입한다. <br>  
***OPERATION_CPLT*** : Operation이 완료되었으므로, 1을 반환한다. <br>  

##### MAIN_EXIT
종료 버튼을 누른 경우(SYSTEM_END_FLAG가 세워진 경우), 시스템을 강제로 MAIN_EXIT로 이동시키며, 수행 중이던 모든 작업을 중단하고, DXL Handler, PSensor Handler를 메모리에서 해제시킨 뒤, MAIN_OFF 단계로 다시 진입한다. <br>  

--- 

각 단계에서는 사용자가 작성한 DXL 제어 코드가 수행된다. 만약, INIT에서 DXL들의 OperatingMode를 Write하고 Read하는 명령을 내리고자 한다면, DXL Handler의 SyncWriteReadOperatingMode() 함수를 호출하면 된다. 

해당 명령이 호출되면, 차례로 DXL Protocol이 해당 Instruction Packet을 반환하여 UART Handler에게 전달하고, UART Handler는 DIR pin을 송신 모드로 세팅하고 해당 Packet을 송신한 뒤, 다시 수신모드로 돌아와 대기한다.

Status Packet이 UART Handler의 버퍼에 도착하면, DXL Protocol은 이 패킷의 CRC 체크를 한 뒤, 유효하다면 DXL Handler는 Packet 파싱을 시작한다. 

해당 Status Packet의 파싱이 완료되면, 처리 완료 Flag를 세우고 1개의 Status Packet이 처리 완료되었음을 알린다.

SyncRead의 경우, 여러 개의 Status Packet이 전달되므로 도착해야하는 모든 Status Packet이 도착하여 처리될 때까지 DXL Handler가 loop 안에서 기다리다가 전부 완료되면 Task Handler에서 호출한 명령이 전부 처리되었음을 알리며 함수가 종료된다. 

```
SyncRead의 연속적인 Status Packet 처리를 하는 동안, DXL Handler가 loop 안에서 기다리는 동기적인 방식이기 때문에, DXL Handler의 Main_Process의 실행주기가 DXL의 전체 응답시간보다 짧아서는 안된다.
만약, Main_Process의 실행주기를 늘릴 수 없는 상황이라면 해당 시스템을 사용하는 것은 다소 위험하다. 
그럼에도 불구하고 이렇게 설계한 이유는 이 시스템을 사용하는 환경이 생산 지그이기에 실행주기를 여유롭게 설정할 수 있다는 점, 그리고 Task Handler 측에서 굉장히 직관적인 사용자 인터페이스를 사용할 수 있다는 점 때문이다. 
Task Handler에서 DXL Handler에게 명령을 내릴 때는 단 하나의 함수만 호출하면 되고, 만약 연속적으로 명령을 내릴 때는 단순히 순차적으로 원하는 명령을 호출하기만 하면 된다. 이러한 동기적인 인터페이스는 사용자가 커스텀 하기에 굉장히 용이할 수 있겠다고 판단하였다.
```


## main.c 세팅

해당 시스템을 베어메탈 환경에서 사용하기 위해서는 총 3가지 세팅이 필요하다. 

##### 전역 변수 선언

```
TaskHandler g_TaskHandler(&huart);
```

필요한 전역변수는 TaskHandler 객체 단 하나이다. 그리고 해당 객체에게 UART 통신에 사용할  huart의 주소값만 넘기면 된다.

---

##### TIM Interupt Callback 함수 세팅

```
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
	if(htim->Instance == TIM6) {
		g_TaskHandler.GPIO_Input();
		g_TaskHandler.GPIO_PWM(&POWER_PWM, &HOMING_PWM, &OPER_PWM);

	}

	if(htim->Instance == TIM7) {
		g_TaskHandler.Main_Process();

	}
}
```

위의 코드처럼 원하는 주기의 Timer에 Main_Process()를 호출해주고, 필요한 경우 다른 주기의 Timer에 GPIO 관련 함수들을 호출해주면 된다.

---

##### UART Tx/Rx Callback 함수 세팅

```
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART1){
		g_TaskHandler.TransmitCplt();
	}
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if(huart->Instance == USART1){
		g_TaskHandler.RecieveCplt(Size);
	}
}
```

위의 코드처럼 TxCpltCallback 함수에서는 Task Handler의 TransmitCplt(), RxEventCallback 함수에서는 RecieveCplt(Size) 를 호출해주면 된다. 

이 시스템에서는 UART 송신 함수로 HAL_UART_Transmit_IT, 수신 함수로 HAL_UARTEx_ReceiveToIdle_IT를 사용하기 때문에 이에 알맞는 콜백 함수를 사용해주면 된다.

## Task Handler 커스텀

위의 3가지 세팅만 하면, main.c에서 더 이상 작업해야할 부분은 없다.

이제부터는 Task Handler 내부만 사용자의 목적에 맞게 커스텀 해주면 된다. 커스텀은 기본적으로 TaskHandler.h와 TaskHandler.cpp 두 파일에서만 진행하면 된다.

(필요 시, DXLHandler.h/.cpp 혹은 DXLDefine.h을 커스텀해도 되지만, 이 경우엔 왠만하면 업데이트를 통해 버전을 바꾸어 관리하는 것을 추천한다. )

Task Handler에서 커스텀 할 부분은 크게 TaskHandler(), GPIO_관련 함수들, Main_Process() 및 관련 함수들로 총 세 부분 이다. 

* **TaskHandler()** : 이 함수는 생성자로 시스템에서 사용할 DXL ID List와 Photo Sensor ID List를 등록하는 파트가 포함되어 있다. 

```
/* USER CODE BEGIN DXL_ID List */
    _pucDXLIDList[0] = DXL_1;
    _pucDXLIDList[1] = DXL_2;
    _pucDXLIDList[2] = DXL_3;
    _pucDXLIDList[3] = DXL_4;
/* USER CODE END DXL_ID List */

/* USER CODE BEGIN PSensor_ID List */
    _pucPSencorIDList[0] = PSENSOR_1;
    _pucPSencorIDList[1] = PSENSOR_2;
/* USER CODE END PSensor_ID List */
```

위와 같이 주석처리로 되어있는 부분에 사용할 DXL, Photo Sensor의 ID를 입력하면 된다. 각 매크로 상수들은 TaskHandler.h에서 추가 및 수정할 수 있다.

---

다음은 GPIO_관련 함수들이다. 기본적으로 작성되어 있는 GPIO_ 관련 함수는 GPIO_Input(), GPIO_PWM() 이다.

* **GPIO_Input()** : PCB에서 연결되어 있는 Input Pin들을 통한 입력값을 읽어서 Flag를 세우는 함수. 예를 들어, PowerSwitch, HomingSwitch, OperationSwitch 등이 있다.

* **GPIO_PWM()** : PCB에서 연결되어 있는 PWM Pin들을 통해 출력값을 조정하여 LED 등을 제어하는 함수. 주로, Main_Process의 진행상황을 알려주기 위해 LED를 키고 끄는 동작을 한다.

---

다음으로 커스텀 할 부분은 Main_Process()와 관련 함수들이다.

기본적으로 제공되는 함수는 Main_Process(), PSensorHoming(), DXLOperation() 이다. 이 함수들 내부에도 기본적으로 제공되는 구조가 있긴 하나, 필요에 따라 단계와 내용을 커스텀 하면된다.

* **Main_Process()** : MainState에 따라 적절한 Process를 진행시키는 함수. 기본적으로 제공되는 State는 MAIN_OFF / MAIN_INIT / MAIN_IDLE / MAIN_HOMING / MAIN_OPERATION / MAIN_EXIT 이다. 각 State에 따라 switch 문으로 case가 구분되어 작성된다. 각 case 별로 내부에서 USER CODE BEGIN / USER CODE END 사이에 코드를 작성하여 커스텀 할 수 있다.

* **PSensorHoming()** : Main_Process() 안에서 MAIN_HOMING 단계에서 실행되는 함수. 기본적으로 HOMING_START -> HOMING_FAST_FAR / HOMING_FAST_CLOSE -> HOMING_SLOW_FAR / HOMING_SLOW_CLOSE -> HOMING_OFFSET_SET -> HOMING_CPLT 순서로 진행되며, FAST 단계에서 조금 빠른 속도로 원점을 1차적으로 맞추고, SLOW 단계에서 매우 느린 속도로 정밀하게 원점을 맞춘다.

* **DXLOperation()** : Main_Process() 안에서 MAIN_OPERATION 단계에서 실행되는 함수. 기본적으로 OPERATION_START -> OPERATION_BUSY -> OPERATION_STOP -> OPERATION_CPLT 순서로 진행되며, 시스템이 본격적으로 수행해야 하는 Task에 대해 자유롭게 작성하면 된다.

 

## 각 Class 별 멤버 변수 및 함수

##### TaskHandler.h / TaksHandler.cpp  

시스템에게 할당할 Task의 진행을 관리하는 객체

해당 파일에서 USER CODE BEGIN / USER CODE END 사이 부분을 수정하여 사용자가 커스텀 할 수 있다.

(필요한 경우, 해당 주석 외부에서도 코드를 수정할 수 있다. 주석은 단지 임시 가이드라인일 뿐...)

Task를 할당할 때는 _DXLHandler의 Read, Write, SyncRead, SyncWrite, WriteRead, SyncWriteRead 등의 함수를 활용하여 작성한다.


|타입|멤버|설명|
|:---|:---|:---|
|DXLHandler 객체        |_DXLHandler                |Dynamixel 인터페이스를 관리하는 DXLHandler 객체            
|PSensorHandler 객체    |_PSensorHandler            |Photo Sensor 인터페이스를 관리하는 PSensorHandler 객체    
|uint8_t* 변수          |_pucDXLIDList              |DXL ID List 배열                                       
|uint8_t* 변수          |_pucPSencorIDList          |Photo Sensor ID List 배열                               
|uint8_t 변수           |_ucMainState               |시스템 전체 Process State 변수                           
|uint8_t 변수           |_ucOperationState          |Operation 시, Operatin Process State 변수              
|uint8_t 변수           |_ucSystemFlag              |System의 시작과 완료, 변경을 알리는 Flag                 
|uint8_t 변수           |_ucHomingFlag              |Homing의 시작과 완료, 변경를 알리는 Flag                  
|uint8_t 변수           |_ucOperationFlag           |Operation의 시작과 완료, 변경를 알리는 Flag
|void 함수              |TransmitCplt()                      |UART 송신 완료 시, 호출되는 함수. HAL_UART_TxCpltCallback() 내부에서 호출한다.                   
|void 함수              |RecieveCplt(uint16_t usPacketSize)  |UART 신 완료 시, 호출되는 함수. HAL_UARTEx_RxEventCallback() 내부에서 호출하고, 수신된 Packet의 Size를 인자로 받는다.         
|void 함수              |DXLInit()                  |시스템이 작동됨과 동시에 _DXLHandler를 시스템에 등록시키고, 상태값들을 초기화 하는 함수
|void 함수              |DXLClear()                 |시스템이 종료됨과 동시에 _DXLHandler를 시스템에서 해제시키고, 메모리를 전부 해제시키는 함수
|void 함수              |PSensorInit()              |시스템이 작동됨과 동시에 _PSensorHandler를 시스템에 등록시키고, 상태값들을 초기화 하는 함수
|void 함수              |PSensorClear()             |시스템이 종료됨과 동시에 _PSensorHandler를 시스템에서 해제시키고, 메모리를 전부 해제시키는 함수          
|void 함수              |Main_Process()             |시스템의 Main Process를 실행하는 함수. HAL_TIM_PeriodElapsedCallback 함수의 원하는 Timer에서 호출한다. _ucMainState에 따라 대응되는 함수를 호출한다.
|void 함수              |GPIO_Input()               |시스템의 GPIO Input을 감지하고, Flag를 세우는 함수. HAL_TIM_PeriodElapsedCallback 함수의 원하는 Timer에서 호출한다.   
|void 함수              |GPIO_PWM(volatile uint32_t* pnPowerPWM, ... )                 |시스템의 MainState, Flag에 따라 여러 PWM을 출력하는 함수. HAL_TIM_PeriodElapsedCallback 함수의 원하는 Timer에서 호출한다.   
|uint8_t 함수           |PSensorHoming(uint8_t ucPSensorID, uint8_t ucDXLID)   |연결되어 있는 Photo Sensor와 DXL의 ID를 인자로 받아 Homing 시키는 함수. Main_Process()의 Homing State에서 호출된다.
|uint8_t 함수           |DXLOperation()             |DXL에게 특정 Operation을 수행시키는 함수. Main_Process()의 Operation State에서 호출된다.             

---

##### DXLHandler.h / DXLHandler.cpp

DXL을 제어할 수 있는 모든 인터페이스를 관리하는 객체

하위에 총 5개의 모듈(파일)을 포함하고 있다.

> **DXLDefine.h** : DXL 제어에 필요한 모든 상수들을 정의해둔 파일. 사용자에 의해 커스텀 될 부분이 거의 없지만, 필요 시에는 커스텀 할 수 있다. <br>  
  **DXLProtocol.h / DXLProtocol.cpp** : 전달된 Target Address 인자에 해당하는 DXL Protocol Packet을 생성해서 반환하고, 수신된 Packet의 CRC 체크를 담당하는 객체. <br>  
  **UARTHandler.h / UARTHandler.cpp** : UART 통신을 통해 전달된 Packet의 송수신과 DIR_PIN 모드를 담당하는 객체 <br>  
  **Packet.h / Packet.cpp** : DXL Protocol에 의해 생성되는 Packet 객체. 필요할 때마다 생성되며, 송신 뒤 곧바로 메모리에서 해제되어 소멸된다.  <br>  
  **tDXLStatus (structure)** : DXL Status 값들을 저장하는 구조체. EEPROM/RAM 값들을 변수로 가지고 있다.
  

|타입|멤버|설명|
|:---|:---|:---|
|UARTHandler 객체       |_UARTHandler                       |UART 인터페이스를 관리하는 객체            
|DXLProtocol 객체       |_DXLProtocol                       |DXL Protocol Packet을 생성 및 관리하는 객체    
|map<uint8_t, tDXLStatus*> |_mDXLStatusList                 |DXLStatus 값들을 저장하고 있는 배열. DXL의 Id값을 key 값으로 지닌다.                                   
|uint8_t 변수           |_ucTotalDXLCnt                     |시스템에 등록되어 있는 DXL 개수                              
|uint8_t 변수           |_ucDxlState                        |DXL Handler의 상태를 나타내는 변수                          
|uint8_t 변수           |_ucParsingType                     |Read Packet 송신 후 수신된 Packet을 파싱할 때, Read Packet에 전달되었던 Target Address 값 
|uint8_t 변수           |_ucTxIdCnt                         |Packet 송신 시에 전달된 DXL ID 개수. 수신되어야할 패킷 개수를 판단하는 기준이 됨.                 
|uint8_t 변수           |_ucRxIdCnt                         |현재까지 수신되어 파싱까지 완료된 패킷의 개수. _ucTxIdCnt와 동일값이 될 때까지 수신모드를 유지함.
|void 함수              |SetDXLInit()                       |DXL Handler 내부 변수들을 전부 초기화 하는 함수                
|void 함수              |SetDXLMapInit(uint8_t* pucDXLIdList)  |_mDXLStatusList에 전달된 pucDXLIdList 있는 ID를 키값으로 배열을 채우는 함수
|void 함수              |SetDXLMapClear()                   |_mDXLStatusList에 저장되어 있는 Status 값들을 전부 제거하고, 할당된 메모리를 해제하는 함수
|void 함수              |SetDxlState(uint8_t ucDxlState)    |_ucDxlState 값을 업데이트 하는 함수
|void 함수              |ParsingRxData()                    |수신된 RxPacket을 파싱하여 필요한 값을 추출한 뒤, _mDXLStatusList에 저장하는 함수
|uint8_t 함수           |GetDxlState()                      |_ucDxlState 값을 읽어오는 함수
|int32_t 함수           |CalculateParams(uint8_t* pucTargetParams, uint8_t ucTargetByte) |ParsingRxData()에서 호출하는 함수. 실제로 Packet의 Params 부분을 전달받아 값을 추출하는 함수
|void 함수              |TransmitPacketCplt()               |Task Handler의 TransmitCplt()에서 호출하는 함수. UARTHandler를 수신모드로 바꾸고, _ucDxlState를 수신대기 상태로 업데이트 한다.
|void 함수              |RecievePacketCplt(uint16_t Size)   |Task Handler의 RecieveCplt()에서 호출하는 함수. CRC 체크를 통과한 경우, ParsingRxData()를 호출하여 파싱을 진행한다.
|void 함수              |WaitUntilCplt()                    |Packet을 송신하고, 해당 Packet에 대한 수신 Packet이 올 때까지 기다리는 함수. 이를 통해 Task Handler 측에서 굉장히 직관적인 동기적 인터페이스를 조성할 수 있다. 다만, 기다리는 시간이 Process 주기에 영향을 주지 않도록 Timeout을 체크해야한다.
|void 함수              |TransmitAndWaitUntilCplt(Packet TxPacket)    |Packet을 송신하고, 수신이 완전히 처리될 때까지 기다리는 함수. 내부에서 Packet 송신 함수, WaitUntilCplt() 등을 호출한다. 
|uint8_t / int32_t 함수 |GetDXLStatus_______(uint8_t ucID)  |전달된 DXL ID값으로 _mDXLStatusList에서 특정 Status 값을 읽어오는 함수
|void 함수              |SyncPing()                         |SyncPing Packet을 송신하는 함수
|void 함수              |Ping(uint8_t ucID)                 |Ping Packet을 송신하는 함수
|int32_t 함수           |Read_______(uint8_t ucID)          |전달된 DXL ID값으로 특정 Address에 해당하는 Read Packet을 송신하고 그 값을 읽어와 반환하는 함수
|void 함수              |Write_______(uint8_t ucID, int32_t nTargetParams)          |전달된 DXL ID값과 Params 값으로 특정 Address에 해당하는 Write Packet을 송신하는 함수
|vector<int32_t> 함수   |SyncRead_______(uint8_t ucIdNum, uint8_t* pucIdList)       |전달된 DXL IdNum값과 IdList 값으로 특정 Address에 해당하는 Sync Read Packet을 송신하고 그 값을 읽어와 vector로 반환하는 함수. 반환형은 {DXL_1_ID, DXL_1_Params, DXL_2_ID, DXL_2_Params, ...} 형태로 반환된다.
|void 함수              |SyncWrite_______(uint8_t ucIdNum, int32_t* pnTargetParams) |전달된 DXL IdNum값과 Params 값으로 특정 Address에 해당하는 Sync Write Packet을 송신하는 함수
|uint8_t 함수           |WriteRead_______(uint8_t ucID, int32_t nTargetParams, uint8_t ucRetry)      |전달된 DXL ID값과 Params 값으로 특정 Address에 해당하는 Write Packet을 송신하고, 실제로 그 값이 제대로 씌여졌는지 다시 Read Packet을 송신하여 확인하는 함수. 확인 실패시, 전달된 ucRetry 횟수만큼 반복하며, 해당 횟수 안에 확인을 실패할 경우, Err 값을 반환한다.
|uint8_t 함수           |SyncWriteRead_______(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry)      |전달된 DXL IdNum값과 Params 값으로 특정 Address에 해당하는 Sync Write Packet을 송신하고, 실제로 그 값이 제대로 씌여졌는지 다시 Sync Read Packet을 송신하여 확인하는 함수. 확인 실패시, 전달된 ucRetry 횟수만큼 반복하며, 해당 횟수 안에 확인을 실패할 경우, Err 값을 반환한다.
|uint8_t 함수           |WriteReadTemplete(uint8_t ucID, int32_t nTargetParams, uint8_t ucRetry, WriteFunction WriteFunc, ReadFunction ReadFunc) |모든 WriteRead 함수의 공통부분을 추출한 Templete 함수. retry loop문이 포함되어 있다.
|uint8_t 함수           |SyncWriteReadTemplete(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucRetry, SyncWriteFunction SyncWriteFunc, SyncReadFunction SyncReadFunc) |모든 SyncWriteRead 함수의 공통부분을 추출한 Templete 함수. retry loop문이 포함되어 있다.

---

##### DXLHandlerV2.h / DXLHandlerV2.cpp

DXL을 제어할 수 있는 모든 인터페이스를 관리하는 객체의 두번째 버전

Read, Write, Get 함수 등이 전부 모듈화된 CMD 함수로 되어있으며, 사용자가 원하는 파라미터를 전달하는 형태로 구성되었다. 
  

|타입|멤버|설명|
|:---|:---|:---|
|UARTHandler 객체       |_UARTHandler                       |UART 인터페이스를 관리하는 객체            
|DXLProtocol 객체       |_DXLProtocol                       |DXL Protocol Packet을 생성 및 관리하는 객체    
|map<uint8_t, tDXLStatus*> |_mDXLStatusList                 |DXLStatus 값들을 저장하고 있는 배열. DXL의 Id값을 key 값으로 지닌다.                                   
|uint8_t 변수           |_ucTotalDXLCnt                     |시스템에 등록되어 있는 DXL 개수                              
|uint8_t 변수           |_ucDxlState                        |DXL Handler의 상태를 나타내는 변수                          
|uint8_t 변수           |_ucParsingType                     |Read Packet 송신 후 수신된 Packet을 파싱할 때, Read Packet에 전달되었던 Target Address 값 
|uint8_t 변수           |_ucTxIdCnt                         |Packet 송신 시에 전달된 DXL ID 개수. 수신되어야할 패킷 개수를 판단하는 기준이 됨.                 
|uint8_t 변수           |_ucRxIdCnt                         |현재까지 수신되어 파싱까지 완료된 패킷의 개수. _ucTxIdCnt와 동일값이 될 때까지 수신모드를 유지함.
|void 함수              |SetDXLInit()                       |DXL Handler 내부 변수들을 전부 초기화 하는 함수                
|void 함수              |SetDXLMapInit(uint8_t* pucDXLIdList)  |_mDXLStatusList에 전달된 pucDXLIdList 있는 ID를 키값으로 배열을 채우는 함수
|void 함수              |SetDXLMapClear()                   |_mDXLStatusList에 저장되어 있는 Status 값들을 전부 제거하고, 할당된 메모리를 해제하는 함수
|void 함수              |SetDxlState(uint8_t ucDxlState)    |_ucDxlState 값을 업데이트 하는 함수
|void 함수              |ParsingRxData()                    |수신된 RxPacket을 파싱하여 필요한 값을 추출한 뒤, _mDXLStatusList에 저장하는 함수
|uint8_t 함수           |GetDxlState()                      |_ucDxlState 값을 읽어오는 함수
|int32_t 함수           |CalculateParams(uint8_t* pucTargetParams, uint8_t ucTargetByte) |ParsingRxData()에서 호출하는 함수. 실제로 Packet의 Params 부분을 전달받아 값을 추출하는 함수
|void 함수              |TransmitPacketCplt()               |Task Handler의 TransmitCplt()에서 호출하는 함수. UARTHandler를 수신모드로 바꾸고, _ucDxlState를 수신대기 상태로 업데이트 한다.
|void 함수              |RecievePacketCplt(uint16_t Size)   |Task Handler의 RecieveCplt()에서 호출하는 함수. CRC 체크를 통과한 경우, ParsingRxData()를 호출하여 파싱을 진행한다.
|void 함수              |WaitUntilCplt()                    |Packet을 송신하고, 해당 Packet에 대한 수신 Packet이 올 때까지 기다리는 함수. 이를 통해 Task Handler 측에서 굉장히 직관적인 동기적 인터페이스를 조성할 수 있다. 다만, 기다리는 시간이 Process 주기에 영향을 주지 않도록 Timeout을 체크해야한다.
|void 함수              |TransmitAndWaitUntilCplt(Packet TxPacket)    |Packet을 송신하고, 수신이 완전히 처리될 때까지 기다리는 함수. 내부에서 Packet 송신 함수, WaitUntilCplt() 등을 호출한다. 
|uint8_t / int32_t 함수 |GetDXLStatusByTargetAddress(uint8_t ucID, uint8_t ucTargetAddress)  |전달된 DXL ID값과 TargetAddress값으로 _mDXLStatusList에서 특정 Status 값을 읽어오는 함수
|void 함수              |SyncPing()                         |SyncPing Packet을 송신하는 함수
|void 함수              |Ping(uint8_t ucID)                 |Ping Packet을 송신하는 함수
|int32_t 함수           |ReadCMD_1Byte/ReadCMD_2Byte/ReadCMD_4Byte(uint8_t ucID, uint8_t ucTargetAddress)          |전달된 DXL ID값과 TargetAddress값으로 특정 Address에 해당하는 Read Packet을 송신하고 그 값을 읽어와 반환하는 함수
|void 함수              |WriteCMD_1Byte/WriteCMD_2Byte/WriteCMD_4Byte(uint8_t ucID, int32_t nTargetParams)          |전달된 DXL ID값과 TargetAddress값, Params 값으로 특정 Address에 해당하는 Write Packet을 송신하는 함수
|vector<int32_t> 함수   |SyncReadCMD_1Byte/SyncReadCMD_2Byte/SyncReadCMD_4Byte(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress)       |전달된 DXL IdNum 값과 IdList 값, TargetAddress값으로 특정 Address에 해당하는 Sync Read Packet을 송신하고 그 값을 읽어와 vector로 반환하는 함수. 반환형은 {DXL_1_ID, DXL_1_Params, DXL_2_ID, DXL_2_Params, ...} 형태로 반환된다.
|void 함수              |SyncWriteCMD_1Byte/SyncWriteCMD_2Byte/SyncWriteCMD_4Byte(uint8_t ucIdNum, int32_t* pnTargetParams, uint8_t ucTargetAddress) |전달된 DXL IdNum 값과 Params 값, TargetAddress값으로 특정 Address에 해당하는 Sync Write Packet을 송신하는 함수
|uint8_t 함수           |WriteReadCMD_1Byte/WriteReadCMD_2Byte/WriteReadCMD_4Byte(uint8_t ucID, uint8_t ucTargetAddress, int32_t nTargetParams, uint8_t ucRetry)      |전달된 DXL ID값과 Params 값, TargetAddress값으로 특정 Address에 해당하는 Write Packet을 송신하고, 실제로 그 값이 제대로 씌여졌는지 다시 Read Packet을 송신하여 확인하는 함수. 확인 실패시, 전달된 ucRetry 횟수만큼 반복하며, 해당 횟수 안에 확인을 실패할 경우, Err 값을 반환한다.
|uint8_t 함수           |SyncWriteReadCMD_1Byte/SyncWriteReadCMD_2Byte/SyncWriteReadCMD_4Byte(uint8_t ucIdNum, uint8_t* pucIdList, int32_t* pnTargetParams, uint8_t ucTargetAddress, uint8_t ucRetry)      |전달된 DXL IdNum값과 Params 값, TargetAddress값으로 특정 Address에 해당하는 Sync Write Packet을 송신하고, 실제로 그 값이 제대로 씌여졌는지 다시 Sync Read Packet을 송신하여 확인하는 함수. 확인 실패시, 전달된 ucRetry 횟수만큼 반복하며, 해당 횟수 안에 확인을 실패할 경우, Err 값을 반환한다.

---

##### DXLProtocol.h / DXLProtocol.cpp

DXL Instruction Packet을 만들어 반환하는 객체

전달된 컨트롤 테이블 주소와 Packet 타입에 맞는 Packet을 만들어 반환하고, 송신 완료 시 해당 Packet을 소멸시킨다.

|타입|멤버|설명|
|:---|:---|:---|                                 
|uint8_t 변수           |_ucTotalLength                         |Packet의 총 길이                              
|uint8_t 변수           |_ucParamsLength                        |Packet의 Params 길이                          
|uint8_t* 변수          |_pucHeader                             |Packet의 Header 
|uint8_t 변수           |_ucID                                  |Packet의 ID       
|uint8_t* 변수          |_pucLength                             |Packet의 Length 
|uint8_t 변수           |_ucInstruction                         |Packet의 Instruction 
|uint8_t* 변수          |_pucParams                             |Packet의 Params 
|void 함수              |SetHeader()                            |Packet의 Header를 세팅하는 함수        
|void 함수              |SetID(uint8_t ucID)                    |Packet의 ID를 세팅하는 함수
|void 함수              |SetLength(uint8_t ucLength)            |Packet의 Length를 세팅하는 함수 
|void 함수              |SetInstruction(uint8_t ucInstruction)  |Packet의 Instruction를 세팅하는 함수  
|void 함수              |SetParams(uint8_t* pucParams, uint8_t ucParamsLength)      |Packet의 Params를 세팅하는 함수  
|void 함수              |SetByteStuffing(uint8_t* pucParams)    |Packet에서 ByteStuffing이 필요한 경우, 해당 세팅을 하는 함수 
|void 함수              |SetCRC(uint8_t* pucPacket)             |Packet의 CRC를 세팅하는 함수
|void 함수              |SetClear()                             |멤버 변수를 전부 메모리에서 해제해버리는 함수. 완성된 Packet 반환 이후 호출됨.
|void 함수              |pucGetPacket()                         |각 멤버 변수에 세팅된 Packet 값들을 하나로 합쳐 완성된 Packet 배열을 반환해주는 함수 
|Packet 함수            |GetTxSyncPingPacket()                  |Sync Ping Packet을 만들어 반환하는 함수
|Packet 함수            |GetTxPingPacket(uint8_t ucID)          |Ping Packet을 만들어 반환하는 함수
|Packet 함수            |GetTxReadPacket(uint8_t ucID, uint8_t ucTargetAddress, uint8_t ucTargetByte)                                      |Read Packet을 만들어 반환하는 함수
|Packet 함수            |GetTxWritePacket(uint8_t ucID, int32_t ucTargetParams, uint8_t ucTargetAddress, uint8_t ucTargetByte)             |Write Packet을 만들어 반환하는 함수
|Packet 함수            |GetTxSyncReadPacket(uint8_t ucIdNum, uint8_t* pucIdList, uint8_t ucTargetAddress, uint8_t ucTargetByte)           |Sync Read Packet을 만들어 반환하는 함수
|Packet 함수            |GetTxSyncWritePacket(uint8_t ucIdNum, int32_t* pucTargetParams, uint8_t ucTargetAddress, uint8_t ucTargetByte)    |Sync Write Packet을 만들어 반환하는 함수
|bool 함수              |bIsCrcValidate(uint8_t* rawRxPacket, uint16_t usRxPacketLength)                                                   |해당 Packet의 CRC가 유효한지 검사하는 함수
|uint16_t 함수          |CRCFunc(uint8_t* pucPacket, uint16_t usPacketLength)                                                              |CRC 값을 계산해주는 함수

---

##### UARTHandler.h / UARTHandler.cpp

UART 통신을 관리하는 객체

|타입|멤버|설명|
|:---|:---|:---|                                 
|UART_HandleTypeDef* 변수   |_huart                             |송수신에 사용할 UART 드라이버                              
|uint8_t* 변수              |_rxBuffer                          |Packet을 수신할 버퍼                          
|uint8_t 변수               |_ucDirectionPin                    |송수신 모드를 담당하는 Dir Pin       
|void 함수                  |SetTransmitMode()                  |UART 송신 모드로 Dir Pin을 세팅하는 함수        
|void 함수                  |SetRecieveMode()                   |UART 수신 모드로 Dir Pin을 세팅하는 함수  
|void 함수                  |TransmitPacket(Packet TxPacket)    |Packet을 송신하는 함수
|void 함수                  |GetRxBuffer()                      |버퍼에 저장된 값을 가져오는 함수  
|void 함수                  |SetRxBufferClear()                 |버퍼에 저장된 값을 전부 지우는 함수  
 