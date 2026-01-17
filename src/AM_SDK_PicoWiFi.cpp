#include <iostream>

#include "AM_SDK_PicoWiFi.h"
#include "AM_Ntp.h"
#include "AM_Alarms.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/opt.h"

#include "lwip/api.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// #include "hardware/rtc.h"
#include "pico/aon_timer.h"

#include "f_util.h"
#include "ff.h"
#include "my_rtc.h"

void AMController::init(
    u16_t port,
    void (*doWork)(void),
    void (*doSync)(void),
    void (*processIncomingMessages)(char *variable, char *value),
    void (*processOutgoingMessages)(void),
    void (*deviceConnected)(void),
    void (*deviceDisconnected)(void),
    void (*processAlarms)(char *alarm))
{
   DEBUG_printf("Library Initialization\n");

   this->port = port;
   this->doWork = doWork;
   this->doSync = doSync;
   this->processIncomingMessages = processIncomingMessages;
   this->processOutgoingMessages = processOutgoingMessages;
   this->deviceConnected = deviceConnected;
   this->deviceDisconnected = deviceDisconnected;
   this->processAlarms = processAlarms;

   sd_manager = new SDManager(this);

   struct tm ctime;

   // aon_timer_get_time_calendar(&d);
   // DEBUG_printf("Current Time %s", asctime(&ctime));

   // Setup current time
   Ntp nttp;

   if (nttp.ntp_init() != ERR_OK)
   {
      printf("Time not set!\n");
   }

   aon_timer_get_time_calendar(&ctime);
   ctime.tm_isdst = 0;
   char buffer[32];
   if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ctime) > 0)
   {
      printf("Current GMT Time: %s\r\n", buffer);
   }

   // Setup SD Card
   time_init();

   time_t now = time(NULL);

   DEBUG_printf("Now time %" PRIi64, now);
   DEBUG_printf("\n");

   FATFS fs;
   FRESULT fr = f_mount(&fs, "", 1);
   if (FR_OK == fr)
   {
      DEBUG_printf("SD Card mounted!\n");
      if (processAlarms != NULL)
      {
         // Initialize Alarms
         alarms.init_alarms();
         add_repeating_timer_ms(ALARMS_CHECKS_PERIOD, this->alarm_timer_callback, this, &alarms_checks_timer);
      }
   }
   else
   {
      printf("Error: SD not mounted!\n");
   }

   f_unmount("");

   // Setup the TCP server

   state.is_device_connected = false;
   state.is_sync_completed = false;
   if (!tcp_server_open(&state, port))
   {
      DEBUG_printf("TCPIP Server not opened\n");
      return;
   }

   while (true)
   {
      doWork();
      if (state.is_device_connected & state.is_sync_completed)
      {
         processOutgoingMessages();
      }

#if PICO_CYW43_ARCH_POLL
      // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
      // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
      cyw43_arch_poll();
      // you can poll as often as you like, however if you have nothing else to do you can
      // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
      cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
      // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
      // is done via interrupt in the background. This sleep is just an example of some (blocking)
      // work you might be doing.
      sleep_ms(1);
#endif
   }
}

bool AMController::alarm_timer_callback(__unused struct repeating_timer *t)
{
   AMController *p = (AMController *)t->user_data;

   p->alarms.check_fire_alarms(p->processAlarms);

   return true;
}

void AMController::write_message(const char *variable, int value)
{
   char buffer[VARIABLELEN + VALUELEN + 3];

   snprintf(buffer, VARIABLELEN + VALUELEN + 2, "%s=%d#", variable, value);

   if (strlen(state.buffer_to_send) + strlen(buffer) > BUF_SIZE - 1)
   {
      DEBUG_printf("Message Discarded\n");
      return;
   }

   strncat(state.buffer_to_send, buffer, VARIABLELEN + VALUELEN + 2);

   size_t len = strlen(state.buffer_to_send);
   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state.buffer_to_send);
      exit(-1);
   }
}

