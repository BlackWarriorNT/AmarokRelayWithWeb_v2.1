//Полный список команд языка Ардуино: https://alexgyver.ru/lessons/arduino-reference/
//Функции: https://alexgyver.ru/lessons/functions/

const String firmwareVer = "Firmware version: 2.1";
const String firmwareBuild = "Firmware builded: " + String(__DATE__) + ", " + String(__TIME__);

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h> //http://wikihandbk.com/wiki/ESP8266:Примеры/Веб-сервер_с_помощью_ESP8266_и_IDE_Arduino
#include <ESP8266mDNS.h>

#define led0 //использовать встроенный светодиод
//#define usePIR

#define mqtt //закоментировать, если не нужен MQTT-протокол
#ifdef mqtt
 #include "PubSubClient.h"                  //https://github.com/Imroy/pubsubclient
 const char *mqtt_server = "192.168.0.251"; //Имя или IP-адрес сервера MQTT
 const int mqtt_port = 1883;                //Порт для подключения к серверу MQTT
 const char *mqtt_user = "";                //Логин от сервера MQTT
 const char *mqtt_pass = "";                //Пароль от сервера MQTT
#endif

#define relay1 //использовать реле№1
#define relay2 //использовать реле№2
#define relay3 //использовать реле№3
#define relay4 //использовать реле№4

#define relayControlLOW //раскомментировать, если реле управляется низким уровнем!
#ifdef relayControlLOW
 const bool relayStateON = 0;
 const bool relayStateOFF = 1;
#endif
#ifndef relayControlLOW
 const bool relayStateON = 1;
 const bool relayStateOFF = 0;
#endif

const String espName = "esp4test2019";     //Имя самой esp8266. Используется для формирования записей MQTT
const char *ssid = "Evangelion"; //Имя вайфай точки доступа
const char *pass = "97910101";   //Пароль от точки доступа

MDNSResponder mdns;
ESP8266WebServer server(80);
String webPage = "";
const String webPageRedirect2Main = "<meta http-equiv=\"refresh\" content=\"0; URL='/'\">";
const String relayOnColor = "228b22";
const String relayOffColor = "cecece";
const byte webPageReloadTime = 10;

#define BUFFER_SIZE 100

unsigned long timingIP, timingRelays, timingUptime;
const int intervalIP = 600;     //интервал отправки своего IP-адреса в секундах
const byte intervalRelays = 15;  //интервал отправки состояний реле в секундах
const byte intervalUptime = 60;  //интервал отправки uptime в секундах

const bool LedState = false;

#ifdef led0
 const byte led0Pin = 2;
#endif
#ifdef usePIR
 const byte pirPin = 5;
 const bool pirMode = 1; //что датчик "шлёт" при сработке
#endif
#ifdef relay1
 bool relayStateCur1;
 const byte relay1Pin = 16;
#endif
#ifdef relay2
 bool relayStateCur2;
 const byte relay2Pin = 14;
#endif
#ifdef relay3
 bool relayStateCur3;
 const byte relay3Pin = 12;
#endif
#ifdef relay4
 bool relayStateCur4;
 const byte relay4Pin = 13;
#endif

WiFiClient wclient;
#ifdef mqtt
 PubSubClient client(wclient, mqtt_server, mqtt_port);
#endif

