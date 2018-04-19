# KXKM - ESP32 Audio & battery module firmware

This repo is a complete Arduino sketchbook with libraries, hardware definitions, etc.

Clone it then point Arduino IDE to it using the Preferences > Sketchbook location.

_This repository contains submodules._

Git expects us to explicitly ask it to download the submodule's content. If you're cloning this repository for the first time, you can use a modified clone command to ensure you download everything, including any submodules:

	git clone --recursive <url>
	
If you already cloned the repository you have to run 

    git submodule update --init --recursive

In order to update the repository submodules content, you can execute the following command regularly:

    git submodule update --recursive
    
## ESP32 installation
Execute `hardware/espressif/esp32/tools/get.py`


## STM32 Flashing
You have to export the compiled binary file first (Arduino > Sketch menu).

### Using JLink
* Keep the push button pressed to supply power while the MCU is being reprogrammed
* Slide the battery type selector to "Custom" position
* Run the following commands :
	cd <sketchName>
	JLinkExe -commanderscript STM32_flash.jlink
