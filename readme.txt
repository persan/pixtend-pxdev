/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <http://www.pixtend.de> or <http://www.pixtend.com>
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

Overview:
---------
pxdev is the PiXtend Linux Development Package and part of the PiXtend(R) project. More infos on www.pixtend.de.

pixtend       - This linux C library is for SPI Communication between the Raspberry Pi and the PiXtend Microcontroller, using wiringPi (https://projects.drogon.net/raspberry-pi/wiringpi/)
pixtendtool   - A linux command line tool to monitor and control the PiXtend with simple commands, using the pixtend library
pixtendtool2s - A linux command line tool to monitor and control the PiXtend V2 -S- with simple commands, using the pixtend library
pixtendtool2l - A linux command line tool to monitor and control the PiXtend V2 -L- with simple commands, using the pixtend library
pxauto        - A linux console application to live monitor and control the PiXtend from a Graphical User Interface. Uses the pixtend library for communication and the curses library for the User Interface.
pxauto2s      - A linux console application to live monitor and control the PiXtend V2 -S- board from a Graphical User Interface. Uses the pixtend library for communication and the curses library for the User Interface.
pxauto2l      - A linux console application to live monitor and control the PiXtend V2 -L- board from a Graphical User Interface. Uses the pixtend library for communication and the curses library for the User Interface. 


Installation Requirements:
--------------------------
pixtend uses the wiringPi Library (https://projects.drogon.net/raspberry-pi/wiringpi/)
Please install it before building this package:

sudo apt-get install git-core
git clone git://git.drogon.net/wiringPi
cd wiringPi
./build

pxauto, pxauto2s and pxauto2l use the curses Library.
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
--> „5 Interfacing Options“ → P4 SPI → „Yes“ → „Ok“

Reboot the Raspberry Pi:

sudo reboot

The SPI kernel module will be loaded at start-up.

--------------------------------------------------------------------------------------------

pixtendtool uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pixtendtool -h

pixtendtool2s uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pixtendtool2s -h

pixtendtool2l uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pixtendtool2l -h

pxauto uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pxauto

pxauto2s uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pxauto2s

pxauto2l uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pxauto2l


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
0.5.3 - pixtend: updated copyrights
        pixtendtool: added parameters for serial read/write functions
        pxauto: initialize UC_CTRL value on startup.
0.5.4 - pxauto2s added to the package to monitor and control the PiXtend V2-S board.
      - pixtendtool2s: added to the package to control the PiXtend V2-S board from the commandline
      - pixtend: library was extended to handle PiXtend V2 -S- boards
      - Programs have been seperated by PiXtend hardware version into their own folders. The pixtend library remains at the top level of the package
      - Internal Beta Version
0.5.5 - pxauto2s: Minor fixes in the user interface, placement of new elements, automatic change in communication speed with the microcontroller if atleast one GPIO is in DHT11/22 mode
      - pixtendtool2s: Improvments on internal handling and speed. Correction of texts.
      - pixtend: Library was extended to recognize wrong temperature and humidity values if the microcontroller had problems reading data from the sensors
      - First release version for pixtendtool2s and pxauto2s.
0.5.6 - First release version for pixtendtool2l and pxauto2l.
      - pixtend: Library was extended to also support the PiXtend V2 -L- board
      - Company name changed to Qube Solutions GmbH
