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
#define cameraOFF 1
#define cameraON 2


#include "dev/radio-sensor.h"
static uint16_t camera_etat = 2;

EVENT_RESOURCE(capteurcamera, METHOD_GET, "sensors/camera", "title=\"Camera\";obs");

void capteurcamera_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  const char *aff = "Appuyer sur le boutton pour activer/désactiver la camera ";
  REST.set_response_payload(response, (uint8_t *)aff, strlen(aff));
}
void capteurcamera_event_handler(resource_t *r)
{
  static uint16_t compteur = 0;
  static char msg[50];
  
  ++compteur;
  if(compteur%2==0)
  {
    camera_etat=1;
    printf("Camera OFF");
  }
  else{ camera_etat=2;
        printf("Camera ON");
      }
  coap_packet_t notif[1];
  coap_init_message(notif, COAP_TYPE_CON, REST.status.OK, 0 );
  coap_set_payload(notif, msg, snprintf(msg, sizeof(msg), "L'état du camera %u",camera_etat));
  REST.notify_subscribers(r, compteur, notif);
}



PERIODIC_RESOURCE(radio, METHOD_GET, "sensors/cameravalue", "title=\"Radio\";obs", 300*CLOCK_SECOND);
void radio_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  const char *msg = "Transfert du radio data chaque 5 minutes";
  REST.set_response_payload(response, msg, strlen(msg));  
}
void radio_periodic_handler(resource_t *r)
{
  static char msg[50];
  uint16_t radio_data = radio_sensor.value(RADIO_SENSOR_LAST_VALUE);
  if(camera_etat == 2){
  coap_packet_t notif[1];
  coap_init_message(notif, COAP_TYPE_NON, REST.status.OK, 0 );
  coap_set_payload(notif, msg, snprintf(msg, sizeof(msg), "%u",radio_data));
  REST.notify_subscribers(r, radio_data, notif);}
}


PROCESS(camera_event, "Camera");
AUTOSTART_PROCESSES(&camera_event);

PROCESS_THREAD(camera_event, ev, data)
{
  PROCESS_BEGIN();
  powertrace_start(CLOCK_SECOND*10);
  SENSORS_ACTIVATE(button_sensor);
  rest_init_engine();
  SENSORS_ACTIVATE(radio_sensor);
  rest_activate_periodic_resource(&periodic_resource_radio);
  rest_activate_event_resource(&resource_capteurcamera);
  while(1) {
    PROCESS_WAIT_EVENT();
 if (ev == sensors_event && data == &button_sensor) {
      PRINTF("\nAppuyer sur le boutton\n");
      capteurcamera_event_handler(&resource_capteurcamera);
    }
  }

  PROCESS_END();
}
