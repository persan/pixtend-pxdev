/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <http://www.pixtend.de> or <http://www.pixtend.com>
#
# Copyright (C) 2014 Nils Mensing, Christian Strobel
# Qube Solutions UG (haftungsbeschränkt), Luitgardweg 18
# 71083 Herrenberg, Germany 
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
#include <string.h>
#include <signal.h>
#include <pixtend.h>

static int fd;

void signal_callback_handler(int signum)
{
	printf("\n\nCaught signal %d\n",signum);
	serialFlush(fd);
	printf("Flushed serial device\n");
	serialClose(fd);
	printf("Closed serial device\n");
	exit(signum);
}

void help()
{
	printf("PiXtend Tool - www.pixtend.de\n");
	printf("usage: pixtendtool [OPTION] [VALUE(s)]\n");
	printf("available options:\n");
	printf("\t-h \t\t\t\tprints this help\n");
	printf("\t-do VALUE \t\t\tset the digital output byte\n");
	printf("\t-di \t\t\t\tget the digital input byte\n");
	printf("\t-ai INDEX \t\t\tget the analog value of AI[INDEX]\n");
	printf("\t-aic VALUE0 VALUE1 \t\twrite AI Control Registers\n");
	printf("\t-ao ENABLE CHANNEL GAIN VALUE \tset the analog output bytes\n");
	printf("\t-rel VALUE \t\t\tset the relay output byte\n");
	printf("\t-gw VALUE \t\t\twrite GPIO Byte\n");
	printf("\t-gr \t\t\t\tread GPIO Byte\n");
	printf("\t-gc VALUE \t\t\twrite GPIO Control Register\n");
	printf("\t-tr CHANNEL \t\t\tread temperature\n");
	printf("\t-hr CHANNEL \t\t\tread humidity\n");
	printf("\t-srv CHANNEL VALUE \t\tset a servo value\n");
	printf("\t-pwm CHANNEL VALUE \t\tset a pwm value\n");
	printf("\t-pwmc VALUE0 VALUE1 VALUE2 \twrite PWM Control Registers\n");
	printf("\t-swc CHAR\t\t\twrites a char on rs232\n");
	printf("\t-sws STRING\t\t\twrites a string on rs232 (max. len=16)\n");
	printf("\t-sr \t\t\t\treads data from rs232 until Ctrl^C\n");
	printf("\t-ucc VALUE \t\t\twrite uC Control Register\n");
	printf("\t-ucr \t\t\t\tReset the microcontroller\n");
	printf("\t-ucs \t\t\t\tRead microcontroller Status Register\n");
	printf("\t-ucv \t\t\t\tRead microcontroller Version\n");
	printf("\t-rasp VALUE \t\t\twrite the RaspberryPi Status Register\n");
	return;
}

