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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pixtend.h>

#define VERSION "0.5.6"

#define SHMSZ 128 //Size of Shared Memory Area in bytes
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

static const uint8_t byModel = 76;
static int nAMRetVal;
static unsigned int nDelayTime = 30;

//PiXtend input data
struct pixtInV2L InputData;

//PiXtend output data
struct pixtOutV2L OutputData;

//PiXtend DAC Output Data
struct pixtOutDAC OutputDataDAC;

static int fd;

static uint8_t gHardware;
static uint8_t gFirmware;
static uint8_t gModelIn;
static int     gRetValVersion;

void signal_callback_handler(int signum)
{
    printf("\n\nCaught signal %d\n",signum);
    serialFlush(fd);
    printf("Flushed serial device\n");
    serialClose(fd);
    printf("Closed serial device\n");
    exit(signum);
}

void save_data_shm()
{
    int shmid;
    int i;
    key_t key;
    char *shm, *s;
    
    /*
     * We'll name our shared memory segment
     * "81 117 98", which means "Qub" in ASCII numbers.
     */
    key = 8111798;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        printf("Could not save data to memory! All settings lost.\n");
        exit(-5);
    }
    
    /*
     * Now attach the segment to data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        printf("Could not save data to memory! All settings lost.\n");
        exit(-5);
    }    

    /*
     * Now save our data to memory, so all user settings will be remembered at next call
     */
    s = shm;
    
    s[0] = OutputData.byModelOut;                                
    s[1] = OutputData.byUCCtrl0;
    s[2] = 255;  //Reserved
    s[3] = OutputData.byUCCtrl1;     
    s[4] = 255; // Reserved
    s[5] = 255; // Reserved
    s[6] = OutputData.byDigitalInDebounce01;
    s[7] = OutputData.byDigitalInDebounce23;
    s[8] = OutputData.byDigitalInDebounce45;
    s[9] = OutputData.byDigitalInDebounce67;
    s[10] = OutputData.byDigitalInDebounce89;
    s[11] = OutputData.byDigitalInDebounce1011;
    s[12] = OutputData.byDigitalInDebounce1213;
    s[13] = OutputData.byDigitalInDebounce1415;    
    s[14] = OutputData.byDigitalOut0;
    s[15] = OutputData.byDigitalOut1;
    s[16] = OutputData.byRelayOut;
    s[17] = OutputData.byGPIOCtrl;
    s[18] = OutputData.byGPIOOut;
    s[19] = OutputData.byGPIODebounce01;
    s[20] = OutputData.byGPIODebounce23;
    s[21] = OutputData.byPWM0Ctrl0;
    s[22] = (uint8_t)(OutputData.wPWM0Ctrl1 & 0xFF);
    s[23] = (uint8_t)((OutputData.wPWM0Ctrl1>>8) & 0xFF);
    s[24] = (uint8_t)(OutputData.wPWM0A & 0xFF);
    s[25] = (uint8_t)((OutputData.wPWM0A>>8) & 0xFF);
    s[26] = (uint8_t)(OutputData.wPWM0B & 0xFF);
    s[27] = (uint8_t)((OutputData.wPWM0B>>8) & 0xFF);
    s[28] = OutputData.byPWM1Ctrl0;
    s[29] = (uint8_t)(OutputData.wPWM1Ctrl1 & 0xFF);
    s[30] = (uint8_t)((OutputData.wPWM1Ctrl1>>8) & 0xFF);
    s[31] = (uint8_t)(OutputData.wPWM1A & 0xFF);
    s[32] = (uint8_t)((OutputData.wPWM1A>>8) & 0xFF);
    s[33] = (uint8_t)(OutputData.wPWM1B & 0xFF);
    s[34] = (uint8_t)((OutputData.wPWM1B>>8) & 0xFF);
    s[35] = OutputData.byPWM2Ctrl0;
    s[36] = (uint8_t)(OutputData.wPWM2Ctrl1 & 0xFF);
    s[37] = (uint8_t)((OutputData.wPWM2Ctrl1>>8) & 0xFF);
    s[38] = (uint8_t)(OutputData.wPWM2A & 0xFF);
    s[39] = (uint8_t)((OutputData.wPWM2A>>8) & 0xFF);
    s[40] = (uint8_t)(OutputData.wPWM2B & 0xFF);
    s[41] = (uint8_t)((OutputData.wPWM2B>>8) & 0xFF);    
    
    s[42] = 255; //Reserved
    
    for (i=0; i <= 63; i++) 
    {
     s[43+i] = OutputData.abyRetainDataOut[i];
    }
    
    s[120] = (uint8_t)(OutputDataDAC.wAOut0 & 0xFF); //AO DAC0
    s[121] = (uint8_t)((OutputDataDAC.wAOut0>>8) & 0xFF); //AO DAC0
    s[122] = (uint8_t)(OutputDataDAC.wAOut1 & 0xFF); //AO DAC1
    s[123] = (uint8_t)((OutputDataDAC.wAOut1>>8) & 0xFF); //AO DAC1

}

