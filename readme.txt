/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <http://www.pixtend.de> or <http://www.pixtend.com>
#
# Copyright (C) 2014-2016 Christian Strobel, Nils Mensing
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

Overview:
---------
pxdev is the PiXtend Linux Development Package and part of the PiXtend(R) project. More infos on www.pixtend.de.

pixtend - This linux C library is for SPI Communication between the Raspberry Pi and the PiXtend Microcontroller, using wiringPi (https://projects.drogon.net/raspberry-pi/wiringpi/)
pixtendtool - A linux command line tool to monitor and control the PiXtend with simple commands, using the pixtend library
pxauto - A linux console application to live monitor and control the PiXtend from a Graphical User Interface. Uses the pixtend library for communication and the curses library for the User Interface. 


Installation Requirements:
--------------------------
pixtend uses the wiringPi Library (https://projects.drogon.net/raspberry-pi/wiringpi/)
Please install it before building this package:

sudo apt-get install git-core
git clone git://git.drogon.net/wiringPi
cd wiringPi
./build

pxauto uses the curses Library.
Please install the following Packages:
sudo apt-get install libncurses5-dev libncursesw5-dev


Installation:
-------------
This Package contains a simple build script.
Please make it executable by running
chmod +x ./build

To build all projects contained in this package, please run
./build

To rebuild a single project, use the make_____ go script located inside the project's folder.

Usage:
------

PiXtend uses the SPI interface of the Raspberry Pi. If not enabled, you have to do the following:

sudo raspi-config
--> "9 Advanced Options" --> "A6 SPI" --> "Would you like the SPI interface to be enabled" --> YES

On older Version of Jessie & Wheezy:

sudo nano /etc/modprobe.d/raspi-blacklist.conf

Add a "#" in front of the line "blacklist spi-bcm2708" or delete this line. Save file and exit nano.

sudo nano /etc/modules

Add new line with text "spidev" (without the speech marks). Save file and exit nano. Reboot the Raspberry Pi:

sudo reboot

The SPI kernal module will be loaded at start-up.

--------------------------------------------------------------------------------------------

pixtendtool uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pixtendtool -h

pxauto uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pxauto


Release History:
----------------
0.2 - pxdev: contains  pixtend, pixtendtool, pxauto, basic build scripts and this documentation.
0.3 - pxdev: added Relay Outputs to DIGOUT Section
0.4 - pxdev: 
	pixtend: modified auto mode to accept byAux0
	pxauto: improved UI, added AIN VRef selection to CTRL section, moved status bytes to new STAT section
0.4.1 - pxauto: minor fixups
0.5.0 - pixtend: library improvements
	 	-added function to switch serial hardware mode between rs232/rs485
		-simplified Spi_Set_Aout function parameters (removed ENABLE and GAIN)
		pixtendtool: improvements and bugfixes
		-added range checks for parameters
		-added parameter to -ai to specify voltage reference (5V/10V)  
		-added parameter to -rh and -rt to specify sensor type (dht11/dht22)
		-improved -di command to allow bitwise reading from digital inputs
		-fixed bug with -pwm value parameter range
		-improved help and output messages 
0.5.1 - pixtend: library improvements
		-added functions to read back current values from DOUT, RELAY
		pixtendtool: 
		-implemented new commands for bitwise read/write access to DOUT, RELAY, GPIOs
		-added uC Version check before executing newly added functions to allow backward compatibility
		pxauto:
		-fixed issue: no more crash if pxauto is executed without root rights or if SPI is not available
0.5.2 - pixtend: bugfix in case of new wiringPi version 2.29
