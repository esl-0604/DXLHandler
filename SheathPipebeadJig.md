# SheathPipebeadJig

## 목차
1. [동작방식](#동작방식)
2. [알아야 하는 사항](#알아야-하는-사항)
2.1 [포토센서](#포토센서)

## 동작방식

- Timer11 (14KHz)인터럽트를 제어주기로 사용

- g_usMainFlag를 변수로 switch문을 통해 state machine 작성

- 0번 모터: 그립용 모터 XM530-W270-R   

- 1번 모터: 위치제어용 모터 XM430-W210-R

![시스파이프비드흐름도](/uploads/c1421d8a1f2d21b666d4b11aef09d924/시스파이프비드흐름도.jpg)

- 흐름도에서는 Slide 스위치를 체크하는 State가 있지만 모든 State에서 풀링방식으로 읽는 방식으로 동작중

## 1. g_usMainFlag의 7번 케이스에서 모터 초기화과정

![쉬스비드7번케이스](/uploads/5100b2eff70fba868b038ed75df858aa/쉬스비드7번케이스.jpg)

- Line 632 : 전원 공급시 일정 시간동안 통신레벨이 불안정하므로 딜레이

- Line 640 : 팩토리 리셋 명령 수행에 필요한 딜레이

## 2. g_usMainFlag의 6번 케이스에서 메인제어 

- g_ucMotorStateFlag로 다이나믹셀의 동작 State를 결정

- 해당 State는 아래와 같다.

**HOMING_INIT**

- 각 모터를 Multi-turn 모드로 설정하여 위치제어

- 1회전 제어시 (4095과 0을 통과하는 위치변경의 경우 원하는 방향으로 제어가 안될 수 있음)

**HOMING_FIRST_GOAL_POS**

- 각 모터를 초기위치로 이동

- 0번 모터 : g_iGripInitPos

- 1번 모터 : g_iInitPos
 
**HOMING_CHECK_PHOTO**

- Photo Sensor로 SheathPipe를 확인

**HOMING_PHOTO_DELAY**

- SheathPipe를 넣었는지 확인하는 State

- 변수 g_iPhotoDelayTime의 값동안 PhotoSensor에 Sheath Pipe가 인식되어야 함

**HOMING_GOTO_POSITION1 & HOMING_GOTO_HOME**

- 1번 모터가 초기위치에서 g_iPosGoal1로 이동하면

- 0번 모터가 Grip & Release 수행

- Grip 위치 : g_iGripPos

- Grip 위치 오프셋 : g_iGripPosOffset

- Release 위치 : g_iGripOffPos

**HOMING_GOTO_POSITION2 & HOMING_CHECK_POSITION2 & HOMING_GRIP_POSITION2**

- 0번 모터가 g_iGripOffPos 의 위치에 이동하면 

- 1번 모터가 g_iPosGoal2로 이동

- 0번 모터가 Grip & Release 수행

**HOMING_GRIP_INIT**

- 1번 모터가 g_iGripInitPos로 이동

**HOMING_PIPE_TAKE**

- _bPhotoSenor가 false면 0번 모터를 g_iGripInitPos로 이동

## 알아야 하는 사항

### 포토센서

- 포토센서의 구동전압은 5V로 설정

![포토센서데이터시트](/uploads/8a2bf40e23c5360e8f096aa65f5af739/포토센서데이터시트.png)

![포토센서데이터시트2](/uploads/aa45fa169fcd8781e23ef89972f3e4c0/포토센서데이터시트2.png)

- Load Current는 50mA로 Vcc(갈색)과 Output 1 Transistor(검은색)을 110옴으로 연결

- 50mA일 때 residual Voltage(On 상태일때 트랜지스터에서 소모하는 전압)은 0.7V

- 포토센서는 커넥터는 (Molex) 5264-04핀으로 커넥터 작업은 아래와 같음

![포토센서핀연결](/uploads/b62bbbb94e643aebbe6b668ec08d9bd7/포토센서핀연결.jpg)

- 출력은 Light On : HIGH , Light OFF : LOW

