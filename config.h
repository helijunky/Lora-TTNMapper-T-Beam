// Comment the next line to use ABP authentication on TTN. Leave it as it is to use recommended OTAA
//#define OTAA

#ifndef LORA_TTNMAPPER_TBEAM_CONFIG_INCLUDED
#define LORA_TTNMAPPER_TBEAM_CONFIG_INCLUDED

#ifndef OTAA
// UPDATE WITH YOUR TTN KEYS AND ADDR.
// Setting for ABP
static PROGMEM u1_t NWKSKEY[16] = { 0xBF, 0xAE, 0x07, 0x15, 0xA8, 0xA7, 0x1A, 0x50, 0xFC, 0x68, 0x41, 0x1F, 0x08, 0x4D, 0x0A, 0xA6 }; // LoRaWAN NwkSKey, network session key 
static u1_t PROGMEM APPSKEY[16] = { 0x6C, 0xFD, 0x25, 0xBB, 0x8E, 0xD5, 0x06, 0xE1, 0x6E, 0x57, 0xFE, 0x73, 0x89, 0xE7, 0xB0, 0x3B }; // LoRaWAN AppSKey, application session key 
static const u4_t DEVADDR = 0x26011C57 ; // LoRaWAN end-device address (DevAddr)
#else
// Settings from OTAA device
static const u1_t PROGMEM DEVEUI[8]={ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 } ; // Device EUI, hex, lsb
static const u1_t PROGMEM APPEUI[8]={ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }; // Application EUI, hex, lsb
static const u1_t PROGMEM APPKEY[16] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }; // App Key, hex, msb

void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}
#endif

#endif //LORA_TTNMAPPER_TBEAM_CONFIG_INCLUDED
