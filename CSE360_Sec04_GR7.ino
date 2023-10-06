#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <SPI.h>
#include <SD.h>
//#define MFRC_RST_PIN  9    
//#define MFRC_SS_PIN   10
#define REPORTING_PERIOD_MS  1500

PulseOximeter pox;
uint32_t tsLastReport = 0;
float b=0;
float a=0;
float bp=0;
float sp=0;
MFRC522 mfrc522(10, 9);   // Create MFRC522 instance
LiquidCrystal_I2C lcd(0x27,16,2); 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
File myFile;

void onBeatDetected() {
    Serial.println("Detected!");
}
void inference(){
  //lcd.init();
  lcd.clear();
  lcd.setCursor(0,0);
  if(b<=37.5){
    Serial.println("Normal Temp");
    lcd.print("Normal Temp");
  } 
  if(b>37.5 && b<39){
    Serial.println("Warmer Temp");
    lcd.print("Warmer Temp");
  } 
  if(b>=39){
    lcd.print("High Temp");
    Serial.println("High Temp");
  } 
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  if(bp<=83){
    lcd.print("Normal Bpm");
    Serial.println("Normal Bpm");
  } 
  if(bp>83 && bp<93){
    lcd.print("Elevated Bpm");
    Serial.println("Elevated Bpm");
  } 
  if(bp>=93){
    lcd.print("High Bpm");
    Serial.println("High Bpm");
  } 
  lcd.setCursor(0,1);
  if(sp<=92){
    lcd.print("Lower SPo2");
    Serial.println("Lower SPo2");
  } 
  if(sp>92 && sp<97){
    lcd.print("Decent SPo2");
    Serial.println("Decent SPo2");
  } 
  if(sp>=97){
    lcd.print("Great SPo2");
    Serial.println("Great SPo2");
  } 
  delay(3000);
}
void pox_loop(){
  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  int i=0;
  bp=0;
  sp=0;
  //int j=0;
  while(1){
    //++j; 
    pox.update();
    // Grab the updated heart rate and SpO2 levels
    if ((millis() - tsLastReport > REPORTING_PERIOD_MS) && pox.getHeartRate()>60 && pox.getSpO2()>88 && pox.getSpO2()<100) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Heart rate:");
        lcd.setCursor(11,0);
        lcd.print(pox.getHeartRate());
        lcd.setCursor(0,1);
        lcd.print("SpO2");
        lcd.setCursor(10,1);
        lcd.print(":");
        lcd.setCursor(11,1);
        lcd.print(pox.getSpO2());
        lcd.setCursor(14,1);
        lcd.print("%");
        bp+=pox.getHeartRate();
        sp+=pox.getSpO2();
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print(" SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        tsLastReport = millis();
        ++i;
    }
    if(i==5){
      bp/=5;
      sp/=5;
      break;
    }
  };
  //Serial.println(j);
}

void temp_loop(){
  if (!mlx.begin()) {
      lcd.clear();
      lcd.print("MLX Lagao");
      while (1);
  };
  b = mlx.readObjectTempC();
  a = mlx.readAmbientTempC();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Body Temp: ");
  Serial.print("Body Temp:");
  lcd.setCursor(11,0);
  lcd.print(b);
  Serial.print(b);
  lcd.setCursor(0,1);
  lcd.print("Ambient  : ");
  Serial.print("Ambient: ");
  lcd.setCursor(11,1);
  lcd.print(a);
  Serial.println(a);
  delay(1500);
}
void setup() {
    Serial.begin(9600); 
    SPI.begin();   
    mfrc522.PCD_Init();   
    lcd.init();
    lcd.clear();         
    lcd.backlight();
    lcd.clear();
    MFRC522::MIFARE_Key key;
    for (byte i=0;i<6;i++) key.keyByte[i] = 0xFF;
    byte block;
    byte len;
    MFRC522::StatusCode status;
}
void loop() {
  // put your main code here, to run repeatedly:
  MFRC522::MIFARE_Key key;
  MFRC522::StatusCode status;
  for (byte i=0;i<6;i++) key.keyByte[i] = 0xFF;
  //some variables we need
  byte block;
  byte len;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CSE360 Group-7");
  while(!mfrc522.PICC_IsNewCardPresent()) {
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.println(F("Card Detected:"));
  SD.begin(4);   
  myFile = SD.open("data.txt", FILE_WRITE);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Card Detected");
  delay(1000);
  byte buffer1[18];
  block = 4;
  len = 18;
  byte buffer2[18];
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  lcd.clear();
  for (uint8_t i = 0; i < 16; i++){
    if (buffer1[i] != 32){
      Serial.write(buffer1[i]);
      if(i!=0){
        lcd.setCursor(i-1,0);
        lcd.write(buffer1[i]);
        myFile.write(buffer1[i]);
      }
    }
  }
  Serial.print(" ");
  myFile.write(" ");
  block = 1;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
//  for (uint8_t i = 0; i < 16; i++) {
//    Serial.write(buffer2[i] );
//    lcd.setCursor(i,1);
//    lcd.write(buffer2[i]);
//    myFile.write(buffer2[i]);
//  }
  lcd.setCursor(0,1);
  lcd.print("Welcome!");
  delay(3000);
  temp_loop(); 
  pox_loop();
  myFile.write("  Body Temp: ");
  myFile.print(b);
  myFile.write("  Ambient Temp: ");
  myFile.print(a);
  myFile.write("  bpm: ");
  myFile.print(bp);
  myFile.write("  SPo2: ");
  myFile.println(sp);
  Serial.println(b);
  Serial.println(a);
  Serial.println(bp);
  Serial.println(sp);
  myFile.close();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(2000);
  //inference();
  Serial.println(F("\n**End Reading**\n"));
}