void load_data_shm()
{
    int shmid;
    int i;
    key_t key;
    char *shm, *s;
    
    /*
     * We'll name our shared memory segment
     * "81 117 98", which means "Qub" in ASCII numbers.
     */
    key = 8111798;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        printf("Could not save data to memory! All settings lost.\n");
        exit(-5);
    }
    
    /*
     * Now attach the segment to data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        printf("Could not save data to memory! All settings lost.\n");
        exit(-5);
    }    

    /*
     * Now read
     */
    s = shm;
    //Check for valid data first
    if (s[2] == 255 && s[4] == 255 && s[5] == 255 && s[42] == 255)
    {
        OutputData.byModelOut = s[0];
        OutputData.byUCCtrl0 = s[1];
        OutputData.byUCCtrl1 = s[3];
        OutputData.byDigitalInDebounce01 = s[6];
        OutputData.byDigitalInDebounce23 = s[7];
        OutputData.byDigitalInDebounce45 = s[8];
        OutputData.byDigitalInDebounce67 = s[9];
        OutputData.byDigitalInDebounce89 = s[10];
        OutputData.byDigitalInDebounce1011 = s[11];
        OutputData.byDigitalInDebounce1213 = s[12];
        OutputData.byDigitalInDebounce1415 = s[13];
        OutputData.byDigitalOut0 = s[14];
        OutputData.byDigitalOut1 = s[15];
        OutputData.byRelayOut = s[16];
        OutputData.byGPIOCtrl = s[17];
        OutputData.byGPIOOut = s[18];
        OutputData.byGPIODebounce01 = s[19];
        OutputData.byGPIODebounce23 = s[20];
        OutputData.byPWM0Ctrl0 = s[21];
        OutputData.wPWM0Ctrl1 = (uint16_t)(s[23]<<8)|(s[22]);
        OutputData.wPWM0A = (uint16_t)(s[25]<<8)|(s[24]);
        OutputData.wPWM0B = (uint16_t)(s[27]<<8)|(s[26]);
        
        OutputData.byPWM1Ctrl0 = s[28];
        OutputData.wPWM1Ctrl1 = (uint16_t)(s[30]<<8)|(s[29]);
        OutputData.wPWM1A = (uint16_t)(s[32]<<8)|(s[31]);
        OutputData.wPWM1B = (uint16_t)(s[34]<<8)|(s[33]);
        
        OutputData.byPWM2Ctrl0 = s[35];
        OutputData.wPWM2Ctrl1 = (uint16_t)(s[37]<<8)|(s[36]);
        OutputData.wPWM2A = (uint16_t)(s[39]<<8)|(s[38]);
        OutputData.wPWM2B = (uint16_t)(s[41]<<8)|(s[40]);
        
        for (i=0; i <= 63; i++) 
        {
            OutputData.abyRetainDataOut[i] = s[43+i];
        }
        
        OutputDataDAC.wAOut0 = (uint16_t)(s[121]<<8)|(s[120]);
        OutputDataDAC.wAOut1 = (uint16_t)(s[123]<<8)|(s[122]);
        
        //Set Delay Time to wait between autoMode() calls - (1 << bit) & byte
        if ((((1 << 4) & OutputData.byGPIOCtrl) == 16) || (((1 << 5) & OutputData.byGPIOCtrl) == 32) || (((1 << 6) & OutputData.byGPIOCtrl) == 64) || (((1 << 7) & OutputData.byGPIOCtrl) == 128))
        {
            nDelayTime = 30; //If DHTs are used, temperature measurment is active, we cannot get data faster than 30 ms
        } else {
            nDelayTime = 10; //Normal operation, use 10 ms waiting time for uC communication
        }
    }
}

void help()
{
    printf("PiXtend Tool V2 -L- http://www.pixtend.de - Version %s\n", VERSION);
    printf("usage: sudo ./pixtendtool2l [OPTION] [VALUE(s)]\n");
    printf("\n");
    printf("Available options:\n");
    printf("\t-h \t\t\t\t\tPrint this help\n");
    printf("\t-do VALUE \t\t\t\tSet the digital output word to VALUE[0-4095]\n");
    printf("\t-do BIT VALUE \t\t\t\tSet the digital output BIT[0-11] to VALUE [0/1]\n");
    printf("\t-dor\t\t\t\t\tGet the digital output word\n");
    printf("\t-dor BIT \t\t\t\tGet the digital output BIT[0-11]\n");
    printf("\t-di \t\t\t\t\tGet the digital input word\n");
    printf("\t-di BIT\t\t\t\t\tGet the digital input BIT[0-15]\n");
    printf("\t-ai CHANNEL \t\t\t\tGet the analog input CHANNEL[0-5] raw value\n");
    printf("\t-ai CHANNEL REF \t\t\tGet the analog input CHANNEL[0-3] value based on REF[5V/10V]\n");
    printf("\t-ai CHANNEL REF \t\t\tGet the analog input CHANNEL[4-5] value as REF[mA]\n");
    printf("\t-ao CHANNEL VALUE \t\t\tSet the analog output CHANNEL[0-1] to VALUE[0-1023]\n");
    printf("\t-rel VALUE \t\t\t\tSet the relay output byte to VALUE[0-255]\n");
    printf("\t-rel BIT VALUE \t\t\t\tSet the relay output BIT[0-3] to VALUE[0/1]\n");
    printf("\t-relr \t\t\t\t\tGet the relay output byte\n");
    printf("\t-relr BIT\t\t\t\tGet the relay output BIT[0-3]\n");
    printf("\t-gw VALUE \t\t\t\tSet GPIO output byte to VALUE[0-255]\n");
    printf("\t-gw BIT VALUE \t\t\t\tSet GPIO output BIT[0-3] to VALUE[0/1]\n");
    printf("\t-gr \t\t\t\t\tGet GPIO input byte\n");
    printf("\t-gr BIT\t\t\t\t\tGet GPIO input BIT[0-3]\n");
    printf("\t-gc VALUE \t\t\t\tSet GPIO control to VALUE[0-255]\n");
    printf("\t-tr CHANNEL TYPE\t\t\tGet temperature from CHANNEL[0-3] of TYPE[DHT11/DHT22]\n");
    printf("\t-hr CHANNEL TYPE\t\t\tGet humidity from CHANNEL[0-3] of TYPE[DHT11/DHT22]\n");
    printf("\t-srv0 CHANNEL VALUE \t\t\tSet servo 0 CHANNEL[0-1] to VALUE[0-16000]\n");
    printf("\t-srv1 CHANNEL VALUE \t\t\tSet servo 1 CHANNEL[0-1] to VALUE[0-16000]\n");
    printf("\t-srv2 CHANNEL VALUE \t\t\tSet servo 2 CHANNEL[0-1] to VALUE[0-16000]\n");
    printf("\t-pwm0 CHANNEL VALUE \t\t\tSet PWM 0 CHANNEL[0-1] to VALUE[0-65535]\n");
    printf("\t-pwm1 CHANNEL VALUE \t\t\tSet PWM 1 CHANNEL[0-1] to VALUE[0-65535]\n");
    printf("\t-pwm2 CHANNEL VALUE \t\t\tSet PWM 2 CHANNEL[0-1] to VALUE[0-65535]\n");
    printf("\t-pwm0c VALUE0 VALUE1 \t\t\tSet PWM 0 control VALUE0[0-255] and VALUE1[0-65535]\n");
    printf("\t-pwm1c VALUE0 VALUE1 \t\t\tSet PWM 1 control VALUE0[0-255] and VALUE1[0-65535]\n");
    printf("\t-pwm2c VALUE0 VALUE1 \t\t\tSet PWM 2 control VALUE0[0-255] and VALUE1[0-65535]\n");
	printf("\t-shw MODE\t\t\t\tChange serial hardware MODE[rs232/rs485]\n");    
    printf("\t-swc SERIALDEVICE BAUDRATE CHAR\t\tWrite a CHAR on SERIALDEVICE\n");
    printf("\t-sws SERIALDEVICE BAUDRATE STRING\tWrite a STRING on SERIALDEVICE (max 255)\n");
    printf("\t-sr SERIALDEVICE BAUDRATE\t\tRead data from SERIALDEVICE until Ctrl^C\n");
    printf("\t-ucc0 VALUE \t\t\t\tSet the microcontroller control 0 to VALUE[0-255]\n");
    printf("\t-ucc1 VALUE \t\t\t\tSet the microcontroller control 1 to VALUE[0-255]\n");
    printf("\t-ucs \t\t\t\t\tGet microcontroller status register\n");
    printf("\t-ucr \t\t\t\t\tReset the microcontroller\n");
    printf("\t-ucv \t\t\t\t\tGet microcontroller version\n");
    printf("\t-ucw \t\t\t\t\tGet microcontroller warnings\n");
    printf("\n");
    printf("\tNote for PWM 0/1/2 and Servo 0/1/2: 0 = channel A and 1 = channel B\n");
    printf("\n");
    printf("Application Notes about the correct usage of Control and Status Bytes\n");
    printf("can be found at http://www.pixtend.de/downloads/\n");
    
    return;
}

