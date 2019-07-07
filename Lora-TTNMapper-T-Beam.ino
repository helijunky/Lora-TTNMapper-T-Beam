#include <lmic.h>
#include <hal/hal.h>
#include <WiFi.h>
#include <Wire.h>

// UPDATE the config.h file in the same folder WITH YOUR TTN KEYS AND ADDR.
#include "config.h"
#include "gps.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

// choose one of the following hardware
//#define T-BEAM
#define TTGO-V1
//#define HELTEC

#ifdef T-BEAM
 #define BUILTIN_LED 14 // T-Beam
#elif defined TTGO-V1
 #define BUILTIN_LED 2  // TTGO
#elif defined HELTEC
 #define BUILTIN_LED 25 // Heltec
#endif

#define OLED_RESET 4 // not used
Adafruit_SSD1306 display(OLED_RESET);

String LoraStatus;

char s[32]; // used to sprintf for Serial output
uint8_t txBuffer[9];
uint16_t txBuffer2[5];
gps gps;

#ifndef OTAA
// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }
#endif

static osjob_t sendjob;
// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 0;
// Physical Payload = header + application payload = 13 + 9 = 22 bytes
// 1% Duty Cycle: bei 22 Bytes Payload und SF7BW125 Coding 4/5 dürfen max. alle 5.6s ein Paket gesendet werden
// Duty Cycle wird von LMIC automatisch eingehalten
// Fair Use Policy (TTN): max. 30s airtime/24h --> max. 530 Pakete/24h bei SF7
// https://docs.google.com/spreadsheets/d/1voGAtQAjC1qBmaVuP1ApNKs1ekgUjavHuVQIXyYSvNc/edit#gid=0
// https://www.loratools.nl/#/airtime

// For battery mesurement
//const uint8_t vbatPin = 35; // T-Beam
const uint8_t vbatPin = 36; // Heltec/TTGO
float VBAT;  // battery voltage from ESP32 ADC read

#ifdef T-BEAM
 // Pin mapping T-Beam
 const lmic_pinmap lmic_pins = {
   .nss = 18,
   .rxtx = LMIC_UNUSED_PIN,
   .rst = LMIC_UNUSED_PIN, // was "14,"
   .dio = {26, 33, 32},
 };
#elif defined TTGO-V1 || defined HELTEC
 // Pin mapping Heltec/TTGO
 const lmic_pinmap lmic_pins = {
   .nss = 18,
   .rxtx = LMIC_UNUSED_PIN,
   .rst = 14,
   .dio = {26, 33, 32},
 };
#endif

void onEvent (ev_t ev) {
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      LoraStatus = "EV_SCAN_TIMEOUT";
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      LoraStatus = "EV_BEACON_FOUND";
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      LoraStatus = "EV_BEACON_MISSED";
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      LoraStatus = "EV_BEACON_TRACKED";
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      LoraStatus = "EV_JOINING";
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      LoraStatus = "EV_JOINED";
      // Disable link check validation (automatically enabled
      // during join, but not supported by TTN at this time).
      LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      LoraStatus = "EV_RFU1";
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      LoraStatus = "EV_JOIN_FAILED";
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      LoraStatus = "EV_REJOIN_FAILED";
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      LoraStatus = "EV_TXCOMPLETE";
      digitalWrite(BUILTIN_LED, LOW);  
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("Received Ack"));
        LoraStatus = "Received Ack";
      }
      if (LMIC.dataLen) {
        sprintf(s, "Received %i bytes of payload", LMIC.dataLen);
        Serial.println(s);
        sprintf(s, "RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);
        Serial.println(s);
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      LoraStatus = "EV_LOST_TSYNC";
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      LoraStatus = "EV_RESET";
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      LoraStatus = "EV_RXCOMPLETE";
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      LoraStatus = "EV_LINK_DEAD";
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      LoraStatus = "EV_LINK_ALIVE";
      break;
    default:
      Serial.println(F("Unknown event"));
      LoraStatus = "Unknown event";
      break;
  }
}

