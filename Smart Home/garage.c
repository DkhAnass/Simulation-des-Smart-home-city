#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "erbium.h"
#include "dev/button-sensor.h"
#include "powertrace.h"
#include "er-coap-07.h"


#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif
#define GARAGEFERME 1
#define GARAGEOUVERT 2


EVENT_RESOURCE(capteurgarage, METHOD_GET, "sensors/garage", "title=\"garage\";obs");
void capteurgarage_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  const char *aff = "Appuyer sur le boutton pour ouvrir/fermer le garage";
  REST.set_response_payload(response, (uint8_t *)aff, strlen(aff));
}

void capteurgarage_event_handler(resource_t *r)
{
  static uint16_t compteur = 0;
  static char msg[50];
  static uint16_t etat_garage = 1;
  ++compteur;  

  if(compteur%2==0)
  {
    etat_garage=1;
    printf("Garage fermé");
  }
  else{ etat_garage=2; 
        printf("Garage ouvert");
      }
  coap_packet_t notif[1];
  coap_init_message(notif, COAP_TYPE_CON, REST.status.OK, 0 ); 
  coap_set_payload(notif, msg, snprintf(msg, sizeof(msg), "L'état du garage %u",etat_garage));
  REST.notify_subscribers(r, compteur, notif); 
}


PROCESS(capteur_garage_event, "Capteur Garage");
AUTOSTART_PROCESSES(&capteur_garage_event);
PROCESS_THREAD(capteur_garage_event, ev, data)
{
  PROCESS_BEGIN();
  powertrace_start(CLOCK_SECOND*10);
  rest_init_engine();
  rest_activate_event_resource(&resource_capteurgarage);
  SENSORS_ACTIVATE(button_sensor);
  while(1) {
    PROCESS_WAIT_EVENT();
  if (ev == sensors_event && data == &button_sensor) {
      PRINTF("\nAppuyer sur le boutton\n");

      capteurgarage_event_handler(&resource_capteurgarage);
    }
  }
  PROCESS_END();
}
