#pragma once

#include <Arduino.h>
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include "sauna.h"

class ESP8266;

class WifiWrap {
    public:
        WifiWrap(Stream* stream);

        bool isAlive();

        bool startHttpSrv();

        bool displayWifiStatus(Adafruit_TFTLCD& lcd);

        bool handleHttpReq(uint8_t& target, uint16_t& temp, state_t& state);


    private:
        ESP8266* wifi;

        bool available;

};
