# Initial file from https://github.com/boringplace/Arduino-builder/blob/master/Makefile

# Set these paths according to your system conf.
ARDUINO_PATH = /usr/share/arduino
ARDUINO_PACKAGES_PATH = /home/tom/.arduino15/packages

BOARD_ID = kxkm:stm32:KXKM_STM32F030:xserial=generic,opt=oslto,pnum=KXKM_F030F4
PROJECT_HARDWARE_DIR := "$(CURDIR)/../hardware/"
PROJECT_LIBRARIES := "$(CURDIR)/../libraries"

ifeq ($(ARDUINO_PATH),)
	$(error Arduino path not found)
endif

ARDUINO_PROJECT := $(notdir $(CURDIR)).ino

ARDUINO_BUILDER = $(ARDUINO_PATH)/arduino-builder
ARDUINO_HARDWARE := $(ARDUINO_PATH)/hardware
ARDUINO_TOOLS_BUILDER := $(ARDUINO_PATH)/tools-builder
ARDUINO_TOOLS_HARDWARE_AVR := $(ARDUINO_PATH)/hardware/tools/avr
ARDUINO_LIBRARIES := $(ARDUINO_PATH)/libraries

BUILD_DIR = build
ARDUINO_BUILD_DIR := "$(CURDIR)/$(BUILD_DIR)/arduino"

all: v1 v2
.PHONY: all
	
v1: $(BUILD_DIR)/$(ARDUINO_PROJECT).v1.hex
.PHONY: v1
	
v2: $(BUILD_DIR)/$(ARDUINO_PROJECT).v2.hex
.PHONY: v2
	
$(BUILD_DIR)/$(ARDUINO_PROJECT).%.hex: *.ino *.h
	echo ""
	echo "Compiling for $*"
	mkdir -p $(BUILD_DIR) $(ARDUINO_BUILD_DIR)
	$(ARDUINO_BUILDER) -compile -logger=humantags -hardware $(ARDUINO_HARDWARE) -hardware $(PROJECT_HARDWARE_DIR) -tools $(ARDUINO_TOOLS_BUILDER) -tools $(ARDUINO_TOOLS_HARDWARE_AVR) -tools $(ARDUINO_PACKAGES_PATH) -built-in-libraries $(ARDUINO_LIBRARIES) -libraries $(PROJECT_LIBRARIES) -fqbn=$(BOARD_ID),hw=$* -ide-version=10808 -build-path $(ARDUINO_BUILD_DIR) -warnings=none -prefs=build.warn_data_percentage=80 $(ARDUINO_PROJECT)
	mv $(ARDUINO_BUILD_DIR)/$(ARDUINO_PROJECT).hex $@

test:
	echo $(ARDUINO_BUILDER)
	echo $(ARDUINO_PATH)