void AMController::write_message(const char *variable, long value)
{
   char buffer[VARIABLELEN + VALUELEN + 3];

   snprintf(buffer, VARIABLELEN + VALUELEN + 2, "%s=%ld#", variable, value);

   if (strlen(state.buffer_to_send) + strlen(buffer) > BUF_SIZE - 1)
   {
      DEBUG_printf("Message Discarded\n");
      return;
   }

   // printf(">>>>>%s<\n",state.buffer_to_send);
   strncat(state.buffer_to_send, buffer, VARIABLELEN + VALUELEN + 2);

   size_t len = strlen(state.buffer_to_send);
   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state.buffer_to_send);
      exit(-1);
   }
}

void AMController::write_message(const char *variable, unsigned long value)
{
   char buffer[VARIABLELEN + VALUELEN + 3];

   snprintf(buffer, VARIABLELEN + VALUELEN + 2, "%s=%lu#", variable, value);

   if (strlen(state.buffer_to_send) + strlen(buffer) > BUF_SIZE - 1)
   {
      DEBUG_printf("Message Discarded\n");
      return;
   }

   // printf(">>>>>%s<\n", state.buffer_to_send);
   strncat(state.buffer_to_send, buffer, VARIABLELEN + VALUELEN + 2);

   size_t len = strlen(state.buffer_to_send);
   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state.buffer_to_send);
      exit(-1);
   }
}

void AMController::write_message(const char *variable, float value)
{
   char buffer[VARIABLELEN + VALUELEN + 3];

   snprintf(buffer, VARIABLELEN + VALUELEN + 2, "%s=%.5g#", variable, value);

   if (strlen(state.buffer_to_send) + strlen(buffer) > BUF_SIZE - 1)
   {
      DEBUG_printf("Message Discarded\n");
      return;
   }

   // printf(">>>>>%s<\n", state.buffer_to_send);
   strncat(state.buffer_to_send, buffer, VARIABLELEN + VALUELEN + 2);

   size_t len = strlen(state.buffer_to_send);
   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state.buffer_to_send);
      exit(-1);
   }
}

void AMController::write_message(const char *variable, const char *value)
{
   char buffer[BUF_SIZE];

   snprintf(buffer, BUF_SIZE, "%s=%s#", variable, value);

   if (strlen(state.buffer_to_send) + strlen(buffer) > BUF_SIZE - 1)
   {
      DEBUG_printf("Message Discarded\n");
      return;
   }

   strncat(state.buffer_to_send, buffer, BUF_SIZE - 1);
   // printf(">>>>>%s<\n", state.buffer_to_send);

   size_t len = strlen(state.buffer_to_send);
   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state.buffer_to_send);
      exit(-1);
   }
}

