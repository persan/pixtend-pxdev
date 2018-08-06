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

struct pixtOutV2S {
    uint8_t byModelOut;
    uint8_t byUCMode;
    uint8_t byUCCtrl0;
    uint8_t byUCCtrl1;
    uint8_t byDigitalInDebounce01;
    uint8_t byDigitalInDebounce23;
    uint8_t byDigitalInDebounce45;
    uint8_t byDigitalInDebounce67;
    uint8_t byDigitalOut;
    uint8_t byRelayOut; 
    uint8_t byGPIOCtrl;
    uint8_t byGPIOOut;
    uint8_t byGPIODebounce01;
    uint8_t byGPIODebounce23;
    uint8_t byPWM0Ctrl0; 
    uint16_t wPWM0Ctrl1;   
    uint16_t wPWM0A;
    uint16_t wPWM0B;
    uint8_t byPWM1Ctrl0;
    uint8_t byPWM1Ctrl1;
    uint8_t byPWM1A; 
    uint8_t byPWM1B;
    uint8_t byJumper10V;
    uint8_t byGPIO0Dht11;
    uint8_t byGPIO1Dht11;
    uint8_t byGPIO2Dht11;
    uint8_t byGPIO3Dht11;
    uint8_t abyRetainDataOut[32];
};

struct pixtOutV2L {
    uint8_t byModelOut;
    uint8_t byUCMode;
    uint8_t byUCCtrl0;
    uint8_t byUCCtrl1;
    uint8_t byDigitalInDebounce01;
    uint8_t byDigitalInDebounce23;
    uint8_t byDigitalInDebounce45;
    uint8_t byDigitalInDebounce67;
    uint8_t byDigitalInDebounce89;
    uint8_t byDigitalInDebounce1011;
    uint8_t byDigitalInDebounce1213;
    uint8_t byDigitalInDebounce1415;
    uint8_t byDigitalOut0;
    uint8_t byDigitalOut1;
    uint8_t byRelayOut; 
    uint8_t byGPIOCtrl;
    uint8_t byGPIOOut;
    uint8_t byGPIODebounce01;
    uint8_t byGPIODebounce23;
    uint8_t byPWM0Ctrl0; 
    uint16_t wPWM0Ctrl1;   
    uint16_t wPWM0A;
    uint16_t wPWM0B;
    uint8_t byPWM1Ctrl0;
    uint16_t wPWM1Ctrl1;
    uint16_t wPWM1A; 
    uint16_t wPWM1B;
    uint8_t byPWM2Ctrl0;
    uint16_t wPWM2Ctrl1;
    uint16_t wPWM2A; 
    uint16_t wPWM2B;
    uint8_t byJumper10V;
    uint8_t byGPIO0Dht11;
    uint8_t byGPIO1Dht11;
    uint8_t byGPIO2Dht11;
    uint8_t byGPIO3Dht11;
    uint8_t abyRetainDataOut[64];
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

struct pixtInV2S {
    uint8_t byFirmware; 
    uint8_t byHardware;
    uint8_t byModelIn;
    uint8_t byUCState;
    uint8_t byUCWarnings;
    uint8_t byDigitalIn;
    uint16_t wAnalogIn0;
    uint16_t wAnalogIn1;
    uint8_t byGPIOIn;
    uint16_t wTemp0;
    uint8_t byTemp0Error;
    uint16_t wTemp1;
    uint8_t byTemp1Error;
    uint16_t wTemp2;
    uint8_t byTemp2Error;
    uint16_t wTemp3;
    uint8_t byTemp3Error;
    uint16_t wHumid0;
    uint16_t wHumid1;
    uint16_t wHumid2;
    uint16_t wHumid3;
    float rAnalogIn0;
    float rAnalogIn1;
    float rTemp0;
    float rTemp1;
    float rTemp2;
    float rTemp3;
    float rHumid0;
    float rHumid1;
    float rHumid2;
    float rHumid3;
    uint8_t abyRetainDataIn[32];
};

struct pixtInV2L {
    uint8_t byFirmware; 
    uint8_t byHardware;
    uint8_t byModelIn;
    uint8_t byUCState;
    uint8_t byUCWarnings;
    uint8_t byDigitalIn0;
    uint8_t byDigitalIn1;
    uint16_t wAnalogIn0;
    uint16_t wAnalogIn1;
    uint16_t wAnalogIn2;
    uint16_t wAnalogIn3;
    uint16_t wAnalogIn4;
    uint16_t wAnalogIn5;
    uint8_t byGPIOIn;
    uint16_t wTemp0;
    uint8_t byTemp0Error;
    uint16_t wTemp1;
    uint8_t byTemp1Error;
    uint16_t wTemp2;
    uint8_t byTemp2Error;
    uint16_t wTemp3;
    uint8_t byTemp3Error;
    uint16_t wHumid0;
    uint16_t wHumid1;
    uint16_t wHumid2;
    uint16_t wHumid3;
    float rAnalogIn0;
    float rAnalogIn1;
    float rAnalogIn2;
    float rAnalogIn3;
    float rAnalogIn4;
    float rAnalogIn5;    
    float rTemp0;
    float rTemp1;
    float rTemp2;
    float rTemp3;
    float rHumid0;
    float rHumid1;
    float rHumid2;
    float rHumid3;
    uint8_t abyRetainDataIn[64];
};

uint16_t crc16_calc(uint16_t crc, uint8_t data);
int Spi_AutoMode(struct pixtOut *OutputData, struct pixtIn *InputData);
int Spi_AutoModeV2S(struct pixtOutV2S *OutputData, struct pixtInV2S *InputData);
int Spi_AutoModeV2L(struct pixtOutV2L *OutputData, struct pixtInV2L *InputData);
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
int Spi_SetupV2(int spi_device);
int Spi_uC_Reset();
int Spi_Get_uC_Status();
uint16_t Spi_Get_uC_Version();
int Change_Gpio_Mode(char pin, char mode);
int Change_Serial_Mode(uint8_t mode);
int Spi_Set_Gpio(int value);
int Spi_Get_Gpio();
