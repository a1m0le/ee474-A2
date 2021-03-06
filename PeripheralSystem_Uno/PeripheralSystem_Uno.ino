#define BASE_TEN_BASE 10
#define PR_PIN_IN 2
#define RR_PIN_IN 2
#define DELAY_TIME 2
#define BP_PIN_IN 3
#define TEMP_INPUT A5
#define SWITCH_IN 7

// function headers
void setup();
void taskDispatcher(byte task, byte subtask);
void writeBack(char* data, char count);

// For measurement
unsigned int temperature(unsigned int data);
unsigned int systolicPress(unsigned int data);
unsigned int diastolicPress(unsigned int data);
unsigned int pulseRate(unsigned int data);
unsigned int respRate(unsigned int data);
unsigned short statusCheck(unsigned short data);

// For compute
double tempCorrected(unsigned int data);
unsigned int sysCorrected(unsigned int data);
double diasCorrected(unsigned int data);
unsigned int prCorrected(unsigned int data);
unsigned int rrCorrected(unsigned int data);

// For alarm
char tempRange(unsigned int data);
char sysRange(unsigned int data);
char diasRange(unsigned int data);
char prRange(unsigned int data);
char rrRange(unsigned int data);

// For warning
char tempHigh(unsigned int data);
char sysHigh(unsigned int data);
char diasHigh(unsigned int data);
char prHigh(unsigned int data);
char rrHigh(unsigned int data);

// Function generator variables
volatile byte PRcount = 0;
volatile byte RRcount = 0;

// Blood Pressure Variables; 
unsigned int BPTimeOut = 0;
unsigned int BPFinished = 0;
unsigned long BPTime = 0;
double BPcount = 90.0;
unsigned int BPFlag = 1;
unsigned int sysMeasure = 1;
unsigned int diasMeasure = 0;

/******************************************
* Function Name: setup
* Function Inputs: None
* Function Outputs: None
* Function Description: Start the serial port and
*            set the global variables
*           to appropriate values 
*
*
* Author: Matt, Michael, Eun Tae
******************************************/
void setup()
{
  // running on the uno - connect to tx1 and rx1 on the mega and to rx and tx on the uno
  // start serial port at 9600 bps and wait for serial port on the uno to open:
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(RR_PIN_IN), isrRR, RISING);
  detachInterrupt(digitalPinToInterrupt(RR_PIN_IN));
  attachInterrupt(digitalPinToInterrupt(PR_PIN_IN), isrPR, RISING);
  detachInterrupt(digitalPinToInterrupt(PR_PIN_IN));
  attachInterrupt(digitalPinToInterrupt(BP_PIN_IN), isrBP, FALLING);
  detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
  pinMode(PR_PIN_IN, INPUT_PULLUP); 
  pinMode(RR_PIN_IN, INPUT_PULLUP);
  pinMode(BP_PIN_IN, INPUT_PULLUP);
  pinMode(SWITCH_IN, INPUT); 
}

/******************************************
* Function Name: loop
* Function Inputs: none
* Function Outputs: none
* Function Description: Waits until Mega writes 2 task bytes
*           First read from the Serial will be 
*           the task (Measure,Compute), and second
*           byte will be the type of task.
*           Based on the received byte,
*           dispatch appropriate function.
* Author: Matt, Michael, Eun Tae
******************************************/
void loop()
{
  while(Serial.available()<2) {
    // Just Wait 
  }
  byte task = Serial.read();
  byte subtask = Serial.read();
  taskDispatcher(task, subtask);
}