void AMController::write_message_immediate(const char *variable, const char *value)
{
   char buffer[BUF_SIZE];

   state.buffer_to_send[0] = 0; // The buffer is emptied so no other data can be sent at the same time

   snprintf(buffer, BUF_SIZE, "%s=%s#", variable, value);

   int retries = 0;
   err_t err;
   while (retries < 20)
   {

      err = tcp_write(state.client_pcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
      if (err == ERR_OK)
      {
         break;
      }
      else
      {
         sleep_ms(100);
      }

      err = tcp_output(state.client_pcb);
      if (err == ERR_OK)
      {
         break;
      }
      else
      {
         sleep_ms(100);
      }

      retries += 1;
   }

   if (retries == 20)
   {
      print_error("write_message_immediate - tcp_output", err);
      exit(-1);
   }
}

void AMController::write_message_buffer(const char *value, uint size)
{
   // DEBUG_printf("b[%d]",size);
   // strncpy(state.buffer_to_send, value, size);
   // state.buffer_to_send[size] = 0;
   // tcp_server_send_data(this, state.client_pcb);

   state.buffer_to_send[0] = 0; // The buffer is emptied so no other data can be sent at the same time

   int retries = 0;
   err_t err;

   while (retries < 20)
   {
      err = tcp_write(state.client_pcb, value, size, TCP_WRITE_FLAG_COPY);
      if (err == ERR_OK)
      {
         break;
      }
      else
      {
         retries += 1;
         sleep_ms(100);
      }
   }
   if (retries == 20)
   {
      print_error("write_message_buffer - tcp_write", err);
      exit(-1);
   }

   while (retries < 20)
   {
      err = tcp_output(state.client_pcb);
      if (err == ERR_OK)
      {
         break;
      }
      else
      {
         retries += 1;
         sleep_ms(100);
      }
   }

   if (retries == 20)
   {
      print_error("write_message_buffer - tcp_output", err);
      exit(-1);
   }
}

void AMController::write_message(const char *variable, float x, float y, float z)
{
   char buffer[3 * VARIABLELEN + 3 * VALUELEN + 1];

   snprintf(buffer, VARIABLELEN + VALUELEN + 3, "%s=%.2f:%.2f:%.2f#", variable, x, y, z);

   if (strlen(state.buffer_to_send) + strlen(buffer) > BUF_SIZE - 1)
   {
      DEBUG_printf("Message Discarded\n");
      // printf("\tTx Buffer >%s<\n", state.buffer_to_send);
      // printf("\tBuffer    >%s<\n", buffer);
      return;
   }

   strcat(state.buffer_to_send, buffer);

   size_t len = strlen(state.buffer_to_send);
   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state.buffer_to_send);
      exit(-1);
   }
}

unsigned long AMController::now()
{
   time_t now = time(NULL);
   return now;
}

void AMController::log(int msg)
{
   write_message("$D$", msg);
}
void AMController::log(long msg)
{
   write_message("$D$", msg);
}

void AMController::log(unsigned long msg)
{
   write_message("$D$", msg);
}

void AMController::log(float msg)
{
   write_message("$D$", msg);
}

void AMController::log(const char *msg)
{
   write_message("$D$", msg);
}

void AMController::logLn(int msg)
{
   write_message("$DLN$", msg);
}

void AMController::logLn(long msg)
{
   write_message("$DLN$", msg);
}

void AMController::logLn(unsigned long msg)
{
   write_message("$DLN$", msg);
}

void AMController::logLn(float msg)
{
   write_message("$DLN$", msg);
}

void AMController::logLn(const char *msg)
{
   write_message("$DLN$", msg);
}

void AMController::log_labels(const char *variable, const char *label1)
{
   sd_manager->sd_log_labels(variable, label1, NULL, NULL, NULL, NULL);
}

void AMController::log_labels(const char *variable, const char *label1, const char *label2)
{
   sd_manager->sd_log_labels(variable, label1, label2, NULL, NULL, NULL);
}

void AMController::log_labels(const char *variable, const char *label1, const char *label2, const char *label3)
{
   sd_manager->sd_log_labels(variable, label1, label2, label3, NULL, NULL);
}

void AMController::log_labels(const char *variable, const char *label1, const char *label2, const char *label3, const char *label4)
{
   sd_manager->sd_log_labels(variable, label1, label2, label3, label4, NULL);
}

void AMController::log_labels(const char *variable, const char *label1, const char *label2, const char *label3, const char *label4, const char *label5)
{
   sd_manager->sd_log_labels(variable, label1, label2, label3, label4, label5);
}

void AMController::log_value(const char *variable, unsigned long time, float v1)
{
   sd_manager->log_value(variable, time, v1);
}

void AMController::log_value(const char *variable, unsigned long time, float v1, float v2)
{
   sd_manager->log_value(variable, time, v1, v2);
}

void AMController::log_value(const char *variable, unsigned long time, float v1, float v2, float v3)
{
   sd_manager->log_value(variable, time, v1, v2, v3);
}

