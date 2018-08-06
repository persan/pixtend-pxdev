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

#include "pxauto2s.h"

#define PXAUTO_HEADERSTRING "PiXtend V2 -S- Auto Tool V0.5.6 - https://www.pixtend.de"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define KEY_RETURN '\n'

//static int tCount;
static int nCount;
static int tCount;
static int nAMRetVal;
static unsigned int uiCount;

static const uint8_t byModel = 83;
static const int nPanWidth = 52;

//update request after screen/terminal window resize
int updateRequest;

//PiXtend V2 -S- input data
struct pixtInV2S InputData;

//PiXtend V2 -S- output data
struct pixtOutV2S OutputData;

//PiXtend V2 -S- DAC Output Data
struct pixtOutDAC OutputDataDAC;

//Timer data and poll timings
int nMenuUpdateCount = 20;
static const long lDefaultPollTime = 10000000;
static const long lDhtPollTime = 30000000;
long lTimerPollTime = 10000000;
timer_t gTimerid;

enum CurMode {MENU_MODE=0, EDIT_MODE};
enum CurMode cur_Mode;

//Boolean Field Values
char *sBoolean[] = { 
					"False\0", 
					"True\0",
					(char *)NULL
				  };

//Vref Field Values
char *sVRef[] = { 
					"5.0V\0", 
					"10.0V\0",
					(char *)NULL
				  };

//DHT11 - DHT22 Choice Field Values
char *sDHTChoice[] = { 
					"DHT11\0", 
					"DHT22\0",
					(char *)NULL
				  };

//Menu Entries				  
char *choices[] = {
					"Home",
					"DIn",
					"AIn",
					"GPIO",
					"DOut",
					"AOut",
					"PWM",
					"Ctrl",
					"Stat",
					"RetIn",
					"RetOut",
					(char *)NULL,
					};

MENU *my_menu;
ITEM **my_items;

FORM  *cur_form;
FORM  *form_DIN;
FORM  *form_DOUT;
FORM  *form_AOUT;
FORM  *form_AIN;
FORM  *form_GPIO;
FORM  *form_PWM;
FORM  *form_CTRL;
FORM  *form_STAT;
FORM  *form_RET_IN;
FORM  *form_RET_OUT;

FIELD *field_DIN[9];
FIELD *field_DOUT[9];
FIELD *field_AOUT[3];
FIELD *field_AIN[19];
FIELD *field_GPIO[9];
FIELD *field_PWM[5];
FIELD *field_CTRL[16];
FIELD *field_STAT[6];
FIELD *field_RET_IN[33];
FIELD *field_RET_OUT[33];

WINDOW *win_menu;
WINDOW *win_header;
WINDOW *win_HOME;
WINDOW *win_DIN;
WINDOW *win_AIN;
WINDOW *win_GPIO;
WINDOW *win_DOUT;
WINDOW *win_AOUT;
WINDOW *win_PWM;
WINDOW *win_CTRL;
WINDOW *win_STAT;
WINDOW *win_RET_IN;
WINDOW *win_RET_OUT;

PANEL *pan_HOME;
PANEL *pan_DIN;
PANEL *pan_AIN;
PANEL *pan_GPIO;
PANEL *pan_DOUT;
PANEL *pan_AOUT;
PANEL *pan_PWM;
PANEL *pan_CTRL;
PANEL *pan_STAT;
PANEL *pan_RET_IN;
PANEL *pan_RET_OUT;

//Taken from the Internet at :
//https://stackoverflow.com/questions/6182877/file-locks-for-linux
int acquireLock (char *fileSpec) {
    int lockFd;

    if ((lockFd = open (fileSpec, O_CREAT | O_RDWR, 0666))  < 0)
        return -1;

    if (flock (lockFd, LOCK_EX | LOCK_NB) < 0) {
        close (lockFd);
        return -1;
    }

    return lockFd;
}

void releaseLock (int lockFd) {
    flock (lockFd, LOCK_UN);
    close (lockFd);
}


//Time Base for timer_callback() calls
void start_timer(void)
{
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = lTimerPollTime;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = lTimerPollTime;
	timer_create (CLOCK_REALTIME, NULL, &gTimerid);
	timer_settime (gTimerid, 0, &value, NULL);
}

void set_new_timer(void)
{
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = lTimerPollTime;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = lTimerPollTime;
	timer_settime (gTimerid, 0, &value, NULL);
}

void stop_timer(void)
{
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	timer_settime (gTimerid, 0, &value, NULL);
}

// Timer Callback, executed every 10ms or 30ms
void timer_callback(int sig) {
    
    if (lTimerPollTime == lDefaultPollTime)
    {
        nMenuUpdateCount = 10;
    }
    if (lTimerPollTime == lDhtPollTime)
    {
        nMenuUpdateCount = 3;
    }

	//Increment static counter
	tCount++;
	uiCount++;

	//Execute every 10ms or 30ms
	autoMode();
	
	//Execute every second
	if (tCount == nMenuUpdateCount)
	{
		tCount = 0;
		nCount++;
		//Initiate Screen Update
		menu_function();	
	}
    
    //Check which Polltime to use - (1 << bit) & byte
    if ((((1 << 4) & OutputData.byGPIOCtrl) == 16) || (((1 << 5) & OutputData.byGPIOCtrl) == 32) || (((1 << 6) & OutputData.byGPIOCtrl) == 64) || (((1 << 7) & OutputData.byGPIOCtrl) == 128))
    {
        if (lTimerPollTime != lDhtPollTime)
        {
            lTimerPollTime =  lDhtPollTime;
            set_new_timer();
        }
    }
    else
    {
        if (lTimerPollTime != lDefaultPollTime)
        {
            lTimerPollTime =  lDefaultPollTime;
            set_new_timer();            
        }
    }    
    
}

void init_HOME() {
	win_HOME = newwin(30, nPanWidth , 8, 16);
	pan_HOME = new_panel(win_HOME);
	print_in_color(win_HOME, 1, 2, "HOME", COLOR_PAIR(2));
	mvwaddstr(win_HOME,  4, 2, "This Tool allows you to monitor and ");
	mvwaddstr(win_HOME,  5, 2, "control your PiXtend V2 -S- Board");
	mvwaddstr(win_HOME,  6, 2, "for test purposes.");
	mvwaddstr(win_HOME,  7, 2, "Feel free to modify and adapt this to");
	mvwaddstr(win_HOME,  8, 2, "your own needs.");	
	mvwaddstr(win_HOME,  9, 2, "");	
	mvwaddstr(win_HOME, 10, 2, "Usage:");
	mvwaddstr(win_HOME, 11, 2, " Use UP and DOWN to navigate the MENU");
	mvwaddstr(win_HOME, 12, 2, " Hit ENTER to toggle EDIT Mode");	
	mvwaddstr(win_HOME, 13, 2, " Enter values by typing them in or");
	mvwaddstr(win_HOME, 14, 2, " use LEFT and RIGHT to change Booleans");
	mvwaddstr(win_HOME, 15, 2, " ");
	mvwaddstr(win_HOME, 16, 2, "Visit http://www.pixtend.de for more...");	
	mvwaddstr(win_HOME, 17, 2, "2014-2018, Qube Solutions GmbH");
	mvwaddstr(win_HOME, 18, 2, " ");
	wnoutrefresh(win_header);
	box(win_HOME, 0, 0);
}

void init_DIN() {
	win_DIN = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields DIN	 
	field_DIN[0] = new_field(1, 5,  2, 8, 0, 0);
	field_DIN[1] = new_field(1, 5,  4, 8, 0, 0);
	field_DIN[2] = new_field(1, 5,  6, 8, 0, 0);
	field_DIN[3] = new_field(1, 5,  8, 8, 0, 0);
	field_DIN[4] = new_field(1, 5, 10, 8, 0, 0);
	field_DIN[5] = new_field(1, 5, 12, 8, 0, 0);
	field_DIN[6] = new_field(1, 5, 14, 8, 0, 0);
	field_DIN[7] = new_field(1, 5, 16, 8, 0, 0);
	field_DIN[8] = NULL;
	
	for(i=0; i<8; i++)
	  {
		set_field_fore(field_DIN[i], COLOR_PAIR(2));
		set_field_back(field_DIN[i], COLOR_PAIR(2));
		field_opts_off(field_DIN[i], O_AUTOSKIP);
		field_opts_off(field_DIN[i], O_ACTIVE);
		field_opts_off(field_DIN[i], O_EDIT);
		set_field_type(field_DIN[i], TYPE_ALPHA, 5);
	  }
	
	//Form stuff	 
	form_DIN = new_form(field_DIN);
	set_form_win(form_DIN, win_DIN);
	set_form_sub(form_DIN, derwin(win_DIN, 28, 42, 2, 2));
	post_form(form_DIN);
	
	pan_DIN = new_panel(win_DIN);
	print_in_color(win_DIN, 1, 2, "DIn", COLOR_PAIR(2));
	box(win_DIN, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<8; i++) {
		mvwprintw(win_DIN, 4+i*2, 2, "DI%d:",i);
	}
	wnoutrefresh(win_DIN);
}

