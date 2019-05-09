# Initial file from https://github.com/boringplace/Arduino-builder/blob/master/Makefile
# 
# make all => builds the current firmware for all HW revisions
# make flash HW_REV=v1  => flashes the built firmware for HW revision 1
# 
# Dependencies : arduino with the STM32 core installed, JLinkExe

# Set these paths according to your system conf.
ARDUINO_PATH = /usr/share/arduino
ARDUINO_PACKAGES_PATH = /home/tom/.arduino15/packages

JLINK_EXECUTABLE = JLinkExe

BOARD_ID = kxkm:stm32:KXKM_STM32F030:xserial=generic,opt=oslto,pnum=KXKM_F030F4
JLINK_DEVICE_NAME = STM32F030F4
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

JLINK_TEMP_DIR := $(BUILD_DIR)/jlink
JLINK_FLASH_SCRIPT := $(JLINK_TEMP_DIR)/flash_stm32_$(HW_REV).jlink

.PHONY: all
all: v1 v2
	
.PHONY: v1
v1: $(BUILD_DIR)/$(ARDUINO_PROJECT).v1.hex
	
.PHONY: v2
v2: $(BUILD_DIR)/$(ARDUINO_PROJECT).v2.hex
	
$(BUILD_DIR)/$(ARDUINO_PROJECT).%.hex: *.ino *.h
	@echo ""
	@echo "Compiling for $*"
	@mkdir -p $(BUILD_DIR) $(ARDUINO_BUILD_DIR)
	@$(ARDUINO_BUILDER) -compile -logger=humantags -hardware $(ARDUINO_HARDWARE) -hardware $(PROJECT_HARDWARE_DIR) -tools $(ARDUINO_TOOLS_BUILDER) -tools $(ARDUINO_TOOLS_HARDWARE_AVR) -tools $(ARDUINO_PACKAGES_PATH) -built-in-libraries $(ARDUINO_LIBRARIES) -libraries $(PROJECT_LIBRARIES) -fqbn=$(BOARD_ID),hw=$* -ide-version=10808 -build-path $(ARDUINO_BUILD_DIR) -warnings=none -prefs=build.warn_data_percentage=80 $(ARDUINO_PROJECT)
	@mv $(ARDUINO_BUILD_DIR)/$(ARDUINO_PROJECT).hex $@
	
flash:
	@mkdir -p $(JLINK_TEMP_DIR)
	echo "" > $(JLINK_FLASH_SCRIPT)
	echo "loadfile \"$(BUILD_DIR)/$(ARDUINO_PROJECT).$(HW_REV).hex\"" >> $(JLINK_FLASH_SCRIPT)
	@echo "r" >> $(JLINK_FLASH_SCRIPT)
	@echo "g" >> $(JLINK_FLASH_SCRIPT)
	@echo "qc" >> $(JLINK_FLASH_SCRIPT)
	@$(JLINK_EXECUTABLE) -device $(JLINK_DEVICE_NAME) -if SWD -speed 4000 -autoconnect 1 -CommanderScript $(JLINK_FLASH_SCRIPT)

test:
	echo $(ARDUINO_BUILDER)
	echo $(ARDUINO_PATH)