/******************************************
* Function Name: taskDispatcher
* Function Inputs: bytes respresenting task and subtask
*          respectively
* Function Outputs: None
* Function Description: Based on the inputs,
*             it will read the next necessary
*           data from Serial.
*           Afterward, it will call appropriate
*           function - measure, compute, alarm,
*           warning, status
* Author: Matt, Michael, Eun Tae
******************************************/
void taskDispatcher(byte task,  byte subtask){
  unsigned int dataIntType;
  double dataDoubleType;
  unsigned short dataShortType;
  unsigned int returnIntDump;
  double returnDoubleDump;
  unsigned short returnShortDump;
  char returnCharDump; 
  switch(task) {  
    case 1:                                             // Case 1: Measure the data
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
      switch(subtask){ 
        case 1:                                         // Case 1: temperatureRaw
          returnIntDump = temperature(dataIntType);
          break;
        case 2:                                         // Case 2: systolicPressRaw    
          returnIntDump = sysPressure(dataIntType);
          break;
        case 3:                                         // Case 3: diastolicCaseRaw
          returnIntDump = diasPressure(dataIntType);
          break;  
        case 4:                                         // Case 4: pulseRateRaw
          returnIntDump = pulseRate();
          break; 
        case 5:                                         // Case 5: respirationRateRaw
          returnIntDump = respRate();
          break;
      }
      writeBack((char*)&returnIntDump, sizeof(unsigned int));
      break; 
    case 2:                                             // Case 2: Compute the data
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
      switch(subtask){                                  
        case 1:                                         // Case 1: tempCorrected
          returnDoubleDump = tempCorrected(dataIntType);
          writeBack((char*)&returnDoubleDump, sizeof(double));
          break;
        case 2:                                         // Case 2: sysCorrected 
          returnIntDump = sysCorrected(dataIntType); 
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break; 
        case 3:                                         // Case 3: diasCorrected
          returnDoubleDump = diasCorrected(dataIntType); 
          writeBack((char*)&returnDoubleDump, sizeof(double)); 
          break; 
        case 4:                                         // Case 4: prCorrected
          returnIntDump = prCorrected(dataIntType); 
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break;
        case 5:                                         // Case 5: rrCorrected
          returnIntDump = rrCorrected(dataIntType);
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break; 
      }
      break;
    case 3:                                             // Case 3: Alarm if out of range
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
        switch(subtask){                                
          case 1:                                       // Case 1: tempAlarm
            returnCharDump = tempRange(dataIntType);
            break;
          case 2:                                       // Case 2: sysAlarm                 
            returnCharDump = sysRange(dataIntType);
            break; 
          case 3:                                       // Case 3: diasAlarm
            returnCharDump = diasRange(dataIntType);
            break; 
          case 4:                                       // Case 4: prAlarm
            returnCharDump = prRange(dataIntType);
            break; 
          case 5:                                       // Case 5: rrAlarm
            returnCharDump = rrRange(dataIntType);
            break;
        }
        writeBack(&returnCharDump, sizeof(char)); 
        break; 
    case 4:                                             // Case 4: Warning if high
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
        switch(subtask){                                
          case 1:                                       // Case 1: tempWarning
            returnCharDump = tempHigh(dataIntType);
            break;
          case 2:                                       // Case 2: sysWarning              
            returnCharDump = sysHigh(dataIntType);
            break; 
          case 3:                                       // Case 3: diasWarning
            returnCharDump = diasHigh(dataIntType);
            break; 
          case 4:                                       // Case 4: prWarning
            returnCharDump = prHigh(dataIntType);
            break; 
          case 5:                                       // Case 5: rrWarning
            returnCharDump = rrHigh(dataIntType);
            break;
        }
        writeBack(&returnCharDump, sizeof(char)); 
        break;  
     // Case 5: Status of battery
     case 5: 
          Serial.readBytes((char*)&dataShortType, sizeof(unsigned short));
          returnShortDump = statusCheck(dataShortType);
          writeBack((char*)&returnShortDump, sizeof(short)); 
          break;
  }
}

/******************************************
* Function Name: temperature
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the temperature
*           based on the voltage reading from the TEMP_INPUT port
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int temperature(unsigned int data)
{
  data = map(analogRead(TEMP_INPUT), 0, 1023, 40, 50);
  return data;
}

/*******************************
 * Function Name:        isrBP
 * Function Inputs:      None
 * Function Outputs:     Increment BPcount or decrement based on the switch
 * Function Description: The interrupt service routine on 
 *                       positive edge. Counts the every beat on 
 *                       pulse rate.
 *                       Update the time isr was called
 * Author: Matt, Michael, Eun Tae
 ************************************/
void isrBP() {
  BPTime = millis();
  if (BPFlag)
    BPcount = 1.1 * BPcount; 
  else
    BPcount = 0.9 * BPcount;
}

