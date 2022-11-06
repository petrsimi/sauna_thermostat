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

        bool getWifiStatus(String& status);

        bool handleHttpReq(uint8_t& target, uint16_t& temp, state_t& state);

        bool joinAp(String ssid, String pwd);

    private:
        ESP8266* wifi;

        bool available;

};