void init_AIN() {
	win_AIN = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields AIN 
	field_AIN[0] = new_field(1, 5,  2, 8, 0, 0);
	field_AIN[1] = new_field(1, 5,  4, 8, 0, 0);
	field_AIN[2] = new_field(1, 5,  6, 8, 0, 0);
	field_AIN[3] = new_field(1, 5,  8, 8, 0, 0);
	field_AIN[4] = new_field(1, 5,  10, 8, 0, 0);
	field_AIN[5] = new_field(1, 5,  12, 8, 0, 0);
	field_AIN[6] = new_field(1, 5,  14, 8, 0, 0);
	field_AIN[7] = new_field(1, 5,  16, 8, 0, 0);
	field_AIN[8] = new_field(1, 5,  18, 8, 0, 0);
	field_AIN[9] = new_field(1, 5,  20, 8, 0, 0);
    field_AIN[10] = new_field(1, 5,  24, 10, 0, 0);
    field_AIN[11] = new_field(1, 5,  24, 31, 0, 0);
    field_AIN[12] = new_field(1, 5,  26, 10, 0, 0);
    field_AIN[13] = new_field(1, 5,  26, 31, 0, 0);
    field_AIN[14] = new_field(1, 5,  6, 37, 0, 0);
    field_AIN[15] = new_field(1, 5,  8, 37, 0, 0);
    field_AIN[16] = new_field(1, 5, 10, 37, 0, 0);
    field_AIN[17] = new_field(1, 5, 12, 37, 0, 0);
	field_AIN[18] = NULL;
	
	for(i=0; i<10; i++)
	{
		set_field_fore(field_AIN[i], COLOR_PAIR(2));
		set_field_back(field_AIN[i], COLOR_PAIR(2));
		field_opts_off(field_AIN[i], O_AUTOSKIP);
		field_opts_off(field_AIN[i], O_EDIT);
        field_opts_off(field_AIN[i], O_ACTIVE);
		set_field_type(field_AIN[i], TYPE_INTEGER, 0, 0,65535);
	}
    
	for(i=10; i<14; i++)
	{
        set_field_fore(field_AIN[i], COLOR_PAIR(2));
        set_field_back(field_AIN[i], COLOR_PAIR(2));
        field_opts_off(field_AIN[i], O_AUTOSKIP);
        field_opts_off(field_AIN[i], O_EDIT);
        field_opts_off(field_AIN[i], O_ACTIVE);
        set_field_type(field_AIN[i], TYPE_INTEGER, 0, 0,1);
	}
    
	for(i=14; i<18; i++)
	  {
		set_field_fore(field_AIN[i], COLOR_PAIR(1));
		set_field_back(field_AIN[i], COLOR_PAIR(1));
		field_opts_off(field_AIN[i], O_AUTOSKIP);
		set_field_type(field_AIN[i], TYPE_ENUM, sDHTChoice, 0,0);
	  }    
    
	//Form stuff	 
	form_AIN = new_form(field_AIN);
	set_form_win(form_AIN, win_AIN);
	set_form_sub(form_AIN, derwin(win_AIN, 28, 42, 2, 2));
	post_form(form_AIN);
	pan_AIN = new_panel(win_AIN);
	print_in_color(win_AIN, 1, 2, "AIn", COLOR_PAIR(2));
    print_in_color(win_AIN, 1, 40, "GPIO Conf", COLOR_PAIR(1));
    print_in_color(win_AIN, 2, 39, "DHT11/DHT22", COLOR_PAIR(1));
	box(win_AIN, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<2; i++) {
		mvwprintw(win_AIN, 4+i*2, 2, "AI%d:",i);
	}
	for(i = 0; i<4; i++) {
		mvwprintw(win_AIN, 8+i*2, 2, "Temp%d:",i);
	}
	for(i = 0; i<4; i++) {
		mvwprintw(win_AIN, 16+i*2, 2, "Humid%d:",i);
	}
    mvwprintw(win_AIN, 26, 2, "T0/H0 Err:");
    mvwprintw(win_AIN, 26, 23, "T1/H1 Err:");
    mvwprintw(win_AIN, 28, 2, "T2/H2 Err:");
    mvwprintw(win_AIN, 28, 23, "T3/H3 Err:");
	wnoutrefresh(win_AIN);
}

void init_DOUT() {
	win_DOUT = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields for DOUT 
	field_DOUT[0] = new_field(1, 5,  2, 8, 0, 0);
	field_DOUT[1] = new_field(1, 5,  4, 8, 0, 0);
	field_DOUT[2] = new_field(1, 5,  6, 8, 0, 0);
	field_DOUT[3] = new_field(1, 5,  8, 8, 0, 0);
	//Init Fields for Relays			
	field_DOUT[4] = new_field(1, 5, 16, 8, 0, 0);
	field_DOUT[5] = new_field(1, 5, 18, 8, 0, 0);
	field_DOUT[6] = new_field(1, 5, 20, 8, 0, 0);
	field_DOUT[7] = new_field(1, 5, 22, 8, 0, 0);
	field_DOUT[8] = NULL;
	
	for(i=0; i<8; i++)
	  {
		set_field_fore(field_DOUT[i], COLOR_PAIR(1));
		set_field_back(field_DOUT[i], COLOR_PAIR(1));
		field_opts_off(field_DOUT[i], O_AUTOSKIP);
		set_field_type(field_DOUT[i], TYPE_ENUM, sBoolean, 0,0);
	  }
	  
	//Form stuff	 
	form_DOUT = new_form(field_DOUT);
	set_form_win(form_DOUT, win_DOUT);
	set_form_sub(form_DOUT, derwin(win_DOUT, 28, 42, 2, 2));
	post_form(form_DOUT);
	pan_DOUT = new_panel(win_DOUT);
	print_in_color(win_DOUT, 1, 2, "DOut", COLOR_PAIR(1));
	print_in_color(win_DOUT, 16, 2, "Relay", COLOR_PAIR(1));
	box(win_DOUT, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<4; i++) {
		mvwprintw(win_DOUT, 4+i*2, 2, "DO%d:",i);
		mvwprintw(win_DOUT, 4+i*2, 21, "<True/False>");
	}
	for(i = 0; i<4; i++) {
		mvwprintw(win_DOUT, 18+i*2, 2, "Rel%d:",i);
		mvwprintw(win_DOUT, 18+i*2, 21, "<True/False>");
	}
	wnoutrefresh(win_DOUT);
}

void init_AOUT() {
	win_AOUT = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields AOUT 
	field_AOUT[0] = new_field(1, 4,  2, 8, 0, 0);
	field_AOUT[1] = new_field(1, 4,  4, 8, 0, 0);
	field_AOUT[2] = NULL;
			
	for(i=0; i<2; i++)
	  {
		set_field_fore(field_AOUT[i], COLOR_PAIR(1));
		set_field_back(field_AOUT[i], COLOR_PAIR(1));
		field_opts_off(field_AOUT[i], O_AUTOSKIP);
		set_field_type(field_AOUT[i], TYPE_INTEGER, 0, 0,1023);
		set_field_just(field_CTRL[i], JUSTIFY_RIGHT);
	  }
	//Form stuff	 
	form_AOUT = new_form(field_AOUT);
	set_form_win(form_AOUT, win_AOUT);
	set_form_sub(form_AOUT, derwin(win_AOUT, 28, 42, 2, 2));
	post_form(form_AOUT);
	pan_AOUT = new_panel(win_AOUT);
	print_in_color(win_AOUT, 1, 2, "AOut", COLOR_PAIR(2));
	box(win_AOUT, 0, 0);	
	
	//Static Label Stuff
	for(i = 0; i<2; i++) {
		mvwprintw(win_AOUT, 4+i*2, 2, "AOut%d:",i);
		mvwprintw(win_AOUT, 4+i*2, 16, "0-1023");
	}
	wnoutrefresh(win_AOUT);
}