void setup() {
  Serial.begin(115200);
  WiFi.persistent(false);  //Если выставить аргумент persistent на false, то SSID и пароль будут записаны на flash-память только в том случае, если новые значения не будут соответствовать тем, что хранятся во flash-памяти.
  WiFi.disconnect();       //обрываем WIFI соединения
  WiFi.softAPdisconnect(); //отключаем точку доступа (если она была)
  WiFi.mode(WIFI_STA);     //режим клиента
  #ifdef usePIR
   pinMode(pirPin, INPUT);
  #endif
  #ifdef led0
   pinMode(led0Pin, OUTPUT);
   digitalWrite(led0Pin, 1);
  #endif
  #ifdef relay1
   pinMode(relay1Pin, OUTPUT);
   digitalWrite(relay1Pin, relayStateOFF);
   relayStateCur1 = digitalRead(relay1Pin);
  #endif
  #ifdef relay2
   pinMode(relay2Pin, OUTPUT);
   digitalWrite(relay2Pin, relayStateOFF);
   relayStateCur2 = digitalRead(relay2Pin);
  #endif
  #ifdef relay3
   pinMode(relay3Pin, OUTPUT);
   digitalWrite(relay3Pin, relayStateOFF);
   relayStateCur3 = digitalRead(relay3Pin);
  #endif
  #ifdef relay4
   pinMode(relay4Pin, OUTPUT);
   digitalWrite(relay4Pin, relayStateOFF);
   relayStateCur4 = digitalRead(relay4Pin);
  #endif
  server.begin();
  Serial.println(F("Welcome!"));
  Serial.println(firmwareVer);
  Serial.println(firmwareBuild);

/////////////////////////////////////////////////////////////
// Web server
  server.on("/", [](){
    webPageMain(); server.send(200, "text/html", webPage);
  });
   server.on("/reset", [](){
//    ESP.restart();
   });
  #ifdef relay1
   server.on("/relay1On", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay1on();
   });
  server.on("/relay1Off", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay1off();
   });
  server.on("/relay1Switch", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay1switch();
   });
  #endif
  #ifdef relay2
   server.on("/relay2On", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay2on();
  });
  server.on("/relay2Off", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay2off();
  });
  server.on("/relay2Switch", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay2switch();
   });
  #endif
  #ifdef relay3
   server.on("/relay3On", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay3on();
  });
  server.on("/relay3Off", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay3off();
  });
  server.on("/relay3Switch", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay3switch();
   });
  #endif
  #ifdef relay4
   server.on("/relay4On", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay4on();
  });
  server.on("/relay4Off", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay4off();
  });
  server.on("/relay4Switch", [](){
    server.send(200, "text/html", webPageRedirect2Main); relay4switch();
   });
  #endif
///////////////////////////////////////////////////////////// 
}

void loop() {
  // подключаемся к wi-fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to " + String(ssid));
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {return;}
    Serial.println(F("WiFi connected"));
    Serial.println("IP address: " + WiFi.localIP().toString());
    Serial.println("Signal strength (RSSI): " + String(WiFi.RSSI()) + " dBm");
  }

  // подключаемся к MQTT серверу
  #ifdef mqtt
  if (WiFi.status() == WL_CONNECTED) {  
    if (!client.connected()) {    
      Serial.println(F("Connecting to MQTT server"));
      if (client.connect(MQTT::Connect(espName + "_"+ String(WiFi.macAddress()))
                         .set_auth(mqtt_user, mqtt_pass))) {
        Serial.println(F("Connected to MQTT server"));
        client.publish(espName + "/ip", WiFi.localIP().toString());          //отправляем наш IP-адрес
        client.publish(espName + "/mac", WiFi.macAddress());                 //отправляем наш MAC-адрес
        client.publish(espName + "/rssi", String(WiFi.RSSI()));              //отправляем уровень сигнала
        client.publish(espName + "/uptime", String(millis()));               //отправляем uptime
        client.publish(espName + "/system", "Welcome!");
        client.subscribe(espName + "/system");
        #ifdef usePIR
         client.publish(espName + "/pir", String(digitalRead(pirPin))); //отправляем состояние датчика движения
        #endif
        #ifdef relay1
         relay1state();
         client.subscribe(espName + "/relay1");                               //подписываемся на топик с данными для реле№1
         client.publish(espName + "/relay1", String(relayStateCur1)); //отправляем состояние реле
        #endif
        #ifdef relay2
         relay2state();
         client.subscribe(espName + "/relay2");                               //подписываемся на топик с данными для реле№2
         client.publish(espName + "/relay2", String(digitalRead(relayStateCur2))); //отправляем состояние реле
        #endif
        #ifdef relay3
         relay3state();
         client.subscribe(espName + "/relay3");                               //подписываемся на топик с данными для реле№3
         client.publish(espName + "/relay3", String(relayStateCur3)); //отправляем состояние реле
        #endif
        #ifdef relay4
         relay4state();
         client.subscribe(espName + "/relay4");                               //подписываемся на топик с данными для реле№4
         client.publish(espName + "/relay4", String(relayStateCur4)); //отправляем состояние реле
        #endif
        client.set_callback(callback);
      } else {
        Serial.println(F("Could not connect to MQTT server"));
      }
    }

    if (client.connected()) {
      client.loop();
    }
  }
#endif

  //webServer
  server.handleClient();  
  checkPIR();
  uptimeSend();
  ipSend();
  relaySend();
}

