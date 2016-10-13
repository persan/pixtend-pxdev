/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <http://www.pixtend.de> or <http://www.pixtend.com>
#
# Copyright (C) 2016 Nils Mensing, Christian Strobel
# Qube Solutions UG (haftungsbeschränkt), Arbachtalstr. 6
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

#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <inttypes.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>


struct pixtOut {
	uint8_t byDigOut; 
	uint8_t byRelayOut; 
	uint8_t byGpioOut; 
	uint16_t wPwm0; 
	uint16_t wPwm1; 
	uint8_t byPwm0Ctrl0; 
	uint8_t byPwm0Ctrl1; 
	uint8_t byPwm0Ctrl2; 
	uint8_t byGpioCtrl; 
	uint8_t byUcCtrl; 
	uint8_t byAiCtrl0; 
	uint8_t byAiCtrl1; 
	uint8_t byPiStatus;
	uint8_t byAux0;
};

struct pixtOutDAC {
	uint16_t wAOut0;
	uint16_t wAOut1;	
};


struct pixtIn {
	uint8_t byDigIn; 
	uint16_t wAi0; 
	uint16_t wAi1; 
	uint16_t wAi2; 
	uint16_t wAi3; 
	uint8_t byGpioIn; 
	uint16_t wTemp0; 
	uint16_t wTemp1; 
	uint16_t wTemp2; 
	uint16_t wTemp3; 
	uint16_t wHumid0; 
	uint16_t wHumid1; 
	uint16_t wHumid2; 
	uint16_t wHumid3;
	uint8_t byUcVersionL; 
	uint8_t byUcVersionH; 
	uint8_t byUcStatus; 
	float rAi0; 
	float rAi1; 
	float rAi2; 
	float rAi3; 
	float rTemp0; 
	float rTemp1; 
	float rTemp2; 
	float rTemp3; 
	float rHumid0; 
	float rHumid1; 
	float rHumid2; 
	float rHumid3;	
};

uint16_t crc16_calc(uint16_t crc, uint8_t data);

int Spi_AutoMode(struct pixtOut *OutputData, struct pixtIn *InputData);

int Spi_AutoModeDAC(struct pixtOutDAC *OutputDataDAC);

int Spi_Set_Dout(int value);

uint8_t Spi_Get_Dout();

int Spi_Get_Din();

uint16_t Spi_Get_Ain(int Idx);

int Spi_Set_Aout(int channel, uint16_t value);

int Spi_Set_Relays(int value);

uint8_t Spi_Get_Relays();

uint16_t Spi_Get_Temp(int Idx);

uint16_t Spi_Get_Hum(int Idx);

int Spi_Set_Servo(int channel, int value);

int Spi_Set_Pwm(int channel, uint16_t value);

int Spi_Set_PwmControl(int value0, int value1, int value2);

int Spi_Set_GpioControl(int value);

int Spi_Set_UcControl(int value);

int Spi_Set_AiControl(int value0, int value1);

int Spi_Set_RaspStat(int value);

int Spi_Setup(int spi_device);

int Spi_uC_Reset();

int Spi_Get_uC_Status();

uint16_t Spi_Get_uC_Version();

int Change_Gpio_Mode(char pin, char mode);

int Change_Serial_Mode(uint8_t mode);

int Spi_Set_Gpio(int value);

int Spi_Get_Gpio();
