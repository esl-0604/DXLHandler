/*
 * Packet.cpp
 *
 *  Created on: Mar 8, 2024
 *      Author: eslee
 */


#include "Packet.h"

Packet::Packet(uint16_t usPacketMaxLength): _usPacketMaxLength(usPacketMaxLength){
	this->_usPacketLength = 0;
	this->_pucPacket = new uint8_t[usPacketMaxLength];
	memset(this->_pucPacket, 0, usPacketMaxLength);
}

Packet::~Packet(){
  delete[] this->_pucPacket;
}

uint8_t& Packet::operator[] (const uint8_t ucIndex){
	assert(ucIndex >= 0);
	assert(ucIndex < this->_usPacketMaxLength);
	return this->_pucPacket[ucIndex];
}


// Get ----------------------------------------------------------------------
uint8_t* Packet::pucGetPacket(){
    return this->_pucPacket;
}

uint16_t Packet::usGetPacketLength(){
	return this->_usPacketLength;
}


// Set ----------------------------------------------------------------------
void Packet::SetPacket(uint8_t* pucPacket, uint16_t usPacketLength){
  memcpy(this->_pucPacket, pucPacket, (int)usPacketLength);
  this->_usPacketLength = usPacketLength;
	delete[] pucPacket;
}

void Packet::SetPacketClear(uint8_t ucClearParam){
  memset(this->_pucPacket, ucClearParam, this->_usPacketMaxLength);
}
