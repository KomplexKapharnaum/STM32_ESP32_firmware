# Install custom KXKMF0

## KXKM custom

```ini
[env:KXKMF030F4]
platform = ststm32
board = KXKMF030F4

framework = arduino
debug_tool = jlink
upload_protocol = jlink
build_flags = -Os -DHAL_IWDG_MODULE_ENABLED
...
```

```ini
[env:KXKMF070F6]
platform = ststm32
board = KXKMF070F6

framework = arduino
debug_tool = jlink
upload_protocol = jlink
build_flags = -Os -DHAL_IWDG_MODULE_ENABLED
...
```

* copier les fichiers boards.txt & repertoire KXKMF030F4 & KXKMF070F6
* dans leur dossier respectif: ....\. :\Users\xxxx\.platformio\packages\framework-arduinoststm32 pour le fichier boards.txt
* et ....\. :\Users\xxxx\.platformio\packages\framework-arduinoststm32\variants\STM32F0xx pour les dossiers KXKMF0
# Doc
* https://github.com/STMicroelectronics/STM32_open_pin_data/blob/master/mcu/STM32F030F4Px.xml
* https://github.com/STMicroelectronics/STM32_open_pin_data/blob/master/mcu/STM32F070F6Px.xml

# add to Boards.txt
```ini
################################################################################################################################################################################################################################################

# KXKMF030F4 board
GenF0.menu.pnum.KXKMF030F4=STM32F030F4 KXKMF030F4
GenF0.menu.pnum.KXKMF030F4.upload.maximum_size=16384
GenF0.menu.pnum.KXKMF030F4.upload.maximum_data_size=4096
GenF0.menu.pnum.KXKMF030F4.build.board=KXKMF030F4
GenF0.menu.pnum.KXKMF030F4.build.product_line=STM32F030x6
GenF0.menu.pnum.KXKMF030F4.build.variant=STM32F0xx/KXKMF030F4
GenF0.menu.pnum.KXKMF030F4.build.variant_h=variant_{build.board}.h

################################################################################################################################################################################################################################################

################################################################################################################################################################################################################################################

# KXKMF070F6 board
GenF0.menu.pnum.KXKMF070F6=STM32F070F6 KXKMF070F6
GenF0.menu.pnum.KXKMF070F6.upload.maximum_size=32768
GenF0.menu.pnum.KXKMF070F6.upload.maximum_data_size=6144
GenF0.menu.pnum.KXKMF070F6.build.board=KXKMF070F6
GenF0.menu.pnum.KXKMF070F6.build.product_line=STM32F070x6
GenF0.menu.pnum.KXKMF070F6.build.variant=STM32F0xx/KXKMF070F6
GenF0.menu.pnum.KXKMF070F6.build.variant_h=variant_{build.board}.h

################################################################################################################################################################################################################################################
```



