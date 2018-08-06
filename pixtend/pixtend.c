/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <https://www.pixtend.de> or <https://www.pixtend.com>
#
# Copyright (C) 2018 Robin Turner
# Qube Solutions GmbH, Arbachtalstr. 6
# 72800 Eningen, Germany 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pixtend.h"

static uint8_t byAux0;
static uint8_t byJumper10V;
static uint8_t byInitFlag = 0;

uint16_t crc16_calc(uint16_t crc, uint8_t data)
{
	int i;
	crc ^= data;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
		{
			crc = (crc >> 1) ^ 0xA001;
		}
		else
		{
			crc = (crc >> 1);
		}
	}
	return crc;
}

int Spi_AutoModeDAC(struct pixtOutDAC *OutputDataDAC) {
	
	Spi_Set_Aout(0, OutputDataDAC->wAOut0);
	Spi_Set_Aout(1, OutputDataDAC->wAOut1);
		
	return 0;
}

int Spi_AutoMode(struct pixtOut *OutputData, struct pixtIn *InputData)
{
	uint16_t crcSum;
	uint16_t crcSumRx;
	int i;
	unsigned char spi_output[34];
	int spi_device = 0;
	int len = 34;
	
	spi_output[0] = 128;                                
	spi_output[1] = 255;                                
	spi_output[2] = OutputData->byDigOut;                           
	spi_output[3] = OutputData->byRelayOut;                         
	spi_output[4] = OutputData->byGpioOut;                           
	spi_output[5] = (uint8_t)(OutputData->wPwm0 & 0xFF);       
	spi_output[6] = (uint8_t)((OutputData->wPwm0>>8) & 0xFF);
	spi_output[7] = (uint8_t)(OutputData->wPwm1 & 0xFF);       
	spi_output[8] = (uint8_t)((OutputData->wPwm1>>8) & 0xFF);
	spi_output[9] = OutputData->byPwm0Ctrl0;                         
	spi_output[10] = OutputData->byPwm0Ctrl1;
	spi_output[11] = OutputData->byPwm0Ctrl2;
	spi_output[12] = OutputData->byGpioCtrl;                         
	spi_output[13] = OutputData->byUcCtrl;                           
	spi_output[14] = OutputData->byAiCtrl0;                          
	spi_output[15] = OutputData->byAiCtrl1;                          
	spi_output[16] = OutputData->byPiStatus;                         
	byAux0 = OutputData->byAux0;
	//Calculate CRC16 Transmit Checksum
	crcSum = 0xFFFF;
	for (i=2; i <= 30; i++) 
	{
		crcSum = crc16_calc(crcSum, spi_output[i]);
	}
	spi_output[31]=crcSum & 0xFF;	//CRC Low Byte
	spi_output[32]=crcSum >> 8;	//CRC High Byte
	spi_output[33] = 128;   //Termination
	
	//Initialise SPI Data Transfer with OutputData
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	//spi_output now contains all returned data, assign values to InputData
	InputData->byDigIn =spi_output[2];
	InputData->wAi0 = (uint16_t)(spi_output[4]<<8)|(spi_output[3]);
	InputData->wAi1 = (uint16_t)(spi_output[6]<<8)|(spi_output[5]);
	InputData->wAi2 = (uint16_t)(spi_output[8]<<8)|(spi_output[7]);
	InputData->wAi3 = (uint16_t)(spi_output[10]<<8)|(spi_output[9]);
	InputData->byGpioIn = spi_output[11];
	InputData->wTemp0 = (uint16_t)(spi_output[13]<<8)|(spi_output[12]);
	InputData->wTemp1 = (uint16_t)(spi_output[15]<<8)|(spi_output[14]);
	InputData->wTemp2 = (uint16_t)(spi_output[17]<<8)|(spi_output[16]);
	InputData->wTemp3 = (uint16_t)(spi_output[19]<<8)|(spi_output[18]);
	InputData->wHumid0 = (uint16_t)(spi_output[21]<<8)|(spi_output[20]);
	InputData->wHumid1 = (uint16_t)(spi_output[23]<<8)|(spi_output[22]);
	InputData->wHumid2 = (uint16_t)(spi_output[25]<<8)|(spi_output[24]);
	InputData->wHumid3 = (uint16_t)(spi_output[27]<<8)|(spi_output[26]);
	InputData->byUcVersionL = spi_output[28];
	InputData->byUcVersionH = spi_output[29];
	InputData->byUcStatus = spi_output[30];
	
	if (byAux0 & (0b00000001)) {
		InputData->rAi0 = (float)(InputData->wAi0) * (10.0 / 1024);
	} 
	else {
		InputData->rAi0 = (float)(InputData->wAi0) * (5.0 / 1024);
	}
	if (byAux0 & (0b00000010)) {
		InputData->rAi1 = (float)(InputData->wAi1) * (10.0 / 1024);
	} 
	else {
		InputData->rAi1 = (float)(InputData->wAi1) * (5.0 / 1024);
	}
	
	InputData->rAi2 = (float)(InputData->wAi2) * 0.024194115990990990990990990991;
	InputData->rAi3 = (float)(InputData->wAi3) * 0.024194115990990990990990990991;
	InputData->rTemp0 = (float)(InputData->wTemp0) / 10.0;
	InputData->rTemp1 = (float)(InputData->wTemp1) / 10.0;
	InputData->rTemp2 = (float)(InputData->wTemp2) / 10.0;
	InputData->rTemp3 = (float)(InputData->wTemp3) / 10.0;
	InputData->rHumid0 = (float)(InputData->wHumid0) / 10.0;
	InputData->rHumid1 = (float)(InputData->wHumid1) / 10.0;
	InputData->rHumid2 = (float)(InputData->wHumid2) / 10.0;
	InputData->rHumid3 = (float)(InputData->wHumid3) / 10.0;
	
	//Calculate CRC16 Receive Checksum
	crcSum = 0xFFFF;
	for (i=2; i <= 30; i++) 
	{
		crcSum = crc16_calc(crcSum, spi_output[i]);
	}
	
	crcSumRx = (spi_output[32]<<8) + spi_output[31];
	
    if (crcSumRx != crcSum)
		return -1;
	else
		return 0;
}

