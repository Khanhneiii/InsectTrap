#include "Waveshare_SIM7600.h"
#include "Base64.h"

String URL = "https://espsim-d13bb-default-rtdb.firebaseio.com";
String key = "3piBtW9aFPfNfbIREVnBnIGEGCPQ90Ss1HUTKAgX";
String stringRecv = "";
bool isRecieved = false;

bool USE_SSL = false;
#define DELAY_MS 1500

void post_data(String data,String data_path)
{
//  SimSerial.println("AT");
  Serial.println("AT");
//  Serial.println(waitResponse());
  delay(DELAY_MS);
  //Start HTTP connection
//  SimSerial.println("AT+HTTPINIT");
  Serial.println("AT+HTTPINIT");
//  Serial.println(waitResponse());
  delay(DELAY_MS);
  
  //Enabling SSL 1.0
  if(USE_SSL == true){
//    SimSerial.println("AT+HTTPSSL=1");
    Serial.println("AT+HTTPSSL=1");
//    Serial.println(waitResponse());
    delay(DELAY_MS);
  }
  
  Serial.println("AT+HTTPPARA=\"URL\"," "\"" +URL+data_path+"?auth="+key + "\"");
  delay(DELAY_MS);
  Serial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  delay(DELAY_MS);
  Serial.println("AT+HTTPDATA=" + String(data.length()) + ",3000");
  
  delay(DELAY_MS);
  
  Serial.println(data);
  delay(DELAY_MS);
  Serial.println("AT+HTTPACTION=1");
  delay(3000);

  Serial.println("AT+HTTPTERM");
  delay(DELAY_MS);
}

void post_image(String data,String image_path)
{
//  SimSerial.println("AT");
  Serial.println("AT");
//  Serial.println(waitResponse());
  delay(DELAY_MS);
  //Start HTTP connection
//  SimSerial.println("AT+HTTPINIT");
  Serial.println("AT+HTTPINIT");
//  Serial.println(waitResponse());
  delay(DELAY_MS);
  
  //Enabling SSL 1.0
  if(USE_SSL == true){
//    SimSerial.println("AT+HTTPSSL=1");
    Serial.println("AT+HTTPSSL=1");
//    Serial.println(waitResponse());
    delay(DELAY_MS);
  }
  
  Serial.println("AT+HTTPPARA=\"URL\"," "\"" +URL+image_path+"?auth="+key + "\"");
  delay(DELAY_MS);
  Serial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  delay(DELAY_MS);
  Serial.println("AT+HTTPDATA=" + String(data.length()+1) + ",5000");
  
  delay(DELAY_MS);
  
  Serial.println(data);
  delay(DELAY_MS);
  Serial.println("AT+HTTPACTION=1");
  delay(10000);

  Serial.println("AT+HTTPTERM");
  delay(DELAY_MS);
}
