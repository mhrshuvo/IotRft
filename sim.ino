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
   GSMSerial.println("AT+CMGS=\"+8801600365372\""); // 1)
  updateSerial();
  GSMSerial.print(msg); // 2) text content 
  updateSerial();
  GSMSerial.write(26); // 3)
}


 void Call(){
   GSMSerial.println("ATD+ +8801600365372;"); //  change ZZ with country code and xxxxxxxxxxx with phone number to dial
   updateSerial();
   delay(20000); // wait for 20 seconds...
   GSMSerial.println("ATH"); //hang up
   updateSerial(); 
  }
