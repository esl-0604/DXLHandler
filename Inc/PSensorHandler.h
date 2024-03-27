/*
 * PSensorHandler.h
 *
 *  Created on: Mar 27, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_PSENSORHANDLER_H_
#define ENDO_INC_PSENSORHANDLER_H_

#pragma once



#define SENSOR_NOT_DETECTED					0x00U
#define SENSOR_DETECTED							0x01U

#define HOMING_START					0x00U
#define HOMING_FAST_FAR				0x01U
#define HOMING_FAST_CLOSE			0x02U
#define HOMING_SLOW_FAR				0x03U
#define HOMING_SLOW_CLOSE			0x04U
#define HOMING_OFFSET_SET			0x05U
#define HOMING_CPLT						0x06U


#include <map>
#include <stdint.h>
#include <string.h>

using namespace std;


struct tPSensorStatus {
		volatile uint8_t ucPSensorDetectFlag = SENSOR_NOT_DETECTED;
		volatile uint8_t ucPSensorHomingState = HOMING_START;
};

class PSensorHandler {
	// Variables
	private:
		map<uint8_t, tPSensorStatus*> _mPSensorStatusList;
		uint8_t _ucTotalPSensorCnt;

	// Methods
	public:
		PSensorHandler(uint8_t ucTotalPSensorCnt);
		virtual ~PSensorHandler();


		// PSensor Status Interface ----------------------------------------------------
		uint8_t GetPSensorStatusSensorFlag(uint8_t ucID);
		uint8_t GetPSensorStatusHomingState(uint8_t ucID);
		void SetPSensorStatusSensorFlag(uint8_t ucID, uint8_t ucFlag);
		void SetPSensorStatusHomingState(uint8_t ucID, uint8_t ucHomingState);


		// PSensor Handler Interface ---------------------------------------------------
		void SetPSensorMapInit(uint8_t* pucIdList);
		void SetPSensorMapClear();

};



#endif /* ENDO_INC_PSENSORHANDLER_H_ */
