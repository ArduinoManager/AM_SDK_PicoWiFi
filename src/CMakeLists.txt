add_library(AM_PicoWiFi INTERFACE)

target_compile_definitions(AM_PicoWiFi INTERFACE
    picow_tcpip_server_background1
    PICO_MAX_SHARED_IRQ_HANDLERS=8u
)

target_sources(AM_PicoWiFi INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/AM_SDK_PicoWiFi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AM_Ntp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AM_Alarms.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AM_SDManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hw_config.cpp
)

add_subdirectory(../../no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

target_include_directories(AM_PicoWiFi INTERFACE
    include
    ${CMAKE_CURRENT_LIST_DIR}
    picow_tcpip_server_background1
)

target_link_libraries(
    pico_stdlib INTERFACE
    hardware_rtc INTERFACE
    pico_aon_timer INTERFACE
    pico_cyw43_arch_lwip_threadsafe_background INTERFACE
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico INTERFACE
)

target_compile_definitions(AM_PicoWiFi INTERFACE
    # DEBUG         # Uncomment this line to debug the library
    DUMP_ALARMS   # Uncomment this line to debug alarms
)