//////////////////////////////////////////////////
void ipSend() {
  if (millis() - timingIP > intervalIP * 1000) {
    #ifdef mqtt
    client.publish(espName + "/ip", WiFi.localIP().toString()); //отправляем наш IP-адрес
    client.publish(espName + "/mac", WiFi.macAddress());        //отправляем наш MAC-адрес
    #endif
    Serial.println("IP address: " + WiFi.localIP().toString());
    Serial.println("MAC address: " + WiFi.macAddress());
    timingIP = millis();
  }
}

void uptimeSend() {
  if (millis() - timingUptime > intervalUptime * 1000) {
    #ifdef mqtt
    client.publish(espName + "/rssi", String(WiFi.RSSI())); //отправляем уровень сигнала
    client.publish(espName + "/uptime", String(millis()));  //отправляем uptime
    #endif
    Serial.println("RSSI: " + String(WiFi.RSSI()));
    Serial.println("Uptime: " + String(millis()));
    timingUptime = millis();
  }
}

void checkPIR() {
 #ifdef usePIR
  if (digitalRead(pirPin) == pirMode) {
    
   }
 #endif
}

void relaySend() {
  if (millis() - timingRelays > intervalRelays * 1000) {
   #ifdef usePIR
    #ifdef mqtt
    client.publish(espName + "/pir", String(digitalRead(pirPin))); //отправляем состояние датчика движения
    #endif
    Serial.println("PIR: " + String(digitalRead(pirPin)));
   #endif
   #ifdef relay1
    relay1state();
    Serial.println("Relay1: " + String(relayStateCur1));
   #endif
   #ifdef relay2
    relay2state();
    #ifdef mqtt
     client.publish(espName + "/relay2", String(relayStateCur2)); //отправляем состояние реле
    #endif
     Serial.println("Relay2: " + String(relayStateCur2));
   #endif
   #ifdef relay3
    relay3state();
    #ifdef mqtt
     client.publish(espName + "/relay3", String(relayStateCur3)); //отправляем состояние реле
    #endif
     Serial.println("Relay3: " + String(relayStateCur3));
   #endif
   #ifdef relay4
    relay4state();
    #ifdef mqtt
     client.publish(espName + "/relay4", String(relayStateCur4)); //отправляем состояние реле
    #endif
     Serial.println("Relay4: " + String(relayStateCur4));
   #endif
    timingRelays = millis();
  }
}

#ifdef relay1
 void relay1state() {
  #ifdef relayControlLOW
   if (digitalRead(relay1Pin) == 0) {relayStateCur1 = 1;} else {relayStateCur1 = 0;}
  #endif
  #ifndef relayControlLOW
   relayStateCur1 = digitalRead(relay1Pin);
  #endif  
 }
 void relay1on() {
  digitalWrite(relay1Pin, relayStateON); relay1state(); blinkLed(); relay1Send();
 }
 void relay1off() {
  digitalWrite(relay1Pin, relayStateOFF); relay1state(); blinkLed(); relay1Send();
 }
 void relay1switch() {
  digitalWrite(relay1Pin, !digitalRead(relay1Pin)); relay1state(); blinkLed(); relay1Send();
 }
 void relay1Send() {
   relay1state();
   #ifdef mqtt
    client.publish(espName + "/relay1", String(relayStateCur1)); //отправляем состояние реле
   #endif
   Serial.println("Relay1: " + String(relayStateCur1));
 }
#endif

#ifdef relay2
 void relay2state() {
  #ifdef relayControlLOW
   if (digitalRead(relay2Pin) == 0) {relayStateCur2 = 1;} else {relayStateCur2 = 0;}
  #endif
  #ifndef relayControlLOW
   relayStateCur2 = digitalRead(relay2Pin);
  #endif  
 }
 void relay2on() {
  digitalWrite(relay2Pin, relayStateON); relay2state(); blinkLed(); relay2Send();
 }
 void relay2off() {
  digitalWrite(relay2Pin, relayStateOFF); relay2state(); blinkLed(); relay2Send();
 }
 void relay2switch() {
  digitalWrite(relay2Pin, !digitalRead(relay2Pin)); relay2state(); blinkLed(); relay2Send();
 }
 void relay2Send() {
  relay2state();
  #ifdef mqtt
   client.publish(espName + "/relay2", String(relayStateCur2)); //отправляем состояние реле
  #endif
   Serial.println("Relay2: " + String(relayStateCur2));
 }
#endif