int limitPar(int *value, int min, int max, const char* par) 
{    
    int ret = 0;
    if(*value < min) {
        *value = min;
        ret = 1;
    }        
    if(*value > max) {
        *value = max;
        ret = 2;
    }
    if(ret) 
        printf("Warning: %s range exceeded. Allowed range: [%d - %d]\n",par,min,max);
    
    return ret;
}

int autoMode(int cyclephase) 
{
    if (cyclephase == 0)
    {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // First Cycle
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //Exchange PiXtend V2 -L- Data
        OutputData.byModelOut = byModel;
        nAMRetVal = Spi_AutoModeV2L(&OutputData, &InputData);
        //Exchange PiXtend V2 -L- DAC Data
        Spi_AutoModeDAC(&OutputDataDAC);       
    }

    if (cyclephase == 1)
    {
        //Wait for PiXtend V2 -L- uC to go to Run and process data
        delay(nDelayTime);
        
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Second Cycle
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
        //Exchange PiXtend V2 -L- Data
        OutputData.byModelOut = byModel;
        nAMRetVal = Spi_AutoModeV2L(&OutputData, &InputData);
        //Exchange PiXtend V2 -L- DAC Data
        Spi_AutoModeDAC(&OutputDataDAC);
    }
    
    return(nAMRetVal);
}

int getUcVersion() 
{
    //Run auto mode for data exchange
    gRetValVersion = autoMode(0);
    
    //Read current uC Firmware Version
    gHardware = InputData.byHardware;
    gFirmware = InputData.byFirmware;  
    gModelIn  = InputData.byModelIn;
    
    return gRetValVersion;
}

int echoModelConflict()     
{
    char c, m;
    
    if (gRetValVersion != 0){
        switch(gRetValVersion){
            case -1: printf("CRC error in data header!\n"); break;
            case -2: c = gModelIn;
                     m = byModel;
                     printf("PiXtend V2 Model Conflict: This tool was made for another PiXtend V2 board.\n");
                     printf("Your current model is: -%c-, but this tool needs the '-%c-' model to function.\n", c, m);                    
                     break;
            case -3: printf("CRC error in PiXtend data found!\n"); break;
        }
    }
    return gRetValVersion;
}

int isCompatibleModel() {
    
    getUcVersion();
    
    if (gModelIn == byModel){
        return 0;
    }else{
        return -1;
    }
}

int isReturnCodeOK(int RetVal){
    
    if (RetVal != 0){
        switch(RetVal){
            case -1: printf("CRC error in data header!\n"); break;
            case -2: echoModelConflict();                   
                     break;
            case -3: printf("CRC error in PiXtend data found!\n"); break;
        }
        return RetVal;
    }else{
        return 0;
    }
}

int doProcessDataReset(){
    //-----------------------------
    //PiXtend V2 -L-
    //-----------------------------
    OutputData.byModelOut = 0;
    OutputData.byUCCtrl0 = 0;          
    OutputData.byUCCtrl1 = 0;            
    OutputData.byDigitalInDebounce01 = 0;            
    OutputData.byDigitalInDebounce23 = 0;              
    OutputData.byDigitalInDebounce45 = 0;             
    OutputData.byDigitalInDebounce67 = 0;      
    OutputData.byDigitalInDebounce89 = 0; 
    OutputData.byDigitalInDebounce1011 = 0; 
    OutputData.byDigitalInDebounce1213 = 0;
    OutputData.byDigitalInDebounce1415 = 0;     
    OutputData.byDigitalOut0 = 0;
    OutputData.byDigitalOut1 = 0;     
    OutputData.byRelayOut = 0;              
    OutputData.byGPIOCtrl = 0;            
    OutputData.byGPIOOut = 0;            
    OutputData.byGPIODebounce01 = 0;             
    OutputData.byGPIODebounce23 = 0;             
    OutputData.byPWM0Ctrl0 = 0;
    OutputData.wPWM0Ctrl1 = 0;
    OutputData.wPWM0A = 0;           
    OutputData.wPWM0B = 0;             
    OutputData.byPWM1Ctrl0 = 0;              
    OutputData.wPWM1Ctrl1 = 0;            
    OutputData.wPWM1A = 0; 
    OutputData.wPWM1B = 0;
    OutputData.byPWM2Ctrl0 = 0;              
    OutputData.wPWM2Ctrl1 = 0;            
    OutputData.wPWM2A = 0; 
    OutputData.wPWM2B = 0;
    //PiXtend V2 -L- DAC        
    OutputDataDAC.wAOut0 = 0;           
    OutputDataDAC.wAOut1 = 0;
    
    return 0;
}