int Spi_AutoModeV2S(struct pixtOutV2S *OutputData, struct pixtInV2S *InputData)
{
	uint16_t crcSumHeader;
	uint16_t crcSumData;
	uint16_t crcSumHeaderRx;
	uint16_t crcSumHeaderRxCalc;
	uint16_t crcSumDataRx;
	uint16_t crcSumDataRxCalc;
    uint16_t wTempValue;
	int i;
	unsigned char spi_output[67];
	int spi_device = 0;
	int len = 67;
	
	spi_output[0] = OutputData->byModelOut;                                
	spi_output[1] = OutputData->byUCMode;
	spi_output[2] = OutputData->byUCCtrl0;
	spi_output[3] = OutputData->byUCCtrl1;
    spi_output[4] = 0; 	//Reserved
    spi_output[5] = 0; 	//Reserved
    spi_output[6] = 0; 	//Reserved
	spi_output[7] = 0; // Reserved for Header CRC value
	spi_output[8] = 0; // Reserver for Header CRC value
	spi_output[9] = OutputData->byDigitalInDebounce01;
	spi_output[10] = OutputData->byDigitalInDebounce23;
	spi_output[11] = OutputData->byDigitalInDebounce45;
	spi_output[12] = OutputData->byDigitalInDebounce67;
	spi_output[13] = OutputData->byDigitalOut;
	spi_output[14] = OutputData->byRelayOut;
	spi_output[15] = OutputData->byGPIOCtrl;
	spi_output[16] = OutputData->byGPIOOut;
	spi_output[17] = OutputData->byGPIODebounce01;
	spi_output[18] = OutputData->byGPIODebounce23;
	spi_output[19] = OutputData->byPWM0Ctrl0;
	spi_output[20] = (uint8_t)(OutputData->wPWM0Ctrl1 & 0xFF);
	spi_output[21] = (uint8_t)((OutputData->wPWM0Ctrl1>>8) & 0xFF);
	spi_output[22] = (uint8_t)(OutputData->wPWM0A & 0xFF);
	spi_output[23] = (uint8_t)((OutputData->wPWM0A>>8) & 0xFF);
	spi_output[24] = (uint8_t)(OutputData->wPWM0B & 0xFF);
	spi_output[25] = (uint8_t)((OutputData->wPWM0B>>8) & 0xFF);
	spi_output[26] = OutputData->byPWM1Ctrl0;
	spi_output[27] = OutputData->byPWM1Ctrl1;
    spi_output[28] = 0; 	//Reserved
	spi_output[29] = OutputData->byPWM1A;
    spi_output[30] = 0; 	//Reserved
	spi_output[31] = OutputData->byPWM1B;
    spi_output[32] = 0; 	//Reserved

	//Add Retain data to SPI output          
	for (i=0; i <= 31; i++) 
	{
		spi_output[33+i] = OutputData->abyRetainDataOut[i];
	}

	spi_output[65] = 0; //Reserved for data CRC
	spi_output[66] = 0; //Reserved for data CRC            
    //Save physical jumper setting given by user for this call          
	byJumper10V = OutputData->byJumper10V;

	//Calculate CRC16 Header Transmit Checksum
	crcSumHeader = 0xFFFF;
	for (i=0; i < 7; i++) 
	{
		crcSumHeader = crc16_calc(crcSumHeader, spi_output[i]);
	}
	spi_output[7]=crcSumHeader & 0xFF;	//CRC Low Byte
	spi_output[8]=crcSumHeader >> 8;	//CRC High Byte

	//Calculate CRC16 Data Transmit Checksum
	crcSumData = 0xFFFF;
	for (i=9; i < 65; i++) 
	{
		crcSumData = crc16_calc(crcSumData, spi_output[i]);
	}
	spi_output[65]=crcSumData & 0xFF; //CRC Low Byte
	spi_output[66]=crcSumData >> 8;	  //CRC High Byte
	
	//-------------------------------------------------------------------------
	//Initialise SPI Data Transfer with OutputData
	wiringPiSPIDataRW(spi_device, spi_output, len);
	//-------------------------------------------------------------------------

	//Calculate Header CRC16 Receive Checksum
	crcSumHeaderRxCalc = 0xFFFF;
	for (i=0; i <= 6; i++) 
	{
		crcSumHeaderRxCalc = crc16_calc(crcSumHeaderRxCalc, spi_output[i]);
	}
	
	crcSumHeaderRx = (spi_output[8]<<8) + spi_output[7];
	
	//Check that CRC sums match
    if (crcSumHeaderRx != crcSumHeaderRxCalc)
		return -1;
	
	// Header Data received is OK
	// spi_output now contains all returned data, assign values to InputData

	InputData->byFirmware = spi_output[0];
	InputData->byHardware = spi_output[1];
	InputData->byModelIn = spi_output[2];
	InputData->byUCState = spi_output[3];
    InputData->byUCWarnings = spi_output[4];
    //spi_output[5]; //Reserved
    //spi_output[6]; //Reserved
    //spi_output[7]; //CRC Reserved
    //spi_output[8]; //CRC Reserved
    
    //Check model
 	if (spi_output[2] != 83)
		return -2;   
    
	//Calculate Data CRC16 Receive Checksum
	crcSumDataRxCalc = 0xFFFF;
	for (i=9; i <= 64; i++) 
	{
		crcSumDataRxCalc = crc16_calc(crcSumDataRxCalc, spi_output[i]);
	}
	
	crcSumDataRx = (spi_output[66]<<8) + spi_output[65];
	
    //Chekc data crc
    if (crcSumDataRxCalc != crcSumDataRx)
		return -3;    
    
	InputData->byDigitalIn =spi_output[9];
	InputData->wAnalogIn0 = (uint16_t)(spi_output[11]<<8)|(spi_output[10]);
	InputData->wAnalogIn1 = (uint16_t)(spi_output[13]<<8)|(spi_output[12]);
	InputData->byGPIOIn = spi_output[14];
    //----------------------------------------------------------------------------------------------------
    //Check Temp0 and Humid0 for value 255, meaning read error
    if (spi_output[16] == 255 && spi_output[15] == 255 && spi_output[18] == 255 && spi_output[17] == 255){
        InputData->byTemp0Error = 1;
    }
    else{
        InputData->wTemp0 = (uint16_t)(spi_output[16]<<8)|(spi_output[15]);
        InputData->wHumid0 = (uint16_t)(spi_output[18]<<8)|(spi_output[17]);
        InputData->byTemp0Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //Check Temp1 and Humid1 for value 255, meaning read error
     if (spi_output[20] == 255 && spi_output[19] == 255 && spi_output[22] == 255 && spi_output[21] == 255){
        InputData->byTemp1Error = 1;
    }
    else{
        InputData->wTemp1 = (uint16_t)(spi_output[20]<<8)|(spi_output[19]);
        InputData->wHumid1 = (uint16_t)(spi_output[22]<<8)|(spi_output[21]);
        InputData->byTemp1Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //Check Temp2 and Humid2 for value 255, meaning read error
     if (spi_output[24] == 255 && spi_output[23] == 255 && spi_output[26] == 255 && spi_output[25] == 255){
        InputData->byTemp2Error = 1;
    }
    else{
        InputData->wTemp2 = (uint16_t)(spi_output[24]<<8)|(spi_output[23]);
        InputData->wHumid2 = (uint16_t)(spi_output[26]<<8)|(spi_output[25]);
        InputData->byTemp2Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //Check Temp3 and Humid3 for value 255, meaning read error
     if (spi_output[28] == 255 && spi_output[27] == 255 && spi_output[30] == 255 && spi_output[29] == 255){
        InputData->byTemp3Error = 1;
    }
    else{
        InputData->wTemp3 = (uint16_t)(spi_output[28]<<8)|(spi_output[27]);
        InputData->wHumid3 = (uint16_t)(spi_output[30]<<8)|(spi_output[29]);
        InputData->byTemp3Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //spi_output[31]; //Reserved
    //spi_output[32]; //Reserved

	if (byJumper10V & (0b00000001)) {
		InputData->rAnalogIn0 = (float)(InputData->wAnalogIn0) * (10.0 / 1024);
	} 
	else {
		InputData->rAnalogIn0 = (float)(InputData->wAnalogIn0) * (5.0 / 1024);
	}
	if (byJumper10V & (0b00000010)) {
		InputData->rAnalogIn1 = (float)(InputData->wAnalogIn1) * (10.0 / 1024);
	} 
	else {
		InputData->rAnalogIn1 = (float)(InputData->wAnalogIn1) * (5.0 / 1024);
	}
	
    //Check if user chose DHT11 or DHT22 sensor at GPIO0, 1 = DHT11 and 0 = DHT22
    if (OutputData->byGPIO0Dht11 == 1){
        InputData->rTemp0 = (float)(InputData->wTemp0 / 256);
        InputData->rHumid0 = (float)(InputData->wHumid0 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp0;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp0 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp0 = (float)(InputData->wTemp0) / 10.0;
        }
        InputData->rHumid0 = (float)(InputData->wHumid0) / 10.0;
    }
    //Check if user chose DHT11 or DHT22 sensor at GPIO1, 1 = DHT11 and 0 = DHT22
     if (OutputData->byGPIO1Dht11 == 1){
        InputData->rTemp1 = (float)(InputData->wTemp1 / 256);
        InputData->rHumid1 = (float)(InputData->wHumid1 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp1;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp1 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp1 = (float)(InputData->wTemp1) / 10.0;
        }
        InputData->rHumid1 = (float)(InputData->wHumid1) / 10.0;
    }
    //Check if user chose DHT11 or DHT22 sensor at GPIO2, 1 = DHT11 and 0 = DHT22
     if (OutputData->byGPIO2Dht11 == 1){
        InputData->rTemp2 = (float)(InputData->wTemp2 / 256);
        InputData->rHumid2 = (float)(InputData->wHumid2 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp2;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp2 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp2 = (float)(InputData->wTemp2) / 10.0;
        }
        InputData->rHumid2 = (float)(InputData->wHumid2) / 10.0;
    }
    //Check if user chose DHT11 or DHT22 sensor at GPIO3, 1 = DHT11 and 0 = DHT22
     if (OutputData->byGPIO3Dht11 == 1){
        InputData->rTemp3 = (float)(InputData->wTemp3 / 256);
        InputData->rHumid3 = (float)(InputData->wHumid3 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp3;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp3 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp3 = (float)(InputData->wTemp3) / 10.0;
        }
        InputData->rHumid3 = (float)(InputData->wHumid3) / 10.0;
    }
	//Get Retain data from SPI input          
	for (i=0; i <= 31; i++) 
	{
		InputData->abyRetainDataIn[i] = spi_output[33+i];
	}

	return 0;
}

int Spi_AutoModeV2L(struct pixtOutV2L *OutputData, struct pixtInV2L *InputData)
{
	uint16_t crcSumHeader;
	uint16_t crcSumData;
	uint16_t crcSumHeaderRx;
	uint16_t crcSumHeaderRxCalc;
	uint16_t crcSumDataRx;
	uint16_t crcSumDataRxCalc;
    uint16_t wTempValue;
	int i;
	unsigned char spi_output[111];
	int spi_device = 0;
	int len = 111;
	
	spi_output[0] = OutputData->byModelOut;                                
	spi_output[1] = OutputData->byUCMode;
	spi_output[2] = OutputData->byUCCtrl0;
	spi_output[3] = OutputData->byUCCtrl1;
    spi_output[4] = 0; 	//Reserved
    spi_output[5] = 0; 	//Reserved
    spi_output[6] = 0; 	//Reserved
	spi_output[7] = 0; // Reserved for Header CRC value
	spi_output[8] = 0; // Reserver for Header CRC value
	spi_output[9] = OutputData->byDigitalInDebounce01;
	spi_output[10] = OutputData->byDigitalInDebounce23;
	spi_output[11] = OutputData->byDigitalInDebounce45;
	spi_output[12] = OutputData->byDigitalInDebounce67;
 	spi_output[13] = OutputData->byDigitalInDebounce89;
	spi_output[14] = OutputData->byDigitalInDebounce1011;
	spi_output[15] = OutputData->byDigitalInDebounce1213;
	spi_output[16] = OutputData->byDigitalInDebounce1415;    
	spi_output[17] = OutputData->byDigitalOut0;
    spi_output[18] = OutputData->byDigitalOut1;
	spi_output[19] = OutputData->byRelayOut;
	spi_output[20] = OutputData->byGPIOCtrl;
	spi_output[21] = OutputData->byGPIOOut;
	spi_output[22] = OutputData->byGPIODebounce01;
	spi_output[23] = OutputData->byGPIODebounce23;
	spi_output[24] = OutputData->byPWM0Ctrl0;
	spi_output[25] = (uint8_t)(OutputData->wPWM0Ctrl1 & 0xFF);
	spi_output[26] = (uint8_t)((OutputData->wPWM0Ctrl1>>8) & 0xFF);
	spi_output[27] = (uint8_t)(OutputData->wPWM0A & 0xFF);
	spi_output[28] = (uint8_t)((OutputData->wPWM0A>>8) & 0xFF);
	spi_output[29] = (uint8_t)(OutputData->wPWM0B & 0xFF);
	spi_output[30] = (uint8_t)((OutputData->wPWM0B>>8) & 0xFF);
    spi_output[31] = OutputData->byPWM1Ctrl0;
 	spi_output[32] = (uint8_t)(OutputData->wPWM1Ctrl1 & 0xFF);
	spi_output[33] = (uint8_t)((OutputData->wPWM1Ctrl1>>8) & 0xFF);
	spi_output[34] = (uint8_t)(OutputData->wPWM1A & 0xFF);
	spi_output[35] = (uint8_t)((OutputData->wPWM1A>>8) & 0xFF);
	spi_output[36] = (uint8_t)(OutputData->wPWM1B & 0xFF);
	spi_output[37] = (uint8_t)((OutputData->wPWM1B>>8) & 0xFF);   
    spi_output[38] = OutputData->byPWM2Ctrl0;
 	spi_output[39] = (uint8_t)(OutputData->wPWM2Ctrl1 & 0xFF);
	spi_output[40] = (uint8_t)((OutputData->wPWM2Ctrl1>>8) & 0xFF);
	spi_output[41] = (uint8_t)(OutputData->wPWM2A & 0xFF);
	spi_output[42] = (uint8_t)((OutputData->wPWM2A>>8) & 0xFF);
	spi_output[43] = (uint8_t)(OutputData->wPWM2B & 0xFF);
	spi_output[44] = (uint8_t)((OutputData->wPWM2B>>8) & 0xFF);

	//Add Retain data to SPI output          
	for (i=0; i <= 63; i++) 
	{
		spi_output[45+i] = OutputData->abyRetainDataOut[i];
	}

	spi_output[109] = 0; //Reserved for data CRC
	spi_output[110] = 0; //Reserved for data CRC            
    //Save physical jumper setting given by user for this call          
	byJumper10V = OutputData->byJumper10V;

	//Calculate CRC16 Header Transmit Checksum
	crcSumHeader = 0xFFFF;
	for (i=0; i < 7; i++) 
	{
		crcSumHeader = crc16_calc(crcSumHeader, spi_output[i]);
	}
	spi_output[7]=crcSumHeader & 0xFF;	//CRC Low Byte
	spi_output[8]=crcSumHeader >> 8;	//CRC High Byte

	//Calculate CRC16 Data Transmit Checksum
	crcSumData = 0xFFFF;
	for (i=9; i <= 108; i++) 
	{
		crcSumData = crc16_calc(crcSumData, spi_output[i]);
	}
	spi_output[109]=crcSumData & 0xFF; //CRC Low Byte
	spi_output[110]=crcSumData >> 8;	  //CRC High Byte
	
	//-------------------------------------------------------------------------
	//Initialise SPI Data Transfer with OutputData
	wiringPiSPIDataRW(spi_device, spi_output, len);
	//-------------------------------------------------------------------------
   
	//Calculate Header CRC16 Receive Checksum
	crcSumHeaderRxCalc = 0xFFFF;
	for (i=0; i <= 6; i++) 
	{
		crcSumHeaderRxCalc = crc16_calc(crcSumHeaderRxCalc, spi_output[i]);
	}
	
	crcSumHeaderRx = (spi_output[8]<<8) + spi_output[7];
	
	//Check that CRC sums match
    if (crcSumHeaderRx != crcSumHeaderRxCalc)
		return -1;

	//-------------------------------------------------------------------------
	// Data received is OK, CRC and model matched
	//-------------------------------------------------------------------------
	//spi_output now contains all returned data, assign values to InputData

	InputData->byFirmware = spi_output[0];
	InputData->byHardware = spi_output[1];
	InputData->byModelIn = spi_output[2];
	InputData->byUCState = spi_output[3];
    InputData->byUCWarnings = spi_output[4];
    //spi_output[5]; //Reserved
    //spi_output[6]; //Reserved
    //spi_output[7]; //CRC Reserved
    //spi_output[8]; //CRC Reserved
    
    //Check model
    if (spi_output[2] != 76)
		return -2;
    
	//Calculate Data CRC16 Receive Checksum
	crcSumDataRxCalc = 0xFFFF;
	for (i=9; i <= 108; i++) 
	{
		crcSumDataRxCalc = crc16_calc(crcSumDataRxCalc, spi_output[i]);
	}
	
	crcSumDataRx = (spi_output[110]<<8) + spi_output[109];
	
    if (crcSumDataRxCalc != crcSumDataRx)
		return -3;    
    
	InputData->byDigitalIn0 = spi_output[9];
    InputData->byDigitalIn1 = spi_output[10];
	InputData->wAnalogIn0 = (uint16_t)(spi_output[12]<<8)|(spi_output[11]);
	InputData->wAnalogIn1 = (uint16_t)(spi_output[14]<<8)|(spi_output[13]);
    InputData->wAnalogIn2 = (uint16_t)(spi_output[16]<<8)|(spi_output[15]);
    InputData->wAnalogIn3 = (uint16_t)(spi_output[18]<<8)|(spi_output[17]);
    InputData->wAnalogIn4 = (uint16_t)(spi_output[20]<<8)|(spi_output[19]);
    InputData->wAnalogIn5 = (uint16_t)(spi_output[22]<<8)|(spi_output[21]);
	InputData->byGPIOIn = spi_output[23];
    
    //----------------------------------------------------------------------------------------------------
    //Check Temp0 and Humid0 for value 255, meaning read error
    if (spi_output[25] == 255 && spi_output[24] == 255 && spi_output[27] == 255 && spi_output[26] == 255){
        InputData->byTemp0Error = 1;
    }
    else{
        InputData->wTemp0 = (uint16_t)(spi_output[25]<<8)|(spi_output[24]);
        InputData->wHumid0 = (uint16_t)(spi_output[27]<<8)|(spi_output[26]);
        InputData->byTemp0Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //Check Temp1 and Humid1 for value 255, meaning read error
     if (spi_output[29] == 255 && spi_output[28] == 255 && spi_output[31] == 255 && spi_output[30] == 255){
        InputData->byTemp1Error = 1;
    }
    else{
        InputData->wTemp1 = (uint16_t)(spi_output[29]<<8)|(spi_output[28]);
        InputData->wHumid1 = (uint16_t)(spi_output[31]<<8)|(spi_output[30]);
        InputData->byTemp1Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //Check Temp2 and Humid2 for value 255, meaning read error
     if (spi_output[33] == 255 && spi_output[32] == 255 && spi_output[35] == 255 && spi_output[34] == 255){
        InputData->byTemp2Error = 1;
    }
    else{
        InputData->wTemp2 = (uint16_t)(spi_output[33]<<8)|(spi_output[32]);
        InputData->wHumid2 = (uint16_t)(spi_output[35]<<8)|(spi_output[34]);
        InputData->byTemp2Error = 0;
    }
    //----------------------------------------------------------------------------------------------------
    //Check Temp3 and Humid3 for value 255, meaning read error
     if (spi_output[37] == 255 && spi_output[36] == 255 && spi_output[39] == 255 && spi_output[38] == 255){
        InputData->byTemp3Error = 1;
    }
    else{
        InputData->wTemp3 = (uint16_t)(spi_output[37]<<8)|(spi_output[36]);
        InputData->wHumid3 = (uint16_t)(spi_output[39]<<8)|(spi_output[38]);
        InputData->byTemp3Error = 0;
    }
    //----------------------------------------------------------------------------------------------------

	if (byJumper10V & (0b00000001)) {
		InputData->rAnalogIn0 = (float)(InputData->wAnalogIn0) * (10.0 / 1024);
	} 
	else {
		InputData->rAnalogIn0 = (float)(InputData->wAnalogIn0) * (5.0 / 1024);
	}
	if (byJumper10V & (0b00000010)) {
		InputData->rAnalogIn1 = (float)(InputData->wAnalogIn1) * (10.0 / 1024);
	} 
	else {
		InputData->rAnalogIn1 = (float)(InputData->wAnalogIn1) * (5.0 / 1024);
	}
	if (byJumper10V & (0b00000100)) {
		InputData->rAnalogIn2 = (float)(InputData->wAnalogIn2) * (10.0 / 1024);
	} 
	else {
		InputData->rAnalogIn2 = (float)(InputData->wAnalogIn2) * (5.0 / 1024);
	}
	if (byJumper10V & (0b00001000)) {
		InputData->rAnalogIn3 = (float)(InputData->wAnalogIn3) * (10.0 / 1024);
	} 
	else {
		InputData->rAnalogIn3 = (float)(InputData->wAnalogIn3) * (5.0 / 1024);
	}
    
	InputData->rAnalogIn4 = (float)(InputData->wAnalogIn4) * 0.020158400229358;
	InputData->rAnalogIn5 = (float)(InputData->wAnalogIn5) * 0.020158400229358;    
    
	// if (byJumper10V & (0b00010000)) {
		// InputData->rAnalogIn4 = (float)(InputData->wAnalogIn4) * (10.0 / 1024);
	// } 
	// else {
		// InputData->rAnalogIn4 = (float)(InputData->wAnalogIn4) * (5.0 / 1024);
	// }
	// if (byJumper10V & (0b00100000)) {
		// InputData->rAnalogIn5 = (float)(InputData->wAnalogIn5) * (10.0 / 1024);
	// } 
	// else {
		// InputData->rAnalogIn5 = (float)(InputData->wAnalogIn5) * (5.0 / 1024);
	// }      
	
    //Check if user chose DHT11 or DHT22 sensor at GPIO0, 1 = DHT11 and 0 = DHT22
    if (OutputData->byGPIO0Dht11 == 1){
        InputData->rTemp0 = (float)(InputData->wTemp0 / 256);
        InputData->rHumid0 = (float)(InputData->wHumid0 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp0;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp0 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp0 = (float)(InputData->wTemp0) / 10.0;
        }
        InputData->rHumid0 = (float)(InputData->wHumid0) / 10.0;
    }
    //Check if user chose DHT11 or DHT22 sensor at GPIO1, 1 = DHT11 and 0 = DHT22
     if (OutputData->byGPIO1Dht11 == 1){
        InputData->rTemp1 = (float)(InputData->wTemp1 / 256);
        InputData->rHumid1 = (float)(InputData->wHumid1 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp1;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp1 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp1 = (float)(InputData->wTemp1) / 10.0;
        }
        InputData->rHumid1 = (float)(InputData->wHumid1) / 10.0;
    }
    //Check if user chose DHT11 or DHT22 sensor at GPIO2, 1 = DHT11 and 0 = DHT22
     if (OutputData->byGPIO2Dht11 == 1){
        InputData->rTemp2 = (float)(InputData->wTemp2 / 256);
        InputData->rHumid2 = (float)(InputData->wHumid2 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp2;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp2 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp2 = (float)(InputData->wTemp2) / 10.0;
        }
        InputData->rHumid2 = (float)(InputData->wHumid2) / 10.0;
    }
    //Check if user chose DHT11 or DHT22 sensor at GPIO3, 1 = DHT11 and 0 = DHT22
     if (OutputData->byGPIO3Dht11 == 1){
        InputData->rTemp3 = (float)(InputData->wTemp3 / 256);
        InputData->rHumid3 = (float)(InputData->wHumid3 / 256);
    }
    else{
        //For DHT22 sensors check bit 15, if set temperature value is negative
        wTempValue = InputData->wTemp3;
        if ((wTempValue >> 15) & 1) {
            wTempValue &= ~(1 << 15);
            InputData->rTemp3 = ((float)(wTempValue) / 10.0) * -1.0;
        }
        else {
            InputData->rTemp3 = (float)(InputData->wTemp3) / 10.0;
        }
        InputData->rHumid3 = (float)(InputData->wHumid3) / 10.0;
    }
	//Get Retain data from SPI input          
	for (i=0; i <= 63; i++) 
	{
		InputData->abyRetainDataIn[i] = spi_output[45+i];
	}

	return 0;
}

int Spi_Set_Dout(int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00000001; // Command
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

uint8_t Spi_Get_Dout()
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00010010; // Command 18
	spi_output[2] = 0b10101010; // readback command
	spi_output[3] = 0b10101010; // read value
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	delay(10);
	
	return spi_output[3];
}

int Spi_Get_Din()
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00000010; // Command
	spi_output[2] = 0b10101010; // readback command
	spi_output[3] = 0b10101010; // read value
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	delay(10);
	
	return spi_output[3];
}

uint16_t Spi_Get_Ain(int Idx)
{
	unsigned char spi_output[5];
	int spi_device = 0;
	int len = 5;
	int i;
	uint16_t high,low;
	uint16_t output;
	
	for(i=0;i<2;i++)
	{
		if(Idx==0)
			spi_output[1]=0b00000011; // Command;
		else if (Idx==1)
			spi_output[1]=0b00000100; // Command;
		else if (Idx==2)
			spi_output[1]=0b00000101; // Command;
		else
			spi_output[1]=0b00000110; // Command;
		
		spi_output[0] = 0b10101010; // Handshake - begin
		spi_output[2] = 0b00000000; // readback command
		spi_output[3] = 0b00000000; // read value low
		spi_output[4] = 0b00000000; // read value high
		
		wiringPiSPIDataRW(spi_device, spi_output, len);
		delay(100);
	}
	
	high = spi_output[3];
	low = spi_output[4] << 8;
	
	output = high | low;
	
	return output;
}

int Spi_Set_Aout(int channel, uint16_t value)
{
	unsigned char spi_output[2];
	int spi_device = 1;
	int len = 2;
	uint16_t tmp;
	
	spi_output[0] = 0b00010000;
	
	if(channel)
	{
		spi_output[0] = spi_output[0] | 0b10000000;
	}
	if(value > 1023)
	{
		value=1023;
	}
	
	tmp = value & 0b1111000000;
	tmp = tmp >> 6;
	spi_output[0]=spi_output[0] | tmp;
	
	tmp = value & 0b0000111111;
	tmp = tmp << 2;
	spi_output[1]=tmp;
		
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_Relays(int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00000111; // Command
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

uint8_t Spi_Get_Relays()
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00010011; // Command 19
	spi_output[2] = 0b10101010; // readback command
	spi_output[3] = 0b10101010; // read value
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	delay(10);
	
	return spi_output[3];
}

uint16_t Spi_Get_Temp(int Idx)
{
	unsigned char spi_output[5];
	int spi_device = 0;
	int len = 5;
	int i;
	uint16_t high,low;
	uint16_t output;
	
	for(i=0;i<2;i++)
	{
		if(Idx==0)
			spi_output[1]=0b00001010; // Command;
		else if (Idx==1)
			spi_output[1]=0b00001011; // Command;
		else if (Idx==2)
			spi_output[1]=0b00001100; // Command;
		else
			spi_output[1]=0b00001101; // Command;
		
		spi_output[0] = 0b10101010; // Handshake - begin
		spi_output[2] = 0b00000000; // readback command
		spi_output[3] = 0b00000000; // read value low
		spi_output[4] = 0b00000000; // read value high
		
		wiringPiSPIDataRW(spi_device, spi_output, len);
		delay(100);
	}
	
	high = spi_output[3];
	low = spi_output[4] << 8;
	
	output = high | low;
	
	return output;
}

uint16_t Spi_Get_Hum(int Idx)
{
	unsigned char spi_output[5];
	int spi_device = 0;
	int len = 5;
	int i;
	uint16_t high,low;
	uint16_t output;
	
	for(i=0;i<2;i++)
	{
		if(Idx==0)
			spi_output[1]=0b00001110; // Command;
		else if (Idx==1)
			spi_output[1]=0b00001111; // Command;
		else if (Idx==2)
			spi_output[1]=0b00010000; // Command;
		else
			spi_output[1]=0b00010001; // Command;
		
		spi_output[0] = 0b10101010; // Handshake - begin
		spi_output[2] = 0b00000000; // readback command
		spi_output[3] = 0b00000000; // read value low
		spi_output[4] = 0b00000000; // read value high
		
		wiringPiSPIDataRW(spi_device, spi_output, len);
		delay(100);
	}
	
	high = spi_output[3];
	low = spi_output[4] << 8;
	
	output = high | low;
	
	return output;
}

int Spi_Set_Servo(int channel, int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	if(channel>0)
	{
		spi_output[1] = 0b10000001; // Command
	}
	else
	{
		spi_output[1] = 0b10000000; // Command
	}
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_Pwm(int channel, uint16_t value)
{
	unsigned char spi_output[5];
	int spi_device = 0;
	int len = 5;
	
	if(channel>0)
	{
		spi_output[1] = 0b10000011; // Command
	}
	else
	{
		spi_output[1] = 0b10000010; // Command
	}
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[2] = value & 0b0000000011111111;
	spi_output[3] = (value & 0b1111111100000000) >> 8;
	spi_output[4] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_PwmControl(int value0, int value1, int value2)
{
	unsigned char spi_output[6];
	int spi_device = 0;
	int len = 6;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10000100; // Command
	spi_output[2] = value0;	
	spi_output[3] = value1;	
	spi_output[4] = value2;	
	spi_output[5] = 0b10101010;
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_GpioControl(int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10000101; // Command
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_UcControl(int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10000110; // Command
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_AiControl(int value0, int value1)
{
	unsigned char spi_output[5];
	int spi_device = 0;
	int len = 5;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10000111; // Command
	spi_output[2] = value0;
	spi_output[3] = value1;
	spi_output[4] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Set_RaspStat(int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10001000; // Command
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Setup(int spi_device)
{	
	int pin_Spi_enable = 5;
	int Spi_frequence = 100000;
	if(byInitFlag < 1)
	{
		wiringPiSetup();
		byInitFlag = 1;
	}
	
	pinMode(pin_Spi_enable, OUTPUT);
	digitalWrite(pin_Spi_enable,1); 
		
	wiringPiSPISetup(spi_device, Spi_frequence);

	return 0;
}

int Spi_SetupV2(int spi_device)
{	
	int pin_Spi_enable = 5;
	int Spi_frequency = 700000;
	if(byInitFlag < 1)
	{
		wiringPiSetup();
		byInitFlag = 1;
	}
	
	pinMode(pin_Spi_enable, OUTPUT);
	digitalWrite(pin_Spi_enable,1); 
	
	wiringPiSPISetup(spi_device, Spi_frequency);

	return 0;
}

int Spi_uC_Reset()
{
	int pin_reset = 4;
	
	wiringPiSetup();
	
	pinMode(pin_reset, OUTPUT);
	digitalWrite(pin_reset,1);
	delay(1000);
	digitalWrite(pin_reset,0);
	return 0;
}

int Spi_Get_uC_Status()
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10001010; // Command
	spi_output[2] = 0b10101010; // readback command
	spi_output[3] = 0b00000000; // read value
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	delay(10);
	
	return spi_output[3];
}

uint16_t Spi_Get_uC_Version()
{
	unsigned char spi_output[5];
	int spi_device = 0;
	int len = 5;
	int i;
	uint16_t version;
	uint16_t high, low;

	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b10001001; // Command;
	spi_output[2] = 0b00000000; // readback command
	spi_output[3] = 0b00000000; // read value low
	spi_output[4] = 0b00000000; // read value high
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	delay(100);
	
	high = (uint16_t)(spi_output[4] << 8);
	low = spi_output[3];
	
	version = high | low;
	return version;
}

int Change_Gpio_Mode(char pin, char mode)
{
	wiringPiSetup();
	
	if(mode==1)
	{
		pinMode(pin,OUTPUT);
	}
	else
	{
		pinMode(pin,INPUT);
	} 
	return 0;
}

int Change_Serial_Mode(uint8_t mode)
{
	int pin_serial = 1;	//Pin 1 ^= GPIO18
	
	wiringPiSetup();
	pinMode(pin_serial, OUTPUT);
	
	if(mode==1)
	{	
		digitalWrite(pin_serial,1);	//RS485
	}
	else
	{	
		digitalWrite(pin_serial,0);	//RS232
	} 
	return 0;
}

int Spi_Set_Gpio(int value)
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00001000; // Command
	spi_output[2] = value;
	spi_output[3] = 0b10101010;
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	
	return 0;
}

int Spi_Get_Gpio()
{
	unsigned char spi_output[4];
	int spi_device = 0;
	int len = 4;
	
	spi_output[0] = 0b10101010; // Handshake - begin
	spi_output[1] = 0b00001001; // Command
	spi_output[2] = 0b10101010; // readback command
	spi_output[3] = 0b10101010; // read value
	
	wiringPiSPIDataRW(spi_device, spi_output, len);
	delay(10);
	
	return spi_output[3];
}
