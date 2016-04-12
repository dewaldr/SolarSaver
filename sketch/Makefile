# SolarSaver sketch Makefile

# Arduino.mk variables
ARDUINO_DIR			= /opt/arduino
ARDMK_DIR			= /usr
AVR_TOOLS_DIR		= /usr

TARGET				= solarsaver
ARDUINO_LIBS		= LowPower RunningAverage

BOARD_TAG			= pro
ARDUINO_PORT		= /dev/ttyUSB0
MONITOR_BAUDRATE	= 9600

include /usr/share/arduino/Arduino.mk

realclean: clean
	rm -f *~
	rm -rf build-pro
