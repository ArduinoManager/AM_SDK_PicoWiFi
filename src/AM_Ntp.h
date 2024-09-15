#ifndef AM_NTTP_H
#define AM_NTTP_H

#include <time.h>

#include "AM_SDK_PicoWiFi.h"

#include "pico/cyw43_arch.h"
#include "lwip/udp.h"

typedef struct NTP_T_
{
    ip_addr_t ntp_server_address;
    bool dns_request_sent;
    struct udp_pcb *ntp_pcb;
    absolute_time_t ntp_test_time;
    alarm_id_t ntp_resend_alarm;
    int tries;
    bool time_set;
} NTP_T;

class Ntp
{
public:
    err_enum_t ntp_init(void);
};



#endif