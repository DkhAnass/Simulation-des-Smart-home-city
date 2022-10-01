#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "erbium.h"
#include "powertrace.h"
#include "dev/button-sensor.h"
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
#define climatiseurOFF 1
#define climatiseurON 2

#include "dev/sht11-sensor.h"

EVENT_RESOURCE(capteurclimatiseur, METHOD_GET, "sensors/climatisateur", "title=\"climatisateur\";obs");

void capteurclimatiseur_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  const char *aff = "Appuyer sur le boutter pour activer/désactiver le climatisateur";
  REST.set_response_payload(response, (uint8_t *)aff, strlen(aff));
}
void capteurclimatiseur_event_handler(resource_t *r)
{
  static uint16_t climatiseur_etat = 2;
  static uint16_t compteur = 0;
  static char msg[50]; 
  ++compteur;
  if(compteur%2==0)
  {
    climatiseur_etat=1;
    printf("Climatiseur OFF");
  }
  else{ climatiseur_etat=2;
        printf("Climatiseur ON");
      }
  coap_packet_t notif[1];
  coap_init_message(notif, COAP_TYPE_CON, REST.status.OK, 0 );
  coap_set_payload(notif, msg, snprintf(msg, sizeof(msg), "L'état du climatiseur %u",climatiseur_etat));
  REST.notify_subscribers(r, compteur, notif);
}

PERIODIC_RESOURCE(temperature, METHOD_GET, "sensors/Température", "title=\"Température\";obs", 300*CLOCK_SECOND);
void temperature_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  const char *msg = "Mesure de la temperature chaque 5 minutes ";
  REST.set_response_payload(response, msg, strlen(msg));  
}
void temperature_periodic_handler(resource_t *r)
{
  
  static char msg[50];
  uint16_t data_temperature = ((sht11_sensor.value(SHT11_SENSOR_TEMP)/10)-396);
  data_temperature = data_temperature/10;
  PRINTF("%u\n",data_temperature);
  coap_packet_t notif[1];
  coap_init_message(notif, COAP_TYPE_NON, REST.status.OK, 0 );
  coap_set_payload(notif, msg, snprintf(msg, sizeof(msg), "%u",data_temperature));
  REST.notify_subscribers(r, data_temperature, notif);
}

PROCESS(climatisateur_event, "Climatisateur & Capteur Température");
AUTOSTART_PROCESSES(&climatisateur_event);

PROCESS_THREAD(climatisateur_event, ev, data)
{
  PROCESS_BEGIN();
  powertrace_start(CLOCK_SECOND*10);
  rest_init_engine();
  SENSORS_ACTIVATE(button_sensor);
  SENSORS_ACTIVATE(sht11_sensor);
  rest_activate_periodic_resource(&periodic_resource_temperature);
  rest_activate_event_resource(&resource_capteurclimatiseur);
  while(1) {
    PROCESS_WAIT_EVENT();
  if (ev == sensors_event && data == &button_sensor) {
      PRINTF("\nAppuyer sur le boutton\n");
      capteurclimatiseur_event_handler(&resource_capteurclimatiseur);
    }
  }
  PROCESS_END();
}
