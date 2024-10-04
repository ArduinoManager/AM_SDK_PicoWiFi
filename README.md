# AM_SDK_PicoWiFi


# How to configure a manual generated project


# Hot to congigure a Arduino Manager Code Generator project

Go to the project folder and enter the following commands:

```
mkdir lib
cd lib
git clone --recurse-submodules https://github.com/ArduinoManager/AM_SDK_PicoWiFi.git
git clone --recurse-submodules https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.git
cd no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
git switch --detach tags/v3.5.1
```


# How to Debug the SD Library

1) Open file: no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/include/my_debug.h
2) add the following lines

    #define USE_PRINTF 1
    #define USE_DBG_PRINTF 1