void AMController::log_value(const char *variable, unsigned long time, float v1, float v2, float v3, float v4)
{
   sd_manager->log_value(variable, time, v1, v2, v3, v4);
}

void AMController::log_value(const char *variable, unsigned long time, float v1, float v2, float v3, float v4, float v5)
{
   sd_manager->log_value(variable, time, v1, v2, v3, v4, v5);
}

unsigned long AMController::log_size(const char *variable)
{
   return sd_manager->sd_log_size(variable);
}

void AMController::log_purge_data(const char *variable)
{
   sd_manager->sd_purge_data(variable);
}

void AMController::gpio_temporary_put(uint pin, bool value, uint ms)
{
   bool previousValue = gpio_get(pin);

   gpio_put(pin, value);
   sleep_ms(ms);
   gpio_put(pin, previousValue);
}

float AMController::to_voltage(uint16_t adc_value, float vref)
{
   const float conversion_factor = vref / (1 << 12);
   return  adc_value * conversion_factor;
}

////////

bool AMController::tcp_server_open(void *arg, u16_t port)
{
   TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
   DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), port);

   struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
   if (!pcb)
   {
      DEBUG_printf("failed to create pcb\n");
      return false;
   }

   err_t err = tcp_bind(pcb, NULL, port);
   if (err)
   {
      DEBUG_printf("failed to bind to port %u\n", port);
      return false;
   }

   state->server_pcb = tcp_listen_with_backlog(pcb, 1);
   if (!state->server_pcb)
   {
      DEBUG_printf("failed to listen\n");
      if (pcb)
      {
         tcp_close(pcb);
      }
      return false;
   }

   tcp_arg(state->server_pcb, this);
   tcp_accept(state->server_pcb, tcp_server_accept);

   return true;
}

err_t AMController::tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   if (err != ERR_OK || client_pcb == NULL)
   {
      DEBUG_printf("Failure in accept\n");
      // tcp_server_result(arg, err);
      return ERR_VAL;
   }
   DEBUG_printf("Client connected\n");

   state->buffer_to_send[0] = 0;

   state->client_pcb = client_pcb;
   tcp_arg(client_pcb, pico);
   tcp_sent(client_pcb, tcp_server_sent);
   tcp_recv(client_pcb, tcp_server_recv);
   tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
   tcp_err(client_pcb, tcp_server_err);

   state->is_device_connected = true;
   state->is_sync_completed = false;
   if (pico->deviceConnected != NULL)
   {
      pico->deviceConnected();
   }

   return ERR_OK;
}

err_t AMController::tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   // printf("tcp_server_sent %u\n", len);
   //  state->buffer_to_send[0] = 0;
   return ERR_OK;
}

err_t AMController::tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   if (p != NULL)
   {
      // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
      // can use this method to cause an assertion in debug mode, if this method is called when
      // cyw43_arch_lwip_begin IS needed
      cyw43_arch_lwip_check();
      if (p->tot_len > 0)
      {
         // DEBUG_printf("tcp_server_recv1 %d/%d err %d\n", p->tot_len, state->recv_len, err);

         // Receive the buffer
         // const uint16_t buffer_left = BUF_SIZE - state->recv_len;
         // state->recv_len += pbuf_copy_partial(p, state->buffer_recv + state->recv_len, p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
         // state->buffer_recv[state->recv_len] = 0;

         strncpy(state->buffer_recv, (char *)p->payload, MIN(p->tot_len, BUF_SIZE - 1));
         state->buffer_recv[MIN(p->tot_len, BUF_SIZE - 1)] = 0;

         DEBUG_printf("Received %s\n", state->buffer_recv);

         pico->process_received_buffer(state->buffer_recv);

         tcp_recved(tpcb, p->tot_len);
      }
      pbuf_free(p);
   }

   return ERR_OK;
}

err_t AMController::tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;
   // DEBUG_printf("tcp_server_poll_fn\n");

   tcp_server_send_data(arg, state->client_pcb);

   return ERR_OK;
}

