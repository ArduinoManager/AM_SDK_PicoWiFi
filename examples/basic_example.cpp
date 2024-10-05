/*
   Test Arduino Manager for iPad / iPhone / Mac

   A simple test program to show the Arduino Manager features.

   Author: Fabrizio Boco - fabboco@gmail.com

   Version: 1.0

   09/15/2024

   All rights reserved

*/

/*
   AMController libraries, example sketches (The Software) and the related documentation (The Documentation) are supplied to you
   by the Author in consideration of your agreement to the following terms, and your use or installation of The Software and the use of The Documentation
   constitutes acceptance of these terms.
   If you do not agree with these terms, please do not use or install The Software.
   The Author grants you a personal, non-exclusive license, under authors copyrights in this original software, to use The Software.
   Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by the Author, including but not limited to any
   patent rights that may be infringed by your derivative works or by other works in which The Software may be incorporated.
   The Software and the Documentation are provided by the Author on an AS IS basis.  THE AUTHOR MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT
   LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE SOFTWARE OR ITS USE AND OPERATION
   ALONE OR IN COMBINATION WITH YOUR PRODUCTS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
   REPRODUCTION AND MODIFICATION OF THE SOFTWARE AND OR OF THE DOCUMENTATION, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
   STRICT LIABILITY OR OTHERWISE, EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#include "AM_SDK_PicoWiFi.h"

#include "dht22-pico.h"

#define TCP_PORT 80
#define WIFI_SSID "homewirelessnetwork"
#define WIFI_PASSWORD "Fabrizio22Camilla22Laura26!"

#define CONNECTIONPIN CYW43_WL_GPIO_LED_PIN
#define POTENTIOMETERPIN 28
#define TEMPERATUREPIN 16
#define YELLOWLEDPIN 14
#define BLUELEDPIN 15

bool led = false;
float pot = 0;
uint16_t blue_led = 0;
float tempC;

AMController am_controller;
// dht_reading dht = dht_init(TEMPERATUREPIN);

float getVoltage(uint16_t adc_value);

void doSync()
{
    printf("---- doSync --------\n");

    am_controller.write_message("Led", led);
    am_controller.write_message("S1", led);
    am_controller.write_message("Knob1", blue_led);
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
    // printf("doWork\n");
    adc_select_input(2);
    pot = adc_read();

    // printf("Reading DHT22 sensor...\n");

    // uint8_t status = dht_read(&dht);
    // if (status == DHT_OK)
    // {
    //     printf("RH: %.1f%%\tTemp: %.1fC\n", dht.humidity, dht.temp_celsius);
    // }
    // else if (status == DHT_ERR_CHECKSUM)
    // {
    //     printf("Bad data (checksum)\n");
    // }
    // else
    // {
    //     printf("Bad data (NaN)\n");
    // }

    adc_select_input(4);
    const float conversionFactor = 3.3f / (1 << 12);
    float adc = (float)adc_read() * conversionFactor;
    tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    sleep_ms(2000);
}

void processIncomingMessages(char *variable, char *value)
{
    printf("processIncomingMessages var: %s value: %s\n", variable, value);

    if (strcmp(variable, "S1") == 0)
    {
        led = atoi(value);
        gpio_put(YELLOWLEDPIN, led);
    }

    if (strcmp(variable, "Knob1") == 0)
    {
        blue_led = atoi(value);
        pwm_set_gpio_level(BLUELEDPIN, blue_led);
    }
}

void processOutgoingMessages()
{
    // printf("processOutgoingMessages\n");

    float r = (rand() % 1000) / 10.0;

    am_controller.write_message("K", r);
    am_controller.write_message("Led", led);

    // am_controller.write_message("T", dht.temp_celsius);
    // am_controller.write_message("H", dht.humidity);

    am_controller.write_message("T", tempC);

    am_controller.write_message("Pot", getVoltage(pot));
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
    led = !led;
    gpio_put(YELLOWLEDPIN, led);
}

void initializeLogFiles()
{
    printf("---- Initialize Log Files --------\n");

    if (am_controller.log_size("SdLog") > 2000)
    {
        am_controller.log_purge_data("SdLog");
        printf("SdLog data purged");
    }
    am_controller.log_labels("SdLog", "L1");
}

/**
  Auxiliary functions
*/

/*
 returns the voltage on the analog input defined by pin
*/
float getVoltage(uint16_t adc_value)
{
    return (adc_value * 3.3 / 4095.0);
}

int main()
{
    stdio_init_all();

    if (cyw43_arch_init())
    {
        printf("Failed to initialize\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    cyw43_arch_gpio_put(CONNECTIONPIN, 0);

    adc_init();
    adc_gpio_init(POTENTIOMETERPIN);

    adc_set_temp_sensor_enabled(true);

    gpio_init(YELLOWLEDPIN);
    gpio_set_dir(YELLOWLEDPIN, GPIO_OUT);
    gpio_put(YELLOWLEDPIN, 0);

    gpio_set_function(BLUELEDPIN, GPIO_FUNC_PWM);
    uint slice_num_15 = pwm_gpio_to_slice_num(BLUELEDPIN);
    pwm_set_wrap(slice_num_15, 255);
    pwm_set_enabled(slice_num_15, true);
    blue_led = 0;
    pwm_set_gpio_level(BLUELEDPIN, blue_led);

    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gateway;

    IP4_ADDR(&ip, 192, 168, 1, 17);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gateway, 192, 168, 1, 1);

    cyw43_arch_lwip_begin();
    dhcp_stop(cyw43_state.netif);                               // turn off DHCP
    netif_set_addr(cyw43_state.netif, &ip, &netmask, &gateway); // set the IP addr, gateway & net mask
    cyw43_arch_lwip_end();

    uint8_t connection_tries = 0;
    while (connection_tries < 3)
    {
        printf("Pico W Connecting to %s...\n", WIFI_SSID);
        int conn_status = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
        if (conn_status == 0)
        {
            printf("Pico W Connected  to %s.\nAvailable at address: %s port: %d\n", WIFI_SSID, ip4addr_ntoa(&ip), TCP_PORT);
            break;
        }
        else
        {
            printf("Failed to connect to %s [Error: %d]\n", WIFI_SSID, conn_status);
            connection_tries += 1;
        }
    }
    if (connection_tries == 3)
    {
        exit(-1);
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
