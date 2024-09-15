#ifndef AM_PICOWIFI_H
#define AM_PICOWIFI_H

#include "AM_Alarms.h"
#include "AM_SDManager.h"

#include "lwip/arch.h"

#include "lwip/tcp.h"

#ifdef DEBUG
#define DEBUG_printf printf
#else
#define DEBUG_printf
#endif

#define BUF_SIZE 2048
#define POLL_TIME_S 0.5

#define VARIABLELEN 14
#define VALUELEN 14

// class AM_PicoWiFi; // This is useful
class SDManager;

typedef struct TCP_SERVER_T_
{
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    char buffer_to_send[BUF_SIZE];
    char buffer_recv[BUF_SIZE];
    int sent_len;
    int recv_len;
    bool is_device_connected;

} TCP_SERVER_T;

class AMController
{
private:
    u16_t port;

    void (*doWork)(void);                                         // Pointer to the function where to put code in place of loop()
    void (*doSync)();                                             // Pointer to the function where widgets are synchronized
    void (*processIncomingMessages)(char *variable, char *value); // Pointer to the function where incoming messages are processed
    void (*processOutgoingMessages)(void);                        // Pointer to the function where outgoing messages are processed
    void (*deviceConnected)(void);                                // Pointer to the function called when a device connects to Arduino
    void (*deviceDisconnected)(void);                             // Pointer to the function called when a device disconnects from Arduino
    void (*processAlarms)(char *alarm);                           // Pointer to the function called when an alarm is fired
    void (*initializeLogFiles)();                                 // Pointer to the function called to initialize the log files

    Alarms alarms;
    struct repeating_timer alarms_checks_timer;
    static bool alarm_timer_callback(__unused struct repeating_timer *t);

    SDManager *sd_manager;

    TCP_SERVER_T state;
    bool is_sending_content = false;

    bool tcp_server_open(void *arg, u16_t port);
    static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);
    static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
    static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
    static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
    static err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb);
    static void tcp_server_err(void *arg, err_t err);
    static err_t tcp_server_result(void *arg, int status);
    static err_t tcp_server_close(void *arg);

public:
    void init(
        u16_t port,
        void (*doWork)(void),
        void (*doSync)(void),
        void (*processIncomingMessages)(char *variable, char *value),
        void (*processOutgoingMessages)(void),
        void (*deviceConnected)(void),
        void (*deviceDisconnected)(void),
        void (*processAlarms)(char *alarm),
        void (*initializeLogFiles)());

    void write_message(const char *variable, int value);

    void write_message(const char *variable, long value);
    void write_message(const char *variable, unsigned long value);
    void write_message(const char *variable, float value);
    void write_message(const char *variable, const char *value);
    void write_message_buffer(const char *value, uint size);

    unsigned long now();

    void log(int msg);
    void log(long msg);
    void log(unsigned long msg);
    void log(float msg);
    void log(const char *msg);
    void logLn(int msg);
    void logLn(long msg);
    void logLn(unsigned long msg);
    void logLn(float msg);
    void logLn(const char *msg);

    void log_labels(const char *variable, const char *label1);
    void log_labels(const char *variable, const char *label1, const char *label2);
    void log_labels(const char *variable, const char *label1, const char *label2, const char *label3);
    void log_labels(const char *variable, const char *label1, const char *label2, const char *label3, const char *label4);
    void log_labels(const char *variable, const char *label1, const char *label2, const char *label3, const char *label4, const char *label5);

    void log_value(const char *variable, unsigned long time, float v1);
    void log_value(const char *variable, unsigned long time, float v1, float v2);
    void log_value(const char *variable, unsigned long time, float v1, float v2, float v3);
    void log_value(const char *variable, unsigned long time, float v1, float v2, float v3, float v4);
    void log_value(const char *variable, unsigned long time, float v1, float v2, float v3, float v4, float v5);

    unsigned long log_size(const char *variable);
    void log_purge_data(const char *variable);

    void processBuffer(char *buffer, int len);
};

#endif