#ifdef relay3
 void relay3state() {
  #ifdef relayControlLOW
   if (digitalRead(relay3Pin) == 0) {relayStateCur3 = 1;} else {relayStateCur3 = 0;}
  #endif
  #ifndef relayControlLOW
   relayStateCur3 = digitalRead(relay3Pin);
  #endif  
 }
 void relay3on() {
  digitalWrite(relay3Pin, relayStateON); relay3state(); blinkLed(); relay3Send();
 }
 void relay3off() {
  digitalWrite(relay3Pin, relayStateOFF); relay3state(); blinkLed(); relay3Send();
 }
 void relay3switch() {
  digitalWrite(relay3Pin, !digitalRead(relay3Pin)); relay3state(); blinkLed(); relay3Send();
 }
 void relay3Send() {
  relay3state();
  #ifdef mqtt
   client.publish(espName + "/relay3", String(relayStateCur3)); //отправляем состояние реле
  #endif
   Serial.println("Relay3: " + String(relayStateCur3));
 }
#endif

#ifdef relay4
 void relay4state() {
  #ifdef relayControlLOW
   if (digitalRead(relay4Pin) == 0) {relayStateCur4 = 1;} else {relayStateCur4 = 0;}
  #endif
  #ifndef relayControlLOW
   relayStateCur4 = digitalRead(relay4Pin);
  #endif  
 }
 void relay4on() {
  digitalWrite(relay4Pin, relayStateON); relay4state(); blinkLed(); relay4Send();
 }
 void relay4off() {
  digitalWrite(relay4Pin, relayStateOFF); relay4state(); blinkLed(); relay4Send();
 }
 void relay4switch() {
  digitalWrite(relay4Pin, !digitalRead(relay4Pin)); relay4state(); blinkLed(); relay4Send();
 }
 void relay4Send() {
  relay4state();
  #ifdef mqtt
   client.publish(espName + "/relay4", String(relayStateCur4)); //отправляем состояние реле
  #endif
   Serial.println("Relay4: " + String(relayStateCur4));
 }
#endif

void blinkLed() {
 #ifdef led0
  digitalWrite(led0Pin, !digitalRead(led0Pin));
  delay(100);
  digitalWrite(led0Pin, !digitalRead(led0Pin));
 #endif
 #ifndef led0
  Serial.println(F("OK"));
 #endif
 }

