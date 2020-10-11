#include "WiFiEsp.h"
#include <IRremote.h>
 #if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

IRsend irsend;                                                              // 적외선 신호 send 객체
char ssid[] = "rnin";            // your network SSID (name)
char pass[] = "qpwoei102938";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
                
char count = 0;
bool G_on = 0;
char repeat = 0;
char GETbuffer[8];                                                       // GET Method를 담을 버퍼
char i = 0;
bool hi_on = 0;
bool ho_on = 0;
float hum;
float temp;
bool on_state = false;

WiFiEspServer server(80);                                                //포트 80으로 웹서버 구축

 unsigned int irSignalon[] =  { 4428,  4300, 624,  1512, 572,  528, 540,  1564, 572,  1564, 572,  528, 564,  504, 568,  1564, 568, 
 504, 564,  508, 560,  1576, 560,  532, 536,  532, 536,  1576, 560,  1576, 560,  532, 536,  1600, 536,  536, 532,  536, 532,  536, 536,  1600, 536, 
 1600, 536,  1600, 536,  1600, 536,  1600, 536,  1600, 536,  1604, 532,  1604, 532,  536, 532,  536, 532,  536, 532,  536, 536,  532, 536,  532, 536,  
 1600, 536,  1600, 536,  1600, 536,  532, 536,  1604, 532,  532, 536,  536, 532,  1604, 532,  536, 532,  536, 532,  536, 532,  1604, 532,  536, 532, 
 1604, 532,  1604, 536,  5172, 4364,  4364, 532,  1604, 532,  536, 532,  1604, 532,  1604, 536,  532, 536,  532, 536,  1604, 532,  536, 532,  536, 532,  
 1604, 532,  536, 532,  536, 532,  1604, 532,  1604, 532,  536, 532,  1604, 532,  536, 532,  536, 532,  536, 532,  1604, 532,  1604, 532,  1608, 528,  
 1608, 532,  1604, 532,  1604, 532,  1604, 532,  1604, 532,  536, 532,  536, 508,  560, 532,  536, 508,  560, 532,  536, 508,  1628, 532,  1608, 508, 
 1628, 508,  560, 508,  1628, 508,  560, 532,  536, 508,  1628, 508,  560, 508,  560, 508,  560, 508,  1628, 508,  560, 508,  1632, 504,  1632, 504, 
 };
                                                                      // 전송할 에어컨 ON 신호의 RAW DATA

 unsigned int irSignaloff[] = { 4428,  4300, 596,  1540, 576,  528, 536,  1568, 572,  1564, 572,  524, 544,  524, 568,  1544, 592,  504, 564,  504,
 564,  1572, 560,  536, 532,  512, 556,  1580, 560,  1576, 560,  532, 536,  1604, 532,  512, 556,  1604, 536,  1600, 536,  1604, 532,  1604, 536,  532,
 536,  1600, 536,  1604, 532,  1604, 532,  536, 536,  532, 532,  536, 532,  536, 536,  1600, 536,  532, 536,  532, 536,  1604, 532,  1604, 532,  1604, 536,
 532, 536,  532, 536,  532, 536,  532, 536,  532, 536,  536, 532,  536, 532,  536, 532,  1604, 532,  1604, 536,  1604, 532,  1604, 532,  1604, 532,  5176,
 4364,  4364, 536,  1600, 536,  536, 532,  1604, 532,  1604, 532,  536, 532,  536, 532,  1604, 536,  532, 532,  536, 536,  1604, 532,  536, 532,  536, 532,
 1604, 532,  1604, 532,  536, 532,  1604, 536,  532, 536,  1604, 532,  1604, 532,  1604, 536,  1604, 532,  536, 532,  1604, 532,  1604, 532,  1604, 532,  536,
 532,  536, 536,  536, 532,  536, 532,  1604, 532,  536, 532,  536, 532,  1604, 532,  1604, 536,  1604, 532,  536, 532,  536, 532,  536, 532,  536, 532,  536,
 532,  536, 532,  536, 532,  536, 532,  1608, 504,  1632, 532,  1604, 508,  1628, 508,  1628, 508,  34740, 296, 
  }; 
                                                                      // 전송할 에어컨 OFF 신호의 RAW DATA

