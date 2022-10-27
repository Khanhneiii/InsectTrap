#include "Waveshare_SIM7600.h"
#include "Base64.h"

String URL = "https://inset-catch-electric.herokuapp.com/updateDeviceData";
String stringRecv = "";
bool isRecieved = false;

bool USE_SSL = false;
#define DELAY_MS 4000

void sendATcommand(String ATcommand, String expected_answer, unsigned int timeout) {

    uint8_t x=0,  answer=0;
    String response = "";
    unsigned long previous;
   // Initialize the string
    
    delay(100);
    
    while( Serial.available() > 0) Serial.read();    // Clean the input buffer
    
    Serial.println(ATcommand);    // Send the AT command 


    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(Serial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = Serial.read();      
//            Serial.print(response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (response.indexOf(expected_answer))    
            {
                answer = 1;
            }
        }
         // Waits for the asnwer with time out
    }while((answer == 0) && ((millis() - previous) < timeout));
    
//    Serial.print("\n");   
}

void post_data(String data)
{
//  SimSerial.println("AT");
//  Serial.println(waitResponse());
  delay(DELAY_MS);
  //Start HTTP connection
//  SimSerial.println("AT+HTTPINIT");
//  sendATcommand("AT+HTTPINIT","OK",DELAY_MS);
  Serial.println("AT+HTTPINIT");
  delay(DELAY_MS);
//  Serial.println(waitResponse());
  
  //Enabling SSL 1.0
  if(USE_SSL == true){
//    SimSerial.println("AT+HTTPSSL=1");
//    Serial.println(waitResponse());
  }
  
  Serial.println("AT+HTTPPARA=\"URL\",\"https://inset-catch-electric.herokuapp.com/updateDeviceData\"");
//  sendATcommand("AT+HTTPPARA=\"URL\",\"https://inset-catch-electric.herokuapp.com/updateDeviceData\"","OK",DELAY_MS);
  delay(DELAY_MS);
  Serial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
//  sendATcommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"","OK",DELAY_MS);
  delay(DELAY_MS);
  Serial.println("AT+HTTPDATA=" + String(data.length()) + ",3000");
//String HTTPDATA = "AT+HTTPDATA=" + String(data.length()) + ",10000";
//  sendATcommand(HTTPDATA,"DOWNLOAD",DELAY_MS);
  
  delay(DELAY_MS);
  
  Serial.println(data);
  delay(10000);
  Serial.println("AT+HTTPACTION=1");
//  sendATcommand("AT+HTTPACTION=1","DOWNLOAD",20000);
  delay(15000);

  Serial.println("AT+HTTPTERM");
  delay(DELAY_MS);
}
