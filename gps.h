#ifndef __GPS_H__
#define __GPS_H__

#include <TinyGPS++.h>
#include <HardwareSerial.h>

//#define GPS_TX 12 // T-Beam
//#define GPS_RX 15 // T-Beam
#define GPS_TX 35 // Heltec/TTGO
#define GPS_RX 12 // Heltec/TTGO


class gps
{
    public:
        void init();
        bool checkGpsFix();
        void buildPacket(uint8_t txBuffer[9]);
        void gdisplay(uint16_t txBuffer2[5]);
        void encode();

    private:
        uint32_t LatitudeBinary, LongitudeBinary;
        uint16_t altitudeGps;
        uint8_t hdopGps;
        char t[32]; // used to sprintf for Serial output
        TinyGPSPlus tGps;
};

#endif
