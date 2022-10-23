
#define POTI_PIN 12


long PotiValue ;
float giatriINra,giatriPhanTram,giatriTB;
void setup() {
  Serial.begin(115200);
  pinMode(POTI_PIN,INPUT);

}

float Energy() {
  PotiValue = analogRead(POTI_PIN);
  for( int i =0 ; i< 10; i++){
      PotiValue += analogRead(POTI_PIN);
  }
  giatriTB = PotiValue/10;
  
  if(giatriTB >= 1703 && giatriTB <= 2751){
    giatriPhanTram = float(((giatriTB - 1703)*100)/1048);
  }
  else if(giatriTB < 1703){
    giatriPhanTram = 0;
  }
  else  {
     giatriPhanTram = 100;
  }
  
  return giatriPhanTram;
}
