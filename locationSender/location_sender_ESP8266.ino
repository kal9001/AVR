#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti wifiMulti;

void getData();

HTTPClient data;

void setup() {
  Serial.begin(250000);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("SKY46717", "YVBSLCLTLT");

  // Wait for the Wi-Fi to connect
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
  }  
  data.begin("http://192.168.0.43:1880/location_sender/?id=1");    
}

void loop() {
  if((wifiMulti.run() == WL_CONNECTED)) {
    delay(100);
    int dataResponseCode = data.GET();
    if(dataResponseCode > 0) {
      if(dataResponseCode == HTTP_CODE_OK) {
        String httpData = data.getString();
        char outputData[22];        
        int j = 0;
        for(int i = 0; i < 26; i++) {
          if(httpData[i] != ',') {
            outputData[j] = httpData[i];
            j++;  
          }
        }
        outputData[21] = '\0';
        Serial.print('?');
        Serial.print(outputData);
        Serial.print('\r');
      }
    }
  }
}