err_t AMController::tcp_server_send_data(void *arg, struct tcp_pcb *tpcb)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   size_t len = strlen(state->buffer_to_send);

   if (len == 0)
   {
      // DEBUG_printf("\tNothing to send\n");
      return ERR_OK;
   }

   if (len > BUF_SIZE - 1)
   {
      printf("Termination Character [len %d]\n", len);
      printf("%s\n", state->buffer_to_send);
      exit(-1);
   }

   // if (state->buffer_to_send[len - 1] != '#')
   // {
   //    printf("Termination Character [len %d]\n", len);
   //    printf("%s\n", state->buffer_to_send);
   //    exit(-1);
   // }

   DEBUG_printf("Sending data %s [%d]\n", state->buffer_to_send, len);

   //  this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
   //  can use this method to cause an assertion in debug mode, if this method is called when
   //  cyw43_arch_lwip_begin IS needed
   cyw43_arch_lwip_check();

   if (tcp_sndbuf(tpcb) < len)
   {
      printf("Not enough buffer space to send data.\n");
      return ERR_MEM; // Return memory error
   }

   err_t err = tcp_write(tpcb, state->buffer_to_send, len, TCP_WRITE_FLAG_COPY);
   if (err != ERR_OK)
   {
      DEBUG_printf("Failed to write data %d [%s]\n", err, lwip_strerr(err));
      // return tcp_server_result(arg, err);
      return err;
   }

   int retries = 0;

   while (retries < 20)
   {
      err = tcp_output(tpcb);
      if (err == ERR_OK)
      {
         break;
      }
      else
      {
         retries += 1;
         // printf("tcp_output failed with error: %d\n", err);
         // exit(-1);
      }
   }

   if (retries == 20)
   {
      printf("tcp_output failed with error: %d\n", err);
      exit(-1);
   }

   state->buffer_to_send[0] = 0;

   return ERR_OK;
}

void AMController::tcp_server_err(void *arg, err_t err)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   if (err == ERR_RST)
   {
      DEBUG_printf("Device Disconnected\n", err);
      state->is_device_connected = false;
      state->buffer_to_send[0] = 0;
      if (pico->deviceDisconnected != NULL)
      {
         pico->deviceDisconnected();
      }
      return;
   }
   if (err == ERR_CLSD)
   {
      DEBUG_printf("Device Disconnected\n", err);
      state->is_device_connected = false;
      state->buffer_to_send[0] = 0;
      if (pico->deviceDisconnected != NULL)
      {
         pico->deviceDisconnected();
      }
      return;
   }
   if (err != ERR_ABRT)
   {
      DEBUG_printf("tcp_client_err_fn %d\n", err);
      pico->tcp_server_result(arg, err);
   }
}

err_t AMController::tcp_server_result(void *arg, int status)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   if (status == 0)
   {
      DEBUG_printf("TCP Server success\n");
   }
   else
   {
      DEBUG_printf("TCP Server failed %d [%s]\n", status, lwip_strerr(status));
   }
   return pico->tcp_server_close(arg);
}

err_t AMController::tcp_server_close(void *arg)
{
   AMController *pico = (AMController *)arg;
   TCP_SERVER_T *state = &pico->state;

   err_t err = ERR_OK;
   if (state->client_pcb != NULL)
   {
      tcp_arg(state->client_pcb, NULL);
      tcp_poll(state->client_pcb, NULL, 0);
      tcp_sent(state->client_pcb, NULL);
      tcp_recv(state->client_pcb, NULL);
      tcp_err(state->client_pcb, NULL);
      err = tcp_close(state->client_pcb);
      if (err != ERR_OK)
      {
         DEBUG_printf("close failed %d, calling abort\n", err);
         tcp_abort(state->client_pcb);
         err = ERR_ABRT;
      }
      state->client_pcb = NULL;
   }
   if (state->server_pcb)
   {
      tcp_arg(state->server_pcb, NULL);
      tcp_close(state->server_pcb);
      state->server_pcb = NULL;
   }

   return err;
}

