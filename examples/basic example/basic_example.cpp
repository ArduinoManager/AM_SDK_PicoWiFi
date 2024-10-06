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
// #include "DHT22.h"

/* Defines */

#define TCP_PORT 180
#define WIFI_SSID "YOUR NETWORK SSID"
#define WIFI_PASSWORD "YOUR NETWORK PASSWORD"

#define CONNECTIONPIN CYW43_WL_GPIO_LED_PIN
#define POTENTIOMETERPIN 28
#define TEMPERATUREPIN 16
#define YELLOWLEDPIN 14
#define BLUELEDPIN 15

/* Gobal variables */

bool led = false;
float pot = 0;
uint16_t blue_led = 0;
float tempC;

// float temperature;
// float humidity;
// unsigned long last_temp_measurement;

AMController am_controller;

/* Local Function Prototypes */

float getVoltage(uint16_t adc_value);

/* Callbacks */

/**
 *
 * This function is called when the iOS/macOS device connects to the Pico board
 *
 */
void deviceConnected()
{
    printf("---- deviceConnected --------\n");

    cyw43_arch_gpio_put(CONNECTIONPIN, 1);
    busy_wait_ms(500);
}

/**
 *
 * This function is called when the iOS/macOS device disconnects to the Pico board
 *
 */
void deviceDisconnected()
{
    printf("---- deviceDisonnected --------\n");

    cyw43_arch_gpio_put(CONNECTIONPIN, 0);
    busy_wait_ms(500);
}

/**
 *
 *
 * This function is called when the iOS/macOS device connects and needs to initialize the position of switches, knobs and other widgets
 *
 */
void doSync()
{
    printf("---- doSync --------\n");

    am_controller.write_message("Led", led);
    am_controller.write_message("S1", led);
    am_controller.write_message("Knob1", blue_led);
    am_controller.write_message("Msg", "Hello, this is your Pico W board!");
}

/**
 *
 *
 * This function is called periodically and its equivalent to the standard loop() function
 *
 */
void doWork()
{
    // printf("doWork\n");
    adc_select_input(2);
    pot = adc_read();

    // DHT22 can be read at most once every 2s
    // if (time_us_64() / 1000 - last_temp_measurement > 2100)
    // {
    //     last_temp_measurement = time_us_64() / 1000;

    //     printf("Reading DHT22 sensor...\n");

    //     uint result = DHT_read(&temperature, &humidity);
    //     if (result == DHT_OK)
    //     {
    //         printf("-->Temperature: %.2f C, Humidity: %.2f%% RH\n", temperature, humidity);
    //     }
    // }

    adc_select_input(4);
    const float conversionFactor = 3.3f / (1 << 12);
    float adc = (float)adc_read() * conversionFactor;
    tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    // sleep_ms(4000);
}

/**
 *
 *
 * This function is called when a new message is received from the iOS/macOS device
 *
 */
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
    if (strcmp(variable, "Push1") == 0)
    {
        // am_controller.gpio_temporary_put(YELLOWLEDPIN, true, 100);
        bool v = atoi(value);
        gpio_put(YELLOWLEDPIN, v);
    }
}

/**
 *
 *
 * This function is called periodically and messages can be sent to the iOS/macOS device
 *
 */
void processOutgoingMessages()
{
    // printf("processOutgoingMessages\n");

    float r = (rand() % 1000) / 10.0;

    am_controller.write_message("K", r);
    am_controller.write_message("Led", led);

    // am_controller.write_message("T", temperature);
    // am_controller.write_message("H", humidity);

    am_controller.write_message("T", tempC);

    am_controller.write_message("Pot", getVoltage(pot));
}

/**
 *
 *
 * This function is called when an alarm is fired
 *
 */
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

/**
 *
 *
 * This function is called once at the program start to initialize log files, if any
 *
 */
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
  Other Auxiliary functions
*/

/*
 returns the voltage on the analog input defined by pin
*/
float getVoltage(uint16_t adc_value)
{
    return (adc_value * 3.3 / 4095.0);
}

/**
 * Main program function used for initial configurations only
 */
int main()
{
    stdio_init_all();

    if (cyw43_arch_init())
    {
        printf("Failed to initialize\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    /** Initialize analog and digital PINs */

    cyw43_arch_gpio_put(CONNECTIONPIN, 0);

    adc_init();
    adc_gpio_init(POTENTIOMETERPIN);

    adc_set_temp_sensor_enabled(true);

    // Initialize the DHT22 sensor on the specified GPIO pin
    // DHT_init(TEMPERATUREPIN);

    gpio_init(YELLOWLEDPIN);
    gpio_set_dir(YELLOWLEDPIN, GPIO_OUT);
    gpio_put(YELLOWLEDPIN, 0);

    gpio_set_function(BLUELEDPIN, GPIO_FUNC_PWM);
    uint slice_num_15 = pwm_gpio_to_slice_num(BLUELEDPIN);
    pwm_set_wrap(slice_num_15, 255);
    pwm_set_enabled(slice_num_15, true);
    blue_led = 0;
    pwm_set_gpio_level(BLUELEDPIN, blue_led);

    /** WiFi network configuration & connection */
    
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
