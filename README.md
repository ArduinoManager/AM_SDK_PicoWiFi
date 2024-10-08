# AM_SDK_PicoWiFi


# How to configure a manual generated project

1) Open Visual Studio Code

2) Using the Raspberry Pico extension create a new Project:

    - Project Name: [Project Name]
    - Board Type: Pico W
    - Location: any folder of choice
    - Pico SDK Version: 2.0.0 or greater
    - Stdio Support: Console over UART, Console over USB or both, depending on your setup
    - Wireless Options: Pico W onboard LED
    - Generate C++ code
    - DebugProbe or SWD depending on your setup

3) Create a new folder named lib

4) Right click the lib folder and select Open in Integrated Terminal

5) enter the following commands:

```
git clone --recurse-submodules https://github.com/ArduinoManager/AM_SDK_PicoWiFi.git
git clone --recurse-submodules https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.git
cd no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
git switch --detach tags/v3.5.1
```
6) From the Raspberry Pico extension select Configure CMake

7) Press F5 to run the program

# How to configure a Arduino Manager Code Generator project

1) Generate the code using the Arduino Manager Code Generator

2) Open Visual Studio Code and using the Raspberry Pico extension import the project

3) Create a new folder named lib

4) Right click the lib folder and select Open in Integrated Terminal

5) enter the following commands:

```
git clone --recurse-submodules https://github.com/ArduinoManager/AM_SDK_PicoWiFi.git
git clone --recurse-submodules https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.git
cd no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
git switch --detach tags/v3.5.1
```
6) From the Raspberry Pico extension select Configure CMake

7) Press F5 to run the program

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