int main(int argc, char *argv[]) 
{
	int i=0;
	int nRet;
	uint16_t value;
	uint16_t uiRet;
	
	if(argc > 1)
	{
		if(strcmp(argv[1],"-h")==0)
		{
			help();
		}
		else if(strcmp(argv[1],"-do")==0 && argc==3)
		{
			uint8_t value;
			Spi_Setup(0);
			value = atoi(argv[2]);
			if (value > 255) 
			{
				value = 255;
			}
			Spi_Set_Dout(value);
			printf("Digital outputs set [%s]\n",argv[2]);
		}
		else if(strcmp(argv[1],"-di")==0 && argc==2)
		{
			Spi_Setup(0);
			nRet = Spi_Get_Din();
			printf("Digital inputs: %d\n",nRet);
		}
		else if(strcmp(argv[1],"-ai")==0 && argc==3)
		{
			Spi_Setup(0);
			uiRet = Spi_Get_Ain(atoi(argv[2]));
			printf("Analog input %s: %d\n",argv[2],uiRet);
		}
		else if(strcmp(argv[1],"-aic")==0 && argc==4)
		{
			Spi_Setup(0);
			Spi_Set_AiControl(atoi(argv[2]),atoi(argv[3]));
			printf("AI Control Registers set to [%s], [%s]\n",argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"-ao")==0 && argc==6)
		{
			value = atoi(argv[5]);
			if (value > 1023)
			{
				value = 1023;
			}	
			Spi_Setup(1);
			Spi_Set_Aout(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),value);
			printf("Analog output %s set to %d [enabled=%s; gain=%s]\n",argv[3],value,argv[2],argv[4]);
		}
		else if(strcmp(argv[1],"-rel")==0 && argc==3)
		{
			uint8_t value;
			value = atoi(argv[2]);
			if (value > 255) 
			{
				value = 255;
			}
			Spi_Setup(0);
			Spi_Set_Relays(value);
			printf("Relay outputs set [%s]\n",argv[2]);
		}
		else if(strcmp(argv[1],"-gr")==0 && argc==2)
		{
			Spi_Setup(0);
			nRet = Spi_Get_Gpio();
			printf("GPIO Byte: %d\n",nRet);
		}
		else if(strcmp(argv[1],"-gw")==0 && argc==3)
		{
			Spi_Setup(0);
			Spi_Set_Gpio(atoi(argv[2]));
			printf("GPIO Byte set [%s]\n",argv[2]);
		}
		else if(strcmp(argv[1],"-gc")==0 && argc==3)
		{
			Spi_Setup(0);
			Spi_Set_GpioControl(atoi(argv[2]));
			printf("GPIO Control Register set [%s]\n",argv[2]);
		}
		else if(strcmp(argv[1],"-tr")==0 && argc==3)
		{
			Spi_Setup(0);
			uiRet = Spi_Get_Temp(atoi(argv[2]));
			printf("Temperature%s: %d\n",argv[2],uiRet);
		}
		else if(strcmp(argv[1],"-hr")==0 && argc==3)
		{
			Spi_Setup(0);
			uiRet = Spi_Get_Hum(atoi(argv[2]));
			printf("Humidity%s: %d\n",argv[2],uiRet);
		}
		else if(strcmp(argv[1],"-srv")==0 && argc==4)
		{	
			Spi_Setup(0);
			nRet = Spi_Set_Servo(atoi(argv[2]),atoi(argv[3]));
			printf("Servo%s set to %s\n",argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"-pwm")==0 && argc==4)
		{	
			value = atoi(argv[3]);
					
			Spi_Setup(0);
			nRet = Spi_Set_Pwm(atoi(argv[2]),value);
			printf("PWM%s set to %s\n",argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"-pwmc")==0 && argc==5)
		{
			Spi_Setup(0);
			Spi_Set_PwmControl(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
			printf("PWM Control Registers set to [%s], [%s], [%s]\n",argv[2],argv[3],argv[4]);
		}
		else if(strcmp(argv[1],"-swc")==0 && argc==3)
		{
			fd = serialOpen("/dev/ttyAMA0",9600);
			if(fd==-1)
			{
				printf("Error: Couldn't open serial device\n");
			}
			else
			{
				printf("Opened serial device [fd=%d]\n",fd);
				serialPutchar(fd,*argv[2]);
				printf("put char %c\n",*argv[2]);
				serialFlush(fd);
				printf("Flushed serial device\n");
				serialClose(fd);
				printf("Closed serial device\n");
			}
		}
		else if(strcmp(argv[1],"-sws")==0 && argc>=3)
		{
			char str[256];
			int i;
			
			strcpy(str,argv[2]);
			for(i=3;i<argc;i++)
			{
				strcat(str," ");
				strcat(str,argv[i]);
			}
			
			fd = serialOpen("/dev/ttyAMA0",9600);
			if(fd==-1)
			{
				printf("Error: Couldn't open serial device\n");
			}
			else
			{
				printf("Opened serial device [fd=%d]\n",fd);
				serialPuts(fd,str);
				printf("put string %s\n",str);
				serialFlush(fd);
				printf("Flushed serial device\n");
				serialClose(fd);
				printf("Closed serial device\n");
			}
		}
		else if(strcmp(argv[1],"-sr")==0 && argc==2)
		{
			int chr;
			int nRet;
			
			signal(SIGINT, signal_callback_handler);
			
			fd = serialOpen("/dev/ttyAMA0",9600);
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
		else if(strcmp(argv[1],"-ucc")==0 && argc==3)
		{
			Spi_Setup(0);
			Spi_Set_UcControl(atoi(argv[2]));
			printf("uC Control Register set [%s]\n",argv[2]);
		}		
		else if(strcmp(argv[1],"-ucr")==0 && argc==2)
		{
			nRet = Spi_uC_Reset();
			printf("Microcontroller reset successful!\n");
		}
		else if(strcmp(argv[1],"-ucs")==0 && argc==2)
		{
			Spi_Setup(0);
			nRet = Spi_Get_uC_Status();
			printf("Microcontroller Status Register: %d\n", nRet);
		}
		else if(strcmp(argv[1],"-ucv")==0 && argc==2)
		{
			uint16_t version;
			Spi_Setup(0);
			version = Spi_Get_uC_Version();
			printf("Microcontroller Version is %d.%d\n", (uint8_t)(version>>8), (uint8_t)(version & 0xFF));
		}
		else if(strcmp(argv[1],"-rasp")==0 && argc==3)
		{
			Spi_Setup(0);
			Spi_Set_RaspStat(atoi(argv[2]));
			printf("Raspberry Status Register set [%s]\n",argv[2]);
		}
		else
		{
			printf("Wrong command: ");
			for(i=0;i<argc;i++)
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
	
	return 0;
}