/******************************************
* Function Name: sysPressure
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the systolic
*           pressure for each button press
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int sysPressure(unsigned int data) {
  BPTimeOut = millis();
  BPTime = millis();
  BPFinished = 0;
  while(!BPFinished){
    attachInterrupt(digitalPinToInterrupt(BP_PIN_IN), isrBP, FALLING);
    if(BPcount <= 150 && BPcount >= 110) {
      data = (unsigned int)floor(BPcount);
      BPFinished = 1;
      BPFlag = 0;
    } else if (millis() - BPTimeOut > 10000) {
      BPFinished = 1;
    }
    detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
  }
  return data;
}

/******************************************
* Function Name: diasPressure
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the systolic
*           pressure for each button press
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int diasPressure(unsigned int data) {
  BPTimeOut = millis();
  BPTime = millis();
  BPFinished = 0;
  while(!BPFinished){
    attachInterrupt(digitalPinToInterrupt(BP_PIN_IN), isrBP, FALLING);
    if(BPcount <= 80 && BPcount >= 50) {
      data = (unsigned int)floor(BPcount);
      BPFinished = 1;
      BPFlag = 1;
    } else if (millis() - BPTimeOut > 10000) {
      BPFinished = 1;
    }
    detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
  }
  BPcount = 90.0;
  return data;
}


/*
unsigned int bloodPressure(unsigned int data) {
  attachInterrupt(digitalPinToInterrupt(BP_PIN_IN), isrBP, FALLING);
  BPFinished = 0;
  while(!BPFinished){ 
    if(BPcount <= 150.0 && BPcount >= 110.0 && sysMeasure) {
      detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
      BPFinished = 1;
      sysMeasure = 0;
      diasMeasure = 1;
      data = (unsigned int)floor(BPcount);
    } 
    else if (BPcount <= 80.0 && BPcount >= 50.0 && diasMeasure){
      detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
      BPFinished = 1;
      sysMeasure = 1;
      diasMeasure = 0;
      data = (unsigned int) floor(BPcount);
      BPcount = 90.0;
    }
    return BPcount;
  //  else if ((millis() - BPTime) >= 50000) { // Button Call Check
  //    BPFinished = 1;
  //  } 
//    else if (millis() - BPTimeOut >= 10000) { // Function Call Check
//      Serial.write(252);
//      BPFinished = 1;
//    }
  }
  return data;
}
*/
/*******************************
 * Function Name:        isrPR
 * Function Inputs:      none
 * Function Outputs:     increment counter
 * Function Description: The interrupt service routine on 
 *                       positive edge. Counts the every beat on 
 *                       pulse rate.
 * Author: Matt, Michael, Eun Tae
 ************************************/
void isrPR() { 
  PRcount++;
}

/******************************************
* Function Name: pulseRate
* Function Inputs: None
* Function Outputs: Integer of processed data
* Function Description: Measure the function generator's 
*                       input voltage using the interrupt 
*                       and convert it to an appropriate value.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int pulseRate() {
  attachInterrupt(digitalPinToInterrupt(PR_PIN_IN), isrPR, RISING);
  delay(1000 * DELAY_TIME);
  unsigned int pulseRateData = (60000 /(1000 * DELAY_TIME) * (PRcount));
  PRcount = 0;
  detachInterrupt(digitalPinToInterrupt(PR_PIN_IN));
  return pulseRateData;
}

/*******************************
 * Function Name:        isrRR
 * Function Inputs:      none
 * Function Outputs:     increment counter
 * Function Description: The interrupt service routine on 
 *                       positive edge. Counts the every beat on 
 *                       respiration rate.
 * Author: Matt, Michael, Eun Tae                       
 ************************************/
void isrRR() {
  RRcount++;
}

/******************************************
* Function Name: respRate
* Function Inputs: None
* Function Outputs: Integer of processed data
* Function Description: Measure the function generator's 
*                       input voltage using the interrupt 
*                       and convert it to an appropriate value.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int respRate() {
  attachInterrupt(digitalPinToInterrupt(RR_PIN_IN), isrRR, RISING);
  delay(1000 * DELAY_TIME);
  unsigned int respRateData = (60000 /(1000 * DELAY_TIME) * (RRcount)) - 20;
  RRcount = 0;
  detachInterrupt(digitalPinToInterrupt(RR_PIN_IN));
  return respRateData;
}

/******************************************
* Function Name: tempCorrected
* Function Inputs: Integer of raw data
* Function Outputs: Double of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in Celsius
* Author: Matt, Michael, Eun Tae
******************************************/
double tempCorrected(unsigned int data) {
  double dataCorrected = 5 + (0.75 * data);
  return dataCorrected;
}

