#ifndef __APP_CONF_CONSTANTS_H_
#define __APP_CONF_CONSTANTS_H_

#define COMPONENT_APPLICATION_NULL 1
#define COMPONENT_APPLICATION_DATACOLLECTION 2
#define COMPONENT_APPLICATION_MESHMESSAGES 3
#define COMPONENT_APPLICATION_RANDOMWALK 4
#define COMPONENT_APPLICATION_POWERCALIBRATION 5

#define COMPONENT_NEIGHBORDISCOVERY_NULL 1
#define COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST 2

#define COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_RSSI 1
#define COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_DISTANCE 2

#define COMPONENT_NETWORK_MAKEFILE 1
#define COMPONENT_NETWORK_IPV6 2
#define COMPONENT_NETWORK_RIME 3

#define COMPONENT_POWERCONTROL_NULL 1
#define COMPONENT_POWERCONTROL_BINARYSEARCH 2
#define COMPONENT_POWERCONTROL_CALIBRATED 3
#define COMPONENT_POWERCONTROL_ADAPTIVE 4

#define COMPONENT_RADIO_AUTODETECT 1
#define COMPONENT_RADIO_CC2420 2
#define COMPONENT_RADIO_CC2520 3
#define COMPONENT_RADIO_COOJA 4

#define COMPONENT_TOPOLOGYCONTROL_NULL 1
#define COMPONENT_TOPOLOGYCONTROL_AKTC 2
#define COMPONENT_TOPOLOGYCONTROL_LKTC 3 
#define COMPONENT_TOPOLOGYCONTROL_LMST 4

#define TOPOLOGYCONTROL_LINKS_HAVE_STATES
#define COMPONENT_TOPOLOGYCONTROL_CMOFLONDEMOLANGUAGE_LSTARKTCALGORITHM 17427
#define COMPONENT_TOPOLOGYCONTROL_IMPL_FILE_CMOFLONDEMOLANGUAGE_LSTARKTCALGORITHM topologycontrol-CMoflonDemoLanguage-LStarKtcAlgorithm.c

#define COMPONENT_TOPOLOGYCONTROL_CMOFLONDEMOLANGUAGE_LMSTALGORITHM 14253
#define COMPONENT_TOPOLOGYCONTROL_IMPL_FILE_CMOFLONDEMOLANGUAGE_LMSTALGORITHM topologycontrol-CMoflonDemoLanguage-LmstAlgorithm.c

#define COMPONENT_TOPOLOGYCONTROL_CMOFLONDEMOLANGUAGE_KTCALGORITHM 2085
#define COMPONENT_TOPOLOGYCONTROL_IMPL_FILE_CMOFLONDEMOLANGUAGE_KTCALGORITHM topologycontrol-CMoflonDemoLanguage-KtcAlgorithm.c

#define COMPONENT_TOPOLOGYCONTROL_CMOFLONDEMOLANGUAGE_THRESHOLDINACTIVATIONALGORITHM 1353
#define COMPONENT_TOPOLOGYCONTROL_IMPL_FILE_CMOFLONDEMOLANGUAGE_THRESHOLDINACTIVATIONALGORITHM topologycontrol-CMoflonDemoLanguage-ThresholdInactivationAlgorithm.c

#define UTILITIES_CONTIKIRANDOM_SEED_STATIC 1
#define UTILITIES_CONTIKIRANDOM_SEED_RANDOM 2

#endif /* __APP_CONF_CONSTANTS_H_ */
