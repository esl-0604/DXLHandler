/*
 * UARTHandler.cpp
 *
 *  Created on: Mar 8, 2024
 *      Author: eslee
 */


#include "UARTHandler.h"

UARTHandler::UARTHandler(UART_HandleTypeDef* huart) : _huart(huart){
	this->_ucDirectionPin = RX_DIRECTION;
}

UARTHandler::~UARTHandler() {
	// 기본 가상 소멸자 사용
}


// UART Interface ----------------------------------------------------------------------------
void UARTHandler::SetTransmitMode(){
	this->_ucDirectionPin = TX_DIRECTION;
	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, TX_DIRECTION);
}

void UARTHandler::SetRecieveMode(){
	this->SetRxBufferClear();
	this->_ucDirectionPin = RX_DIRECTION;
	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, RX_DIRECTION);
	HAL_UARTEx_ReceiveToIdle_IT(this->_huart, this->_rxBuffer, RX_BUFFER_SIZE);
}

void UARTHandler::TransmitPacket(Packet TxPacket){
	this->SetTransmitMode();
	HAL_UART_Transmit_IT(this->_huart, TxPacket.pucGetPacket(), TxPacket.usGetPacketLength());
}


// Buffer Inerface ----------------------------------------------------------------------------
uint8_t* UARTHandler::GetRxBuffer(){
	return this->_rxBuffer;
}

void UARTHandler::SetRxBufferClear(){
	memset(this->_rxBuffer, 0, RX_BUFFER_SIZE);
}







