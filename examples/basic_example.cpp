#include <stdio.h>
#include "pico/stdlib.h"

#include "AM_SDK_PicoWiFi.h"

#define TCP_PORT 80
#define WIFI_SSID "YOUR NETWORK SSID (name)"
#define WIFI_PASSWORD "YOUR NETWORK PASSWORD"

#define CONNECTIONPIN CYW43_WL_GPIO_LED_PIN
bool led = false;

AMController am_controller;

void doSync()
{
    printf("---- doSync --------\n");

    am_controller.write_message("Led", led);
    am_controller.write_message("S1", led);
    am_controller.write_message("Msg", "Hello, this is your Pico W board!");
}

void deviceConnected()
{
    printf("---- deviceConnected --------\n");

    cyw43_arch_gpio_put(CONNECTIONPIN, 1);
    busy_wait_ms(500);
}

void deviceDisconnected()
{
    printf("---- deviceDisonnected --------\n");

    cyw43_arch_gpio_put(CONNECTIONPIN, 0);
    busy_wait_ms(500);
}

void doWork()
{
    //printf("doWork\n");
    busy_wait_ms(800);

    float r = (rand() % 1000) / 10.0;
    if (r > 50)
    {
        unsigned long now = am_controller.now();

        am_controller.logLn(now);
        am_controller.logLn(r);

        am_controller.log_value("SdLog", now, r);
    }
}

void processIncomingMessages(char *variable, char *value)
{
    printf("processIncomingMessages var: %s value: %s\n", variable, value);

    if (strcmp(variable, "S1") == 0)
    {
        led = atoi(value);
    }
}

void processOutgoingMessages()
{
    //printf("processOutgoingMessages\n");

    float r = (rand() % 1000) / 10.0;

    am_controller.write_message("K", r);
    am_controller.write_message("Led", led);
}

void processAlarms(char *alarmId)
{
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Alarm %s fired\n", alarmId);
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}

void initializeLogFiles()
{
    if (am_controller.log_size("SdLog") > 2000)
    {        
        am_controller.log_purge_data("SdLog");
        printf("SdLog data purged");
    }
    am_controller.log_labels("SdLog", "L1");
}

int main()
{
    stdio_init_all();

    if (cyw43_arch_init())
    {
        printf("failed to initialize\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    cyw43_arch_gpio_put(CONNECTIONPIN, 0);

    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gateway;

    IP4_ADDR(&ip, 192, 168, 1, 38);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 1, 1);

    cyw43_arch_lwip_begin();
    dhcp_stop(cyw43_state.netif);                               // turn off DHCP
    netif_set_addr(cyw43_state.netif, &ip, &netmask, &gateway); // set the IP addr, gateway & net mask
    cyw43_arch_lwip_end();

    printf("Pico W Connecting to %s...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to %s\n", WIFI_SSID);
        return 1;
    }
    else
    {
        printf("Pico W Connected  to %s.\nAvailable at address: %s port: %d\n", WIFI_SSID, ip4addr_ntoa(&ip), TCP_PORT);
    }

    am_controller.init(
        TCP_PORT,
        &doWork,
        &doSync,
        &processIncomingMessages,
        &processOutgoingMessages,
        &deviceConnected,
        &deviceDisconnected,
        &processAlarms,
        &initializeLogFiles);

    cyw43_arch_deinit();
}
