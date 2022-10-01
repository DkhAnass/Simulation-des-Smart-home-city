#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include "sys/clock.h"
#include "net/rime.h"
#include "powertrace.h"
#include <stdio.h>
#include "string.h"
static struct etimer t;
static uint8_t compteur;

PROCESS(feu_de_circulation, "Feu de circulation");
AUTOSTART_PROCESSES(&feu_de_circulation);

static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  
}
static struct broadcast_conn broadcast;
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

PROCESS_THREAD(feu_de_circulation, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
  PROCESS_BEGIN();
  powertrace_start(CLOCK_SECOND*10);
  broadcast_open(&broadcast, 129, &broadcast_call);
  compteur = 0;
  while(1) {
    etimer_set(&t, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    leds_off(LEDS_ALL);
    leds_on(LEDS_GREEN);
    if(compteur <= 20) {
      leds_on(LEDS_GREEN);
    } else if (compteur <= 40) {
      leds_off(LEDS_ALL);
      leds_on(LEDS_RED);
    }
    compteur++;
    if (compteur >= 40)
      compteur = 0;
  }

  PROCESS_END();
}