/*****************************************************/
/* Process a buffer containing one or more
/*         variable=value#
/*****************************************************/

void AMController::process_received_buffer(char *buffer)
{
   DEBUG_printf("Buffer >>>%s<<<\n", buffer);

   char *pHash;

   pHash = strtok(buffer, "#");
   while (pHash != NULL)
   {
      DEBUG_printf("\t>%s<\n", pHash);
      char buffer1[strlen(pHash) + 1];
      strcpy(buffer1, pHash);

      char variable[VARIABLELEN];
      char value[VALUELEN];

      char *pEqual = strchr(pHash, '=');
      if (pEqual != NULL)
      {
         strncpy(variable, pHash, MIN(VARIABLELEN, pEqual - pHash));
         variable[pEqual - pHash] = '\0';
         strncpy(value, pEqual + 1, VALUELEN);

         // DEBUG_printf("\t\tvariable %s - value: %s\n", variable, value);

         if (strcmp(value, "Start") > 0 && strcmp(variable, "Sync") == 0)
         {
            // Process sync messages for the variable in value field
            doSync();
            state.is_sync_completed = true;
         }
         else if (
             (strcmp(variable, "$AlarmId$") == 0 || strcmp(variable, "$AlarmT$") == 0 || strcmp(variable, "$AlarmR$") == 0) &&
             strlen(value) > 0)
         {
            alarms.process_alarm_request(variable, value);
         }
         else if ((strcmp(variable, "SD") == 0 || strcmp(variable, "$SDDL$") == 0) && strlen(value) > 0)
         {
            sd_manager->process_sd_request(variable, value);
         }
         else if (strcmp(variable, "$SDLogData$") == 0 && strlen(value) > 0)
         {
            sd_manager->sd_send_log_data(value);
         }
         else if (strcmp(variable, "$SDLogPurge$") == 0 && strlen(value) > 0)
         {
            sd_manager->process_sd_request(variable, value);
         }
         else
         {
            this->processIncomingMessages(variable, value);
         }
      }

      pHash = strtok(NULL, "#");
   }

   DEBUG_printf("processBuffer completed \n");
}

/**
 * Prints the error code in readable form
 */
void AMController::print_error(const char *msg, err_t error)
{
   switch (error)
   {
   case ERR_MEM:
      printf("%s: Out of memory error\n", msg);
      break;

   case ERR_BUF:
      printf("%s: Buffer error\n", msg);
      break;

   case ERR_TIMEOUT:
      printf("%s: Timeout error\n", msg);
      break;

   case ERR_RTE:
      printf("%s: Routing problem\n", msg);
      break;

   case ERR_INPROGRESS:
      printf("%s: Operation in progress\n", msg);
      break;

   case ERR_VAL:
      printf("%s: Illegal value\n", msg);
      break;

   case ERR_WOULDBLOCK:
      printf("%s: Operation would block\n", msg);
      break;

   case ERR_USE:
      printf("%s: Address in use\n", msg);
      break;

   case ERR_ALREADY:
      printf("%s: Already connecting\n", msg);
      break;

   case ERR_ISCONN:
      printf("%s: Conn already established\n", msg);
      break;

   case ERR_CONN:
      printf("%s: Not connected\n", msg);
      break;

   case ERR_IF:
      printf("%s: Low-level netif error\n", msg);
      break;

   case ERR_ABRT:
      printf("%s: Connection aborted\n", msg);
      break;

   case ERR_RST:
      printf("%s: Connection reset\n", msg);
      break;

   case ERR_CLSD:
      printf("%s: Connection closed\n", msg);
      break;

   case ERR_ARG:
      printf("%s: Illegal argument\n", msg);
      break;

   default:
      break;
   }

   fflush(stdout);
}
