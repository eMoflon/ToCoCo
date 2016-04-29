ifeq ($(CONTIKI_REPLACEMENTS_LOCAL),1)
PROJECTDIRS += lib/contiki-replacements
PROJECT_SOURCEFILES += csma.c packetbuf.c

ifeq ($(NETWORK_IPV6),1)
	PROJECT_SOURCEFILES += sicslowpan.c
else
	PROJECT_SOURCEFILES += mesh.c route.c
endif

endif