void init_GPIO() {
	win_GPIO = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields GPIO 
	field_GPIO[0] = new_field(1, 5,  1,  8, 0, 0);
	field_GPIO[1] = new_field(1, 5,  3,  8, 0, 0);
	field_GPIO[2] = new_field(1, 5,  5,  8, 0, 0);
	field_GPIO[3] = new_field(1, 5,  7,  8, 0, 0);
	field_GPIO[4] = new_field(1, 5,  13, 8, 0, 0);
	field_GPIO[5] = new_field(1, 5,  15, 8, 0, 0);
	field_GPIO[6] = new_field(1, 5,  17, 8, 0, 0);
	field_GPIO[7] = new_field(1, 5,  19, 8, 0, 0);
	field_GPIO[8] = NULL;
			
	for(i=0; i<4; i++)
	  {
		set_field_fore(field_GPIO[i], COLOR_PAIR(2));
		set_field_back(field_GPIO[i], COLOR_PAIR(2));
		field_opts_off(field_GPIO[i], O_AUTOSKIP);
		field_opts_off(field_GPIO[i], O_EDIT);
        field_opts_off(field_GPIO[i], O_ACTIVE);
		set_field_type(field_GPIO[i], TYPE_INTEGER, 0, 0,1);
	  }
	for(i=4; i<8; i++)
	  {
		set_field_fore(field_GPIO[i], COLOR_PAIR(1));
		set_field_back(field_GPIO[i], COLOR_PAIR(1));
		field_opts_off(field_GPIO[i], O_AUTOSKIP);
		set_field_type(field_GPIO[i], TYPE_ENUM, sBoolean, 0,0);
	  }  	
	  
	//Form stuff	 
	form_GPIO = new_form(field_GPIO);
	set_form_win(form_GPIO, win_GPIO);
	set_form_sub(form_GPIO, derwin(win_GPIO, 28, 42, 2, 2));
	post_form(form_GPIO);
	pan_GPIO = new_panel(win_GPIO);
	print_in_color(win_GPIO, 1, 2, "GPIO In", COLOR_PAIR(2));
	print_in_color(win_GPIO, 13, 2, "GPIO Out", COLOR_PAIR(1));
	box(win_GPIO, 0, 0);	
	
	//Static Label Stuff
	for(i = 0; i<4; i++) {
		mvwprintw(win_GPIO, 3+i*2, 2, "In%d:",i);
	}
	//Static Label Stuff
	for(i = 0; i<4; i++) {
		mvwprintw(win_GPIO, 15+i*2, 2, "Out%d:",i);
		mvwprintw(win_GPIO, 15+i*2, 21, "<True/False>");       
	}
	wnoutrefresh(win_GPIO);
}

void init_PWM() {
	win_PWM = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields PWM 
	field_PWM[0] = new_field(1, 5,  2, 8, 0, 0);
	field_PWM[1] = new_field(1, 5,  4, 8, 0, 0);
	field_PWM[2] = new_field(1, 5,  6, 8, 0, 0);
	field_PWM[3] = new_field(1, 5,  8, 8, 0, 0);	
	field_PWM[4] = NULL;
			
	for(i=0; i<2; i++)
	  {
		set_field_fore(field_PWM[i], COLOR_PAIR(1));
		set_field_back(field_PWM[i], COLOR_PAIR(1));
		field_opts_off(field_PWM[i], O_AUTOSKIP);
		set_field_type(field_PWM[i], TYPE_INTEGER, 0, 0,65535);
		set_field_just(field_PWM[i], JUSTIFY_RIGHT);
	  }
	for(i=2; i<4; i++)
	  {
		set_field_fore(field_PWM[i], COLOR_PAIR(1));
		set_field_back(field_PWM[i], COLOR_PAIR(1));
		field_opts_off(field_PWM[i], O_AUTOSKIP);
		set_field_type(field_PWM[i], TYPE_INTEGER, 0, 0,255);
		set_field_just(field_PWM[i], JUSTIFY_RIGHT);
	  }
	  
	//Form stuff	 
	form_PWM = new_form(field_PWM);
	set_form_win(form_PWM, win_PWM);
	set_form_sub(form_PWM, derwin(win_PWM, 28, 42, 2, 2));
	post_form(form_PWM);
	pan_PWM = new_panel(win_PWM);
	print_in_color(win_PWM, 1, 2, "PWM0 (A&B) - PWM1 (A&B)", COLOR_PAIR(1));
	box(win_PWM, 0, 0);	
	
	//Static Label Stuff
	mvwprintw(win_PWM,  4, 2, "PWM0A:");
	mvwprintw(win_PWM,  4, 16, "0-65535");
	mvwprintw(win_PWM,  6, 2, "PWM0B:");
	mvwprintw(win_PWM,  6, 16, "0-65535");
	
	mvwprintw(win_PWM,  8, 2, "PWM1A:");
	mvwprintw(win_PWM,  8, 16, "0-255");
	mvwprintw(win_PWM, 10, 2, "PWM1B:");
	mvwprintw(win_PWM, 10, 16, "0-255");
	
	wnoutrefresh(win_PWM);
}

void init_CTRL() {
	win_CTRL = newwin(30, nPanWidth , 8, 16);
	int i;
	//Init Fields CTRL 
	field_CTRL[0]  = new_field(1, 5,  2,  13, 0, 0);
	field_CTRL[1]  = new_field(1, 3,  3,  13, 0, 0);
	field_CTRL[2]  = new_field(1, 3,  4,  13, 0, 0);
	field_CTRL[3]  = new_field(1, 3,  5,  13, 0, 0);
	field_CTRL[4]  = new_field(1, 3,  7, 13, 0, 0);
	field_CTRL[5]  = new_field(1, 3,  8, 13, 0, 0);
	field_CTRL[6]  = new_field(1, 3,  9, 13, 0, 0);
	field_CTRL[7]  = new_field(1, 3,  11, 13, 0, 0);
	field_CTRL[8]  = new_field(1, 3,  12, 13, 0, 0);
	field_CTRL[9]  = new_field(1, 3,  13, 13, 0, 0);
	field_CTRL[10] = new_field(1, 3,  14, 13, 0, 0);
	field_CTRL[11] = new_field(1, 3,  16, 13, 0, 0);
	field_CTRL[12] = new_field(1, 3,  17, 13, 0, 0);
	field_CTRL[13] = new_field(1, 5,  22, 13, 0, 0);
	field_CTRL[14] = new_field(1, 5,  23, 13, 0, 0);
	field_CTRL[15] = NULL;

	//Control Bytes
	i=0;
	set_field_fore(field_CTRL[i], COLOR_PAIR(1));
	set_field_back(field_CTRL[i], COLOR_PAIR(1));
	field_opts_off(field_CTRL[i], O_AUTOSKIP);
	set_field_type(field_CTRL[i], TYPE_INTEGER, 0, 0,65535);
	set_field_just(field_CTRL[i], JUSTIFY_RIGHT);	
	
	for(i=1; i<13; i++)
	  {
		set_field_fore(field_CTRL[i], COLOR_PAIR(1));
		set_field_back(field_CTRL[i], COLOR_PAIR(1));
		field_opts_off(field_CTRL[i], O_AUTOSKIP);
		set_field_type(field_CTRL[i], TYPE_INTEGER, 0, 0,255);
		set_field_just(field_CTRL[i], JUSTIFY_RIGHT);
	  }
	
	//Jumper 10V Bytes 
	for(i=13; i<15; i++)
	  {
		set_field_fore(field_CTRL[i], COLOR_PAIR(1));
		set_field_back(field_CTRL[i], COLOR_PAIR(1));
		field_opts_off(field_CTRL[i], O_AUTOSKIP);
		set_field_type(field_CTRL[i], TYPE_ENUM, sVRef, 0,0);
	  }
	
	//Form stuff	 
	form_CTRL = new_form(field_CTRL);
	set_form_win(form_CTRL, win_CTRL);
	set_form_sub(form_CTRL, derwin(win_CTRL, 28, 42, 2, 2));
	post_form(form_CTRL);
	pan_CTRL = new_panel(win_CTRL);
	print_in_color(win_CTRL, 1, 2, "Ctrl", COLOR_PAIR(1));
	print_in_color(win_CTRL, 22, 2, "Jumper", COLOR_PAIR(1));
	box(win_CTRL, 0, 0);	
	
	//Static Label Stuff
	mvwaddstr(win_CTRL,   4, 2, "PWM0Ctrl1");
	mvwaddstr(win_CTRL,   5, 2, "PWM0Ctrl0");
	mvwaddstr(win_CTRL,   6, 2, "PWM1Ctrl1");
	mvwaddstr(win_CTRL,   7, 2, "PWM1Ctrl0");
	mvwaddstr(win_CTRL,   9, 2, "GPIOCtrl");
	mvwaddstr(win_CTRL,  10, 2, "GPIODeb01");
	mvwaddstr(win_CTRL,  11, 2, "GPIODeb23");
	mvwaddstr(win_CTRL,  13, 2, "DINDeb01");
	mvwaddstr(win_CTRL,  14, 2, "DINDeb23");
	mvwaddstr(win_CTRL,  15, 2, "DINDeb45");
	mvwaddstr(win_CTRL,  16, 2, "DINDeb67");
	mvwaddstr(win_CTRL,  18, 2, "UCCtrl0");
	mvwaddstr(win_CTRL,  19, 2, "UCCtrl1");
	mvwaddstr(win_CTRL,  24, 2, "AI0VRef");
	mvwaddstr(win_CTRL,  25, 2, "AI1VRef");
	
	//Units
	mvwaddstr(win_CTRL,  4, 21, "0-65535");
	mvwaddstr(win_CTRL,  5, 21, "0-255");
	mvwaddstr(win_CTRL,  6, 21, "0-255");
	mvwaddstr(win_CTRL,  7, 21, "0-255");
	mvwaddstr(win_CTRL,  9, 21, "0-255");
	mvwaddstr(win_CTRL, 10, 21, "0-255");
	mvwaddstr(win_CTRL, 11, 21, "0-255");
	mvwaddstr(win_CTRL, 13, 21, "0-255");
	mvwaddstr(win_CTRL, 14, 21, "0-255");
	mvwaddstr(win_CTRL, 15, 21, "0-255");
	mvwaddstr(win_CTRL, 16, 21, "0-255");
	mvwaddstr(win_CTRL, 18, 21, "0-255");
	mvwaddstr(win_CTRL, 19, 21, "0-255");
	mvwaddstr(win_CTRL, 24, 21, "<5.0V / 10.0V>");
	mvwaddstr(win_CTRL, 25, 21, "<5.0V / 10.0V>");  

	wnoutrefresh(win_CTRL);
}

