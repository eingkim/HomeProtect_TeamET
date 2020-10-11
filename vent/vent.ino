#include <DHT.h>
#include <Servo.h>
#include "WiFiEsp.h"

Servo myservo;
int pos = 0;    // 서보모터의 구동을 위한 각도 변수
bool webstat = LOW; // 환풍기의 현재 구동 상태 확인용 변수
bool button = LOW; // 웹페이지의 버튼 

const char ssid[] = "rnin";         // 와이파이 SSID
const char pass[] = "qpwoei102938";     // 와이파이 비밀번호
int status = WL_IDLE_STATUS;        // Wifi Radio의 상태

#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiEspServer server(80);

void setup() {
  Serial.begin(9600);
  dht.begin();
  myservo.attach(9);     // 서보모터 핀 : 9
  myservo.write(135);    // 모터 초기화.
  WiFi.init(&Serial);   // ESP모듈 초기화
  while (status != WL_CONNECTED) {   // 와이파이 연결 시도.
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);    // Connect to WPA/WPA2 network
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
  }
  server.begin();     // 서버 가동
}

/* HTML 코드 */
const char HTTP_HEAD[] PROGMEM     = "<!DOCTYPE html>"
                                     "<html>"
                                     "<head>";
const char HTTP_STYLE[] PROGMEM    = "<style type=\"text/css\">"
"* {font-family: \"맑은 고딕\"}"
"button{border:2;border-color:#fff;border-radius:0.3rem;background-color:#6E6E6E;color:#ffffff;line-height:2.4rem;font-size:1.2rem;width:10%}"
"</style>";
const char BODY[] PROGMEM = "<body bgcolor='#a4a4a4' style=\"text-align: center\">"
                            "<div style=\"margin: 0 auto\">"
                            "<p><b>Vent</b></p>";
const char STAT_ON[] PROGMEM = "<p style=\"text-align: right\">status: <font color= 'red'><b> ON</b></font></p>";
const char STAT_OFF[] PROGMEM = "<p style=\"text-align: right\">status: <font color= 'blue'><b> OFF</b></font></p>";
const char BUTTON_ON[] PROGMEM   = "<p><a href=\"/B/off\"><button style=\"background-color:#000000;\">ON</button></a>";
const char BUTTON_OFF[] PROGMEM  = "<p><a href=\"/B/on\"><button style=\"background-color:#6E6E6E;\">OFF</button></a>";
const char HUMDTY[] PROGMEM      = "<p>Humidity: <b>";
const char TMP[] PROGMEM      = "<br>Temperature: <b>";
const char HTTP_END[] PROGMEM      = "</div></body></html>";

/**
 * startVent - 환풍기를 켜도록 서보모터를 돌림.
 * 반환값 - 현재 환풍기의 상태. HIGH : 켜져있음
 */
bool startVent(){
    for(pos = 135; pos < 178; pos += 1) {                                 
     myservo.write(pos);             
    delay(15);                       
   }
    delay(3000);
    return HIGH;
}

/**
 * stopVent - 환풍기를 끄도록 서보모터를 돌림.
 * 반환값 - 현재 환풍기의 상태. LOW : 꺼져있음
 */
bool stopVent(){
     for(pos = 135; pos >= 73; pos -= 1) {                                 
     myservo.write(pos);             
     delay(15);                       
   }  
   delay(3000);
   return LOW;
}

/**
 * resetServo - 모터를 초기상태로 둠.
 */
void resetServo(){
   pos = 135;
   myservo.write(pos);
   delay(3000);
}

void loop() {
  // 온도와 습도값을 받음
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  /*클라이언트를 기다림*/
  WiFiEspClient client = server.available();  
  if (client) {                               
    while (client.connected()) {              
      if (client.available()) {               
        String income_AP = client.readStringUntil('\n');

/*  웹의 버튼을 통한 수동 제어문 */
        if (income_AP.indexOf(F("B/off")) != -1) {
          Serial.println(F("button on"));
          button = LOW;
          webstat = startVent();
        } else if (income_AP.indexOf(F("B/on")) != -1) {
          Serial.println(F("button off"));
          button = HIGH;
          webstat = stopVent();
        }
        resetServo();
        client.flush();
        
//        웹 제어문
        client.println(F("HTTP/1.1 200 OK"));  // HTTP Protocol Header
        client.println(F("Content-type:text/html"));
        client.println(F("Connection: close"));
        client.println();

        String page;
        page  = (const __FlashStringHelper *)HTTP_HEAD;
        page += (const __FlashStringHelper *)HTTP_STYLE;
        page += (const __FlashStringHelper *)BODY;

//      status 표시용 조건문
        if (webstat == HIGH) { 
            page += (const __FlashStringHelper *)STAT_ON;
          } 
        else if (webstat == LOW) {
          page += (const __FlashStringHelper *)STAT_OFF;
        }
//      버튼 전환용 조건문.
        if (button == HIGH || webstat == LOW) {
          page += (const __FlashStringHelper *)BUTTON_ON;
        } else if (button == LOW || webstat == HIGH){
          page += (const __FlashStringHelper *)BUTTON_OFF;
        }
        page += (const __FlashStringHelper *)HUMDTY;
        page += h;
        page += "%</b>";
        page += (const __FlashStringHelper *)TMP;
        page += t;
        page += " C</b></p>";
        page += (const __FlashStringHelper *)HTTP_END;

        //HTML 전송.
        client.print(page);
        client.println();
        delay(1);
        break;
      }
    }
    client.stop(); 
  }
}
