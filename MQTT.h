
String Broker = "tcp://test.mosquitto.org:1883";
String SubTopic = "/save";
String client_name = "Sim";
int delay_time = 1500;
int qos = 1;
String POSTDATA = "sendNewDeviceData";
String POSTIMAGE = "sendNewImg";
String UPDATE = "userUpdateDeviceData";

void MQTT_init(){
  Serial.println("AT+CMQTTSTART");
  delay(delay_time);
  
  Serial.println("AT+CMQTTACCQ=0,\""  + client_name + "\"");
  delay(delay_time);

  Serial.println("AT+CMQTTCONNECT=0,\"" + Broker + "\",20,1");
  delay(delay_time);

  Serial.println("AT+CMQTTSUB=0," + String(SubTopic.length()) + "," + String(qos));
  delay(delay_time);

  Serial.println(SubTopic);
  delay(delay_time);
}

void MQTT_deinit() {
  Serial.println("AT+CMQTTUNSUB=0," + String(SubTopic.length()) + "0");
  delay(delay_time);

  Serial.println("AT+CMQTTDISC=0,120");
  delay(delay_time);

  Serial.println("AT+CMQTTREL=0");
  delay(delay_time);

  Serial.println("AT+CMQTTSTOP");
  delay(delay_time);
}

int recieved(String &mess){
  String data = "";
  while (Serial.available() > 0) {
    delay(100);
    data = Serial.readString();
    if (data.indexOf("PAYLOAD") != -1) {
      
      int idx1 = data.indexOf("PAYLOAD");
//      Serial.println(idx1);
      
      String temp1 = data.substring(idx1);
//      Serial.println(temp1);
      
      int idx2 = temp1.indexOf("\n");
//      Serial.println(idx2);
      
      String tempp = temp1.substring(idx2 + 1);
//      Serial.println(tempp);
      
      int idx4 = tempp.indexOf("\n");
//      Serial.println(idx4);
      
      String message = tempp.substring(0,idx4);
      message.remove(message.length() - 1);
      
//      Serial.println(message);
      mess = message;
      if (message.indexOf(POSTDATA)) return 1;
      else if (message.indexOf(POSTIMAGE)) return 2;
      else if (message.indexOf(UPDATE)) return 3;
    }
  }
   return 0;  
}
