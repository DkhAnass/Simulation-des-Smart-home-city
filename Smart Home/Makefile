all: er-example-server er-example-client

WITH_COAP=13

# variable for Makefile.include
WITH_UIP6=1
# for some platforms
UIP_CONF_IPV6=1
# IPv6 make config disappeared completely
CFLAGS += -DUIP_CONF_IPV6=1

CONTIKI=../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# variable for Makefile.include
ifneq ($(TARGET), minimal-net)
CFLAGS += -DUIP_CONF_IPV6_RPL=1
else
# minimal-net does not support RPL under Linux and is mostly used to test CoAP only
${info INFO: compiling without RPL}
CFLAGS += -DUIP_CONF_IPV6_RPL=0
CFLAGS += -DHARD_CODED_ADDRESS=\"fdfd::10\"
${info INFO: compiling with large buffers}
CFLAGS += -DUIP_CONF_BUFFER_SIZE=2048
CFLAGS += -DREST_MAX_CHUNK_SIZE=1024
CFLAGS += -DCOAP_MAX_HEADER_SIZE=640
endif

# linker optimizations
SMALL=1

${info INFO: compiling with CoAP-08}
CFLAGS += -DWITH_COAP=7
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-07


APPS += erbium
APPS += powertrace

include $(CONTIKI)/Makefile.include


$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64

connect-minimal:
	sudo ip address add fdfd::1/64 dev tap0
