/*
 * DXLDefine.h
 *
 *  Created on: Mar 22, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_DXLDEFINE_H_
#define ENDO_INC_DXLDEFINE_H_

#pragma once
#include "main.h"


/* DXL E-Manual -------------------------------------------------------------------------- */

// DXL Instruction
#define PING											0x01U
#define READ											0x02U
#define WRITE											0x03U
#define REG_WRITE									0x04U
#define ACTION										0x05U
#define FACTORY_RESET							0x06U
#define REBOOT										0x08U
#define CLEAR											0x10U
#define CONTROL_TABLE_BACKUP			0x20U
#define SYNC_READ									0x82U
#define SYNC_WRITE								0x83U
#define FAST_SYNC_READ						0x8AU
#define BULK_READ									0x92U
#define BULK_WRITE								0x93U
#define FAST_BULK_READ						0x9AU

// DXL Control Table EEPROM
#define ID_ADDR             				07    	//range 0~252
#define BUAD_RATE_ADDR           		08    	//range 1~7                   3=1Mbps
#define RETURN_DELAY_TIME_ADDR   		09    	//range 0~254                 default 250
#define DRIVE_MODE_ADDR          		10    	//range 0~5
#define OPERATING_MODE_ADDR      		11    	//
#define SECONDARY_ID_ADDR        		12    	//range 0~252                 default 255
#define PROTOCOL_TYPE_ADDR        	13    	//range 0~252                 default 255
#define HOMING_OFFSET_ADDR      		20    	//range -1,044,479~1,044,479  default 0
#define CURRENT_LIMIT_ADDR   				38
#define VELOCITY_LIMIT_ADDR   			44
#define MAX_POSITION_LIMIT_ADDR  		48    	//range 0~4,095               default 4095
#define MIN_POSITION_LIMIT_ADDR  		52    	//range 0~4,085               default 0
#define SHUTDOWN_ADDR            		63   	 	//default 52

// DXL Control Table RAM
#define TORQUE_ENABLE_ADDR          64    	//Range 0: OFF 1: ON
#define LED_ADDR                   	65    	//Range 0: OFF 1: ON
#define STATUS_RETURN_LEVEL_ADDR   	68    	//Range 0~2
#define POSITION_D_GAIN_ADDR				80
#define POSITION_I_GAIN_ADDR       	82    	//Range 0~16,383
#define POSITION_P_GAIN_ADDR				84
#define GOAL_PWM_ADDR								100 		//2BYTE
#define GOAL_CURRENT_ADDR						102 		//2BYTE
#define GOAL_VELOCITY_ADDR         	104   	//Range -330~330
#define PROFILE_ACCELERATION_ADDR  	108
#define PROFILE_VELOCITY_ADDR      	112
#define GOAL_POSITION_ADDR         	116 	  //Range Min Position Limit(52)~Max Position Limit(48)
#define MOVING_ADDR				        	122
#define PRESENT_CURRENT_ADDR       	126
#define PRESENT_VELOCITY_ADDR      	128
#define PRESENT_POSITION_ADDR      	132
#define VELOCITY_TRAJECTORY_ADDR   	136
#define POSITION_TRAJECTORY_ADDR   	140
/* DXL E-Manual -------------------------------------------------------------------------- */



/* DXL Handler --------------------------------------------------------------------------- */

// DXL State
#define DXL_IDLE							0x00U
#define DXL_TRANSMIT_WAIT			0x01U
#define DXL_TRANSMIT_ERR			0x02U
#define DXL_RECIEVE_WAIT			0x03U
#define DXL_RECIEVE_PARSING		0x04U
#define DXL_RECIEVE_ERR				0x05U
#define DXL_CPLT							0x06U
#define DXL_REGISTER_ERR			0x07U

// Parsing Type
#define NO_INST									0x00U
#define P_WRITE									0x01U
#define P_SYNC_WRITE						0x02U
#define P_PING									0x03U
#define P_OPERATING_MODE				0x04U
#define P_HOMING_OFFSET					0x05U
#define P_CURRENT_LIMIT					0x06U
#define P_VELOCITY_LIMIT				0x07U
#define P_MAX_POSITION_LIMIT		0x08U
#define P_MIN_POSITION_LIMIT		0x09U
#define P_TORQUE_ENABLE					0x0AU
#define P_LED										0x0BU
#define P_STATUS_RETURN_LEVEL		0x0CU
#define P_GOAL_CURRENT					0x0DU
#define P_GOAL_VELOCITY					0x0EU
#define P_GOAL_POSITION					0x0FU
#define P_MOVING								0x10U
#define P_PRESENT_CURRENT				0x11U
#define P_PRESENT_VELOCITY			0x12U
#define P_PRESENT_POSITION			0x13U

// DXL Packet Index
#define DXL_PACKET_ID								4
#define DXL_PACKET_LEN							5
#define DXL_PACKET_INST							7
#define DXL_PACKET_PARAMS						8
#define DXL_STATUS_PACKET_ERR				8
#define DXL_STATUS_PACKET_PARAMS		9

// DXL Target Byte
#define DXL_DATA_1_BYTE 			1
#define DXL_DATA_2_BYTE 			2
#define DXL_DATA_4_BYTE 			4
#define DXL_SHIFT_8_BIT				8

// System Interface
#define DXL_TIMEOUT								100
#define DXL_ENABLE								1
#define DXL_DISABLE								0
/* DXL Handler --------------------------------------------------------------------------- */



/* DXL Protocol -------------------------------------------------------------------------- */

// DXL Broadcast
#define BROADCAST_ID							0xFE

// DXL Packet Length
#define DXL_PING_PACKET_LENGTH			3
#define DXL_READ_PACKET_LENGTH			7

// DXL Packet Size
#define DXL_PACKET_MAX_SIZE 					256
#define DXL_PACKET_HEADER_SIZE				4
#define DXL_PACKET_ID_SIZE						1
#define DXL_PACKET_LENGTH_SIZE				2
#define DXL_PACKET_INST_SIZE					1
#define DXL_PACKET_TARGET_ADDR_SIZE		2
#define DXL_PACKET_TARGET_BYTE_SIZE		2
#define DXL_PACKET_CRC_SIZE						2
/* DXL Protocol -------------------------------------------------------------------------- */



/* UART Handler -------------------------------------------------------------------------- */

// UART Interface
#define TX_DIRECTION			GPIO_PIN_SET
#define RX_DIRECTION			GPIO_PIN_RESET
#define RX_BUFFER_SIZE		40
/* UART Handler -------------------------------------------------------------------------- */




#endif /* ENDO_INC_DXLDEFINE_H_ */
