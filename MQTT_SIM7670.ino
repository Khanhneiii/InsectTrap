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
String ID = "save";

#define uS_TO_S_FACTOR 1000000ULL 

// GPS variable
float voltage ,Lat = 0, Log = 0, Time = 0;
String strLat, strLog, strTime, Date;
String hexled = "0xFF0000";
int brightness = 0;
DHT dht(DHTPIN, DHTTYPE); 
String path = "/post.json";
JSONVar PACKET;
JSONVar GPS;
JSONVar Data;
uint8_t TIME_TO_SLEEP = 3600;
uint8_t WakeUpTime;
uint8_t SleepTime;
String MQTTMessage = "";
bool isRaining = false;
bool GridStatus = false;

//timer set up
hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;
int timer_value = 0;
int wakeup_time = 0;
int sleep_time = 0;


esp_sleep_wakeup_cause_t wakeup_cause;
bool flagsleep = false;

void Get_SendDataSensor() 
{ 
  PACKET["id"] = ID;
  float h = dht.readHumidity();
  Data["humi"] = 10;
  
  float t = dht.readTemperature();
  Data["temp"] = 15;

  //get image
  Data["img"] = Photo2Base64();
  
  float light = read_light_data();
  Data["optic"] = 14;
  
//  GPS read data
  sim7600.GPSPositioning(Lat,Log,Date,Time);
  if (Lat != 0 && Log !=0) {
    strLat = String(Lat, 6);
    GPS["latitude"] = "10.21414";
    strLog = String(Log, 6);
    GPS["longitude"] = "213.2313131";
    //strTime = String(Time, 0);
    //GPS["Time"] = strTime;
    //GPS["Date"] = Date;
  }
  Data["coordinates"] = GPS;
  Data["rain"] = isRaining;
  Data["statusGrid"] = GridStatus;
  PACKET["Data"] = Data;
   post_data(JSON.stringify(PACKET));
   
}

void SendImage2FB(String mess) {
  JSONVar Image;
  Image["img"] = Photo2Base64();
  JSONVar Data;
  Data["id"] = ID;
  Data["data"] = Image;
  post_data(JSON.stringify(PACKET));
}

void update_data(String mess) {
    int idx = mess.indexOf('\n');
  int idx1 = mess.indexOf("0x");
  String ledColor = mess.substring(idx1,idx1+8);
  hexled = ledColor;
  
  idx1 = mess.indexOf("brightness");
  String temp = mess.substring(idx1);
  int t_idx = temp.indexOf(':');
  int t_idx2 = temp.indexOf(',');
  String brightness_str = temp.substring(t_idx+1,t_idx2);
  brightness = brightness_str.toInt() * 100 / 255;
  Config_led(hexled,brightness);
  
  idx1 = mess.indexOf("timeSend");
   temp = mess.substring(idx1);
   t_idx = temp.indexOf(':');
   t_idx2 = temp.indexOf(',');
  String timeSend = temp.substring(t_idx+1,t_idx2);
  timer_value = timeSend.toInt()*60;
  timerAlarmWrite(timer, timer_value, true);
  
  idx1 = mess.indexOf("hour");
   temp = mess.substring(idx1);
   t_idx = temp.indexOf(':');
   t_idx2 = temp.indexOf(',');
  String hourStart = temp.substring(t_idx+1,t_idx2);
  Serial.println(hourStart);
  idx1 = mess.indexOf("min");
   temp = mess.substring(idx1);
   t_idx = temp.indexOf(':');
   t_idx2 = temp.indexOf('}');
  String minStart = temp.substring(t_idx+1,t_idx2);
  wakeup_time = hourStart.toInt() * 60 + minStart.toInt();

  int idx_n = mess.indexOf("timeEnd");
  Serial.println(idx_n);
  String temp2 = mess.substring(idx_n);
  Serial.println(temp2);
  idx1 = temp2.indexOf("hour");
   temp2 = temp2.substring(idx1);
   t_idx = temp2.indexOf(':');
   t_idx2 = temp2.indexOf(',');
  String hourEnd = temp2.substring(t_idx+1,t_idx2);
  Serial.println(hourEnd);
  idx1 = temp2.indexOf("min");
   temp2 = temp2.substring(idx1);
   t_idx = temp2.indexOf(':');
   t_idx2 = temp2.indexOf('}');
  String minEnd = temp2.substring(t_idx+1,t_idx2);
  Serial.println(minEnd);
  sleep_time = hourEnd.toInt() * 60 + minEnd.toInt();
}



String Data2JSON(String key, String value) {
  return "{\"" + key + "\":\"" + value + "\"}";
}

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  lastIsrAt = millis();
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  Get_SendDataSensor();
  //check time
  // if (time_gps > sleep_time) flagsleep = true;
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
  MQTT_init();
  setup_fastled();
  //sim7600.PowerOn();   
  delay(500);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timer_value, true);
  timerAlarmEnable(timer);
  timerStart(timer);
}


void loop(){
  if  (flagsleep){
    Serial.println("Sleep");
    wakeup_cause = sleep();
    if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER) {
      Get_SendDataSensor();
      // get time 
      // if (gps_time > wakeup_time) flagsleep = false;
    }
    if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
      int mess = recieved(MQTTMessage);
      if (mess == DATA) Get_SendDataSensor();
      else if (mess == IMAGE) post_data(Photo2Base64());
      else if (mess == UPDATE) update_data(MQTTMessage);
      
    }
  }
  if (!flagsleep){
    int mess = recieved(MQTTMessage);
    if (mess == DATA) Get_SendDataSensor();
    else if (mess == IMAGE) post_data(Photo2Base64());
    else if (mess == UPDATE) update_data(MQTTMessage);
    int rain = digitalRead(RAINPIN);
    if (rain == LOW && GridStatus == true) {
      // dang mua
      // tat luoi
      Serial.println("Dang mua");
      digitalWrite(RELAYPIN, LOW);
      isRaining = true;
      GridStatus = false;
       }
    else if (rain == HIGH && GridStatus == false) {
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("Troi khong mua");
      isRaining = false;
      GridStatus = true;
      }
  }
}
