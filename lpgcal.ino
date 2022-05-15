  void Calibrate(){
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ2PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");
  }


  
/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE_MQ2*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float RS_AIR_val=0,r0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {                     //take multiple samples
    RS_AIR_val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  RS_AIR_val = RS_AIR_val/CALIBARAION_SAMPLE_TIMES;              //calculate the average value

  r0 = RS_AIR_val/RO_CLEAN_AIR_FACTOR_MQ2;                      //RS_AIR_val divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                                 //according to the chart in the datasheet 

  return r0; 
}

/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function uses different equations representing curves of each gas to 
         calculate the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{ 
  if ( accuracy == 0 ) {
  if ( gas_id == GAS_HYDROGEN ) {
    return (pow(10,((-2.109*(log10(rs_ro_ratio))) + 2.983)));
  } else if ( gas_id == GAS_LPG ) {
    return (pow(10,((-2.123*(log10(rs_ro_ratio))) + 2.758)));
  } else if ( gas_id == GAS_METHANE ) {
    return (pow(10,((-2.622*(log10(rs_ro_ratio))) + 3.635)));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,((-2.955*(log10(rs_ro_ratio))) + 4.457)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,((-2.692*(log10(rs_ro_ratio))) + 3.545)));
  } else if ( gas_id == GAS_SMOKE ) {
    return (pow(10,((-2.331*(log10(rs_ro_ratio))) + 3.596)));
  } else if ( gas_id == GAS_PROPANE ) {
    return (pow(10,((-2.174*(log10(rs_ro_ratio))) + 2.799)));
  }    
} 

  else if ( accuracy == 1 ) {
    if ( gas_id == GAS_HYDROGEN ) {
    return (pow(10,((-2.109*(log10(rs_ro_ratio))) + 2.983)));
  } else if ( gas_id == GAS_LPG ) {
    return (pow(10,((-2.123*(log10(rs_ro_ratio))) + 2.758)));
  } else if ( gas_id == GAS_METHANE ) {
    return (pow(10,((-2.622*(log10(rs_ro_ratio))) + 3.635)));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,((-2.955*(log10(rs_ro_ratio))) + 4.457)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,((-2.692*(log10(rs_ro_ratio))) + 3.545)));
  } else if ( gas_id == GAS_SMOKE ) {
    return (pow(10,(-0.976*pow((log10(rs_ro_ratio)), 2) - 2.018*(log10(rs_ro_ratio)) + 3.617)));
  } else if ( gas_id == GAS_PROPANE ) {
    return (pow(10,((-2.174*(log10(rs_ro_ratio))) + 2.799)));
  }
}    
  return 0;
}
