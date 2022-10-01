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
#define PORTEFERMEE 1
#define PORTEOUVERTE 2


EVENT_RESOURCE(capteurporte, METHOD_GET, "sensors/porte", "title=\"porte\";obs");

// Cette fonction handler 1 répond à une request GET que l'utilisateur offre avec le output texte "La porte est fermé : état 0 "
void capteurporte_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  const char *aff = "Appuyer sur le boutton pour ouvrir/fermer la porte";
  REST.set_response_payload(response, (uint8_t *)aff, strlen(aff));
}

void capteurporte_event_handler(resource_t *r)
{
  static uint16_t compteur = 0;
  static char msg[50];
  static uint16_t etat_porte = 1;
  ++compteur;  

  if(compteur%2==0)
  {
    etat_porte=1;
    printf("Porte fermée");
  }
  else{ etat_porte=2; 
        printf("Porte Ouverte");
      }
  /* Etablir la notification qu'on on va envoyer a tous les abonnés de cet evénement (dans ce cas l'utilisateur)*/
  coap_packet_t notif[1]; /* Pour que notre packet peut etre traité comme un pointeur qui pointe sur les var du struct coap_packet_t */
  coap_init_message(notif, COAP_TYPE_CON, REST.status.OK, 0 ); 
  coap_set_payload(notif, msg, snprintf(msg, sizeof(msg), "L'état de la porte %u",etat_porte));
  // Notifie l'utilisateur avec le message du notification.
  REST.notify_subscribers(r, compteur, notif); 
}


PROCESS(capteur_porte_event, "Capteur Porte");
AUTOSTART_PROCESSES(&capteur_porte_event);
PROCESS_THREAD(capteur_porte_event, ev, data)
{
  PROCESS_BEGIN();
  powertrace_start(CLOCK_SECOND*10);
  rest_init_engine(); // Aprés la déclaration du EVENT_RESOURCE il faut initializer le REST framework par cette commande
  rest_activate_event_resource(&resource_capteurporte);// Pour que le resource déclarer soit accessible
  SENSORS_ACTIVATE(button_sensor); //Activer les capteurs et les dispositifs
  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  if (ev == sensors_event && data == &button_sensor) {
      PRINTF("\nAppuyer sur le boutton\n");
      // L'appel la fonction handle 2 (event_handler) lors de l'appui sur le boutton
      capteurporte_event_handler(&resource_capteurporte);
    }
  }
  PROCESS_END();
}
