// Link fastled: https://github.com/FastLED/FastLED/archive/master.zip
#include "HTTP_FIREBASE.h"
#include "MQTT.h"
#include "Light.h"
#include "DHT.h"
#include "Waveshare_SIM7600.h"
#include <Arduino_JSON.h>
#include "cam.h"
#include "fastled.h"

#define RAINPIN 14
#define RELAYPIN 4
#define DHTTYPE DHT22
#define DHTPIN 2
#define myID "save"
#define DATA 1
#define IMAGE 2
#define UPDATE 3
#define WAKE_UP 4
#define SLEEP 5

#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  3600 

// GPS variable
float voltage ,Lat = 0, Log = 0, Time = 0;
String strLat, strLog, strTime, Date;
String hexled = "0xFF0000";
DHT dht(DHTPIN, DHTTYPE); 
String path = "/post.json";
JSONVar PACKET;
JSONVar GPS;
JSONVar Data;

String MQTTMessage = "";


esp_sleep_wakeup_cause_t wakeup_cause;
bool flagsleep = false;

void Get_SendDataSensor() 
{ 
  PACKET["id"] = myID;
  float h = dht.readHumidity();
  Data["humi"] = h;
  
  float t = dht.readTemperature();
  Data["temp"] = t;

  //get image
  Data["img"] = Photo2Base64();
  
  float light = read_light_data();
  Data["optic"] = light;
  
//  GPS read data
  sim7600.GPSPositioning(Lat,Log,Date,Time);
  if (Lat != 0 && Log !=0) {
    strLat = String(Lat, 6);
    GPS["latitude"] = strLat;
    strLog = String(Log, 6);
    GPS["longitude"] = strLog;
    strTime = String(Time, 0);
    GPS["Time"] = strTime;
    GPS["Date"] = Date;
  }
  Data["GPS"] = GPS;
}

String Data2JSON(String key, String value) {
  return "{\"" + key + "\":\"" + value + "\"}";
}


esp_sleep_wakeup_cause_t sleep() {
  while (Serial.available()) Serial.read();

     esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
     delay(100);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_GPIO3);
     gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);
     esp_sleep_enable_gpio_wakeup();
     // Enter light sleep mode
     esp_err_t check_sleep = esp_light_sleep_start();
     if (check_sleep != ESP_OK) {
     }
     // Restore GPIO3 function as U0RXD
     PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_U0RXD);
     esp_sleep_wakeup_cause_t wakeup_cause;
     wakeup_cause = esp_sleep_get_wakeup_cause();
     return wakeup_cause;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  camera_init();
  delay(500);
  //MQTT_init();
  setup_fastled();
  //sim7600.PowerOn();   
  delay(500);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);
}


void loop(){
//  if  (flagsleep){
//    Serial.println("Sleep");
//    wakeup_cause = sleep();
//    if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
//      int mess = recieved(MQTTMessage);
//      if (mess == DATA) Get_SendDataSensor();
//      else if (mess == IMAGE) ;
//      else if (mess == RGB_CONFIG) Config_led(hexled);
//      else if (mess == WAKE_UP) flagsleep = false;
//    }
//  }
//  if (!flagsleep){
//    
//    int mess = recieved(MQTTMessage);
//    if (mess == DATA) Get_SendDataSensor();
//    else if (mess == IMAGE) ;
//    else if (mess == RGB_CONFIG) Config_led(MQTTMessage);
//    else if (mess == SLEEP) {
//      flagsleep = true;
//      return;
//    }
    Config_led(hexled);
    Get_SendDataSensor();
    int rain = digitalRead(RAINPIN);
    if (rain == LOW) {
      // dang mua
      // tat luoi
      Serial.println("Dang mua");
      digitalWrite(RELAYPIN, LOW);
      Data["Raining"] = rain;
       }
    else {
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("Troi khong mua");
       Data["Sunny"] = rain;
      }
  PACKET["Data"] = Data;
  post_data(JSON.stringify(PACKET),path);
}
//}