void init_STAT() {
	win_STAT = newwin(30, nPanWidth, 8, 16);
	int i;
	//Init Fields STAT 
	field_STAT[0] = new_field(1, 3,  2,  13, 0, 0);
	field_STAT[1] = new_field(1, 3,  4,  13, 0, 0);
	field_STAT[2] = new_field(1, 3,  6,  13, 0, 0);
	field_STAT[3] = new_field(1, 3,  8,  13, 0, 0);
    field_STAT[4] = new_field(1, 3, 10,  13, 0, 0);
	field_STAT[5] = NULL;
	
	//Status Bytes (read only)
	for(i=0; i<5; i++)
	  {
		set_field_fore(field_STAT[i], COLOR_PAIR(2));
		set_field_back(field_STAT[i], COLOR_PAIR(2));
		field_opts_off(field_STAT[i], O_AUTOSKIP);
		field_opts_off(field_STAT[i], O_EDIT);
		set_field_type(field_STAT[i], TYPE_INTEGER, 0, 0,255);
		set_field_just(field_STAT[i], JUSTIFY_RIGHT);
	  }  
	
	//Form stuff	 
	form_STAT = new_form(field_STAT);
	set_form_win(form_STAT, win_STAT);
	set_form_sub(form_STAT, derwin(win_STAT, 28, 42, 2, 2));
	post_form(form_STAT);
	pan_STAT = new_panel(win_STAT);
	print_in_color(win_STAT, 1, 2, "Stat", COLOR_PAIR(2));
	box(win_STAT, 0, 0);	
	
	//Static Label Stuff
	mvwaddstr(win_STAT, 2+2, 2, "Firmware");
	mvwaddstr(win_STAT, 2+4, 2, "Hardware");
	mvwaddstr(win_STAT, 2+6, 2, "UCState");
	mvwaddstr(win_STAT, 2+8, 2, "ModelIn");
    mvwaddstr(win_STAT,2+10, 2, "UCWarnings");

	wnoutrefresh(win_STAT);
}

void init_RET_IN() {
	win_RET_IN = newwin(30, nPanWidth, 8, 16);
	int i;
	//Init Fields for RET_IN
	field_RET_IN[0]  = new_field(1, 3,  2, 6, 0, 0);
	field_RET_IN[1]  = new_field(1, 3,  4, 6, 0, 0);
	field_RET_IN[2]  = new_field(1, 3,  6, 6, 0, 0);
	field_RET_IN[3]  = new_field(1, 3,  8, 6, 0, 0);
	field_RET_IN[4]  = new_field(1, 3,  10, 6, 0, 0);
	field_RET_IN[5]  = new_field(1, 3,  12, 6, 0, 0);
	field_RET_IN[6]  = new_field(1, 3,  14, 6, 0, 0);
	field_RET_IN[7]  = new_field(1, 3,  16, 6, 0, 0);
	field_RET_IN[8]  = new_field(1, 3,  18, 6, 0, 0);
	field_RET_IN[9]  = new_field(1, 3,  20, 6, 0, 0);
	field_RET_IN[10] = new_field(1, 3,  22, 6, 0, 0);
	field_RET_IN[11] = new_field(1, 3,  24, 6, 0, 0);
	field_RET_IN[12] = new_field(1, 3,  2, 20, 0, 0);
	field_RET_IN[13] = new_field(1, 3,  4, 20, 0, 0);
	field_RET_IN[14] = new_field(1, 3,  6, 20, 0, 0);
	field_RET_IN[15] = new_field(1, 3,  8, 20, 0, 0);
	field_RET_IN[16] = new_field(1, 3,  10, 20, 0, 0);
	field_RET_IN[17] = new_field(1, 3,  12, 20, 0, 0);
	field_RET_IN[18] = new_field(1, 3,  14, 20, 0, 0);
	field_RET_IN[19] = new_field(1, 3,  16, 20, 0, 0);
	field_RET_IN[20] = new_field(1, 3,  18, 20, 0, 0);
	field_RET_IN[21] = new_field(1, 3,  20, 20, 0, 0);
	field_RET_IN[22] = new_field(1, 3,  22, 20, 0, 0);
	field_RET_IN[23] = new_field(1, 3,  24, 20, 0, 0);
	field_RET_IN[24] = new_field(1, 3,  2, 34, 0, 0);
	field_RET_IN[25] = new_field(1, 3,  4, 34, 0, 0);
	field_RET_IN[26] = new_field(1, 3,  6, 34, 0, 0);
	field_RET_IN[27] = new_field(1, 3,  8, 34, 0, 0);
	field_RET_IN[28] = new_field(1, 3,  10, 34, 0, 0);
	field_RET_IN[29] = new_field(1, 3,  12, 34, 0, 0);
	field_RET_IN[30] = new_field(1, 3,  14, 34, 0, 0);
	field_RET_IN[31] = new_field(1, 3,  16, 34, 0, 0);
	field_RET_IN[32] = NULL;
	
	for(i=0; i<32; i++)
	  {
		set_field_fore(field_RET_IN[i], COLOR_PAIR(2));
		set_field_back(field_RET_IN[i], COLOR_PAIR(2));
		field_opts_off(field_RET_IN[i], O_AUTOSKIP);
		field_opts_off(field_RET_IN[i], O_ACTIVE);
		field_opts_off(field_RET_IN[i], O_EDIT);		
		set_field_type(field_RET_IN[i], TYPE_INTEGER, 0, 0,255);
		set_field_just(field_RET_IN[i], JUSTIFY_RIGHT);
	  }
	
	//Form stuff	 
	form_RET_IN = new_form(field_RET_IN);
	set_form_win(form_RET_IN, win_RET_IN);
	set_form_sub(form_RET_IN, derwin(win_RET_IN, 28, 42, 2, 2));
	post_form(form_RET_IN);
	
	pan_RET_IN = new_panel(win_RET_IN);
	print_in_color(win_RET_IN, 1, 2, "RetainDataIn", COLOR_PAIR(2));
	box(win_RET_IN, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<12; i++) {
		if(i < 10){
		   mvwprintw(win_RET_IN, 4+i*2, 2, "RI0%d:",i);
		}
		else{
			mvwprintw(win_RET_IN, 4+i*2, 2, "RI%d:",i);
		}
	}
	for(i = 0; i<12; i++) {
			mvwprintw(win_RET_IN, 4+i*2, 16, "RI%d:",i+12);
	}
	for(i = 0; i<8; i++) {
		mvwprintw(win_RET_IN, 4+i*2, 30, "RI%d:",i+24);
	}
	wnoutrefresh(win_RET_IN);
}


