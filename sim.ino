  void SIMInitialize(){
   Serial.println("Initializing...");
   GSMSerial.println("AT");
   updateSerial();
   GSMSerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
   updateSerial();
   GSMSerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
   updateSerial();
   GSMSerial.println("AT+CREG?"); //Check whether it has registered in the network
   updateSerial();
   GSMSerial.println("AT+CMEE=1"); //Check whether it has registered in the network
   updateSerial();
 // Configuring module in TEXT mode
   GSMSerial.println("AT+CMGF=1");
   updateSerial();

  }

   void SendSMS(String msg)
{
  GSMSerial.print("AT+CMGS=\"+8801939261025\"");  //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  GSMSerial.print(msg);       //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  GSMSerial.print((char)26);// (required according to the datasheet)
  delay(500);
  GSMSerial.println();
  Serial.println("Text Sent." + msg);
  delay(500);
}

 void Call(){
   GSMSerial.println("ATD+ +8801799410877;"); //  change ZZ with country code and xxxxxxxxxxx with phone number to dial
   updateSerial();
   delay(20000); // wait for 20 seconds...
   GSMSerial.println("ATH"); //hang up
   updateSerial();
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