void setup()
{
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);                                               //라즈베리파이로 신호 전달 핀.
  
  Serial.begin(9600);                                                //시리얼 모니터
  Serial1.begin(9600);                                              //아두이노 메가의 하드웨어 시리얼1.
  WiFi.init(&Serial1);                                              // WIFI 객체 초기화
  pinMode(10, OUTPUT);
  #if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
  #endif
  #if defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
      delay(2000); // To be able to connect Serial monitor after reset and before first printout
  #endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
     
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
   
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
   printWifiStatus();                                                 //와이파이 연결 상태 출력
   
 
  server.begin();                                                     //웹서버 시작
  GETbuffer[7] = '\0';
}


void loop()
{ 

  // listen for incoming clients
  
  WiFiEspClient client = server.available();
  if (client) {
    
    Serial.println("New client");
   
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {                                    //Client 연결 확인
    
      if (client.available()) {                                     //client request 시 동작
       
        char c = client.read();             
       
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          Serial.println("Sending response");-
          client.print(
            "HTTP/1.1 200 OK\r\n" "Content-Type: text/html\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "Refresh: 20\r\n"        // refresh the page automatically every 20 sec
            "\r\n");
          client.print("<!DOCTYPE HTML>\r\n");
          client.print("<html>\r\n");
          client.print("<h1>Soomin!</h1>\r\n");
          client.print("<br>\r\n");
          if(on_state)
          {
          client.print("<h1>Airconditioner_on</h1>\r\n");
          client.print("<br>\r\n");
          }
          else 
            {
          client.print("<h1>Airconditioner_off</h1>\r\n");
          client.print("<br>\r\n");
          }
          client.print("Temp : ");
          client.print(temp);

         
          client.print("<br>\r\n");
          client.print("Humid : ");
          client.print(hum);
          client.print("<br>\r\n");
          client.print("<br>\r\n");
          client.print("led_on <a href=\"/hi\"> here </a>  <br>");                        //파이퍼링크로 http Get Method 사용 가능
          client.print("<br>\r\n");
          client.print("led_off <a href=\"/ho\"> here </a>  <br>");
          client.print("<br>\r\n");
          client.print("reset <a href=\"\"> here </a>  <br>");
         client.print("</html>\r\n");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }

        
       
        if((c == 'G')&&(G_on))                             // 맨처음 GET 입력에서 버퍼를 뽑아내기 위함.
        {
          count = 1;
        }
        
         if(count)                                         //request에서 http GET Method를 버퍼에 저장
         {
            GETbuffer[i] = c;
            i++;
            if(i>6)
                {
                  G_on = false;
                  count = 0;
                  i = 0;
                }
         }        
      }
    }
    // give the web browser time to receive the data
    delay(100);

    // close the connection:
    client.stop();                                            //Client 연결 헤제
    repeat = 0;
   Serial.println("Client disconnected");    
  }
  
    if((0==(strcmp(GETbuffer, "GET /hi"))))                     //GET 버퍼 확인
                {
              Serial.println("Getbuffer is hizzzzzzzzzzzzzzzzzzzzzzzzz");
              Serial.println(GETbuffer);
              strcpy(GETbuffer,"0000000");
              irsend.sendRaw(irSignalon, 199,38 );              //적외선 송신기로 저장된 에어컨 ON 신호의 RAW DATA 전송 
              delay(10);
              on_state = true;
                digitalWrite(7, LOW);
                }
    else if((0==(strcmp(GETbuffer, "GET /ho"))))
                  { 
              Serial.println("Getbuffer is hozzzzzzzzzzzzzzzzzzzzz");
              Serial.println(GETbuffer);
              strcpy(GETbuffer,"0000000");
              irsend.sendRaw(irSignaloff,201,38 );            //적외선 송신기로 저장된 에어컨 OFF 신호의 RAW DATA 전송 
              delay(10);
              on_state = false;
                digitalWrite(7, LOW);
                }
   else if((0==(strcmp(GETbuffer, "GET /ca"))))
                  { 
                  Serial.println("Getbuffer is czczczczczczacacacacacacacacacac");
                  strcpy(GETbuffer,"0000000");
                  digitalWrite(7, HIGH);                    //라즈베리파이의 스트리밍 정지 신호
                  delay(10);
                  digitalWrite(7, LOW);
                   }          
  G_on = true;
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