//#############################################################################
// Main Program
//#############################################################################
int main(int argc, char *argv[]) 
{
    int i = 0;
    //int nRet;
    int RetVal;

    uint16_t uiRet;
    errno = 0;
    
    Spi_SetupV2(0); //use SPI device 0.0 (PiXtend V2 -L- uC)
    Spi_SetupV2(1); //use SPI device 0.1 (PiXtend V2 -L- DAC)
    
    if(argc > 1)
    {
        //Load data from RAM to continue work
        load_data_shm();
        
        if(strcmp(argv[1],"-h")==0)
        {
            help();
        }
        else if(strcmp(argv[1],"-do")==0 && argc==3)
        {
            int value;
            
            value = atoi(argv[2]);
            //Limit value
            limitPar(&value, 0,4095, "VALUE");
            
            //Set data for output
            OutputData.byDigitalOut0 = (uint8_t)(value & 0xFF);
            OutputData.byDigitalOut1 = (uint8_t)((value >> 8) & 0xFF);            
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            

            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Digital output word set to [%d]\n",value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-do")==0 && argc==4)
        {
            int bit;
            int bit_old;            
            int value;
            uint8_t fvalue;
            uint8_t currentValue;
            
            //Get Bit to modify
            bit = atoi(argv[2]);
            limitPar(&bit, 0,11, "BIT");
            //Get value to set
            value = atoi(argv[3]);
            limitPar(&value, 0,1, "VALUE");
            //Get old value from RAM
            bit_old = bit;
            if (bit > 7) {
                currentValue = OutputData.byDigitalOut1;
                bit = bit - 8;
            }else{
               currentValue = OutputData.byDigitalOut0; 
            }
            
            //Modify bit
            if (value == 0) {
                fvalue = (uint8_t)(currentValue & ~(1<<bit));
            }
            else if (value == 1) {
                fvalue = (uint8_t)(currentValue | (1<<bit));
            }            
            
            //Set data for output
            bit = bit_old;
            if (bit > 7) {
                OutputData.byDigitalOut1 = fvalue;
            }else{
                OutputData.byDigitalOut0 = fvalue;
            }
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Digital output bit %d set to [%d]\n",bit,value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-dor")==0 && argc==2)
        {
            uint16_t wOutputWord;
            wOutputWord = (uint16_t)(OutputData.byDigitalOut1<<8)|(OutputData.byDigitalOut0);
            //Print out the data from memory
            printf("Digital output word value is [%d]\n", wOutputWord);
        }
        else if(strcmp(argv[1],"-dor")==0 && argc==3)
        {
            int bit;
            uint8_t fvalue;

            //Get bit
            bit = atoi(argv[2]);
            limitPar(&bit, 0,11, "BIT");

            if (bit > 7) {
                bit = bit - 8;
                if(OutputData.byDigitalOut1 & (1<<bit)) {
                    fvalue = 1;
                }
                else {
                    fvalue = 0;
                }                  
            }else{
                 if(OutputData.byDigitalOut0 & (1<<bit)) {
                    fvalue = 1;
                }
                else {
                    fvalue = 0;
                }          
            }
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            printf("Digital output bit %d value is [%d]\n",bit,fvalue);
        }
        else if(strcmp(argv[1],"-di")==0 && argc==2)
        {
            uint16_t wInputWord;
                        
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                wInputWord = (uint16_t)(InputData.byDigitalIn1<<8)|(InputData.byDigitalIn0);
                printf("Digital input word is [%d]\n", wInputWord);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-di")==0 && argc==3)
        {
            int bit;
            uint8_t fvalue;
            uint16_t wInputWord;
            
            bit = atoi(argv[2]);
            limitPar(&bit, 0,15, "BIT");

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                //Get BIT value
                wInputWord = (uint16_t)(InputData.byDigitalIn1<<8)|(InputData.byDigitalIn0);
                if(wInputWord & (1<<bit)) {
                    fvalue = 1;
                }
                else {
                    fvalue = 0;
                }
                printf("Digital input bit %d value is [%d]\n", bit, fvalue);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-ai")==0 && argc==3)
        {            
            
            int channel = atoi(argv[2]);
            limitPar(&channel,0,5,"CHANNEL");            
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                //Determine analog channel and get value
                if (channel == 0){
                    uiRet = InputData.wAnalogIn0;
                }else if (channel == 1){
                    uiRet = InputData.wAnalogIn1;
                }else if (channel == 2){
                    uiRet = InputData.wAnalogIn2;
                }else if (channel == 3){
                    uiRet = InputData.wAnalogIn3;
                }else if (channel == 4){
                    uiRet = InputData.wAnalogIn4;
                }else if (channel == 5){
                    uiRet = InputData.wAnalogIn5;
                }else{
                    uiRet = 65535;
                }
                //Print result to console
                printf("Analog input %d value is [%d]\n", channel, uiRet);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-ai")==0 && argc==4)
        {
            int isCurrent;
            
            isCurrent = 0;
            int channel = atoi(argv[2]);
            limitPar(&channel,0,5,"CHANNEL");

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                //Determine analog channel and get value
                if (channel == 0){
                    uiRet = InputData.wAnalogIn0;
                }else if (channel == 1){
                    uiRet = InputData.wAnalogIn1;
                }else if (channel == 2){
                    uiRet = InputData.wAnalogIn2;
                }else if (channel == 3){
                    uiRet = InputData.wAnalogIn3;
                }else if (channel == 4){
                    uiRet = InputData.wAnalogIn4;
                    isCurrent = 1;
                }else if (channel == 5){
                    uiRet = InputData.wAnalogIn5;
                    isCurrent = 1;
                }else{
                    uiRet = 65535;
                }
                //Check which
                if (isCurrent == 0){
                    if (strcmp(argv[3],"5V")==0) {
                        printf("Analog input %d voltage is [%2.2f]V\n",channel,(float)uiRet*5.0/1024.0);
                    }
                    else if(strcmp(argv[3],"10V")==0) {
                        printf("Analog input %d voltage is [%2.2f]V\n",channel,(float)uiRet*10.0/1024.0);
                    }
                    else{
                        printf("Error: Unknown reference voltage %s. Valid Options are: [5V/10V]\n",argv[3]);
                    }
                }else{
                    if (strcmp(argv[3],"mA")==0) {
                        printf("Analog input %d current is [%2.2f]mA\n",channel,(float)uiRet*0.024194115990990990990990990991); 
                    }
                    else{
                        printf("Error: Unknown reference for current measurment (%s). Only valid option for channels 4/5 is: [mA]\n",argv[3]);
                    }
                }
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-aic")==0 && argc==4)
        {
            printf("AI Control Registers are NOT available in this version!\n");
            help();
        }
        else if(strcmp(argv[1],"-ao")==0 && argc==4)
        {
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");            
            
            int value = atoi(argv[3]);
            limitPar(&value,0,1023,"VALUE");
            
            if (channel == 0){
              OutputDataDAC.wAOut0 = value;  
            }
            if (channel == 1){
              OutputDataDAC.wAOut1 = value; 
            }            
            
            //Exchange PiXtend V2 -L- DAC Data
            Spi_AutoModeDAC(&OutputDataDAC);
            
            printf("Analog output channel %d set to [%d]\n",channel, value);
        }
        else if(strcmp(argv[1],"-rel")==0 && argc==3)
        {     
            int value;
            
            value = atoi(argv[2]);
            //Limit value
            limitPar(&value, 0,255, "VALUE");
            //Set data for output
            OutputData.byRelayOut = value;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Relay outputs set to [%d]\n", value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-rel")==0 && argc==4)
        {
            int bit;
            int value;
            uint8_t fvalue;
            uint8_t currentValue;            

            //Get bit to modify
            bit = atoi(argv[2]);
            limitPar(&bit, 0,3, "BIT");
            //Get value to set
            value = atoi(argv[3]);
            limitPar(&value, 0,1, "VALUE");
            
            currentValue = OutputData.byRelayOut;
            
            if (value == 0) {
                fvalue = (uint8_t)(currentValue & ~(1<<bit));
            }
            else if (value == 1) {
                fvalue = (uint8_t)(currentValue | (1<<bit));
            }
            
            //Set data for output
            OutputData.byRelayOut = fvalue;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Relay output bit %d set to [%d]\n",bit,value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-relr")==0 && argc==2)
        {
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            //Return data from RAM
            printf("Relay output byte value is [%d]\n",OutputData.byRelayOut);
        }
        else if(strcmp(argv[1],"-relr")==0 && argc==3)
        {
            int bit;
            uint8_t fvalue;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            bit = atoi(argv[2]);
            limitPar(&bit, 0,3, "BIT");
            
            if(OutputData.byRelayOut & (1<<bit)) {
                fvalue = 1;
            }
            else {
                fvalue = 0;
            }
            
            printf("Relay output bit %d value is [%d]\n",bit,fvalue);
        }
        else if(strcmp(argv[1],"-gr")==0 && argc==2)
        {
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            } 

            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("GPIO byte value is [%d]\n",InputData.byGPIOIn);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-gr")==0 && argc==3)
        {
            int bit;
            uint8_t fvalue;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            bit = atoi(argv[2]);
            limitPar(&bit, 0,3, "BIT");

            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                if(InputData.byGPIOIn & (1<<bit)) {
                    fvalue = 1;
                }
                else {
                    fvalue = 0;
                }
                //Return value from memory
                printf("GPIO bit %d value is [%d]\n",bit,fvalue);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-gw")==0 && argc==3)
        {
            int value = atoi(argv[2]);
            limitPar(&value, 0,255, "VALUE");
            
            //Set data for output
            OutputData.byGPIOOut = value;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("GPIO byte set to [%d]\n",value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-gw")==0 && argc==4)
        {
            int bit;
            int value;
            uint8_t fvalue;
            uint8_t currentValue;
            
            bit = atoi(argv[2]);
            limitPar(&bit, 0,3, "BIT");
            
            value = atoi(argv[3]);
            limitPar(&value, 0,1, "VALUE");

            currentValue = OutputData.byGPIOOut;
            
            if (value == 0) {
                fvalue = (uint8_t)(currentValue & ~(1<<bit));
            }
            else if (value == 1) {
                fvalue = (uint8_t)(currentValue | (1<<bit));
            }

            //Set data for output
            OutputData.byGPIOOut = fvalue;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("GPIO output bit %d set to [%d]\n",bit,fvalue);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-gc")==0 && argc==3)
        {           
            int value = atoi(argv[2]);
            limitPar(&value, 0,255, "VALUE");

            //Set data for output
            OutputData.byGPIOCtrl = value;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("GPIO control register set to [%d]\n",value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-tr")==0 && (argc==3||argc==4)) 
        {            
            int channel = atoi(argv[2]);
            limitPar(&channel,0,3,"CHANNEL");
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                //Determine analog channel and get value
                if (channel == 0){
                    uiRet = InputData.wTemp0;
                }else if (channel == 1){
                    uiRet = InputData.wTemp1;
                }else if (channel == 2){
                    uiRet = InputData.wTemp2;
                }else if (channel == 3){
                    uiRet = InputData.wTemp3;
                }else{
                    uiRet = 65535;
                }
                if ((argc==4 && strcmp(argv[3],"DHT22")==0) || argc==3) {            
                    printf("Temperature Channel %d: %2.1f deg[C]\n",channel,(float)uiRet/10.0);
                }
                else if(argc==4 && strcmp(argv[3],"DHT11")==0) {
                    printf("Temperature Channel %d: %2.1f deg[C]\n",channel,(float)(uiRet>>8)*1.0);
                }
                else  {
                    printf("Error: Unknown Sensor Type %s. Valid Options are: [DHT11/DHT22]\n",argv[3]);
                }
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-hr")==0 && (argc==3 || argc==4))
        {            
            int channel = atoi(argv[2]);
            limitPar(&channel,0,3,"CHANNEL");
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                //Determine analog channel and get value
                if (channel == 0){
                    uiRet = InputData.wHumid0;
                }else if (channel == 1){
                    uiRet = InputData.wHumid1;
                }else if (channel == 2){
                    uiRet = InputData.wHumid2;
                }else if (channel == 3){
                    uiRet = InputData.wHumid3;
                }else{
                    uiRet = 65535;
                }
                if ((argc==4 && strcmp(argv[3],"DHT22")==0) || argc==3) {            
                    printf("Humidity Channel %d: %2.1f %%\n",channel,(float)uiRet/10.0);
                }
                else if(argc==4 && strcmp(argv[3],"DHT11")==0) {
                    printf("Humidity Channel %d: %2.1f %%\n",channel,(float)(uiRet>>8)*1.0);
                }
                else  {
                    printf("Error: Unknown sensor type %s. Valid options are: [DHT11/DHT22]\n",argv[3]);
                }
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-srv0")==0 && argc==4)
        {
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");
            
            int value = atoi(argv[3]);
            limitPar(&value, 0,16000, "VALUE");
            //Check if Servo Mode is on
            if((CHECK_BIT(OutputData.byPWM0Ctrl0,0) == 0) && (CHECK_BIT(OutputData.byPWM0Ctrl0,1) == 0)){
                if (channel == 0){
                    //Set data for output
                    OutputData.wPWM0A = (uint16_t) value;  
                }else if (channel == 1){
                    //Set data for output
                    OutputData.wPWM0B = (uint16_t) value;  
                }

                //Check for compatible PiXtend V2 -L- model
                if (isCompatibleModel() == -1){
                    errno = echoModelConflict();
                    return errno;
                }                
                
                //Do data transfer / communication
                RetVal = autoMode(1);
                //Check return code, 0 = OK
                if (isReturnCodeOK(RetVal) == 0){
                    printf("Servo 0 Channel %d set to [%d]\n", channel, value);
                }else{
                    printf("Servo 0 Channel %d communication error!\n", channel);
                    errno = RetVal;
                }       
            }else{
                printf("Servo 0 Channel %d NOT set. Wrong PWM Mode!\n", channel);
                errno = -4;//Wrong mode
            }
        }
        else if(strcmp(argv[1],"-srv1")==0 && argc==4)
        {
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");
            
            int value = atoi(argv[3]);
            limitPar(&value, 0,16000, "VALUE");
            //Check if Servo Mode is on
            if((CHECK_BIT(OutputData.byPWM1Ctrl0,0) == 0) && (CHECK_BIT(OutputData.byPWM1Ctrl0,1) == 0)){
                if (channel == 0){
                    //Set data for output
                    OutputData.wPWM1A = (uint16_t) value;  
                }else if (channel == 1){
                    //Set data for output
                    OutputData.wPWM1B = (uint16_t) value;  
                }

                //Check for compatible PiXtend V2 -L- model
                if (isCompatibleModel() == -1){
                    errno = echoModelConflict();
                    return errno;
                }
                
                //Do data transfer / communication
                RetVal = autoMode(1);
                //Check return code, 0 = OK
                if (isReturnCodeOK(RetVal) == 0){
                    printf("Servo 1 Channel %d set to [%d]\n", channel, value);
                }else{
                    printf("Servo 1 Channel %d communication error!\n", channel);
                    errno = RetVal;
                }       
            }else{
                printf("Servo 1 Channel %d NOT set. Wrong PWM Mode!\n", channel);
                errno = -4;//Wrong mode
            }
        }
        else if(strcmp(argv[1],"-srv2")==0 && argc==4)
        {   
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");
            
            int value = atoi(argv[3]);
            limitPar(&value, 0,16000, "VALUE");
            //Check if Servo Mode is on
            if((CHECK_BIT(OutputData.byPWM2Ctrl0,0) == 0) && (CHECK_BIT(OutputData.byPWM2Ctrl0,1) == 0)){
                if (channel == 0){
                    //Set data for output
                    OutputData.wPWM2A = (uint16_t) value;  
                }else if (channel == 1){
                    //Set data for output
                    OutputData.wPWM2B = (uint16_t) value;  
                }

                //Check for compatible PiXtend V2 -L- model
                if (isCompatibleModel() == -1){
                    errno = echoModelConflict();
                    return errno;
                }                
                
                //Do data transfer / communication
                RetVal = autoMode(1);
                //Check return code, 0 = OK
                if (isReturnCodeOK(RetVal) == 0){
                    printf("Servo 2 Channel %d set to [%d]\n", channel, value);
                }else{
                    printf("Servo 2 Channel %d communication error!\n", channel);
                    errno = RetVal;
                }       
            }else{
                printf("Servo 2 Channel %d NOT set. Wrong PWM Mode!\n", channel);
                errno = -4;//Wrong mode
            }
        }
        else if(strcmp(argv[1],"-pwm0")==0 && argc==4)
        {            
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");
            
            int value = atoi(argv[3]);
            limitPar(&value, 0,65535, "VALUE");
            
            if (channel == 0){
                //Set data for output
                OutputData.wPWM0A = (uint16_t) value;  
            }else if (channel == 1){
                //Set data for output
                OutputData.wPWM0B = (uint16_t) value;  
            }            

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("PWM 0 Channel %d set to [%d]\n",channel,value);
            }else{
                //printf("PWM 0 Channel %d - Error setting value!\n",channel,value);
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-pwm1")==0 && argc==4)
        {
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");
            
            int value = atoi(argv[3]);
            limitPar(&value, 0,65535, "VALUE");
            
            if (channel == 0){
                //Set data for output
                OutputData.wPWM1A = (uint16_t) value;  
            }else if (channel == 1){
                //Set data for output
                OutputData.wPWM1B = (uint16_t) value;  
            }            

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("PWM 1 Channel %d set to [%d]\n",channel,value);
            }else{
                //printf("PWM 1 Channel %d - Error setting value!\n",channel,value);
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-pwm2")==0 && argc==4)
        {            
            int channel = atoi(argv[2]);
            limitPar(&channel,0,1,"CHANNEL");
            
            int value = atoi(argv[3]);
            limitPar(&value, 0,65535, "VALUE");
            
            if (channel == 0){
                //Set data for output
                OutputData.wPWM2A = (uint16_t) value;  
            }else if (channel == 1){
                //Set data for output
                OutputData.wPWM2B = (uint16_t) value;  
            }            

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("PWM 2 Channel %d set to [%d]\n",channel,value);
            }else{
                //printf("PWM 2 Channel %d - Error setting value!\n",channel,value);
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-pwm0c")==0 && argc==4)
        {   
            int value0 = atoi(argv[2]);
            limitPar(&value0, 0,255, "VALUE0");
            int value1 = atoi(argv[3]);
            limitPar(&value1, 0,65535, "VALUE1");
            
            OutputData.byPWM0Ctrl0 = (uint8_t) value0;
            OutputData.wPWM0Ctrl1 = (uint16_t) value1; 
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("PWM0Ctrl0 set to [%d] and PWM0Ctrl1 set to [%d]\n",value0, value1);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-pwm1c")==0 && argc==4)
        {            
            int value0 = atoi(argv[2]);
            limitPar(&value0, 0,255, "VALUE0");
            int value1 = atoi(argv[3]);
            limitPar(&value1, 0,65535, "VALUE1");
            
            OutputData.byPWM1Ctrl0 = (uint8_t) value0;
            OutputData.wPWM1Ctrl1 = (uint16_t) value1; 
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("PWM1Ctrl0 set to [%d] and PWM1Ctrl1 set to [%d]\n",value0, value1);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-pwm2c")==0 && argc==4)
        {           
            int value0 = atoi(argv[2]);
            limitPar(&value0, 0,255, "VALUE0");
            int value1 = atoi(argv[3]);
            limitPar(&value1, 0,65535, "VALUE1");
            
            OutputData.byPWM2Ctrl0 = (uint8_t) value0;
            OutputData.wPWM2Ctrl1 = (uint16_t) value1; 

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("PWM2Ctrl0 set to [%d] and PWM2Ctrl1 set to [%d]\n",value0, value1);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-retw")==0 && argc==4)
        {           
            int retindex = atoi(argv[2]);
            limitPar(&retindex, 0,63, "RETINDEX");
            int value = atoi(argv[3]);
            limitPar(&value, 0,255, "VALUE");
            
            OutputData.abyRetainDataOut[retindex] = value;

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Retain index [%d] set to value [%d]\n", retindex, value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-retr")==0 && argc==3)
        {   
            int value;
            int retindex = atoi(argv[2]);
            limitPar(&retindex, 0,63, "RETINDEX");

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                value = InputData.abyRetainDataIn[retindex];
                printf("Retain index [%d] has value [%d]\n", retindex, value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-shw")==0 && argc==3)
        {
			if (strcmp(argv[2],"rs232")==0) {
				Change_Serial_Mode(0);
				printf("Changed serial hardware mode to RS232\n");
			}
			else if(strcmp(argv[2],"rs485")==0) {
                Change_Serial_Mode(1);
				printf("Changed serial hardware mode to RS485\n");
			}
			else  {
				printf("Error: Unknown hardware mode %s. Valid options are: [rs232/rs485]\n",argv[2]);
			}
        }
        else if(strcmp(argv[1],"-swc")==0 && argc==5)
        {
            char strDevice[256];
            int  baudRate;

            strcpy(strDevice, argv[2]);
            baudRate = atoi(argv[3]);

            fd = serialOpen(strDevice, baudRate);
            if(fd==-1)
            {
                printf("Error: Couldn't open serial device\n");
            }
            else
            {
                printf("Opened serial device [fd=%d]\n",fd);
                serialPutchar(fd,*argv[4]);
                printf("put char %c\n",*argv[4]);
                serialFlush(fd);
                printf("Flushed serial device\n");
                serialClose(fd);
                printf("Closed serial device\n");
            }
        }
        else if(strcmp(argv[1],"-sws")==0 && argc>=5)
        {
            char str[256];
            char strDevice[256];
            int  baudRate;

            int i;
            strcpy(strDevice, argv[2]);
            baudRate = atoi(argv[3]);

            strcpy(str,argv[4]);
            for(i=5;i<argc;i++)
            {
                strcat(str," ");
                strcat(str,argv[i]);
            }

            fd = serialOpen(strDevice,baudRate);
            if(fd==-1)
            {
                printf("Error: Couldn't open serial device\n");
            }
            else
            {
                printf("Opened serial device [fd=%d]\n",fd);
                serialPuts(fd,str);
                printf("put string %s\n",str);
                serialClose(fd);
                printf("Closed serial device\n");
            }
        }
        else if(strcmp(argv[1],"-sr")==0 && argc==4)
        {
            int chr;
            int nRet;
            char strDevice[256];
            int  baudRate;

            strcpy(strDevice, argv[2]);
            baudRate = atoi(argv[3]);

            signal(SIGINT, signal_callback_handler);

            fd = serialOpen(strDevice,baudRate);
            if(fd==-1)
            {
                printf("Error: Couldn't open serial device\n");
            }
            else
            {
                printf("Opened serial device [fd=%d]\n",fd);
                printf("Recv: ");
                fflush(stdout);
                while(1)
                {
                    nRet = serialDataAvail(fd);
                    if (nRet>=0)
                    {
                        chr = serialGetchar(fd);
                        if(chr!=-1)
                        {
                            printf("%c",chr);
                            fflush(stdout);
                        }
                    }
                    else
                    {
                        printf("Error: serialDataAvail=-1\n");
                        serialFlush(fd);
                        printf("Flushed serial device\n");
                        serialClose(fd);
                        printf("Closed serial device\n");
                        return 0;
                    }

                }
            }
        }
        else if(strcmp(argv[1],"-ucc1")==0 && argc==3)
        {            
            int value = atoi(argv[2]);
            limitPar(&value, 0,255, "VALUE");
            
            //Set data for output
            OutputData.byUCCtrl1 = value;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Microcontroller control register 1 set to [%d]\n",value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-ucr")==0 && argc==2)
        {
            doProcessDataReset();         
            
            Spi_uC_Reset();
            printf("Microcontroller reset successful!\n");
        }
        else if(strcmp(argv[1],"-ucs")==0 && argc==2)
        {
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Microcontroller status register is [%d]\n",InputData.byUCState);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-ucv")==0 && argc==2)
        {
            char str[4] = "";
            unsigned char byte = 42;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            byte = gModelIn;        
            snprintf(str, sizeof(str), "%hhu", byte);            
            printf("Microcontroller version is [%d.%d] on PiXtend V2 -L- model number %s\n", gHardware, gFirmware, str);
        }
        else if(strcmp(argv[1],"-ucw")==0 && argc==2)
        {
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Microcontroller warnings register is [%d]\n",InputData.byUCWarnings);
            }else{
                errno = RetVal;
            }
        }        
        else if(strcmp(argv[1],"-ucc0")==0 && argc==3)
        {           
            int value = atoi(argv[2]);
            limitPar(&value, 0,255, "VALUE");
            //Set data for output
            OutputData.byUCCtrl0 = value;
            
            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }            
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                printf("Microcontroller control register 0 set to [%d]\n",value);
            }else{
                errno = RetVal;
            }
        }
        else if(strcmp(argv[1],"-all")==0 && argc==33)
        {
            //-----------------------------
            //PiXtend V2 -L-
            //-----------------------------
            int value = atoi(argv[2]);
            limitPar(&value, 0,255, "VALUE");
            OutputData.byUCCtrl0 = value;
            value = atoi(argv[3]);
            limitPar(&value, 0,255, "VALUE");            
            OutputData.byUCCtrl1 = value; 
            value = atoi(argv[4]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce01 = value;
            value = atoi(argv[5]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce23 = value;
            value = atoi(argv[6]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce45 = value;
            value = atoi(argv[7]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce67 = value;
            value = atoi(argv[8]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce89 = value;
            value = atoi(argv[9]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce1011 = value;  
            value = atoi(argv[10]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce1213 = value;
            value = atoi(argv[11]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalInDebounce1415 = value;
            value = atoi(argv[12]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalOut0 = value;
            value = atoi(argv[13]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byDigitalOut1 = value;
            value = atoi(argv[14]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byRelayOut = value;
            value = atoi(argv[15]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byGPIOCtrl = value;
            value = atoi(argv[16]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byGPIOOut = value;
            value = atoi(argv[17]);
            limitPar(&value, 0,255, "VALUE");             
            OutputData.byGPIODebounce01 = value;
            value = atoi(argv[18]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byGPIODebounce23 = value;
            value = atoi(argv[19]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byPWM0Ctrl0 = value;
            value = atoi(argv[20]);
            limitPar(&value, 0,65535, "VALUE");
            OutputData.wPWM0Ctrl1 = value;
            value = atoi(argv[21]);
            limitPar(&value, 0,65535, "VALUE");
            OutputData.wPWM0A = value;
            value = atoi(argv[22]);
            limitPar(&value, 0,65535, "VALUE");            
            OutputData.wPWM0B = value;
             value = atoi(argv[23]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byPWM1Ctrl0 = value;
            value = atoi(argv[24]);
            limitPar(&value, 0,65535, "VALUE");
            OutputData.wPWM1Ctrl1 = value;
            value = atoi(argv[25]);
            limitPar(&value, 0,65535, "VALUE");
            OutputData.wPWM1A = value;
            value = atoi(argv[26]);
            limitPar(&value, 0,65535, "VALUE");            
            OutputData.wPWM1B = value;
            value = atoi(argv[27]);
            limitPar(&value, 0,255, "VALUE");              
            OutputData.byPWM2Ctrl0 = value;
            value = atoi(argv[28]);
            limitPar(&value, 0,65535, "VALUE");
            OutputData.wPWM2Ctrl1 = value;
            value = atoi(argv[29]);
            limitPar(&value, 0,65535, "VALUE");
            OutputData.wPWM2A = value;
            value = atoi(argv[30]);
            limitPar(&value, 0,65535, "VALUE");            
            OutputData.wPWM2B = value;            
            
            //PiXtend V2 -L- DAC
            value = atoi(argv[31]);
            limitPar(&value, 0,65535, "VALUE");            
            OutputDataDAC.wAOut0 = value;
            value = atoi(argv[32]);
            limitPar(&value, 0,65535, "VALUE");            
            OutputDataDAC.wAOut1 = value;

            //Check for compatible PiXtend V2 -L- model
            if (isCompatibleModel() == -1){
                errno = echoModelConflict();
                return errno;
            }
            
            //Do data transfer / communication
            RetVal = autoMode(1);
            //Check return code, 0 = OK
            if (isReturnCodeOK(RetVal) == 0){
                
                printf("ALL-DATA: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",InputData.byFirmware, InputData.byHardware, InputData.byModelIn, InputData.byUCState, InputData.byUCWarnings, InputData.byDigitalIn0, InputData.byDigitalIn1, InputData.wAnalogIn0, InputData.wAnalogIn1, InputData.wAnalogIn2, InputData.wAnalogIn3, InputData.wAnalogIn4, InputData.wAnalogIn5, InputData.byGPIOIn, InputData.wTemp0, InputData.byTemp0Error, InputData.wTemp1, InputData.byTemp1Error, InputData.wTemp2, InputData.byTemp2Error, InputData.wTemp3, InputData.byTemp3Error, InputData.wHumid0, InputData.wHumid1, InputData.wHumid2, InputData.wHumid3);
            }else{
                errno = RetVal;
            }
        }        
        else
        {
            printf("Wrong command: ");
            for(i = 0; i < argc; i++)
            {
                printf("%s ",argv[i]);
            }
            printf("\n\n");
            help();
        }
    }
    else
    {
        help();
    }
   
    //Save current data to memory
    save_data_shm();
    
    // Wait in case user calls this program again right after it ends
    // or uses it in a loop for maximum performance.
    delay(nDelayTime);
    
    return errno;
}