void init_RET_OUT() {
	win_RET_OUT = newwin(30, nPanWidth, 8, 16);
	int i;
	//Init Fields for RET_IN
	field_RET_OUT[0]  = new_field(1, 3,  2, 6, 0, 0);
	field_RET_OUT[1]  = new_field(1, 3,  4, 6, 0, 0);
	field_RET_OUT[2]  = new_field(1, 3,  6, 6, 0, 0);
	field_RET_OUT[3]  = new_field(1, 3,  8, 6, 0, 0);
	field_RET_OUT[4]  = new_field(1, 3,  10, 6, 0, 0);
	field_RET_OUT[5]  = new_field(1, 3,  12, 6, 0, 0);
	field_RET_OUT[6]  = new_field(1, 3,  14, 6, 0, 0);
	field_RET_OUT[7]  = new_field(1, 3,  16, 6, 0, 0);
	field_RET_OUT[8]  = new_field(1, 3,  18, 6, 0, 0);
	field_RET_OUT[9]  = new_field(1, 3,  20, 6, 0, 0);
	field_RET_OUT[10] = new_field(1, 3,  22, 6, 0, 0);
	field_RET_OUT[11] = new_field(1, 3,  24, 6, 0, 0);
	field_RET_OUT[12] = new_field(1, 3,  2, 20, 0, 0);
	field_RET_OUT[13] = new_field(1, 3,  4, 20, 0, 0);
	field_RET_OUT[14] = new_field(1, 3,  6, 20, 0, 0);
	field_RET_OUT[15] = new_field(1, 3,  8, 20, 0, 0);
	field_RET_OUT[16] = new_field(1, 3,  10, 20, 0, 0);
	field_RET_OUT[17] = new_field(1, 3,  12, 20, 0, 0);
	field_RET_OUT[18] = new_field(1, 3,  14, 20, 0, 0);
	field_RET_OUT[19] = new_field(1, 3,  16, 20, 0, 0);
	field_RET_OUT[20] = new_field(1, 3,  18, 20, 0, 0);
	field_RET_OUT[21] = new_field(1, 3,  20, 20, 0, 0);
	field_RET_OUT[22] = new_field(1, 3,  22, 20, 0, 0);
	field_RET_OUT[23] = new_field(1, 3,  24, 20, 0, 0);
	field_RET_OUT[24] = new_field(1, 3,  2, 34, 0, 0);
	field_RET_OUT[25] = new_field(1, 3,  4, 34, 0, 0);
	field_RET_OUT[26] = new_field(1, 3,  6, 34, 0, 0);
	field_RET_OUT[27] = new_field(1, 3,  8, 34, 0, 0);
	field_RET_OUT[28] = new_field(1, 3,  10, 34, 0, 0);
	field_RET_OUT[29] = new_field(1, 3,  12, 34, 0, 0);
	field_RET_OUT[30] = new_field(1, 3,  14, 34, 0, 0);
	field_RET_OUT[31] = new_field(1, 3,  16, 34, 0, 0);
	field_RET_OUT[32] = NULL;
	
	for(i=0; i<32; i++)
	{
		set_field_fore(field_RET_OUT[i], COLOR_PAIR(1));
		set_field_back(field_RET_OUT[i], COLOR_PAIR(1));
		field_opts_off(field_RET_OUT[i], O_AUTOSKIP);
		set_field_type(field_RET_OUT[i], TYPE_INTEGER, 0, 0,255);
		set_field_just(field_RET_OUT[i], JUSTIFY_RIGHT);
	}
	
	//Form stuff	 
	form_RET_OUT = new_form(field_RET_OUT);
	set_form_win(form_RET_OUT, win_RET_OUT);
	set_form_sub(form_RET_OUT, derwin(win_RET_OUT, 28, 42, 2, 2));
	post_form(form_RET_OUT);
	
	pan_RET_OUT = new_panel(win_RET_OUT);
	print_in_color(win_RET_OUT, 1, 2, "RetainDataOut -/- Values: 0 - 255", COLOR_PAIR(1));
	box(win_RET_OUT, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<12; i++) {
		if(i < 10){
		   mvwprintw(win_RET_OUT, 4+i*2, 2, "RO0%d:",i);
		}
		else{
			mvwprintw(win_RET_OUT, 4+i*2, 2, "RO%d:",i);
		}
	}
	for(i = 0; i<12; i++) {
			mvwprintw(win_RET_OUT, 4+i*2, 16, "RO%d:",i+12);
	}
	for(i = 0; i<8; i++) {
		mvwprintw(win_RET_OUT, 4+i*2, 30, "RO%d:",i+24);
	}
	wnoutrefresh(win_RET_OUT);
}

void quit() {
	// Free some memory 
	int i;
	int n;
	
	unpost_form(form_DIN);
	unpost_form(form_AIN);	
	unpost_form(form_DOUT);
	unpost_form(form_AOUT);
	unpost_form(form_GPIO);
	unpost_form(form_PWM);
	unpost_form(form_CTRL);
	unpost_form(form_STAT);
	unpost_form(form_RET_IN);
	unpost_form(form_RET_OUT);
	
	free_form(form_DIN);
	free_form(form_AIN);
	free_form(form_DOUT);
	free_form(form_AOUT);
	free_form(form_GPIO);
	free_form(form_PWM);
	free_form(form_CTRL);
	free_form(form_STAT);
	free_form(form_RET_IN);
	free_form(form_RET_OUT);

	n = ARRAY_SIZE(field_DIN);
	for(i = 0; i < n; ++i) {
		free_field(field_DIN[i]); 
	}
	
	n = ARRAY_SIZE(field_AIN);
	for(i = 0; i < n; ++i) {
		free_field(field_AIN[i]); 
	}
	
	n = ARRAY_SIZE(field_DOUT);
	for(i = 0; i < n; ++i) {
		free_field(field_DOUT[i]); 
	}
	
	n = ARRAY_SIZE(field_AOUT);
	for(i = 0; i < n; ++i) {
		free_field(field_AOUT[i]); 
	}
	
	n = ARRAY_SIZE(field_GPIO);
	for(i = 0; i < n; ++i) {
		free_field(field_GPIO[i]); 
	}
	
	n = ARRAY_SIZE(field_PWM);
	for(i = 0; i < n; ++i) {
		free_field(field_PWM[i]); 
	}
	
	n = ARRAY_SIZE(field_CTRL);
	for(i = 0; i < n; ++i) {
		free_field(field_CTRL[i]); 
	}

	n = ARRAY_SIZE(field_STAT);
	for(i = 0; i < n; ++i) {
		free_field(field_STAT[i]); 
	}
	
	n = ARRAY_SIZE(field_RET_IN);
	for(i = 0; i < n; ++i) {
		free_field(field_RET_IN[i]); 
	}
	
	n = ARRAY_SIZE(field_RET_OUT);
	for(i = 0; i < n; ++i) {
		free_field(field_RET_OUT[i]); 
	}
	
	unpost_menu(my_menu);
	free_menu(my_menu);
	n = ARRAY_SIZE(choices);
	for(i = 0; i < n; ++i) {
		free_item(my_items[i]);
		}
	endwin();
}

void init_header() {

	// Create window for the header 
	win_header = newwin(8, 68, 0, 0);

}

void init_menu() {
	// Create window for the menu 
	win_menu = newwin(30, 15, 8, 0);
	
	// Create menu items 
	int i;
	int n_choices;	
	n_choices = ARRAY_SIZE(choices);
	my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
	for(i = 0; i < n_choices; ++i) {
		my_items[i] = new_item(choices[i], choices[i]);
		set_item_userptr(my_items[i], menu_function);
	}
	
	// Create menu 
	my_menu = new_menu((ITEM **)my_items);

	// Enable keyboard inputs in win_menu
	keypad(win_menu, TRUE);

	// Align menu vertically 
	set_menu_format(my_menu, n_choices, 1);	
	
	// Set menu option not to show the description 
	menu_opts_off(my_menu, O_SHOWDESC);
	
	// Set main window and sub window 
	set_menu_win(my_menu, win_menu);
	set_menu_sub(my_menu, derwin(win_menu, 25, 13, 4, 2));

	// Set indicator for selected menu item 
	set_menu_mark(my_menu, " >");
		
	update_menu();
}

