# Initial file from https://github.com/boringplace/Arduino-builder/blob/master/Makefile

# Set these paths according to your system conf.
ARDUINO_PATH = /usr/share/arduino
ARDUINO_PACKAGES_PATH = /home/tom/.arduino15/packages

PROJECT_HARDWARE_DIR = "$(CURDIR)/../hardware/"
BOARD_ID = kxkm:stm32:KXKM_STM32F030:xserial=generic,opt=oslto,pnum=KXKM_F030F4,hw=v2

ifeq ($(ARDUINO_PATH),)
	$(error Arduino path not found)
endif

ARDUINO_PROJECT := $(notdir $(CURDIR)).ino

ARDUINO_BUILDER = $(ARDUINO_PATH)/arduino-builder
ARDUINO_HARDWARE := $(ARDUINO_PATH)/hardware
ARDUINO_TOOLS_BUILDER := $(ARDUINO_PATH)/tools-builder
ARDUINO_TOOLS_HARDWARE_AVR := $(ARDUINO_PATH)/hardware/tools/avr
ARDUINO_LIBRARIES := $(ARDUINO_PATH)/libraries

BUILD_DIR := "$(CURDIR)/build"
BUILD_LIBRARIES := "$(CURDIR)/../libraries"

all: build/$(ARDUINO_PROJECT).hex

build/%.ino.hex: %.ino
	mkdir -p $(BUILD_DIR)
	$(ARDUINO_BUILDER) -compile -logger=humantags -hardware $(ARDUINO_HARDWARE) -hardware $(PROJECT_HARDWARE_DIR) -tools $(ARDUINO_TOOLS_BUILDER) -tools $(ARDUINO_TOOLS_HARDWARE_AVR) -tools $(ARDUINO_PACKAGES_PATH) -built-in-libraries $(ARDUINO_LIBRARIES) -libraries $(BUILD_LIBRARIES) -fqbn=$(BOARD_ID) -ide-version=10808 -build-path $(BUILD_DIR) -warnings=none -prefs=build.warn_data_percentage=80 $<

test:
	echo $(ARDUINO_BUILDER)
	echo $(ARDUINO_PATH)
