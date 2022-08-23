#pragma once

//Packet setting
#define PACKET_SIZE         32
#define PACKET_NUMBER       8
#define PACKET_HEADER       8
#define NO_SYNC_READ        false
#define SYNC_READ           true
#define PACKET_NOTFOUND     PACKET_SIZE
#define PING_PACKET_SIZE    14
#define REBOOT_PACKET_SIZE  11
#define MAX_SYNCID_NUM      9

//Instruction Protocol
#define PING_INST           0x01U
#define READ_INST           0x02U
#define WRITE_INST          0x03U
#define REG_WRITE_INST      0x04U
#define FACTORY_RESET       0x06U
#define REBOOT              0x08U
#define DXL_CLEAR           0x10U
#define SYNC_READ_INST      0x82U
#define SYNC_WRITE_INST     0x83U
#define STNC_FAST_READ_INST 0x8AU

#define STATUS_PACKET       0x55U

//Status Return Level
#define RETURN_FOR_READ 1
#define RETURN_FOR_ALL  2

//Control Table EEPROM
#define ID_ADDR             07    //range 0~252
#define BUAD_RATE           08    //range 1~7                   3=1Mbps
#define RETURN_DELAY_TIME   09    //range 0~254                 default 250
#define DRIVE_MODE          10    //range 0~5
#define OPERATING_MODE      11    //
#define SECONDARY_ID        12    //range 0~252                 default 255
#define HOMING_OFFSET       20    //range -1,044,479~1,044,479  default 0
#define MAX_POSITION_LIMIT  48    //range 0~4,095               default 4095
#define MIN_POSITION_LIMIT  52    //range 0~4,085               default 0    
#define SHUTDOWN            63    //default 52

//Control Table RAM 
#define TORQUE_ADDR           64    //Range 0: OFF 1: ON
#define LED                   65    //Range 0: OFF 1: ON
#define STATUS_RETURN_LEVEL   68    //Range 0~2
#define POSITION_P_GAIN       84
#define POSITION_I_GAIN       82    //Range 0~16,383
#define POSITION_D_GAIN       80
#define GOAL_VELOCITY         104   //Range -330~330
#define PROFILE_ACCELERATION  108
#define PROFILE_VELOCITY      112
#define GOAL_POSITION         116   //Range Min Position Limit(52)~Max Position Limit(48)   
#define PRESENT_CURRENT       126
#define PRESENT_VELOCITY      128
#define PRESENT_POSITION      132
#define VELOCITY_TRAJECTORY   136
#define POSITION_TRAJECTORY   140

//Rxstate
#define RESETCHECK          0x0000U
#define RXSTATEMASK         0x0002U

//Broad Casting
#define BROAD_CAST_ID   0xFEU

//HOMING mode
#define HOMING_INIT                 0x00U
#define HOMING_FIRST_GOAL_POS       0x01U
#define HOMING_CHECK_POS            0x02U
#define HOMING_WAIT                 0x03U
#define HOMING_GOTO_PHOTO           0x04U
#define HOMING_CHECK_PHOTO          0x05U
#define HOMING_PHOTO_DELAY          0x06U
#define HOMING_GOTO_POSITION1       0x07U
#define HOMING_GOTO_HOME            0x08U
#define HOMING_GOTO_POSITION2       0x09U
#define HOMING_CHECK_POSITION2      0x10U
#define HOMING_GRIP_POSITION2       0x11U
#define HOMING_GRIP_INIT            0x12U
#define HOMING_PIPE_TAKE            0x13U
#define TEST_END                    0x55U
//#define HOMING_EMER_EXIT            0x08U
//#define HOMING_GOTO_CARTRIDGE       0x09U
//#define HOMING_STEP_GOING           0x10U
#define HOMING_EXIT                 0xFFU

#define HOMING_POSITION_MODE  4


#define HOMING_COMPAREOFFSET  2


//Dynamxiel Parameter
#define THRESHOLD_CURRENT 25
#define END_OF_LM_GUIDE   -160000
#define MID_OF_LM_GUIDE   -55000
#define ONE_STEP_DISTANCE 10000
//Control
#define PERIOD_NONE     0x00U
#define PERIOD_POSITION 0x01U
#define PERIOD_CURRENT  0x02U
#define PERIOD_TORQUE   0x03U

//Error Flag
#define ERROR_NONE            0x0U
#define ERROR_CRC             0x1U
#define ERROR_HEADER_NOTFOUND 0x2U

//Torque status Flag
#define TORQUE_STATUS_OFF     0x0U
#define TORQUE_STATUS_READ    0x1U
#define TORQUE_STATIUS_CHECK  0x2U
#define TORQUE_STATUS_EXIT    0x4U

//Motor OPFlag

#define MOTOR_INIT    0x7E
#define MOTOR_WAIT    0x7F
#define MOTOR1_UP     0x3F
#define MOTOR1_DOWN   0x5F
#define MOTOR2_UP     0x6F
#define MOTOR2_DOWN   0x77
#define MOTORALL_UP   0x7B
#define MOTORALL_DOWN 0x7D