int autoMode() {
	char str[40];
	int j;
    int k;
    
	//Exchange PiXtend V2 -S- Data
	OutputData.byModelOut = byModel;
	nAMRetVal = Spi_AutoModeV2S(&OutputData, &InputData);

    if (nAMRetVal == -1)
    {
        attron(COLOR_PAIR(4));
        j = snprintf(str, 20, "Err: NO COM /");
    }
    else
    {
        attron(COLOR_PAIR(5));
        j = snprintf(str, 20, "Err: OK     /");
    }
    
	
    k = (int)(lTimerPollTime/1000000);

	snprintf(str+j, 40, " CC: %u (%i)", uiCount, k);
	mvprintw(LINES - 3, 0, str);
    
	//Exchange PiXtend V2 -S- DAC Data
	Spi_AutoModeDAC(&OutputDataDAC);
	
	return(0);
}

//This gets called on every resize of the terminal window
void handle_winch(int sig)
{
	++updateRequest;
}

//Process update Request
void updateRequestHandler() {

	endwin();
	
	update_panels();
	doupdate();
	update_menu();
	doupdate();
	update_header();
	doupdate();	
	updateRequest = 0;
	
	initscr();	
}

int main()
{
	int c;
    int i;
    int fd;
	updateRequest = 0;
	
    if ((fd = acquireLock ("/tmp/pxauto2s.lck")) < 0) {
        fprintf (stderr, "Cannot start application! Is it already running?\n");
        return 1;
    }
    
    //Reset uC on start of application, in case user activated the uC SafeState
    Spi_uC_Reset();
    
	//Setup SPI using wiringPi	
	Spi_SetupV2(0); //use SPI device 0.0 (PiXtend V2 -S-)
	Spi_SetupV2(1); //use SPI device 0.1 (PiXtend V2 -S- DAC)
	
	//Initialize curses 
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);		
	
	init_pair(1, COLOR_BLACK, COLOR_YELLOW);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);
	init_pair(3, COLOR_GREEN, COLOR_WHITE);
    init_pair(4, COLOR_WHITE, COLOR_RED);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);
	
	//Handle for resize Event 
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handle_winch;
	sigaction(SIGWINCH, &sa, NULL);
	
	//Initialize windows and data 
	init_header();
	init_menu();	
	init_HOME();
	init_DIN();
	init_AIN();
	init_DOUT();
	init_AOUT();
	init_PWM();
	init_GPIO();
	init_CTRL();
	init_STAT();
	init_RET_IN();
	init_RET_OUT();
    
    //Set DigitalOut defaults, all fields set to False
    for(i = 0; i < 9; i++)
    {
        set_field_buffer(field_DOUT[i], 0, "False");
    }
    
    //Set GPIOOut defaults, at start all bits are False
	for(i = 4; i < 8; i++)
	{
		set_field_buffer(field_GPIO[i], 0, "False");
	}
    
    //Set AI Ref to 10 volts
	for(i = 0; i < 2; i++)
    {
        set_field_buffer(field_CTRL[13+i], 0, "10.0V");
	}
    update_CTRL();
    
    //Set default DHT11/22 settings for GPIOs on AIN panel
    set_field_buffer(field_AIN[14],0,"DHT11");
    set_field_buffer(field_AIN[15],0,"DHT11");
    set_field_buffer(field_AIN[16],0,"DHT11");
    set_field_buffer(field_AIN[17],0,"DHT11");
    update_AIN();
	
	cur_form = form_DIN;
	top_panel(pan_HOME);	

	update_panels();
	doupdate();
	update_menu();
	doupdate();
	update_header();
	doupdate();
		
	//Connect Timer Signal with 10 ms or 30 ms refresh rate 
	(void) signal(SIGALRM, timer_callback);
	start_timer();

	//Main Loop
	while (1) {
	
		//Detect if any keys were pressed in the menu window, other than RETURN.  If RETURN is pressed, loop exits.
		keypad(win_menu, TRUE);
		while((c = wgetch(win_menu)) != KEY_RETURN)
		{	  

			doupdate();
			
			switch(c)
			{	
				case KEY_DOWN:
					//Select next Menu Entry
					menu_driver(my_menu, REQ_DOWN_ITEM);
				break;
				case KEY_UP:
					//Select previous Menu Entry
					menu_driver(my_menu, REQ_UP_ITEM);
				break;
				case 'q':
					//Quit the application
					quit();
                    releaseLock (fd);
					return(0);
				break;
				case KEY_RESIZE:
					wnoutrefresh(win_header);
					doupdate();
				break;
				
				case ERR:
				
				break;
			}
			
			//Update the main content to publish currently active Menu Items Contents
			menu_function();
			
			//Execute if update request occurred
			//@todo: move outside
			if(updateRequest) {
				updateRequestHandler();
			}
		}
		//Disable Keypad for win_menu
		keypad(win_menu, FALSE);
		
		//Enable Keypad for Standard Screen
		keypad(stdscr, TRUE);
		
		//Switch o EDIT_MODE
		cur_Mode=EDIT_MODE;
		
		//mvwprintw(win_header, 6 ,50, "EDIT");
		//wnoutrefresh(win_header);
		
		//Position the form cursor in the current form
		pos_form_cursor(cur_form);
		update_panels();
		doupdate();
		
		//Detect if any keys were pressed in the standard screen, If RETURN is pressed, move on to the menu loop
		while((c = getch()) != KEY_RETURN)
		{	   
			if(updateRequest) {
				updateRequestHandler();
			}
			
			switch(c)
				{	
				case KEY_DOWN:
					//Select next Field
					form_driver(cur_form, REQ_NEXT_FIELD);
				break;
				case KEY_UP:
					//Select previous Field
					form_driver(cur_form, REQ_PREV_FIELD);
				break;
					
				case KEY_LEFT:
					//Select previous choice
					form_driver(cur_form, REQ_PREV_CHOICE);
				break;
				case KEY_RIGHT:
					//Select next choice
					form_driver(cur_form, REQ_NEXT_CHOICE);
				break;
					
				case KEY_ENTER:
				case KEY_RETURN:     
				break;
				case KEY_BACKSPACE:
				case KEY_DC:
				case 8:
					//Clear Fields contents
					form_driver(cur_form, REQ_CLR_FIELD);
				break;
					
				default:
					//Standard Action
					form_driver(cur_form, c);					
				break;
			}
			//Position Form cursor
			pos_form_cursor(cur_form);
			update_panels();
			doupdate();
			
		}
        //On Enter, check if value is allowed, if not clear field
        if (form_driver(cur_form, REQ_VALIDATION) != E_OK) {
            //Clear field
            form_driver(cur_form, REQ_CLR_FIELD);
        }        
        
		//Disable Keypad for Standard Screen
		keypad(stdscr, FALSE);
		
		//Switch Back to MENU_MODE
		cur_Mode=MENU_MODE;
		pos_menu_cursor(my_menu);
		update_panels();
		doupdate();
		
	}	

	endwin();
	return(0);
}

void print_in_color(WINDOW *win, int y, int x, char *string, chtype color)
{
	if(win == NULL) 
	{
		win = stdscr;
	}
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
}


//Menu function is used to display a new top_panel after user navigated to a new menu item.
//Also called from timer callback to update live data.
void menu_function() {
	
	ITEM *cur;
	char *name;

	cur = current_item(my_menu);
	name = (char*)item_name(cur);
	
	if(!strcmp(name, "Home")) {
		update_HOME();		
		top_panel(pan_HOME);		
	}
	else if(!strcmp(name, "DIn")) {
		update_DIN();		
		top_panel(pan_DIN);
		cur_form = form_DIN;
	} 
	else if(!strcmp(name, "AIn")) {
		update_AIN();		
		top_panel(pan_AIN);	
		cur_form = form_AIN;
	}
	else if(!strcmp(name, "GPIO")) {
		update_GPIO();		
		top_panel(pan_GPIO);	
		cur_form = form_GPIO;
	}
	else if(!strcmp(name, "DOut")) {
		update_DOUT();		
		top_panel(pan_DOUT);
		cur_form = form_DOUT;	
	}
	else if(!strcmp(name, "AOut")) {
		update_AOUT();		
		top_panel(pan_AOUT);	
		cur_form = form_AOUT;
	}
	else if(!strcmp(name, "PWM")) {
		update_PWM();		
		top_panel(pan_PWM);	
		cur_form = form_PWM;
		}
	else if(!strcmp(name, "Ctrl")) {
		update_CTRL();		
		top_panel(pan_CTRL);	
		cur_form = form_CTRL;
		}
	else if(!strcmp(name, "Stat")) {
		update_STAT();		
		top_panel(pan_STAT);	
		cur_form = form_STAT;
		}
	else if(!strcmp(name, "RetIn")) {
		update_RET_IN();
		top_panel(pan_RET_IN);
		cur_form = form_RET_IN;
		}
	else if(!strcmp(name, "RetOut")) {
		update_RET_OUT();
		top_panel(pan_RET_OUT);
		cur_form = form_RET_OUT;
		}
		
	if(cur_Mode==EDIT_MODE)
	{
		pos_form_cursor(cur_form);
	}
	update_panels();	
	if(cur_Mode==MENU_MODE)
	{
		wnoutrefresh(win_menu);	
		pos_menu_cursor(my_menu);	
	}	
	doupdate();
}

