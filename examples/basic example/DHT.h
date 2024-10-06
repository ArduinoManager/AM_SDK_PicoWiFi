/*
   Simple class to read temperature and humidity whith a DHT22 sensor

   Author: Fabrizio Boco - fabboco@gmail.com

   Version: 1.0

   10/05/2024

   All rights reserved

   This work has been derived from here: https://github.com/eleanor-em/pico-dht22/tree/main

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

#ifndef DHTH
#define DHTH

#include <math.h>

#define DHT_OK 0
#define DHT_ERR_CHECKSUM 1
#define DHT_ERR_NAN 2

class DHT
{
private:
    /* data */
    uint8_t pin;
    float humidity;
    float temp_celsius;

    uint32_t wait_for(uint8_t pin, uint8_t expect);
    inline float word(uint8_t first, uint8_t second)
    {
        return (float)((first << 8) + second);
    };

public:
    DHT(/* args */);
    ~DHT();

    void init(uint8_t pin);
    uint8_t read();
    float getTemperature();
    float getHumitiy();
};

DHT::DHT(/* args */)
{
}

DHT::~DHT()
{
}

void DHT::init(uint8_t pin)
{
    this->pin = pin;
    gpio_init(pin);
}

uint8_t DHT::read()
{
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // request a sample
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
    sleep_ms(10);
    gpio_put(pin, 1);
    sleep_us(40);

    // wait for acknowledgement
    gpio_set_dir(pin, GPIO_IN);
    wait_for(pin, 0);
    wait_for(pin, 1);
    wait_for(pin, 0);


    // read sample (40 bits = 5 bytes)
    for (uint8_t bit = 0; bit < 40; ++bit) {
        wait_for(pin, 1);
        uint8_t count = wait_for(pin, 0);
        data[bit / 8] <<= 1;
        if (count > 50) {
            data[bit / 8] |= 1;
        }
    }

    // pull back up to mark end of read
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 1);

    // checksum
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
    {
        float humidity = word(data[0], data[1]) / 10;
        float temp = word(data[2] & 0x7F, data[3]) / 10;

        // if the highest bit is 1, temperature is negative
        if (data[2] & 0x80)
        {
            temp = -temp;
        }

        // check if checksum was OK but something else went wrong
        if (isnan(temp) || isnan(humidity) || temp == 0)
        {
            return DHT_ERR_NAN;
        }
        else
        {
            this->humidity = humidity;
            this->temp_celsius = temp;
            return DHT_OK;
        }
    }
    else
    {
        return DHT_ERR_CHECKSUM;
    }
}

float DHT::getTemperature()
{
    return temp_celsius;
}

float DHT::getHumitiy()
{
    return humidity;
}

uint32_t DHT::wait_for(uint8_t pin, uint8_t expect)
{
    uint32_t then = time_us_32();
    while (expect != gpio_get(pin)) {
        sleep_us(10);
    }
    return time_us_32() - then;
}

#endif