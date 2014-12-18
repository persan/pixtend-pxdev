/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <http://www.pixtend.de> or <http://www.pixtend.com>
#
# Copyright (C) 2014 Christian Strobel, Nils Mensing
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

PiXtend uses the SPI interface of the Raspberry Pi. If not enabled, you have to do the following steps:

sudo nano /etc/modprobe.d/raspi-blacklist.conf

Add a "#" in front of the line "blacklist spi-bcm2708" or delete this line. Save file and exit nano.

sudo nano /etc/modules

Add new line with text "spidev" (without the speech marks). Save file and exit nano. Reboot the Raspberry Pi:

sudo reboot

The SPI kernal module will be loaded at start-up.



pixtendtool uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pixtendtool -h

pxauto uses wiringPi and therefore must be executed with the sudo command all the time:
sudo ./pxauto


Release History:
----------------
1.0 - pxdev: contains  pixtend, pixtendtool, pxauto, basic build scripts and this documentation
