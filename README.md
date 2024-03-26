## DXL Interface

####Packet
: 일회용 packet 객체

####UART Handler
: UART 송수신 담당

####DXL Protocol
: DXL Packet 생성 담당

####DXL Handler
: DXL 제어 Interface 총 담당 (while 문을 활용하여 모든 명령을 완료 시점까지 기다리는 형태의 동기적인 Interface)



## Task Interface

####Task Handler
: 프로세스의 모든 단계를 총 담당

####Photo Sensor
: 프로세스 단계 중 Homing 단계를 보조하는 객체