void update_header() {
	mvwaddstr(win_header, 1, 2, "    ____  _ _  ____                 __   _    _____        _____");
	mvwaddstr(win_header, 2, 2, "   / __ \\(_) |/ / /____  ____  ____/ /  | |  / /__ \\      / ___/");
	mvwaddstr(win_header, 3, 2, "  / /_/ / /|   / __/ _ \\/ __ \\/ __  /   | | / /__/ / ____ \\__ \\ ");
	mvwaddstr(win_header, 4, 2, " / ____/ //   / /_/  __/ / / / /_/ /    | |/ // __/ /___/__ / / ");
	mvwaddstr(win_header, 5, 2, "/_/   /_//_/|_\\__/\\___/_/ /_/\\__,_/     |___//____/     /____/  ");
	mvwaddstr(win_header, 6, 5, PXAUTO_HEADERSTRING);
	box(win_header, 0, 0);
	wnoutrefresh(win_header);
}

void update_menu() {
	print_in_color(win_menu, 1, 6, "Menu", COLOR_PAIR(1));
	mvwaddch(win_menu, 2, 0, ACS_LTEE);
	mvwhline(win_menu, 2, 1, ACS_HLINE, 38);
	mvwaddch(win_menu, 2, 39, ACS_RTEE);
	mvprintw(LINES - 2, 0, "q=Exit, RETURN=Toggle Edit, DEL/BACKSPACE=Clear");
	box(win_menu, 0, 0);
	post_menu(my_menu);
	wnoutrefresh(win_menu);
}

void update_HOME() {
	wnoutrefresh(win_HOME);
}

void update_AIN() {
	char str[6];
    int i;
    char* bufCont;
	snprintf(str, 6, "%3.2f", InputData.rAnalogIn0);
	set_field_buffer(field_AIN[0], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rAnalogIn1);
	set_field_buffer(field_AIN[1], 0, str);
	
	snprintf(str, 6, "%3.2f", InputData.rTemp0);
	set_field_buffer(field_AIN[2], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rTemp1);
	set_field_buffer(field_AIN[3], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rTemp2);
	set_field_buffer(field_AIN[4], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rTemp3);
	set_field_buffer(field_AIN[5], 0, str);
	
	snprintf(str, 6, "%3.2f", InputData.rHumid0);
	set_field_buffer(field_AIN[6], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rHumid1);
	set_field_buffer(field_AIN[7], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rHumid2);
	set_field_buffer(field_AIN[8], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rHumid3);
	set_field_buffer(field_AIN[9], 0, str);
	
	if (OutputData.byJumper10V & 0b00000001) {
		print_Value_Bar(win_AIN, InputData.rAnalogIn0, 20, 10.0, 4, 16); 
	} else {
		print_Value_Bar(win_AIN, InputData.rAnalogIn0, 20,  5.0, 4, 16);
	}
	if (OutputData.byJumper10V & 0b00000010) {
		print_Value_Bar(win_AIN, InputData.rAnalogIn1, 20, 10.0, 6, 16);
	} else {
		print_Value_Bar(win_AIN, InputData.rAnalogIn1, 20,  5.0, 6, 16);
	}
	
	print_Value_Bar(win_AIN, InputData.rTemp0, 20, 60.0, 8, 16);
	print_Value_Bar(win_AIN, InputData.rTemp1, 20, 60.0, 10, 16);
	print_Value_Bar(win_AIN, InputData.rTemp2, 20, 60.0, 12, 16);
	print_Value_Bar(win_AIN, InputData.rTemp3, 20, 60.0, 14, 16);
	
	print_Value_Bar(win_AIN, InputData.rHumid0, 20, 100.0, 16, 16);
	print_Value_Bar(win_AIN, InputData.rHumid1, 20, 100.0, 18, 16);
	print_Value_Bar(win_AIN, InputData.rHumid2, 20, 100.0, 20, 16);
	print_Value_Bar(win_AIN, InputData.rHumid3, 20, 100.0, 22, 16);
    
    i = 10;
    if(InputData.byTemp0Error == 0) 
    {
        set_field_buffer(field_AIN[i], 0, "False");
    }
    else
    {
        set_field_buffer(field_AIN[i], 0, "True");
    }
    i = 11;
    if(InputData.byTemp1Error == 0) 
    {
        set_field_buffer(field_AIN[i], 0, "False");
    }
    else
    {
        set_field_buffer(field_AIN[i], 0, "True");
    }
    i = 12;
    if(InputData.byTemp2Error == 0) 
    {
        set_field_buffer(field_AIN[i], 0, "False");
    }
    else
    {
        set_field_buffer(field_AIN[i], 0, "True");
    }
    i = 13;
    if(InputData.byTemp3Error == 0) 
    {
        set_field_buffer(field_AIN[i], 0, "False");
    }
    else
    {
        set_field_buffer(field_AIN[i], 0, "True");
    }
    
	// Calculate GPIO DHT11/22 setting
	for(i = 0; i < 4; i++) {
		bufCont = field_buffer(field_AIN[i+14],0);
		if(!strcmp(bufCont, "DHT11")) {
            if (i == 0){OutputData.byGPIO0Dht11 = 1;}
            if (i == 1){OutputData.byGPIO1Dht11 = 1;}
            if (i == 2){OutputData.byGPIO2Dht11 = 1;} 
			if (i == 3){OutputData.byGPIO3Dht11 = 1;} 
		}
		if(!strcmp(bufCont, "DHT22")) {
            if (i == 0){OutputData.byGPIO0Dht11 = 0;}
            if (i == 1){OutputData.byGPIO1Dht11 = 0;}
            if (i == 2){OutputData.byGPIO2Dht11 = 0;} 
			if (i == 3){OutputData.byGPIO3Dht11 = 0;} 
		}
	}     

	wnoutrefresh(win_AIN);
}

// Draw a bar for analog value
void print_Value_Bar(WINDOW* win, float value, int max_width, float max_value, int posY, int posX) {

	int i;
	//Draw limit bars
	mvwaddstr(win, posY, posX, "|");
	mvwaddstr(win, posY, posX+max_width+1, "|");
	//Draw bar indicator
	for(i = 0; i < max_width; i++) {
		if(i < (max_width * value / max_value)) {
			mvwaddstr(win, posY, posX+1+i, "=");
		}
		else {
			mvwaddstr(win, posY, posX+1+i, " ");
		}
	}
}

void update_DIN() {
	int i;
	int valueIn = 0;
    
	for(i = 0; i<8; i++) {
		if(InputData.byDigitalIn & (1<<i)) 
		{
			set_field_buffer(field_DIN[i], 0, "True");
			valueIn +=  (1<<(i));
		}
		else
		{
			set_field_buffer(field_DIN[i], 0, "False");
		}
	}
	mvwprintw(win_DIN, 1, 21, "DigitalIn: %3d",valueIn);
	wnoutrefresh(win_DIN);
}

