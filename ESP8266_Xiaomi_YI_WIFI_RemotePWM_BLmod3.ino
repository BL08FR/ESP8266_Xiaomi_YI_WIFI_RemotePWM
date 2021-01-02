/*
 *  This sketch demonstrates how to use ESP8266 to control Xiaomi Yi Camera.
 *
 * @author  Rok Rodic alias GreenEyedExplorer alias Hacker007, www.rodic.si
 * @version 0.1
 * @since   2017-07-01
 * 
 */
//Find SSID containing "YDXJ_". Password "1234567890". Connect to 192.168.42.1:7878.
// * Get token:           {"msg_id":257,"token":x}
// * Take photo:          {"msg_id":769,"token":x}
// * Start record video:  {"msg_id":513,"token":x}
// * Stop record video:   {"msg_id":514,"token":x}
// Get live stream:     {"msg_id":259,"token":x,"param":"none_force"}
// Get bat stat:        {"msg_id":13,"token":x} -> {"rval":0,"msg_id":13,"type":"adapter","param":"100"}
// Get all settings:    {"msg_id":3,"token":x}
// Get setting choices: {"msg_id":9,"param":"video_resolution","token":x}
// Get single setting:  {"msg_id":1,"type":"video_resolution","token":x}
// Get SDCARD space:    {"msg_id":5,"type":"free","token":x}
// Set single setting:  {"msg_id":2,"type":"video_resolution","param":"1920x1080 60P 16:9","token":x}
// Activate log:        {"msg_id":2,"type":"save_log","param":"on","token":x} -> READ telnet x.x.x.x + tail -f /tmp/fuse_a/firmware.avtive.log
// VLC stream: rtsp://192.168.42.1:7878/live

#include "ESP8266WiFi.h"
#include "Bounce2.h"

byte PWM_PIN = 4;
int pwm_value;
int photoPWM = 1250;
int videoPWM = 1700;
int maxPWM = 2300;
long timeinmem = 0;
int stabePWMdelay = 80;
int photorequestnumber = 0;
 
WiFiClient client;
String YI_SSID;
const int buttonPin1 = 13;          // input pin for pushbutton
const int buttonPin2 = 12;          // input pin for pushbutton
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
boolean RecON = false;
boolean forcephotorequest = false;

void searchCamera() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(10);
  int cnt = WiFi.scanNetworks();
  Serial.print("Networks: ");
  if (cnt > 0) {
    for (int i = 0; i < cnt; ++i) {
      Serial.print(WiFi.SSID(i) + ",");
      if (WiFi.SSID(i).startsWith("YDXJ_")) {
        YI_SSID = WiFi.SSID(i);
        break;
      }
    }
  }
  Serial.println();
}

void connectToCamera() {
  bool result = true;
  short retry = 30;
  const int jsonPort = 7878;
  char password[11] = "1234567890";
  char ssid[30];
  Serial.print("Con: ");
  YI_SSID.toCharArray(ssid, YI_SSID.length() + 1);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if (retry == 0) {
      result = false;
      break;
    }
    delay(500);
    retry--;
  }
  Serial.print(" -> wifi con:");
  if (result == true) Serial.print("OK "); else Serial.print("XX ");

  if (!client.connect("192.168.42.1", jsonPort)) result = false;
  Serial.print(" IP con:");
  if (result == true) Serial.print("OK."); else Serial.print("XX.");
  Serial.println();
}

void setup() {
  //Only For debug
  Serial.begin(115200);
  Serial.println("STARTUP");
  searchCamera();
  connectToCamera();
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  debouncer1.attach(buttonPin1); debouncer1.interval(50);
  debouncer2.attach(buttonPin2); debouncer2.interval(50);
}
 
String requestToken() {
  String token;
  // This will send the request token msg to the server
  client.print("{\"msg_id\":257,\"token\":0}\n\r");
  //delay(1000);
  yield(); delay(250); yield(); delay(250); yield(); delay(250); yield(); delay(250);
  // Read all the lines of the reply from server and print them to Serial
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }
  // Search token in to the stream
  int offset = response.lastIndexOf(':');
  if (offset != -1) {
    for (int i = offset + 1; i < response.length(); ++i) {
      if ((response.charAt(i) != ' ') && (response.charAt(i) != '}')) {
        token.concat(response.charAt(i));
      }
    }
  }

  return token;
}
 
