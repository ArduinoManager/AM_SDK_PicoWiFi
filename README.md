# AM_SDK_PicoWiFi


# How to configure a manual generated project


# How to configure a Arduino Manager Code Generator project

Go to the project folder and enter the following commands:

```
mkdir lib
cd lib
git clone --recurse-submodules https://github.com/ArduinoManager/AM_SDK_PicoWiFi.git
git clone --recurse-submodules https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.git
cd no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
git switch --detach tags/v3.5.1
```

# How to Debug the Library

1) Open the file

```
 lib/AM_SDK_PicoWiFi/src/CMakeLists.txt
```
2) Uncomment the required defines:

```
    # DEBUG         # Uncomment this line to debug the library
    # DUMP_ALARMS   # Uncomment this line to dump the alarms
    # DEBUG_SD      # Uncomment this line to debug SD Card operations
```

# How to Debug the SD Library

1) Open the file
```
 no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/include/my_debug.h
 ```
2) add the following lines

```
    #define USE_PRINTF 1
    #define USE_DBG_PRINTF 1
```

# Update the library

```
git pull origin main
```