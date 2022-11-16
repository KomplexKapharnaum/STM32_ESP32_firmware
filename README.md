# KXKM - ESP32 Audio & battery module firmware V3 for STM32F070F6

This repo is a platformIO code to V3 kxkm

_This repository contains submodules._

Git expects us to explicitly ask it to download the submodule's content. If you're cloning this repository for the first time, you can use a modified clone command to ensure you download everything, including any submodules:

	git clone --recursive <url>
	
If you already cloned the repository you have to run 

    git submodule update --init --recursive

In order to update the repository submodules content, you can execute the following command regularly:

    git submodule update --recursive
    
## ESP32 installation
Execute `hardware/espressif/esp32/tools/get.py`

## STM32 Flashing Using JLink
needed the jlink software : https://www.segger.com/downloads/jlink/

* Keep the push button pressed to supply power while the MCU is being reprogrammed
* Slide the battery type selector to "Custom" position


## STM32 calibrating ADC value and store in option byte to accurate reading battery voltage

##### 1 - upload VS-STM32-ADC-calib
##### 2 - connect serial with TX/RX reversed to read serial from the STM32 and not from the ESP
##### 3 - power the board with 24V power to calibrate ADC 4 - reset board and read serial ouptut :


When:  -- STORED --  as printed, the adc are calibrated and stored in option byte



## STM32 coprocessor - battery monitoring function
change board ID 	before upload VS_STM32-KXKM-Battery_monitoring			
