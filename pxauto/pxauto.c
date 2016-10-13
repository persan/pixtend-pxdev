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

#include "pxauto.h"

#define PXAUTO_HEADERSTRING "PiXtend Auto Tool - V0.4.2 - http://www.pixtend.de"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define KEY_RETURN '\n'

static int tCount;
static int nCount;


//update request after screen/terminal window resize
int updateRequest;

//PiXtend input data
struct pixtIn InputData;

//PiXtend output data
struct pixtOut OutputData;

//PiXtend DAC Output Data
struct pixtOutDAC OutputDataDAC;

timer_t gTimerid;

enum CurMode {MENU_MODE=0, EDIT_MODE};
enum CurMode cur_Mode;

//Boolean Field Values
char *sBoolean[] = { 
					"FALSE\0", 
					"TRUE\0",
					(char *)NULL
				  };

//Vref Field Values
char *sVRef[] = { 
					"5.0V\0", 
					"10.0V\0",
					(char *)NULL
				  };

//Menu Entries				  
char *choices[] = {
					"HOME",
					"DIN",
					"AIN",
					"GPIO",
					"DOUT",
					"AOUT",
					"PWM",
					"CTRL",
					"STAT",
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

FIELD *field_DIN[9];
FIELD *field_DOUT[11];
FIELD *field_AOUT[3];
FIELD *field_AIN[13];
FIELD *field_GPIO[9];
FIELD *field_PWM[3];
FIELD *field_CTRL[10];
FIELD *field_STAT[4];

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

PANEL *pan_HOME;
PANEL *pan_DIN;
PANEL *pan_AIN;
PANEL *pan_GPIO;
PANEL *pan_DOUT;
PANEL *pan_AOUT;
PANEL *pan_PWM;
PANEL *pan_CTRL;
PANEL *pan_STAT;

//Time Base for timer_callback() calls
void start_timer(void)
{
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 200000000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 200000000;
	timer_create (CLOCK_REALTIME, NULL, &gTimerid);
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

// Timer Callback, executed every 200ms
void timer_callback(int sig) {

	//Increment static counter
	tCount++;

	//Execute every 200ms
	autoMode();
	
	//Execute every second
	if (tCount == 5)
    {
        tCount = 0;
		nCount++;
		//Initiate Screen Update
		menu_function();	
    }
}

void init_HOME() {
	win_HOME = newwin(30, 44 , 8, 16);
	pan_HOME = new_panel(win_HOME);
	print_in_color(win_HOME, 1, 2, "HOME", COLOR_PAIR(2));
	mvwaddstr(win_HOME,  4, 2, "This Tool allows you to monitor and ");
	mvwaddstr(win_HOME,  5, 2, "control your PiXtend Board");
	mvwaddstr(win_HOME,  6, 2, "for test purposes.");
	mvwaddstr(win_HOME,  7, 2, "Feel free to modify and adapt this to");
	mvwaddstr(win_HOME,  8, 2, "your special needs.");	
	mvwaddstr(win_HOME,  9, 2, "");	
	mvwaddstr(win_HOME, 10, 2, "Usage:");
	mvwaddstr(win_HOME, 11, 2, " Use UP and DOWN to navigate the MENU");
	mvwaddstr(win_HOME, 12, 2, " Hit ENTER to toggle EDIT Mode");	
	mvwaddstr(win_HOME, 13, 2, " Enter values by typing them in or");
	mvwaddstr(win_HOME, 14, 2, " use LEFT and RIGHT to change Booleans");
	mvwaddstr(win_HOME, 15, 2, " ");
	mvwaddstr(win_HOME, 16, 2, "Visit http://www.pixtend.de for more...");	
	mvwaddstr(win_HOME, 17, 2, " 2014-2016, Qube Solutions UG");
	mvwaddstr(win_HOME, 18, 2, " ");
	wnoutrefresh(win_header);	
	box(win_HOME, 0, 0);	
}

void init_DIN() {
	win_DIN = newwin(30, 44 , 8, 16);
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
	print_in_color(win_DIN, 1, 2, "DIN", COLOR_PAIR(2));
	box(win_DIN, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<8; i++) {
		mvwprintw(win_DIN, 4+i*2, 2, "DI%d:",i);
	}
	wnoutrefresh(win_DIN);
}

void init_AIN() {
	win_AIN = newwin(30, 44 , 8, 16);
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
	field_AIN[10] = new_field(1, 5, 22, 8, 0, 0);
	field_AIN[11] = new_field(1, 5, 24, 8, 0, 0);
	field_AIN[12] = NULL;
	
	for(i=0; i<12; i++)
	{
	set_field_fore(field_AIN[i], COLOR_PAIR(2));
	set_field_back(field_AIN[i], COLOR_PAIR(2));
	field_opts_off(field_AIN[i], O_AUTOSKIP);
	field_opts_off(field_AIN[i], O_EDIT);		
	set_field_type(field_AIN[i], TYPE_INTEGER, 0, 0,65535);
	}
	
	//Form stuff 	
	form_AIN = new_form(field_AIN);
	set_form_win(form_AIN, win_AIN);
    set_form_sub(form_AIN, derwin(win_AIN, 28, 42, 2, 2));
	post_form(form_AIN);
	pan_AIN = new_panel(win_AIN);
	print_in_color(win_AIN, 1, 2, "AIN", COLOR_PAIR(2));
	box(win_AIN, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<4; i++) {
		mvwprintw(win_AIN, 4+i*2, 2, "AI%d:",i);
	}
	for(i = 4; i<8; i++) {
		mvwprintw(win_AIN, 4+i*2, 2, "Temp%d:",i-4);
	}
	for(i = 8; i<12; i++) {
		mvwprintw(win_AIN, 4+i*2, 2, "Humid%d:",i-8);
	}
	wnoutrefresh(win_AIN);
}

void init_DOUT() {
	win_DOUT = newwin(30, 44 , 8, 16);
	int i;
	//Init Fields for DOUT 
	field_DOUT[0] = new_field(1, 5,  2, 8, 0, 0);
	field_DOUT[1] = new_field(1, 5,  4, 8, 0, 0);
	field_DOUT[2] = new_field(1, 5,  6, 8, 0, 0);
	field_DOUT[3] = new_field(1, 5,  8, 8, 0, 0);
	field_DOUT[4] = new_field(1, 5, 10, 8, 0, 0);
	field_DOUT[5] = new_field(1, 5, 12, 8, 0, 0);
	//Init Fields for Relays            
	field_DOUT[6] = new_field(1, 5, 16, 8, 0, 0);
	field_DOUT[7] = new_field(1, 5, 18, 8, 0, 0);
	field_DOUT[8] = new_field(1, 5, 20, 8, 0, 0);
	field_DOUT[9] = new_field(1, 5, 22, 8, 0, 0);
	field_DOUT[10] = NULL;
	
	for(i=0; i<10; i++)
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
	print_in_color(win_DOUT, 1, 2, "DOUT", COLOR_PAIR(1));
	print_in_color(win_DOUT, 16, 2, "RELAY", COLOR_PAIR(1));
	box(win_DOUT, 0, 0);
	
	//Static Label Stuff
	for(i = 0; i<6; i++) {
		mvwprintw(win_DOUT, 4+i*2, 2, "DO%d:",i);
		mvwprintw(win_DOUT, 4+i*2, 21, "<TRUE/FALSE>");
	}
	for(i = 0; i<4; i++) {
		mvwprintw(win_DOUT, 18+i*2, 2, "REL%d:",i);
		mvwprintw(win_DOUT, 18+i*2, 21, "<TRUE/FALSE>");
	}
	wnoutrefresh(win_DOUT);
}

void init_AOUT() {
	win_AOUT = newwin(30, 44 , 8, 16);
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
	print_in_color(win_AOUT, 1, 2, "AOUT", COLOR_PAIR(2));
	box(win_AOUT, 0, 0);	
	
	//Static Label Stuff
	for(i = 0; i<2; i++) {
		mvwprintw(win_AOUT, 4+i*2, 2, "AOUT%d:",i);
		mvwprintw(win_AOUT, 4+i*2, 21, "0-1023");
	}
	wnoutrefresh(win_AOUT);
}

void init_GPIO() {
	win_GPIO = newwin(30, 44 , 8, 16);
	int i;
	//Init Fields GPIO 
	field_GPIO[0] = new_field(1, 5,  2,  8, 0, 0);
	field_GPIO[1] = new_field(1, 5,  4,  8, 0, 0);
	field_GPIO[2] = new_field(1, 5,  6,  8, 0, 0);
	field_GPIO[3] = new_field(1, 5,  8,  8, 0, 0);
	field_GPIO[4] = new_field(1, 5,  12, 8, 0, 0);
	field_GPIO[5] = new_field(1, 5,  14, 8, 0, 0);
	field_GPIO[6] = new_field(1, 5,  16, 8, 0, 0);
	field_GPIO[7] = new_field(1, 5,  18, 8, 0, 0);
	field_GPIO[8] = NULL;
			
	for(i=0; i<4; i++)
	  {
		set_field_fore(field_GPIO[i], COLOR_PAIR(2));
		set_field_back(field_GPIO[i], COLOR_PAIR(2));
		field_opts_off(field_GPIO[i], O_AUTOSKIP);
		field_opts_off(field_GPIO[i], O_EDIT);
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
	print_in_color(win_GPIO, 1, 2, "GPIO IN", COLOR_PAIR(2));
	print_in_color(win_GPIO, 12, 2, "GPIO OUT", COLOR_PAIR(1));
	box(win_GPIO, 0, 0);	
	
	//Static Label Stuff
	for(i = 0; i<4; i++) {
		mvwprintw(win_GPIO, 4+i*2, 2, "IN%d:",i);
	}
	//Static Label Stuff
	for(i = 0; i<4; i++) {
		mvwprintw(win_GPIO, 14+i*2, 2, "OUT%d:",i);
		mvwprintw(win_GPIO, 14+i*2, 21, "<TRUE/FALSE>");
	}
	wnoutrefresh(win_GPIO);
}

void init_PWM() {
	win_PWM = newwin(30, 44 , 8, 16);
	int i;
	//Init Fields PWM 
	field_PWM[0] = new_field(1, 5,  2, 8, 0, 0);
	field_PWM[1] = new_field(1, 5,  4, 8, 0, 0);
	field_PWM[2] = NULL;
			
	for(i=0; i<2; i++)
	  {
		set_field_fore(field_PWM[i], COLOR_PAIR(1));
		set_field_back(field_PWM[i], COLOR_PAIR(1));
		field_opts_off(field_PWM[i], O_AUTOSKIP);
		set_field_type(field_PWM[i], TYPE_INTEGER, 0, 0,65535);
		set_field_just(field_PWM[i], JUSTIFY_RIGHT);
	  }
	//Form stuff 	
	form_PWM = new_form(field_PWM);
	set_form_win(form_PWM, win_PWM);
    set_form_sub(form_PWM, derwin(win_PWM, 28, 42, 2, 2));
	post_form(form_PWM);
	pan_PWM = new_panel(win_PWM);
	print_in_color(win_PWM, 1, 2, "PWM", COLOR_PAIR(1));
	box(win_PWM, 0, 0);	
	
	//Static Label Stuff
	for(i = 0; i<2; i++) {
		mvwprintw(win_PWM, 4+i*2, 2, "PWM%d:",i);
		mvwprintw(win_PWM, 4+i*2, 21, "0-65535");
	}
	wnoutrefresh(win_PWM);
}

void init_CTRL() {
	win_CTRL = newwin(30, 44 , 8, 16);
	int i;
	//Init Fields CTRL 
	field_CTRL[0] = new_field(1, 3,  2,  13, 0, 0);
	field_CTRL[1] = new_field(1, 3,  4,  13, 0, 0);
	field_CTRL[2] = new_field(1, 3,  6,  13, 0, 0);
	field_CTRL[3] = new_field(1, 3,  8,  13, 0, 0);
	field_CTRL[4] = new_field(1, 3,  10, 13, 0, 0);
	field_CTRL[5] = new_field(1, 3,  12, 13, 0, 0);
	field_CTRL[6] = new_field(1, 3,  14, 13, 0, 0);
	field_CTRL[7] = new_field(1, 5,  18, 13, 0, 0);
	field_CTRL[8] = new_field(1, 5,  20, 13, 0, 0);
	field_CTRL[9] = NULL;

	
	//Control Bytes
	for(i=0; i<7; i++)
	  {
		set_field_fore(field_CTRL[i], COLOR_PAIR(1));
		set_field_back(field_CTRL[i], COLOR_PAIR(1));
		field_opts_off(field_CTRL[i], O_AUTOSKIP);
		set_field_type(field_CTRL[i], TYPE_INTEGER, 0, 0,255);
		set_field_just(field_CTRL[i], JUSTIFY_RIGHT);
	  }
	
	//AUX Bytes 
	for(i=7; i<9; i++)
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
	print_in_color(win_CTRL, 1, 2, "CTRL", COLOR_PAIR(1));
	print_in_color(win_CTRL, 18, 2, "AUX", COLOR_PAIR(1));
	box(win_CTRL, 0, 0);	
	
	//Static Label Stuff
	mvwaddstr(win_CTRL,  2+2, 2, "PWM_CTRL0");
	mvwaddstr(win_CTRL,  2+4, 2, "PWM_CTRL1");
	mvwaddstr(win_CTRL,  2+6, 2, "PWM_CTRL2");
	mvwaddstr(win_CTRL,  2+8, 2, "GPIO_CTRL");
	mvwaddstr(win_CTRL, 2+10, 2, "UC_CTRL");
	mvwaddstr(win_CTRL, 2+12, 2, "AI_CTRL0");
	mvwaddstr(win_CTRL, 2+14, 2, "AI_CTRL1");
	mvwaddstr(win_CTRL, 2+18, 2, "AI0VRef");
	mvwaddstr(win_CTRL, 2+20, 2, "AI1VRef");
	
	//Units
	mvwaddstr(win_CTRL,  2+2, 21, "0-255");
	mvwaddstr(win_CTRL,  2+4, 21, "0-255");
	mvwaddstr(win_CTRL,  2+6, 21, "0-255");
	mvwaddstr(win_CTRL,  2+8, 21, "0-255");
	mvwaddstr(win_CTRL, 2+10, 21, "0-255, 16: Start");
	mvwaddstr(win_CTRL, 2+12, 21, "0-255");
	mvwaddstr(win_CTRL, 2+14, 21, "0-255");
	mvwaddstr(win_CTRL, 2+18, 21, "<5.0V / 10.0V>");
	mvwaddstr(win_CTRL, 2+20, 21, "<5.0V / 10.0V>");  

	wnoutrefresh(win_CTRL);
}

void init_STAT() {
	win_STAT = newwin(30, 44 , 8, 16);
	int i;
	//Init Fields STAT 
	field_STAT[0] = new_field(1, 3,  2,  13, 0, 0);
	field_STAT[1] = new_field(1, 3,  4,  13, 0, 0);
	field_STAT[2] = new_field(1, 3,  6,  13, 0, 0);
	field_STAT[3] = NULL;
	
	//Status Bytes (read only)
	for(i=0; i<3; i++)
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
	print_in_color(win_STAT, 1, 2, "STAT", COLOR_PAIR(2));
	box(win_STAT, 0, 0);	
	
	//Static Label Stuff
	mvwaddstr(win_STAT, 2+2, 2, "UC_VERSIONL");
	mvwaddstr(win_STAT, 2+4, 2, "UC_VERSIONH");
	mvwaddstr(win_STAT, 2+6, 2, "UC_STATUS");

	wnoutrefresh(win_STAT);
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
	
	free_form(form_DIN);
	free_form(form_AIN);
	free_form(form_DOUT);
	free_form(form_AOUT);
	free_form(form_GPIO);
	free_form(form_PWM);
	free_form(form_CTRL);

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
	win_header = newwin(8, 60, 0, 0);

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
	
	//Exchange PiXtend Data 
	Spi_AutoMode(&OutputData, &InputData);
	
	//Exchange PiXtendDAC Data
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
	updateRequest = 0;
	
	//Setup SPI using wiringPi	
	Spi_Setup(0); //use SPI device 0.0 (PiXtend), exit on failure 
	Spi_Setup(1); //use SPI device 0.1 (PiXtend DAC), exit on failure
	
	//Initialize curses 
	initscr();
	start_color();
    cbreak();
    noecho();
	keypad(stdscr, TRUE);		
	
	init_pair(1, COLOR_BLACK, COLOR_YELLOW);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);
	init_pair(3, COLOR_GREEN, COLOR_WHITE);
	
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
	
	cur_form = form_DIN;
	top_panel(pan_HOME);	

	update_panels();
	doupdate();
	update_menu();
	doupdate();
	update_header();
	doupdate();
	
	
		
	//Connect Timer Signal with 200ms refresh rate 
	(void) signal(SIGALRM, timer_callback);
	start_timer();
	
	//@todo: implement field type sensitve Form Handling
	//FIELD *field;
    //FIELDTYPE *type;

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
					/*
					//@todo: implement field type sensitve Form Handling
					if ((field = current_field(cur_form)) != 0) {
						if ((type = field_type(field)) != 0) {
							if (type == TYPE_ALNUM)
							waddstr(win_header, "ALNUM");
							else if (type == TYPE_ALPHA)
							waddstr(win_header, "ALPHA");
							else if (type == TYPE_ENUM) {
								waddstr(win_header, "ENUM");
								form_driver(cur_form, REQ_PREV_CHOICE);
								}
							else if (type == TYPE_INTEGER)
							waddstr(win_header, "INTEGER");
							else if (type == TYPE_NUMERIC)
							waddstr(win_header, "NUMERIC");
							else if (type == TYPE_REGEXP)
							waddstr(win_header, "REGEXP");
							else
							waddstr(win_header, "other");
						}
					}
					*/
					//Select previous choice
					form_driver(cur_form, REQ_PREV_CHOICE);
				break;
				case KEY_RIGHT:
					//Select next choice
					form_driver(cur_form, REQ_NEXT_CHOICE);
				break;
					
				case KEY_ENTER:
				case KEY_RETURN:
					//mvwprintw(win_header, 6 ,50, "TAKE");
					//wnoutrefresh(win_header);
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
//Also called from timer callback every second to update live data.
void menu_function() {
	
	ITEM *cur;
	char *name;

	cur = current_item(my_menu);
	name = (char*)item_name(cur);
	
	if(!strcmp(name, "HOME")) {
		update_HOME();		
		top_panel(pan_HOME);		
	}
	else if(!strcmp(name, "DIN")) {
		update_DIN();		
		top_panel(pan_DIN);
		cur_form = form_DIN;
	} 
	else if(!strcmp(name, "AIN")) {
		update_AIN();		
		top_panel(pan_AIN);	
		cur_form = form_AIN;
	}
	else if(!strcmp(name, "GPIO")) {
		update_GPIO();		
		top_panel(pan_GPIO);	
		cur_form = form_GPIO;
	}
	else if(!strcmp(name, "DOUT")) {
		update_DOUT();		
		top_panel(pan_DOUT);
		cur_form = form_DOUT;	
	}
	else if(!strcmp(name, "AOUT")) {
		update_AOUT();		
		top_panel(pan_AOUT);	
		cur_form = form_AOUT;
	}
	else if(!strcmp(name, "PWM")) {
		update_PWM();		
		top_panel(pan_PWM);	
		cur_form = form_PWM;
		}
	else if(!strcmp(name, "CTRL")) {
		update_CTRL();		
		top_panel(pan_CTRL);	
		cur_form = form_CTRL;
		}
	else if(!strcmp(name, "STAT")) {
		update_STAT();		
		top_panel(pan_STAT);	
		cur_form = form_STAT;
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
	mvwaddstr(win_header, 1, 3, "    ____     _    _  __   __                      __");
	mvwaddstr(win_header, 2, 3, "   / __ \\   (_)  | |/ /  / /_  ___    ____   ____/ /");
	mvwaddstr(win_header, 3, 3, "  / /_/ /  / /   |   /  / __/ / _ \\  / __ \\ / __  / ");
	mvwaddstr(win_header, 4, 3, " / ____/  / /   /   |  / /_  /  __/ / / / // /_/ /  ");
	mvwaddstr(win_header, 5, 3, "/_/      /_/   /_/|_|  \\__/  \\___/ /_/ /_/ \\__,_/   ");
	mvwaddstr(win_header, 6, 5, PXAUTO_HEADERSTRING);
	box(win_header, 0, 0);
	wnoutrefresh(win_header);
}

void update_menu() {
	print_in_color(win_menu, 1, 7, "Menu", COLOR_PAIR(1));
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
	snprintf(str, 6, "%3.2f", InputData.rAi0);
	set_field_buffer(field_AIN[0], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rAi1);
	set_field_buffer(field_AIN[1], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rAi2);
	set_field_buffer(field_AIN[2], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rAi3);
	set_field_buffer(field_AIN[3], 0, str);
	
	snprintf(str, 6, "%3.2f", InputData.rTemp0);
	set_field_buffer(field_AIN[4], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rTemp1);
	set_field_buffer(field_AIN[5], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rTemp2);
	set_field_buffer(field_AIN[6], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rTemp3);
	set_field_buffer(field_AIN[7], 0, str);
	
	snprintf(str, 6, "%3.2f", InputData.rHumid0);
	set_field_buffer(field_AIN[8], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rHumid1);
	set_field_buffer(field_AIN[9], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rHumid2);
	set_field_buffer(field_AIN[10], 0, str);
	snprintf(str, 6, "%3.2f", InputData.rHumid3);
	set_field_buffer(field_AIN[11], 0, str);
	
	if (OutputData.byAux0 & 0b00000001) {
		print_Value_Bar(win_AIN, InputData.rAi0, 20, 10.0, 4, 16); 
	} else {
		print_Value_Bar(win_AIN, InputData.rAi0, 20,  5.0, 4, 16);
	}
	if (OutputData.byAux0 & 0b00000010) {
		print_Value_Bar(win_AIN, InputData.rAi1, 20, 10.0, 6, 16);
	} else {
		print_Value_Bar(win_AIN, InputData.rAi1, 20,  5.0, 6, 16);
	}
	print_Value_Bar(win_AIN, InputData.rAi2, 20, 24.0, 8, 16);
	print_Value_Bar(win_AIN, InputData.rAi3, 20, 24.0, 10, 16);
	
	print_Value_Bar(win_AIN, InputData.rTemp0, 20, 60.0, 12, 16);
	print_Value_Bar(win_AIN, InputData.rTemp1, 20, 60.0, 14, 16);
	print_Value_Bar(win_AIN, InputData.rTemp2, 20, 60.0, 16, 16);
	print_Value_Bar(win_AIN, InputData.rTemp3, 20, 60.0, 18, 16);
	
	print_Value_Bar(win_AIN, InputData.rHumid0, 20, 100.0, 20, 16);
	print_Value_Bar(win_AIN, InputData.rHumid1, 20, 100.0, 22, 16);
	print_Value_Bar(win_AIN, InputData.rHumid2, 20, 100.0, 24, 16);
	print_Value_Bar(win_AIN, InputData.rHumid3, 20, 100.0, 26, 16);

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
		if(InputData.byDigIn & (1<<i)) 
		{
			set_field_buffer(field_DIN[i], 0, "TRUE");
			valueIn +=  (1<<(i));
		}
		else
		{
			set_field_buffer(field_DIN[i], 0, "FALSE");
		}
	}
	mvwprintw(win_DIN, 1, 21, "byDigIn: %3d",valueIn);
	wnoutrefresh(win_DIN);
}

void update_GPIO() {
	int i;
	int valueIn = 0;
	// Update GPIO Inputs
	for(i = 0; i<4; i++) {
		if(InputData.byGpioIn & (1<<i)) 
		{
			set_field_buffer(field_GPIO[i], 0, "TRUE");
			valueIn +=  (1<<(i));
		}
		else
		{
			set_field_buffer(field_GPIO[i], 0, "FALSE");
		}
	}
	mvwprintw(win_GPIO, 1, 21, "byGpioIn: %3d",valueIn);
	
	// Calculate Output Value for GPIO Outputs
	int valueOut = 0;
	char* bufCont;
	for(i = 0; i < 4; i++) {
		bufCont = field_buffer(field_GPIO[i+4],0);
		if(!strcmp(bufCont, "TRUE ")) {
			valueOut +=  (1<<(i));
		}
		if(!strcmp(bufCont, "FALSE")) {
			//nothing to do
		}
	}
	
	mvwprintw(win_GPIO, 12, 21, "byGpioOut: %3d",valueOut);
	OutputData.byGpioOut = valueOut;
	
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
	int i;
	char* bufCont;
	
	//Digital Outputs
	value = 0;
	for(i = 0; i < 6; i++) {
		bufCont = field_buffer(field_DOUT[i],0);
		if(!strcmp(bufCont, "TRUE ")) {
			value +=  (1<<i);
		}
		if(!strcmp(bufCont, "FALSE")) {
		}
	}
	OutputData.byDigOut = value;	
	mvwprintw(win_DOUT, 1, 21, "byDigOut: %3d",value);
	
	//Relay Outputs
	value = 0;
	for(i = 0; i < 4; i++) {
		bufCont = field_buffer(field_DOUT[6+i],0);
		if(!strcmp(bufCont, "TRUE ")) {
			value +=  (1<<i);
		}
		if(!strcmp(bufCont, "FALSE")) {
		}
	}
	OutputData.byRelayOut = value;
	mvwprintw(win_DOUT, 16, 21, "byRelayOut: %3d",value);
	
	wnoutrefresh(win_DOUT);
}

void update_PWM() {
	uint16_t value0, value1;
	value0 =  atoi(field_buffer(field_PWM[0], 0));
	value1 =  atoi(field_buffer(field_PWM[1], 0));
	OutputData.wPwm0 = value0;
	OutputData.wPwm1 = value1;	
	wnoutrefresh(win_PWM);
}

void update_CTRL() {
	
	uint8_t value;
	value = atoi(field_buffer(field_CTRL[0], 0));
	OutputData.byPwm0Ctrl0 = value; 
	
	value = atoi(field_buffer(field_CTRL[1], 0));
	OutputData.byPwm0Ctrl1 = value; 
	
	value = atoi(field_buffer(field_CTRL[2], 0));
	OutputData.byPwm0Ctrl2 = value; 
	
	value = atoi(field_buffer(field_CTRL[3], 0));
	OutputData.byGpioCtrl = value; 
	
	value = atoi(field_buffer(field_CTRL[4], 0));
	OutputData.byUcCtrl = value; 
	
	value = atoi(field_buffer(field_CTRL[5], 0));
	OutputData.byAiCtrl0 = value; 
	
	value = atoi(field_buffer(field_CTRL[6], 0));
	OutputData.byAiCtrl1 = value; 
	
	//AI VRef Selection
	value = 0;
	int i;
	char* bufCont;
	for(i = 0; i < 2; i++) {
		bufCont = field_buffer(field_CTRL[7+i],0);
		if(!strcmp(bufCont, "10.0V")) {
			//mvwprintw(win_DOUT, 4 + 2*i, 25, "ON ");
			value +=  (1<<i);
		}
		if(!strcmp(bufCont, "5.0V ")) {
			//mvwprintw(win_DOUT, 4 + 2*i, 25, "OFF");
		}
	}
	OutputData.byAux0 = value;	
	mvwprintw(win_CTRL, 18, 21, "byAux0: %3d",value);
	
	wnoutrefresh(win_CTRL);
}

void update_STAT() {
	
	char str[4];
	snprintf(str, 4, "%d", InputData.byUcVersionL);
	set_field_buffer(field_STAT[0], 0, str);
	snprintf(str, 4, "%d", InputData.byUcVersionH);
	set_field_buffer(field_STAT[1], 0, str);
	snprintf(str, 4, "%d", InputData.byUcStatus);
	set_field_buffer(field_STAT[2], 0, str);
	wnoutrefresh(win_STAT);
}