void webPageMain() {
  webPage  = "<html><head><style>";
  webPage += "span.header{font-size: 350%; text-shadow: 1px 1px white, 2px 2px #777; color: #333; transition: all 1s;}";
  webPage += "button{width: 150px; height: 35px; border-radius: 10px; font-size: 125%;}";
  webPage += "table{border-radius: 10px; border-spacing: 5px 5px; border: 1px solid #fff;}";
  webPage += "</style>";
  webPage += "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">";
  webPage += "<meta http-equiv=\"refresh\" content=\"" + String(webPageReloadTime) + "; URL=\"/\"\">";
  webPage += "<title>" + espName + "</title></head><body bgcolor=\"#cecece\"><center>";
  webPage += "<table style=\"border: 0px;\"><tr>";
  webPage += "<td><span class=\"header\">" + espName + "</span></td><td align=\"right\">RSSI:&nbsp;<br>Uptime:&nbsp;</td><td align=\"right\">" + String(WiFi.RSSI()) + " dBm<br>" + String(millis()/1000) + " sec</td>";
  webPage += "</tr></table><br>";
  #ifdef usePIR
   //////////////////////////////////////
   // Датчик движения - начало кода
   webPage += "<table bgcolor=\"#";
    if (digitalRead(pirPin) == pirMode) {webPage += relayOnColor;} else {webPage += relayOffColor;}
   webPage += "\"><tr><td>Движение</td></tr></table><br>";
   // Датчик движения - конец кода
   //////////////////////////////////////
  #endif  
  #ifdef relay1
   //////////////////////////////////////
   // Relay 1 - начало кода
   webPage += "<table bgcolor=\"#";
    if (digitalRead(relay1Pin) == relayStateON) {webPage += relayOnColor;} else {webPage += relayOffColor;}
   webPage += "\"><th colspan=\"2\">Relay 1</th><tr>";
   webPage += "<td><a href=\"relay1On\"><button>On</button></a></td><td><a href=\"relay1Off\"><button>Off</button></a></td>";
   webPage += "</tr></table><br>";
   // Relay 1 - конец кода
   //////////////////////////////////////
  #endif
  #ifdef relay2
   //////////////////////////////////////
   // Relay 2 - начало кода
   webPage += "<table bgcolor=\"#";
    if (digitalRead(relay2Pin) == relayStateON) {webPage += relayOnColor;} else {webPage += relayOffColor;}
   webPage += "\"><th colspan=\"2\">Relay 2</th><tr>";
   webPage += "<td><a href=\"relay2On\"><button>On</button></a></td><td><a href=\"relay2Off\"><button>Off</button></a></td>";
   webPage += "</tr></table><br>";
   // Relay 2 - конец кода
   //////////////////////////////////////
  #endif
  #ifdef relay3
   //////////////////////////////////////
   // Relay 3 - начало кода
   webPage += "<table bgcolor=\"#";
    if (digitalRead(relay3Pin) == relayStateON) {webPage += relayOnColor;} else {webPage += relayOffColor;}
   webPage += "\"><th colspan=\"2\">Relay 3</th><tr>";
   webPage += "<td><a href=\"relay3On\"><button>On</button></a></td><td><a href=\"relay3Off\"><button>Off</button></a></td>";
   webPage += "</tr></table><br>";
   // Relay 3 - конец кода
   //////////////////////////////////////
  #endif
  #ifdef relay4
   //////////////////////////////////////
   // Relay 4 - начало кода
   webPage += "<table bgcolor=\"#";
    if (digitalRead(relay4Pin) == relayStateON) {webPage += relayOnColor;} else {webPage += relayOffColor;}
   webPage += "\"><th colspan=\"2\">Relay 4</th><tr>";
   webPage += "<td><a href=\"relay4On\"><button>On</button></a></td><td><a href=\"relay4Off\"><button>Off</button></a></td>";
   webPage += "</tr></table><br>";
   // Relay 4 - конец кода
   //////////////////////////////////////
  #endif
   //////////////////////////////////////
   // Подвал - начало кода
   webPage += "<table style=\"border: 0px; width: 320px;\"><td align=\"right\"><font size=\"2\">" + firmwareVer + "<br>" + firmwareBuild + "</font></td></table>";
   // Подвал - конец кода
   //////////////////////////////////////  
  webPage += "</center></body></html>";
  }

////////////////////////////////////////////
// Функция получения данных от MQTT-сервера
#ifdef mqtt
void callback(const MQTT::Publish& pub) {
  String payload = pub.payload_string();
  #ifdef relay1
   if (String(pub.topic()) == espName + "/relay1") //проверяем из нужного ли нам топика пришли данные
   {
    relay1state();
    int stled = payload.toInt();                  //преобразуем полученные данные в тип integer
    if (stled == 2) {relay1switch();}
    if (stled == 0 and stled != relayStateCur1) {relay1off();}
    if (stled == 1 and stled != relayStateCur1) {relay1on();}
   }
  #endif
  #ifdef relay2
   if (String(pub.topic()) == espName + "/relay2") //проверяем из нужного ли нам топика пришли данные
   {
    relay2state();
    int stled = payload.toInt();                  //преобразуем полученные данные в тип integer
    if (stled == 2) {relay2switch();}
    if (stled == 0 and stled != relayStateCur2) {relay2off();}
    if (stled == 1 and stled != relayStateCur2) {relay2on();}
   }
  #endif
  #ifdef relay3
   if (String(pub.topic()) == espName + "/relay3") //проверяем из нужного ли нам топика пришли данные
   {
    relay3state();
    int stled = payload.toInt();                  //преобразуем полученные данные в тип integer
    if (stled == 2) {relay3switch();}
    if (stled == 0 and stled != relayStateCur3) {relay3off();}
    if (stled == 1 and stled != relayStateCur3) {relay3on();}
   }
  #endif
  #ifdef relay4
   if (String(pub.topic()) == espName + "/relay4") //проверяем из нужного ли нам топика пришли данные
   {
    relay4state();
    int stled = payload.toInt();                  //преобразуем полученные данные в тип integer
    if (stled == 2) {relay4switch();}
    if (stled == 0 and stled != relayStateCur4) {relay4off();}
    if (stled == 1 and stled != relayStateCur4) {relay4on();}
   }
  #endif
  if (String(pub.topic()) == espName + "/system") //проверяем из нужного ли нам топика пришли данные
  {
    int stled = payload.toInt();                  //преобразуем полученные данные в тип integer
    if (stled == 3) {}
  }
}
#endif
