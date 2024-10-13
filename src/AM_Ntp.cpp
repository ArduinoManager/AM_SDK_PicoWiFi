#include "AM_Ntp.h"
#include "lwip/dns.h"
#include "hardware/rtc.h"

#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME (30 * 1000)
#define NTP_RESEND_TIME (10 * 1000)
#define MAX_TRIES 5

static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void ntp_result(NTP_T *state, int status, time_t *result);
static int64_t ntp_failed_handler(alarm_id_t id, void *user_data);
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);
static void ntp_request(NTP_T *state);

err_enum_t Ntp::ntp_init(void)
{
    NTP_T *state = (NTP_T *)calloc(1, sizeof(NTP_T));
    if (!state)
    {
        DEBUG_printf("failed to allocate state\n");
        return ERR_ARG;
    }
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb)
    {
        DEBUG_printf("failed to create pcb\n");
        free(state);
        return ERR_ARG;
    }
    udp_recv(state->ntp_pcb, ntp_recv, state);

    // Getting ip address

    while (!state->time_set && state->tries < MAX_TRIES)
    {
        int64_t dt = absolute_time_diff_us(get_absolute_time(), state->ntp_test_time);

        DEBUG_printf("Try # %d ", state->tries);
        DEBUG_printf("Delta time %" PRIi64, dt);
        DEBUG_printf(" - Dns request sent %d\n", state->dns_request_sent);
        sleep_ms(500);

        if (dt < 0 && !state->dns_request_sent && !state->time_set)
        {
            state->ntp_resend_alarm = add_alarm_in_ms(NTP_RESEND_TIME, ntp_failed_handler, state, true);
            cyw43_arch_lwip_begin();
            int err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found, state);
            cyw43_arch_lwip_end();

            state->dns_request_sent = true;
            if (err == ERR_OK)
            {
                DEBUG_printf("Send a new request\n");
                ntp_request(state); // Cached result
            }
            else if (err != ERR_INPROGRESS)
            { // ERR_INPROGRESS means expect a callback
                DEBUG_printf("dns request failed\n");
                ntp_result(state, -1, NULL);
            }
        }

        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(state->dns_request_sent ? at_the_end_of_time : state->ntp_test_time);
    }

    if (state->time_set)
    {
        free(state);
        return ERR_OK;
    }
    else
    {
        free(state);
        return ERR_TIMEOUT;
    }
}

// NTP data received
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    NTP_T *state = (NTP_T *)arg;
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &state->ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN && mode == 0x4 && stratum != 0)
    {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
        time_t epoch = seconds_since_1970;
        ntp_result(state, 0, &epoch);
    }
    else
    {
        DEBUG_printf("invalid ntp response\n");
        ntp_result(state, -1, NULL);
    }
    pbuf_free(p);
}

// Called with results of operation

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

static void ntp_result(NTP_T *state, int status, time_t *result)
{
    state->tries += 1;

    if (status == 0 && result)
    {
        state->time_set = true;
        struct tm *utc = gmtime(result);
        DEBUG_printf("got ntp response: %02d/%02d/%04d %02d:%02d:%02d\n", utc->tm_mday, utc->tm_mon + 1, utc->tm_year + 1900, utc->tm_hour, utc->tm_min, utc->tm_sec);

        datetime_t t = {
            .year = utc->tm_year + 1900,
            .month = utc->tm_mon + 1,
            .day = utc->tm_mday,
            //.dotw = 4, // 0 is Sunday, so 5 is Thursday
            .dotw = utc->tm_wday,
            .hour = utc->tm_hour,
            .min = utc->tm_min,
            .sec = utc->tm_sec};

        rtc_set_datetime(&t);
    }

    if (state->ntp_resend_alarm > 0)
    {
        cancel_alarm(state->ntp_resend_alarm);
        state->ntp_resend_alarm = 0;
    }
    state->ntp_test_time = make_timeout_time_ms(NTP_TEST_TIME);
    state->dns_request_sent = false;
}
#pragma GCC diagnostic pop


static int64_t ntp_failed_handler(alarm_id_t id, void *user_data)
{
    NTP_T *state = (NTP_T *)user_data;
    DEBUG_printf("ntp request failed\n");
    ntp_result(state, -1, NULL);
    return 0;
}

// Call back with a DNS result
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    NTP_T *state = (NTP_T *)arg;
    if (ipaddr)
    {
        state->ntp_server_address = *ipaddr;
        DEBUG_printf("ntp address %s\n", ipaddr_ntoa(ipaddr));
        ntp_request(state);
    }
    else
    {
        DEBUG_printf("ntp dns request failed\n");
        ntp_result(state, -1, NULL);
    }
}

// Make an NTP request
static void ntp_request(NTP_T *state)
{
    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *)p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}