void TakePhoto(String token) {
  photorequestnumber ++;
  if (RecON) {
    RecordOFF(token);
    RecON = false;
    String token = requestToken();
    if (token.length() == 0) exit;
  }
  client.print("{\"msg_id\":769,\"token\":");
  client.print(token);
  client.print("}\n\r");
  Serial.print("Photo - Response: ");
  yield(); delay(25); yield(); delay(25); yield(); delay(25); yield(); delay(25);
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }
  //Serial.println(response);
  if (response.startsWith("0", 8)){//if photo is done dont force photo
    forcephotorequest = false;
    photorequestnumber = 0;
    Serial.print("Photo request accepted. Time = ") && Serial.println(millis());
    }
  else{
    forcephotorequest = true;}
    //Serial.print("Photo request rejected, force a new one! Time= ")&& Serial.println(millis());}//if photo isn't done, force it
  if (RecON) {
    String token = requestToken();
    if (token.length() == 0) exit;
    RecordON(token);
  }
}

void RecordON(String token) {
  client.print("{\"msg_id\":513,\"token\":");
  client.print(token);
  client.print("}\n\r");
  Serial.println("RecON - Response: ");
  yield(); delay(250); yield(); delay(250); yield(); delay(250); yield(); delay(250);
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }
  //Serial.println(response);
  if (response.startsWith("0", 8))// if camera return video record is accepted recon= true, if not recon= false (next loop will ask again to record)
    RecON = true;
  else{RecON = false;}
  //Serial.print("RecOn in recordon function = ") && Serial.println(RecON);
  
}

void RecordOFF(String token) {
  client.print("{\"msg_id\":514,\"token\":");
  client.print(token);
  client.print("}\n\r");
  //Serial.print("RecOFF - Response: ");
  yield(); delay(250); yield(); delay(250); yield(); delay(250); yield(); delay(250);
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }
  //Serial.println(response);
}


void loop() {
  pwm_value = pulseIn(PWM_PIN, HIGH); //BL
  Serial.print("PWM read =") && Serial.println(pwm_value); //BL
  if(photorequestnumber >= 10){
    photorequestnumber = 0;
    forcephotorequest = false;}
  
  //1)to stop record if low pwm
  while(pwm_value < videoPWM && RecON == true) {//do it if pwm last stablePWMdelay, same logic for photo(2) and video(3)
    if(timeinmem == 0){timeinmem = millis();}
    if(millis() - timeinmem >= stabePWMdelay){
      String token = requestToken();
      if (token.length() != 0) {
        RecordOFF(token);
        timeinmem = 0;
        RecON = false;//tell video record stopped
        break;}
     }
    pwm_value = pulseIn(PWM_PIN, HIGH);//update pwm value, same logic for photo(2) and video(3)
    }
  if(timeinmem != 0){timeinmem = 0;}

  //2)to take photo if pwm in photo range after small delay in range
  while(pwm_value >= photoPWM && pwm_value < videoPWM || forcephotorequest == true){
    if(timeinmem == 0){timeinmem = millis();}
    if(millis() - timeinmem >= stabePWMdelay || forcephotorequest == true){
      String token = requestToken();
      if (token.length() != 0){
      Serial.print("Photo requested in loop. Time= ") && Serial.println(millis());
      TakePhoto(token);
      timeinmem = 0;
      break;}
    }
    pwm_value = pulseIn(PWM_PIN, HIGH);
    }
  if(timeinmem != 0){timeinmem = 0;}

  //3)to take video if within pwm range (stop with one or two
  while(pwm_value >= videoPWM && pwm_value <= maxPWM) {
    //Serial.println("Inside video while");
    if(timeinmem == 0){timeinmem = millis();}
    if(millis() - timeinmem >= stabePWMdelay){
      //Serial.println("in if video before to request token");
      String token = requestToken();
      if (token.length() != 0 && RecON == false) {
      RecordON(token);
      }
    pwm_value = pulseIn(PWM_PIN, HIGH);
    }
  }
  if(timeinmem != 0){timeinmem = 0;}
  
  yield();
  }