/******************************************
* Function Name: sysCorrected
* Function Inputs: Integer of raw data
* Function Outputs: Double of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int sysCorrected(unsigned int data) {
  unsigned int dataCorrected = 9 + (2 * data);
  return dataCorrected;
}

/******************************************
* Function Name: diasCorrected
* Function Inputs: Integer of raw data
* Function Outputs: Double of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
double diasCorrected(unsigned int data) {
  double dataCorrected = 6 + (1.5 * data);
  return dataCorrected;
}

/******************************************
* Function Name: prCorrected
* Function Inputs: Integer of raw data
* Function Outputs: int of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in BPM
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int prCorrected(unsigned int data) {
  unsigned int dataCorrected = 8 + (3 * data);
  return dataCorrected;
}

/******************************************
* Function Name: rrCorrected
* Function Inputs: Integer of raw data
* Function Outputs: int of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in breath per minute
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int rrCorrected(unsigned int data) {
  unsigned int dataCorrected = 7 + (3 * data);
  return dataCorrected;
}

/******************************************
* Function Name: tempRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input temperature
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char tempRange(unsigned int data) {                 
  char result = 1; 
  double dataCorrected = 5 + (0.75 * data);
  if (dataCorrected >= 36.1 && dataCorrected <= 37.8) { 
    result = 0; 
  }
  if (dataCorrected < 34.295 || dataCorrected > 39.69){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: sysRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input systolic
*           pressure is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char sysRange(unsigned int data) {                  
  char result = 1;
  unsigned int dataCorrected = 9 + (2 * data); 
  if (dataCorrected >= 120 && dataCorrected <=130) { 
    result = 0; 
  } 
  if (dataCorrected <  114 || dataCorrected > 136.5){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: diasRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input diastolic
*           pressure is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char diasRange(unsigned int data) {                 
  char result = 1;
  double dataCorrected = 6 + (1.5 * data); 
  if (dataCorrected >= 70 && dataCorrected <=80) { 
    result = 0; 
  } 
  if (dataCorrected < 66.5 || dataCorrected > 84){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: prRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input pulse rate
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char prRange(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (3 * data);
  if (dataCorrected >= 60 && dataCorrected <= 100) { 
    result = 0; 
  }
  if (dataCorrected < 57 || dataCorrected > 105){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: rrRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input breath per minute data
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char rrRange(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 7 + (3 * data);
  if (dataCorrected >= 12 && dataCorrected <= 25) { 
    result = 0; 
  }
  if (dataCorrected < 11.4 || dataCorrected > 26.25){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: tempHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input temperature
*             is above 37.8 Celsius
* Author: Matt, Michael, Eun Tae
******************************************/
char tempHigh(unsigned int data) {                  
  char result = 1; 
  double dataCorrected = 5 + (0.75 * data);
  if (dataCorrected <= 37.8) { 
    result = 0; 
  }
  return result; 
} 

/******************************************
* Function Name: sysHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input systolic
*             pressure is above 120 mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
char sysHigh(unsigned int data) {                  
  char result = 1;
  unsigned int dataCorrected = 9 + (2 * data); 
  if (dataCorrected <= 156) { 
    result = 0; 
  } 
  return result; 
} 

/******************************************
* Function Name: diasHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input diastolic
*             pressure is above 80 mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
char diasHigh(unsigned int data) {                 
  char result = 1;
  double dataCorrected = 6 + (1.5 * data); 
  if (dataCorrected <= 80) { 
    result = 0; 
  } 
  return result; 
} 

/******************************************
* Function Name: prHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input pulse
*             rate is above 100 BPM
* Author: Matt, Michael, Eun Tae
******************************************/
char prHigh(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (3 * data);
  if (dataCorrected <= 100) { 
    result = 0; 
  }
  return result; 
} 

/******************************************
* Function Name: rrHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input data
*             is above 25 breath per minute
* Author: Matt, Michael, Eun Tae
******************************************/
char rrHigh(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (7 * data);
  if (dataCorrected <= 100) { 
    result = 0; 
  }
  return result; 
} 

/******************************************
* Function Name: statusCheck
* Function Inputs: unsigned short data
* Function Outputs: unsinged short noting battery status
* Function Description: Checks whether given input systolic
*             pressure is above 120 mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned short statusCheck(unsigned short data) { 
  return --data; 
}

/******************************************
* Function Name: writeBack
* Function Inputs: string of data and char, count
* Function Outputs: none
* Function Description: writes the array of data
            input into Serial for Mega
            to take in.
* Author: Matt, Michael, Eun Tae
******************************************/
void writeBack(char* data, char count){
    for (char i = 0; i < count; i++){
      Serial.write(data[i]);
    }
}

//  end of EE 474 code