void do_send(osjob_t* j) {  

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
    LoraStatus = "OP_TXRXPEND, not sending";
  }
  else
  { 
    if (gps.checkGpsFix())
    {
      // Prepare upstream data transmission at the next possible time.
      gps.buildPacket(txBuffer);
      LMIC_setTxData2(1, txBuffer, sizeof(txBuffer), 0);
      sprintf(s, "Packet queued (%d bytes)", sizeof(txBuffer));
      Serial.println(s);
      //Serial.println(F("Packet queued"));
      digitalWrite(BUILTIN_LED, HIGH);
      LoraStatus = "Packet queued";
    }
    else
    {
      //try again in 3 seconds
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(3), do_send);
    }
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("TTN Mapper"));

  #if defined TTGO-V1 || defined HELTEC
   pinMode(16,OUTPUT);     // Heltec/TTGO
   digitalWrite(16, LOW);  // Heltec/TTGO set GPIO16 low to reset OLED
   delay(50);              // Heltec/TTGO
   digitalWrite(16, HIGH); // Heltec/TTGO while OLED is running, must set GPIO16 in high、
  #endif

  #ifdef T-BEAM
   pinMode(vbatPin, INPUT);// Battery mesurement
  #endif

  #ifdef T-BEAM
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C, 0, 21, 22, 800000); // T-Beam
  #elif defined TTGO-V1 || defined HELTEC
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C, 0, 4, 15, 800000); // Heltec/TTGO
  #endif
  
  display.clearDisplay();
  // set text color / Textfarbe setzen
  display.setTextColor(WHITE);
    // set text size / Textgroesse setzen
  display.setTextSize(2);
  // set text cursor position / Textstartposition einstellen
  display.setCursor(1,0);
  // show text / Text anzeigen
  display.println("TTN Mapper");
  display.setCursor(0,16);
  display.println("powered by");
  display.setCursor(0,32);
  display.println("TTGO-LoRa");
  display.setCursor(0,48);
  display.println("T-Beam");
  display.display();
  
  //Turn off WiFi and Bluetooth
  WiFi.mode(WIFI_OFF);
  btStop();
  gps.init();

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  #ifndef OTAA 
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  #endif

  #ifdef CFG_eu868
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  #endif
  
  #ifdef CFG_us915
    LMIC_selectSubBand(1);
  
    //Disable FSB2-8, channels 16-72
    for (int i = 16; i < 73; i++) {
      if (i != 10)
        LMIC_disableChannel(i);
    }
  #endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  //LMIC.rps = setBw(LMIC.rps, BW250);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14); 

  do_send(&sendjob);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  
}

void loop() {
    VBAT = (float)(analogRead(vbatPin)) / 4095*2*3.3*1.1;// Battery Voltage
    
    os_runloop_once();
    if (gps.checkGpsFix())
    { 
    gps.gdisplay(txBuffer2);
    float hdop = txBuffer2[4] / 10.0;
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("SAT: " + String(txBuffer2[0]));
    display.setCursor(97,0);
    display.println(VBAT);
    display.setCursor(122,0);
    display.println("V");
    display.setCursor(128,0);
    display.println("Speed: " + String(txBuffer2[1]));
    display.setCursor(0,20);
    display.println("Course: " + String(txBuffer2[2]));
    display.setCursor(0,30);
    display.println("Alt: " + String(txBuffer2[3]));
    display.setCursor(0,40);
    display.println("HDOP: ");
    display.setCursor(35,40);
    display.println(hdop,1);
    display.setCursor(0,50);
    display.println("LoRa: ");
    display.setCursor(35,50);
    display.println(LoraStatus);
    
    display.display();
    }
    else
    {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("No valid");
    display.setCursor(0,16);
    display.println("GPS fix");
    display.setTextSize(1);
    display.setCursor(97,0);
    display.println(VBAT);
    display.setCursor(122,0);
    display.println("V");
    display.display();
    }
}
