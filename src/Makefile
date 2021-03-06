all: $(CONTIKI_PROJECT)

include Makefile-conf.include

include components/application/Makefile
include components/neighbordiscovery/Makefile
include components/network/Makefile
include components/powercontrol/Makefile
include components/radio/Makefile
include components/topologycontrol/Makefile
include lib/Makefile
include tests/Makefile

ifeq ($(NETWORK_IPV6),1)
	CONTIKI_WITH_IPV6 = 1
	CONTIKI_WITH_RPL = 1
	PROJECT_SOURCEFILES += rimestats.c # TODO cc2420 has a hard dependency on rimestats but it's not build with rime, check later if it's fixed yet in contiki master
else
	CONTIKI_WITH_RIME = 1
endif

ifndef CONTIKI
    $(error CONTIKI is undefined)
endif
ifndef CONTIKI_REPLACEMENTS_LOCAL
    $(error CONTIKI_REPLACEMENTS_LOCAL is undefined)
endif

TARGET_LIBFILES += -lm
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DCONTIKI_REPLACEMENTS_LOCAL=$(CONTIKI_REPLACEMENTS_LOCAL)
CFLAGS += -ggdb
# Turn off warnings about unused code because the generated code contains numerous unused variables and functions.
# The corresponding warning messages could obfuscate relevant warning messages.
CFLAGS += -Wno-unused-variable -Wno-unused-function

# The following block adds a preprocessor #define for COMPONENT_TOPOLOGYCONTROL if the environment variable
# TOPOLOGYCONTROL_PREDEFINED_ALGORITHM is set.
# This can be used to pre-configure the active topology control module from outside
ifdef TOPOLOGYCONTROL_PREDEFINED_ALGORITHM
    CFLAGS += -DCOMPONENT_TOPOLOGYCONTROL=$(TOPOLOGYCONTROL_PREDEFINED_ALGORITHM)
endif

#
# The following block adds a preprocessor define for COMPONENT_TOPOLOGYCONTROL if the environment variable TOPOLOGYCONTROL_PREDEFINED_ALGORITHM is set from outside
#
ifdef TOPOLOGYCONTROL_PREDEFINED_ALGORITHM
  CFLAGS += -DCOMPONENT_TOPOLOGYCONTROL=$(TOPOLOGYCONTROL_PREDEFINED_ALGORITHM)
endif

ifndef UNUSED
	UNUSED := ${shell rm -f app.$(TARGET)} # always recompile app.c that git version string is up2date
	UNUSED := ${shell rm -f obj_$(TARGET)/utilities.*} # always recompile utilities.c that random seed is updated every time
endif

ifndef APPRELSTR
	APPRELSTR := ${shell git --git-dir ../.git describe --tags --always}
endif
ifneq ($(APPRELSTR),)
	CFLAGS += -DAPP_GIT_VERSION=\"$(APPRELSTR)\"
endif

include $(CONTIKI)/Makefile.include
