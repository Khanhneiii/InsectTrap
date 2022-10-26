// Link fastled: https://github.com/FastLED/FastLED/archive/master.zip
#include "HTTP_FIREBASE.h"
#include "MQTT.h"
#include "Light.h"
#include "DHT.h"
#include "Waveshare_SIM7600.h"
#include <Arduino_JSON.h>
#include "cam.h"
#include "fastled.h"
#include "esp_timer.h"
#include "readPIN.h"

#define RAINPIN 14
#define RELAYPIN 4
#define DHTTYPE DHT22
#define DHTPIN 2
#define myID "save"

#define DATA 1
#define IMAGE 2
#define UPDATE 3
#define WAKE_UP 4

String ID = "save";

#define uS_TO_S_FACTOR 1000000ULL

// GPS variable
float voltage , Lat = 0, Log = 0, Time = 0;
String strLat, strLog, strTime, Date;
String hexled = "0xFF0000";
int brightness = 0;
DHT dht(DHTPIN, DHTTYPE);
JSONVar PACKET;
JSONVar GPS;
JSONVar Data;
uint8_t WakeUpTime;
uint8_t SleepTime;
String MQTTMessage = "";
bool isRaining = false;
bool GridStatus = false;
int current_time = 650;

//timer set up
esp_timer_create_args_t periodic_timer_args;
esp_timer_handle_t periodic_timer;
int timer_value = 40;
int wakeup_time = 0;
int sleep_time = 620;
bool is_timer_start = false;

esp_sleep_wakeup_cause_t wakeup_cause;
bool flagsleep = true;

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
  sim7600.GPSPositioning(Lat, Log, Date, Time);
  if (Lat != 0 && Log != 0) {
    strLat = String(Lat, 6);
    GPS["latitude"] = "10.21414";
    strLog = String(Log, 6);
    GPS["longitude"] = "213.2313131";
    strTime = String(Time, 0);
    String hourTime = strTime.substring(0, 1);
    current_time = hourTime.toInt() * 60;
    String minTime = strTime.substring(2, 3);
    current_time += minTime.toInt();
    //GPS["Time"] = strTime;
    //GPS["Date"] = Date;
  }
  Data["coordinates"] = GPS;
  Data["battery"] = String(Energy());
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
  String ledColor = mess.substring(idx1, idx1 + 8);
  hexled = ledColor;

  idx1 = mess.indexOf("brightness");
  String temp = mess.substring(idx1);
  int t_idx = temp.indexOf(':');
  int t_idx2 = temp.indexOf(',');
  String brightness_str = temp.substring(t_idx + 1, t_idx2);
  brightness = brightness_str.toInt() * 100 / 255;
  Config_led(hexled, brightness);

  idx1 = mess.indexOf("timeSend");
  temp = mess.substring(idx1);
  t_idx = temp.indexOf(':');
  t_idx2 = temp.indexOf(',');
  String timeSend = temp.substring(t_idx + 1, t_idx2);
  timer_value = timeSend.toInt() * 60;
  if (is_timer_start) {
    ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
    is_timer_start = false;
  }

  idx1 = mess.indexOf("hour");
  temp = mess.substring(idx1);
  t_idx = temp.indexOf(':');
  t_idx2 = temp.indexOf(',');
  String hourStart = temp.substring(t_idx + 1, t_idx2);
  Serial.println(hourStart);
  idx1 = mess.indexOf("min");
  temp = mess.substring(idx1);
  t_idx = temp.indexOf(':');
  t_idx2 = temp.indexOf('}');
  String minStart = temp.substring(t_idx + 1, t_idx2);
  wakeup_time = hourStart.toInt() * 60 + minStart.toInt();

  int idx_n = mess.indexOf("timeEnd");
  Serial.println(idx_n);
  String temp2 = mess.substring(idx_n);
  Serial.println(temp2);
  idx1 = temp2.indexOf("hour");
  temp2 = temp2.substring(idx1);
  t_idx = temp2.indexOf(':');
  t_idx2 = temp2.indexOf(',');
  String hourEnd = temp2.substring(t_idx + 1, t_idx2);
  Serial.println(hourEnd);
  idx1 = temp2.indexOf("min");
  temp2 = temp2.substring(idx1);
  t_idx = temp2.indexOf(':');
  t_idx2 = temp2.indexOf('}');
  String minEnd = temp2.substring(t_idx + 1, t_idx2);
  Serial.println(minEnd);
  sleep_time = hourEnd.toInt() * 60 + minEnd.toInt();
}



String Data2JSON(String key, String value) {
  return "{\"" + key + "\":\"" + value + "\"}";
}


void periodic_timer_callback(void* arg)
{
    int64_t time_since_boot = esp_timer_get_time();
    printf("Periodic timer called, time since boot: %lld us\n", time_since_boot);
    Get_SendDataSensor();
    if (current_time > sleep_time) flagsleep = true;
}

esp_sleep_wakeup_cause_t sleep() {
  while (Serial.available()) Serial.read();

  esp_sleep_enable_timer_wakeup(timer_value * uS_TO_S_FACTOR);
  delay(100);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_GPIO3);
  gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  // Enter light sleep mode
  esp_err_t check_sleep = esp_light_sleep_start();
  // Restore GPIO3 function as U0RXD
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_U0RXD);
  esp_sleep_wakeup_cause_t wakeup_cause;
  wakeup_cause = esp_sleep_get_wakeup_cause();
  //Serial.println("Wakeup");
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
  periodic_timer_args.callback = &periodic_timer_callback;
            /* name is optional, but may help identify the timer when debugging */
  periodic_timer_args.name = "periodic";
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  PIN_setup();
}


void loop() {
  if  (flagsleep) {
    if (is_timer_start) {
      ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
      is_timer_start = false;
    }
    Serial.println("Sleep");
      wakeup_cause = sleep();
      if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER) {
        Get_SendDataSensor();
        // get time
        // if (gps_time > wakeup_time) flagsleep = false;
        if (current_time > wakeup_time) flagsleep = false;
      }
      if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
        int mess = recieved(MQTTMessage);
        delay(1000);
        Serial.println(MQTTMessage);
        Serial.println(mess);
        delay(3000);
        if (mess == DATA) Get_SendDataSensor();
        else if (mess == IMAGE) post_data(Photo2Base64());
        else if (mess == UPDATE) update_data(MQTTMessage);
      }
    }
  if (!flagsleep) {
    if (!is_timer_start) {
      ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, timer_value * uS_TO_S_FACTOR));
      is_timer_start = true;
    }
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
