#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#define DHTTYPE DHT11   // DHT 11
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#define         MQ2PIN                       (A0)     //define which analog input channel you are going to use
#define         RL_VALUE_MQ2                 (1)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR_MQ2      (9.577)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation

/**********************Application Related Macros**********************************/
#define         GAS_HYDROGEN                  (0)
#define         GAS_LPG                       (1)
#define         GAS_METHANE                   (2)
#define         GAS_CARBON_MONOXIDE           (3)
#define         GAS_ALCOHOL                   (4)
#define         GAS_SMOKE                     (5)
#define         GAS_PROPANE                   (6)
#define         accuracy                      (0)   //for linearcurves
//#define         accuracy                    (1)   //for nonlinearcurves, un comment this line and comment the above line if calculations 
                                                    //are to be done using non linear curve equations
/*****************************Globals************************************************/
float           Ro = 0;

const char* ssid = "Rafti";
const char* password = "Rafti1234";

//https://api.telegram.org/bot5235051335:AAEYvaM90WJlriuDB1nuuVIPyq-oS37Blqk/getUpdates

#define BOTtoken "5235051335:AAEYvaM90WJlriuDB1nuuVIPyq-oS37Blqk" 
#define CHAT_ID "-1001573046381" //-1001573046381
#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

//sim800l TXD to esp8266 D7
//sim800l RXD to esp8266 D8
SoftwareSerial GSMSerial(D7,D8);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 500;
unsigned long lastTimeBotRan;

#define dht_dpin 2
DHT dht(dht_dpin, DHTTYPE); 

const int flame = D0;
const int buzz = D1; 
int ledpin = 4; 
int button = 0;
int buttonState=0;

void setup(void)
{ 
  dht.begin();
  pinMode(flame,INPUT);
  pinMode(buzz,OUTPUT);
  pinMode(ledpin, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(ledpin, HIGH);
  Serial.begin(9600);
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  GSMSerial.begin(9600);
  SIMInitialize();
  Calibrate();
  delay(2000);
  
}
void loop() {
   updateSerial();
    Serial.println(dht.readTemperature());
    Serial.println(dht.readHumidity());
    Serial.println(digitalRead(flame));
    Serial.println(MQGetGasPercentage(MQRead(MQ2PIN)/Ro,GAS_LPG));
    
   if(digitalRead(flame) == 0){
        alarm();
        digitalWrite(ledpin, LOW);
        bot.sendMessage(CHAT_ID, "Flame is detected", "");
        Call();
        updateSerial();
        SendSMS("Flame deected fire. fire. In Floor 1, Need Help.Fire : https://maps.google.com/?q=23.8883,90.3907"); 
        updateSerial();
        
   }
  
    if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("numNewMessages");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

//  bot.sendMessage(CHAT_ID, "temp = " +String(dht.readTemperature()) +" * c", "");
  
//  if(dht.readTemperature() > 45 && dht.readHumidity() < 40 ){
//    SendSMS("Temp is higher then 45 and humidity is less then 40");
//      bot.sendMessage(CHAT_ID, "Temp is higher then 45 and humidity is less then 40", "");
//    if(MQGetGasPercentage(MQRead(MQ2PIN)/Ro,GAS_LPG) > 150){
//      SendSMS("3rd stage Smoke deected");
//       bot.sendMessage(CHAT_ID, "3rd stage Smoke deected", "");
//      if(digitalRead(flame) == 0){
//        tone(buzz, 1000, 5000);
//        SendSMS("4th stage flame deected fire! fire!");
//        bot.sendMessage(CHAT_ID, "4th stage flame deected fire! fire!", "");
//      }
//    }
//  }
buttonState=digitalRead(button); // put your main code here, to run repeatedly:
 if (buttonState == 1)
 {
 digitalWrite(ledpin, HIGH); 
 delay(200);
 }
 if (buttonState==0)
 { digitalWrite(ledpin,LOW); 
 delay(200);
 }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command to get current readings.\n\n";
      welcome += "/readings \n";
      welcome += "/temperature \n";
      welcome += "/humidity \n\n";
      welcome += "/gas \n";
      welcome += "Developed by \n Sumaia Akter Rafti \t 18203011 Tajrian Jahan Mou \n 18103367 \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/readings") {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }  
    if (text == "/temperature") {
      String readings = getTemperature();
      bot.sendMessage(chat_id, readings, "");
    }  
    if (text == "/humidity") {
      String readings = getHumidity();
      bot.sendMessage(chat_id, readings, "");
    }  
    if (text == "/gas") {
      String readings = getGas();
      bot.sendMessage(chat_id, readings, "");
    }
    if (text == "/flame") {
      String readings = getFlame();
      bot.sendMessage(chat_id, readings, "");
    }
    
  }
}

String getTemperature(){
  float temperature;
  temperature = dht.readTemperature();
  String message = "Temperature: " + String(temperature) + " ºC \n";
  return message;
}
String getHumidity(){
  float humidity;
  humidity = dht.readHumidity();
  String message ="Humidity: " + String (humidity) + " % \n";
  return message;
}
String getGas(){
  float gas;
  gas = MQGetGasPercentage(MQRead(MQ2PIN)/Ro,GAS_LPG);
  String message ="Gas: " + String (gas) + "\n";
  return message;
}
String getFlame(){
  int flame = digitalRead(flame);
  if (flame==0) {      
      //tone(pin, frequency, duration)     
      //tone(buzz, 1000, 5000);
      String message ="Flame ditected";
  return message;
   }
   else{
     //noTone(buzz);
     String message ="There is no flame";
     return message;
   }
}
String getReadings(){
  String message = String(getTemperature()) + "\n";
  message += String (getHumidity()) + "\n";
  return message;
}


void updateSerial() {
  delay(500);
  while (Serial.available()) {
   GSMSerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }

  while (GSMSerial.available()) {
    Serial.write(GSMSerial.read());//Forward what Software Serial received to Serial Port
  }
}
