// Comment the next line to use ABP authentication on TTN. Leave it as it is to use recommended OTAA
//#define OTAA

#ifndef LORA_TTNMAPPER_TBEAM_CONFIG_INCLUDED
#define LORA_TTNMAPPER_TBEAM_CONFIG_INCLUDED

#ifndef OTAA
// UPDATE WITH YOUR TTN KEYS AND ADDR.
// Setting for ABP
static PROGMEM u1_t NWKSKEY[16] = { 0xEC, 0x35, 0x36, 0x14, 0xA8, 0x67, 0xE0, 0x1F, 0x4B, 0xB6, 0x9A, 0xE1, 0x27, 0x56, 0x4E, 0xC1 }; // LoRaWAN NwkSKey, network session key 
static u1_t PROGMEM APPSKEY[16] = { 0x5E, 0x66, 0xB9, 0xF6, 0x60, 0xBB, 0xF3, 0x22, 0xD1, 0x8F, 0x93, 0xD0, 0xF5, 0xD2, 0x22, 0x2E }; // LoRaWAN AppSKey, application session key 
static const u4_t DEVADDR = 0x26011E12 ; // LoRaWAN end-device address (DevAddr)
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
