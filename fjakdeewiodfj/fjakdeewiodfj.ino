/*
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX


//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 2
const char* ssid = "I-GEOSCAN123";
const char* password = "zzzzzzzz";
//
WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  delay(2000);
  Serial.begin(9600);
  WiFi.mode(WIFI_AP);
  //  Serial.print("wifi status = ");
  //  Serial.println(WiFi.status());
  WiFi.softAP("think2", "");
  //  WiFi.begin(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  pinMode(D5, OUTPUT);

  /* station mode needed
    Serial.print("\nConnecting to "); Serial.println(ssid);
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
    if(i == 21){
    Serial.print("Could not connect to"); Serial.println(ssid);
    while(1) delay(500);
    }
  */
  //start UART and the server
  //  Serial.begin(9600);
  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.print(myIP);
  Serial.println(" 23' to connect");

}

void loop() {

  uint8_t i;
  //check if there are any new clients 새로운 클라이언트가 들어올 때
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot 클라이언트가 연결이 안되었을 때
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data 클라이언트가 보낸 데이터를 확인할 때
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        //get data from the telnet client and push it to the UART

        while (serverClients[i].available()) {
          char receive_data = serverClients[i].read();
          Serial.write(receive_data);
          if (receive_data == '1') {
            digitalWrite(D5, HIGH);
          }
          if (receive_data == '2') {
            digitalWrite(D5, LOW);

          }

        }

      }
    }
  }
  //check UART for data 시리얼 통신(=UART) 데이터 확인하기
  if (Serial.available()) {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    Serial.println(sbuf[0]);

    //push UART data to all connected telnet clients 시리얼 통신(=UART) 데이터를 모든 연결된 클라이언트 한테 보내기
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].write(sbuf, len);
        delay(1);


      }
    }
  }
}
