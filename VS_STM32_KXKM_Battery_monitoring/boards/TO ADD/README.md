# Install custom KXKMF030F4

## KXKM custom

```ini
[env:KXKMF030F4]
platform = ststm32
board = KXKMF030F4

framework = arduino
debug_tool = jlink
upload_protocol = jlink
build_flags = -Os -flto -DHAL_IWDG_MODULE_ENABLED
...
```

* copier le dossier **boards** dans le projet
* copier les fichiers user(https://github.com/KomplexKapharnaum/platform-ststm32/tree/vs_code_pio/fichier%20user)
* dans leur dossier respectif: ....\. :\Users\xxxx\.platformio\packages\framework-arduinoststm32 pour le fichier boards.txt
* et ....\. :\Users\xxxx\.platformio\packages\framework-arduinoststm32\variants\STM32F0xx pour le dossier KXKMF030F4

# Doc
https://github.com/STMicroelectronics/STM32_open_pin_data/blob/master/mcu/STM32F030F4Px.xml

# Boards.txt
```ini
################################################################################

# KXKMF030F4 board
GenF0.menu.pnum.KXKMF030F4=STM32F030F4 KXKMF030F4
GenF0.menu.pnum.KXKMF030F4.upload.maximum_size=16384
GenF0.menu.pnum.KXKMF030F4.upload.maximum_data_size=4096
GenF0.menu.pnum.KXKMF030F4.build.board=KXKMF030F4
GenF0.menu.pnum.KXKMF030F4.build.product_line=STM32F030x6
GenF0.menu.pnum.KXKMF030F4.build.variant=STM32F0xx/KXKMF030F4
GenF0.menu.pnum.KXKMF030F4.build.variant_h=variant_{build.board}.h

################################################################################
```



