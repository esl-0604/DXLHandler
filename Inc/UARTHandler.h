/*
 * UARTHandler.h
 *
 *  Created on: Mar 8, 2024
 *      Author: eslee
 */

#ifndef ENDO_INC_UARTHANDLER_H_
#define ENDO_INC_UARTHANDLER_H_

#pragma once
#include <stdint.h>
#include "Packet.h"
#include "DXLDefine.h"

class Packet;

class UARTHandler {
  // Variables
	private:
		UART_HandleTypeDef* _huart;
		uint8_t _rxBuffer[RX_BUFFER_SIZE];
		uint8_t _ucDirectionPin;


	// Methods
	public:
		UARTHandler(UART_HandleTypeDef* huart);
		virtual ~UARTHandler();


		// UART Interface ----------------------
		void SetTransmitMode();
		void SetRecieveMode();
		void TransmitPacket(Packet TxPacket);

		// Buffer Inerface ---------------------
		uint8_t* GetRxBuffer();
		void SetRxBufferClear();
};

#endif /* ENDO_INC_UARTHANDLER_H_ */
