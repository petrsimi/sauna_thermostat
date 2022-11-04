#include  "WifiWrap.h"

#include <ESP_AT_Lib.h>


// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY50  0x7BEF


//<meta http-equiv='refresh' content='10'>\


static const char httpContent[] = "<html>\
<head>\
<meta http-equiv='refresh' content='10'>\
<title>Sauna</title>\
<style>button {font-size:40pt;width:90pt;height:90pt;margin:10pt;}</style>\
</head>\
<body style='text-align:center'>\
<p style='font-size:160pt;margin:30pt;color:%s'><b>%u.%u</b> &#176;C</p>\
<form action='.' method='post'>\
<button type='submit' name='btnMinus'>-</button>\
<span style='font-size:50pt;display:inline-block;width:200pt;'>%u &#176;C</span>\
<button type='submit' name='btnPlus'>+</button><button type='submit' name='btnOnOff'>%s</button>\
</form>\
</body>\
</html>";


WifiWrap::WifiWrap(Stream* stream)
{
    wifi = new ESP8266(stream);
}


bool WifiWrap::isAlive()
{
    available = wifi->kick();
    return available;
}


bool WifiWrap::startHttpSrv()
{
    // Is the WiFi modul connected?
    if (!available) {
        return false;
    }

    if (!wifi->setOprToStation()) {
        Serial.println(F("ERROR: Failed to set WiFi Station mode"));
        return false;
    }

    if (!wifi->enableMUX()) {
        Serial.println(F("ERROR: enableMUX failed"));
        return false;
    }
/*
    if (!wifi->joinAP("simi", "KockaLezeDirouPesOknem")) {
        Serial.println("Failed to connect to the AP.");
        return false;
    }
*/
    if (!wifi->setAutoConnect(true)) {
        Serial.println(F("ERROR: setAutoConnect failed"));
        return false;      
    }
/*
    Serial.println(F("Network settings:"));
    String ip = wifi->getStationIp();
    Serial.println(ip.c_str());
*/
    if (!wifi->startTCPServer(80)) {
        Serial.println(F("ERROR: Failed to start TCP server"));
        return false;
    }

    if (!wifi->setTCPServerTimeout(10)) {
        Serial.println(F("ERROR: Failed to set TCP server timeout"));
        return false;
    }


/*
  while (1) {
    if (Serial.available()) {
      Serial1.write(Serial.read());
    }  
    if (Serial1.available()) {
      Serial.write(Serial1.read());    
    }
  }
*/
    return true;
}





bool WifiWrap::getWifiStatus(String& status)
{
    if (!available) return false;

    status = F("WiFi status: ");

    int start, end;
    String ssid = wifi->getNowConecAp();
    start = ssid.indexOf("+CWJAP:");
    if (start < 0) {
        status += F("not connected");
        return false;
    }          
    start += 8;
    end = ssid.indexOf('"', start);
    if (end < 0) {
        status += F("not connected");
        return false;
    }
    // display SSID
    status += ssid.substring(start, end);
    // display rssi
    status += " (";
    start = ssid.lastIndexOf(',') + 1;
    end = ssid.indexOf('\r', start);
    status += ssid.substring(start, end);
    status += "dBm) ";

    String ip = wifi->getStationIp();
    start = ip.indexOf("ip:");
    if (start >= 0) {
        start += 4;
        end = ip.indexOf('"', start);
        if (end >= 0) {
            status += ip.substring(start, end);
        }
    }            

    return true;
}


bool WifiWrap::handleHttpReq(uint8_t& target, uint16_t& temp, state_t& state)
{
    char buffer[1024] = {0};
    uint8_t mux_id;

    // Any HTTP POST/GET request?
    uint32_t len = wifi->recv(&mux_id, buffer, sizeof(buffer) - 1, 100);
    if (len > 0) {

    Serial.println(wifi->getIPStatus().c_str());

    // Reqeust to Power ON/OFF ?
    if (memcmp(buffer, "POST", 4) == 0) {
      if (strstr(buffer, "btnPlus=") != NULL) {
        Serial.println(F("Plus button was pressed"));
        target++;
      } else if (strstr(buffer, "btnMinus=") != NULL) {
        Serial.println(F("Minus button was pressed"));
        target--;
      } else if (strstr(buffer, "btnOnOff") != NULL) {
        Serial.println(F("ON/OFF button was pressed"));
        if (state == OFF) {
          state = ON;          
        } else {
          state = OFF;
        }
      } else {
        Serial.println(F("Unknown POST message"));
      }
      //Serial.println((char*)buffer);
    }

    // convert raw temperature to fixed point Celsius
    uint8_t div = temp / 128;
    uint8_t mod = (temp % 128) * 10 / 128; // we care only about one digit


    // Send the HTTP web page
    int size = snprintf(buffer, 1024, httpContent,
        (state == HEATING) ? "red" : (state == OFF ? "black" : "limegreen"),
        div,
        mod,
        target,
        state == OFF ? "ON" : "OFF"
    );
        
    if (!wifi->send(mux_id, buffer, size)) {
        Serial.println(F("ERROR: Failed to send HTTP content"));
    }

    if (!wifi->releaseTCP(mux_id)) {
        Serial.print(F("ERROR: Failed to release TCP "));
        Serial.println(mux_id);
    }
  }
}

