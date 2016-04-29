ifeq ($(CONTIKI_REPLACEMENTS_LOCAL),1)
PROJECTDIRS += lib/contiki-replacements
PROJECT_SOURCEFILES += csma.c packetbuf.c 

ifeq ($(NETWORK_IPV6),0)
PROJECT_SOURCEFILES += route.c multihop.c mesh.c
endif

endif