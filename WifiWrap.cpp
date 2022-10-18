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



static const char httpHead1[] = "<html><head><meta http-equiv='refresh' content='10'><title>Sauna</title><style>button {font-size:40pt;width:90pt;height:90pt;margin:10pt;}</style></head><body style='text-align:center'><p style='font-size:160pt;margin:30pt;color:";
// red/green/black
static const char httpRed[] = "red";
static const char httpGreen[] = "limegreen";
static const char httpBlack[] = "black";

static const char httpHead2[] = "'><b>";

static const char httpTail1[] = "</b> &#176;C</p><form action='.' method='post'><button type='submit' name='btnMinus'>-</button><span style='font-size:50pt;display:inline-block;width:200pt;'>";


static const char httpTail2[] = " &#176;C</span><button type='submit' name='btnPlus'>+</button><button type='submit' name='btnOnOff'>";

static const char httpOff[] = "OFF";
static const char httpOn[] = "ON";

static const char httpTail3[] = "</button></form></body></html>";





WifiWrap::WifiWrap(Stream* stream)
{
    wifi = new ESP8266(stream);
}


bool WifiWrap::isAlive()
{
    return wifi->kick();
}


bool WifiWrap::startHttpSrv()
{
    Serial.println(wifi->getVersion().c_str());

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





bool WifiWrap::displayWifiStatus(Adafruit_TFTLCD& lcd)
{
    lcd.fillRect(10, 135, 300, 7, BLACK);
    lcd.setCursor(10, 135);
    lcd.setTextSize(1);
    lcd.setTextColor(BLUE);
    lcd.print("WiFi: ");

    int start, end;
    String ssid = wifi->getNowConecAp();
    start = ssid.indexOf("+CWJAP:");
    if (start < 0) {
        goto err;
    }          
    start += 8;
    end = ssid.indexOf('"', start);
    if (end < 0) {
        goto err;
    }
    // display SSID
    lcd.print(ssid.substring(start, end));
    // display rssi
    lcd.print(" (");
    start = ssid.lastIndexOf(',') + 1;
    end = ssid.indexOf('\r', start);
    lcd.print(ssid.substring(start, end));
    lcd.print("dBm) ");

    String ip = wifi->getStationIp();
    start = ip.indexOf("ip:");
    if (start >= 0) {
        start += 4;
        end = ip.indexOf('"', start);
        if (end >= 0) {
            lcd.print(ip.substring(start, end));
        }
    }            

    return true; // to repeat the action - false to stop

err:
    lcd.print("not connected");
    return false;
}


bool WifiWrap::handleHttpReq(uint8_t& target, uint16_t& temp, state_t& state)
{
  uint8_t buffer[1024] = {0};
  uint8_t mux_id;

  // Any HTTP POST/GET request?
  uint32_t len = wifi->recv(&mux_id, buffer, sizeof(buffer), 100);
  if (len > 0) {

    Serial.println(wifi->getIPStatus().c_str());

    // Reqeust to Power ON/OFF ?
    if (memcmp(buffer, "POST", 4) == 0) {
      if (strstr((char*)buffer, "btnPlus=") != NULL) {
        Serial.println(F("Plus button was pressed"));
        target++;
      } else if (strstr((char*)buffer, "btnMinus=") != NULL) {
        Serial.println(F("Minus button was pressed"));
        target--;
      } else if (strstr((char*)buffer, "btnOnOff") != NULL) {
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

    // Display the HTTP web page

    if (!wifi->send(mux_id, httpHead1, sizeof(httpHead1)-1)) {
      Serial.println(F("ERROR: Failed to send HTTP header"));
    }

    switch (state) {
        case HEATING:
            if (!wifi->send(mux_id, httpRed, sizeof(httpRed)-1)) {
              Serial.println(F("ERROR: Failed to send HTTP header"));
            }
            break;
        case ON:
        case WAITING:
            if (!wifi->send(mux_id, httpGreen, sizeof(httpGreen)-1)) {
              Serial.println(F("ERROR: Failed to send HTTP header"));
            }
            break;
        default:
            if (!wifi->send(mux_id, httpBlack, sizeof(httpBlack)-1)) {
              Serial.println(F("ERROR: Failed to send HTTP header"));
            }
            break;
    }


    if (!wifi->send(mux_id, httpHead2, sizeof(httpHead2)-1)) {
      Serial.println(F("ERROR: Failed to send HTTP header"));
    }

    // convert raw temperature to fixed point Celsius
    uint8_t div = temp / 128;
    uint8_t mod = (temp % 128) * 10 / 128; // we care only about one digit
 
    // convert to string
    char buff[10];
    snprintf(buff, 10, "%u.%u", div, mod);
    String number = String(buff);

    if (!wifi->send(mux_id, number.c_str(), number.length())) {
      Serial.println(F("ERROR: Failed to send HTTP body"));
    }

    if (!wifi->send(mux_id, httpTail1, sizeof(httpTail1)-1)) {
      Serial.println(F("ERROR: Failed to send HTTP tail"));
    }

    // Requested temperature
    number = String(target);

    if (!wifi->send(mux_id, number.c_str(), number.length())) {
      Serial.println(F("ERROR: Failed to send HTTP body"));
    }

    if (!wifi->send(mux_id, httpTail2, sizeof(httpTail2)-1)) {
      Serial.println(F("ERROR: Failed to send HTTP tail"));
    }

    if (state == OFF) {
        if (!wifi->send(mux_id, httpOn, sizeof(httpOn)-1)) {
          Serial.println(F("ERROR: Failed to send HTTP tail"));
        }
    } else {
        if (!wifi->send(mux_id, httpOff, sizeof(httpOff)-1)) {
          Serial.println(F("ERROR: Failed to send HTTP tail"));
        }
    }

    if (!wifi->send(mux_id, httpTail3, sizeof(httpTail3)-1)) {
      Serial.println(F("ERROR: Failed to send HTTP tail"));
    }

    if (!wifi->releaseTCP(mux_id)) {
      Serial.print(F("ERROR: Failed to release TCP "));
      Serial.println(mux_id);
    }
  }
}