void update_GPIO() {
	int i;
	int valueIn = 0;
    int gpioout_saved;
    int InputError;
    
    InputError = 0;
    //Backup GPIOOut value
    gpioout_saved = 0;
    gpioout_saved = OutputData.byGPIOOut;
    
	// Update GPIO Inputs
	for(i = 0; i<4; i++) {
		if(InputData.byGPIOIn & (1<<i)) 
		{
			set_field_buffer(field_GPIO[i], 0, "True");
			valueIn +=  (1<<(i));
		}
		else
		{
			set_field_buffer(field_GPIO[i], 0, "False");
		}
	}
	mvwprintw(win_GPIO, 1, 21, "GPIOIn: %3d",valueIn);

    //Check if input value is ok
    if (form_driver(cur_form, REQ_VALIDATION) != E_OK) {
        // do something
        form_driver(cur_form, REQ_CLR_FIELD);
        InputError = 100;
    }  
    //All ok
    if (InputError == 0)
    {	
        // Calculate Output Value for GPIO Outputs
        int valueOut = 0;
        char* bufCont;
        for(i = 0; i < 4; i++) {
            bufCont = field_buffer(field_GPIO[i+4],0);
            if(!strcmp(bufCont, "True ")) {
                valueOut +=  (1<<(i));
            }
            if(!strcmp(bufCont, "False")) {
                //nothing to do
            }
        }
        mvwprintw(win_GPIO, 13, 21, "GPIOOut: %3d",valueOut);
        OutputData.byGPIOOut = valueOut;
    }
    else   //Error
    {
        //GPIOOut restore and set old values
        OutputData.byGPIOOut = gpioout_saved;
        for(i = 0; i < 4; i++) {
            if(OutputData.byGPIOOut & (1<<i)) 
            {
                set_field_buffer(field_GPIO[i+4], 0, "True");
            }
            else
            {
                set_field_buffer(field_GPIO[i+4], 0, "False");
            }
        }
        mvwprintw(win_GPIO, 13, 21, "GPIOOut: %3d",gpioout_saved);     
    }
	
	gpioout_saved = 0;
	wnoutrefresh(win_GPIO);
}

void update_AOUT() {
	uint16_t value0, value1;
	
	value0 = atoi(field_buffer(field_AOUT[0], 0));
	value1 = atoi(field_buffer(field_AOUT[1], 0));
	
	OutputDataDAC.wAOut0 = value0;
	OutputDataDAC.wAOut1 = value1;
	
	wnoutrefresh(win_AOUT);	
}

void update_DOUT() {
	int value;
    int dovalue_saved;
    int relvalue_saved;
	int i;
	char* bufCont;
    int InputError;

    InputError = 0;
    dovalue_saved = 0;
    relvalue_saved = 0;	
    dovalue_saved = OutputData.byDigitalOut;
    relvalue_saved = OutputData.byRelayOut;
    
    if (form_driver(cur_form, REQ_VALIDATION) != E_OK) {
        // do something
        form_driver(cur_form, REQ_CLR_FIELD);
        InputError = 100;
    }  
    
    if (InputError == 0)
    {
        //Digital Outputs
        value = 0;
        for(i = 0; i < 4; i++) {
            bufCont = field_buffer(field_DOUT[i],0);
            if(!strcmp(bufCont, "True ")) {
                value +=  (1<<i);
            }
            if(!strcmp(bufCont, "False")) {
            }
        }
        OutputData.byDigitalOut = value;
        mvwprintw(win_DOUT, 1, 21, "DigitalOut: %3d",value);
        
        //Relay Outputs
        value = 0;
        for(i = 0; i < 4; i++) {
            bufCont = field_buffer(field_DOUT[4+i],0);
            if(!strcmp(bufCont, "True ")) {
                value +=  (1<<i);
            }
            if(!strcmp(bufCont, "False")) {
            }
        }
        OutputData.byRelayOut = value;
	    mvwprintw(win_DOUT, 16, 21, "RelayOut: %3d",value);        
    }
    else
    {
        //Digital Outputs
        OutputData.byDigitalOut = dovalue_saved;
        for(i = 0; i < 4; i++) {
            if(OutputData.byDigitalOut & (1<<i)) 
            {
                set_field_buffer(field_DOUT[i], 0, "True");
            }
            else
            {
                set_field_buffer(field_DOUT[i], 0, "False");
            }
        }
        mvwprintw(win_DOUT, 1, 21, "DigitalOut: %3d",dovalue_saved);
        
        //Relay Outputs
        OutputData.byRelayOut = relvalue_saved;
        for(i = 0; i < 4; i++) {
            if(OutputData.byRelayOut & (1<<i)) 
            {
                set_field_buffer(field_DOUT[4+i], 0, "True");
            }
            else
            {
                set_field_buffer(field_DOUT[4+i], 0, "False");
            }
        }
	    mvwprintw(win_DOUT, 16, 21, "RelayOut: %3d",relvalue_saved);         
    }
    
    dovalue_saved = 0;
    relvalue_saved = 0;	    
    
	wnoutrefresh(win_DOUT);
}

void update_PWM() {
	uint16_t value0, value1;
	uint8_t value2, value3;
	value0 =  atoi(field_buffer(field_PWM[0], 0));
	value1 =  atoi(field_buffer(field_PWM[1], 0));
	value2 =  atoi(field_buffer(field_PWM[2], 0));
	value3 =  atoi(field_buffer(field_PWM[3], 0));	
	OutputData.wPWM0A = value0;
	OutputData.wPWM0B = value1;
	OutputData.byPWM1A = value2;
	OutputData.byPWM1B = value3;
	wnoutrefresh(win_PWM);
}

void update_CTRL() {
	
	uint8_t value;
	uint16_t value16;
	value16 = atoi(field_buffer(field_CTRL[0], 0));
	OutputData.wPWM0Ctrl1 = value16; 
	value = atoi(field_buffer(field_CTRL[1], 0));
	OutputData.byPWM0Ctrl0 = value; 
	
	value = atoi(field_buffer(field_CTRL[2], 0));
	OutputData.byPWM1Ctrl1 = value;
	value = atoi(field_buffer(field_CTRL[3], 0));
	OutputData.byPWM1Ctrl0 = value; 

	value = atoi(field_buffer(field_CTRL[4], 0));
	OutputData.byGPIOCtrl = value; 
	value = atoi(field_buffer(field_CTRL[5], 0));
	OutputData.byGPIODebounce01 = value;
	value = atoi(field_buffer(field_CTRL[6], 0));
	OutputData.byGPIODebounce23 = value;	
 
	value = atoi(field_buffer(field_CTRL[7], 0));
	OutputData.byDigitalInDebounce01 = value;
	value = atoi(field_buffer(field_CTRL[8], 0));
	OutputData.byDigitalInDebounce23 = value;
	value = atoi(field_buffer(field_CTRL[9], 0));
	OutputData.byDigitalInDebounce45 = value;
	value = atoi(field_buffer(field_CTRL[10], 0));
	OutputData.byDigitalInDebounce67 = value;
	
	value = atoi(field_buffer(field_CTRL[11], 0));
	OutputData.byUCCtrl0 = value;
	value = atoi(field_buffer(field_CTRL[12], 0));
	OutputData.byUCCtrl1 = value; 
	
	//AI VRef Selection
	value = 0;
	int i;
	char* bufCont;
	for(i = 0; i < 2; i++) {
		bufCont = field_buffer(field_CTRL[13+i],0);
		if(!strcmp(bufCont, "10.0V")) {
			value +=  (1<<i);
		}
		if(!strcmp(bufCont, "5.0V ")) {
		}
	}
	OutputData.byJumper10V = value;	
	mvwprintw(win_CTRL, 22, 21, "Jumper10V: %3d",value);
	
	wnoutrefresh(win_CTRL);
}

void update_STAT() {
	
	char str[4];
	snprintf(str, 4, " %d", InputData.byFirmware);
	set_field_buffer(field_STAT[0], 0, str);
	snprintf(str, 4, "%d", InputData.byHardware);
	set_field_buffer(field_STAT[1], 0, str);
	snprintf(str, 4, " %d", InputData.byUCState);
	set_field_buffer(field_STAT[2], 0, str);
	snprintf(str, 4, " %d", InputData.byModelIn);
	set_field_buffer(field_STAT[3], 0, str);
    snprintf(str, 4, " %d", InputData.byUCWarnings);
	set_field_buffer(field_STAT[4], 0, str);    
	wnoutrefresh(win_STAT);
}

void update_RET_IN() {
	
	char str[4];
	int  i;
	i = 0;
	
	for(i = 0; i<32; i++) {
		snprintf(str, 4, "%d", InputData.abyRetainDataIn[i]);
		set_field_buffer(field_RET_IN[i], 0, str);
	}
	
	wnoutrefresh(win_RET_IN);
}

void update_RET_OUT() {
	
    uint8_t value0;
	int  i;
	i = 0;
	
	for(i = 0; i<32; i++) {
		value0 =  atoi(field_buffer(field_RET_OUT[i], 0));
		OutputData.abyRetainDataOut[i] = value0;
	}
	
	wnoutrefresh(win_RET_OUT);
}



