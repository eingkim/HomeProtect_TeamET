#include "WiFiEsp.h"
#include <Servo.h>
 

const char ssid[] = "SSID";             // 네트워크 SSID
const char pass[] = "password";         // 네트워크 비밀번호
int status = WL_IDLE_STATUS;            // Wifi radio의 상태


Servo myservo;     // 서보모터 객체 생성.
int angle = 0;    // 서보모터의 position을 저장하기 위한 변수
int standard = 0;
bool webstat = LOW;
bool button = LOW; // off

#define servoPin 9

WiFiEspServer server(80);

void setup() {  
  Serial.begin(9600);
  myservo.attach(9);  // 서보모터 핀 : 9
  myservo.write(95); // 모터 초기화.
  WiFi.init(&Serial);   // ESP 모듈 초기화.
  while ( status != WL_CONNECTED) {   // 와이파이 연결 시도
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);    // WPA/WPA2 네트워크 연결 시도.
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
  }
  server.begin();   // 서버 가동
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
                            "<p><b>Door</b></p>";
const char STAT_ON[] PROGMEM = "<p style=\"text-align: right\">status: <font color= 'red'><b> ON</b></font></p>";
const char STAT_OFF[] PROGMEM = "<p style=\"text-align: right\">status: <font color= 'blue'><b> OFF</b></font></p>";
const char BUTTON_ON[] PROGMEM   = "<p><a href=\"/B/off\"><button style=\"background-color:#000000;\">ON</button></a>";
const char BUTTON_OFF[] PROGMEM  = "<p><a href=\"/B/on\"><button style=\"background-color:#6E6E6E;\">OFF</button></a>";
const char HTTP_END[] PROGMEM      = "</div></body></html>";

/*
 * lockServo - 잠금 작업 시 서보모터의 작동
 */
void lockServo(){
      for(angle = 0; angle < 95; angle++) { 
        myservo.write(angle); 
        delay(15);
    }   
}

/**
 * unlockServo - 잠금 해제 작업 시 서보모터의 작동
 */
void unlockServo(){
  for(angle = 95; angle > 0; angle--) { 
      myservo.write(angle); 
      delay(15); 
  } 
}

void loop() {
  
  /*클라이언트를 기다림*/
  WiFiEspClient client = server.available();  
  if (client) {                               
    while (client.connected()) {              
      if (client.available()) {               
        String income_AP = client.readStringUntil('\n');

        // 잠금
        if (income_AP.indexOf(F("B/on")) != -1) {
          Serial.println(F("button on"));
          button = HIGH;
          lockServo();
          webstat = LOW;
          
        // 잠금 해제
        } else if (income_AP.indexOf(F("B/off")) != -1) {
          Serial.println(F("button off"));
          button = LOW;
          unlockServo();
          webstat = HIGH;
        }
      }
        client.flush();

//        웹 제어문
        client.println(F("HTTP/1.1 200 OK"));  // HTTP Protocol header
        client.println(F("Content-type:text/html"));
        client.println(F("Connection: close"));
        client.println();
        
        String page;
        page  = (const __FlashStringHelper *)HTTP_HEAD;
        page += (const __FlashStringHelper *)HTTP_STYLE;
        page += (const __FlashStringHelper *)BODY;
        if (button == HIGH) {
          page += (const __FlashStringHelper *)BUTTON_ON;
        } else {
          page += (const __FlashStringHelper *)BUTTON_OFF;
        }
        if (webstat == LOW) { 
            page += (const __FlashStringHelper *)STAT_OFF;
          } 
        else if (webstat == HIGH){
          page += (const __FlashStringHelper *)STAT_ON;
        }
        page += (const __FlashStringHelper *)HTTP_END;

        // HTML 전송.
        client.print(page);
        client.println();
        delay(1);
        break;
      }
    }
    client.stop();
}
