/*
 * this revision:
 * this code adds the outputs for the email signals
 * 
 * 
 * date of last revision: 6/28/2024
 */
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

//#define DEBUG_D1
//#define DEBUG_D2
//#define DEBUG_CURRENT_TRANSFORMER
//#define showTimers

#define max_Humidity 50
#define Humidity_offset 1
#define Temp_offset 0
#define functional_Dehumidifier_Temp 60
#define humidity_deadband 5
#define DHT_TYPE DHT11 // DHT sensor
#define DHT_PIN 12     // the temperature and humidity sensor
#define DHT_sensor_Delay 2000
#define defumidifier_1_Pin 5
#define defumidifier_2_Pin 4
#define dehumidifier_Status_pin A0
#define RELAY_ON 0
#define RELAY_OFF 1
#define sampleSize 1000    // the sample size for the current transformer, in milliseconds
#define minimumTransformerValue 2.250   // this is the voltage value that will be used to tell when the dehumidifier has turned off
#define one_second 1000

#define D1_Fault_delay  30000  // 30 seconds in millis
#define min_Runtime  1800000 // 30 minutes in millis
#define min_Offtime  300000 // 5 minutes in millis

#define D1_faultPin 9
#define HH_faultPin 10

/*
#define D1_Fault_delay  5000  // for diagnostics
#define min_Runtime  120000 // for diagnostics
#define min_Offtime  30000 // for diagnostics
*/

unsigned long secondsTimer = 0;   // this is the value used to keep the timer print function from running unnecessarily
unsigned long D1_Fault_Timer = 0;
unsigned long DHT_Timer = 0;
unsigned long defumidifier_1_runtime = 0;
unsigned long defumidifier_2_runtime = 0;
unsigned long defumidifier_1_offtime = 0;
unsigned long defumidifier_2_offtime = 0;

float currentHumidity = 0;
float fahrenheitVal = 0;

int overDrive_Humidity;
int Dehumidifier_STATUS;
int currentMax_amps;
int count_var = 0;

boolean D1_fault = 0;
boolean tooCold = 0;
boolean D1_firstPass = 1;
boolean D2_firstPass = 1;
boolean D1_outputStatus = 0;
boolean D2_outputStatus = 0;
boolean tooHumid = 0;
boolean D1_MinOffTimersDone = 0;
boolean D1_offHumidityLvl = 0;
boolean D1_MinRuntimersDone = 0;
boolean extraHumid = 0;
boolean D2_MinOffTimersDone = 0;
boolean D2_offHumidityLvl = 0;
boolean D2_MinRuntimersDone = 0;
boolean notSeeingVoltage = 0;
boolean FaultTimer_ranOut = 0;

float transformer_VPP;   // Voltage output of current transformer

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  overDrive_Humidity = max_Humidity + humidity_deadband;
  dht.begin();
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  pinMode(defumidifier_1_Pin, OUTPUT);
  pinMode(defumidifier_2_Pin, OUTPUT);
  pinMode(dehumidifier_Status_pin, INPUT);
  pinMode(D1_faultPin, OUTPUT);
  pinMode(HH_faultPin, OUTPUT);
  digitalWrite(defumidifier_1_Pin, RELAY_OFF);
  digitalWrite(defumidifier_2_Pin, RELAY_OFF);
  digitalWrite(D1_faultPin, LOW);
  digitalWrite(HH_faultPin, LOW);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - DHT_Timer >= DHT_sensor_Delay) {
    DHT_Timer = currentMillis;
    fahrenheitVal = dht.readTemperature(true) + Temp_offset;
    currentHumidity = dht.readHumidity() + Humidity_offset;
    tooCold = fahrenheitVal <= functional_Dehumidifier_Temp;



 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Serial monitor printout
    Serial.println();
    Serial.print("Temperature = ");
    Serial.println(fahrenheitVal);
    Serial.print("Humidity = ");
    Serial.println(currentHumidity);
    Serial.print("relay 1 (D1_outputStatus) = ");
    Serial.println(D1_outputStatus);
    Serial.print("relay 2 (D2_outputStatus) = ");
    Serial.println(D2_outputStatus);


 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // LCD printout
    lcd.clear();
    lcd.setCursor(0, 0); // go to start of 1st line
    lcd.print("Temp = ");
    lcd.print(fahrenheitVal, 0);
    lcd.print((char)223);
    lcd.print("F");
    lcd.setCursor(0, 1); // go to start of 2nd line
    lcd.print("Humidity = ");
    lcd.print(currentHumidity, 0);
    lcd.print("%");

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//DEBUGGING PRINTOUTS

    #ifdef showTimers
      Serial.println();
      Serial.println();
      Serial.print("  Dehumidifier 1 ON Time: ");
      printableTimer(currentMillis - defumidifier_1_runtime);
        Serial.print("currentMillis - defumidifier_1_runtime = ");
        Serial.println(currentMillis - defumidifier_1_runtime);
      Serial.println();
      Serial.print("  Dehumidifier 1 OFF Time: ");
      printableTimer(currentMillis - defumidifier_1_offtime);
      Serial.println();
      Serial.println("  ____________________________________");
      Serial.println();
      Serial.print("  Dehumidifier 2 ON Time: ");
      printableTimer(currentMillis - defumidifier_2_runtime);
      Serial.println();
      Serial.print("  Dehumidifier 2 OFF Time: ");
      printableTimer(currentMillis - defumidifier_2_offtime);
        Serial.print("currentMillis - defumidifier_2_runtime = ");
        Serial.println(currentMillis - defumidifier_2_runtime);
      Serial.println();
      Serial.println();
    #endif





#ifdef DEBUG_D1


/*
  //conditions for turning ON D1
  tooHumid = currentHumidity >= max_Humidity;  //this checks to see if its too humid
  D1_MinOffTimersDone = defumidifier_1_offtime >= min_Offtime;  //this verifies that the minimum time has passed since the unit was last turned OFF
*/
  Serial.println();
  Serial.println("D1 ON CONDITIONS:");
  Serial.println();
  Serial.print("tooHumid = ");
  Serial.println(tooHumid);
  Serial.print("tooCold = ");
  Serial.println(tooCold);
  Serial.print("D1_MinOffTimersDone = ");
  Serial.println(D1_MinOffTimersDone);
  Serial.print("D1_firstPass = ");
  Serial.println(D1_firstPass);

  Serial.print("D1 ON WHEN: tooHumid && !tooCold && (D1_MinOffTimersDone || D1_firstPass) =>>> ");
  Serial.print(tooHumid);
  Serial.print(" && ");
  Serial.print(!tooCold);
  Serial.print(" && (");
  Serial.print(D1_MinOffTimersDone);
  Serial.print(" || ");
  Serial.print(D1_firstPass);
  Serial.println(" ) ");
  Serial.println();

  //this is the OFF timer analysis
    Serial.println("  D1 minimum OFF timer condition for running:");
    Serial.print("  D1_MinOffTimersDone = ");
    Serial.println(D1_MinOffTimersDone);
    Serial.print("  currentMillis = ");
    Serial.println(currentMillis);
    Serial.print("  defumidifier_1_offtime = ");
    Serial.println(defumidifier_1_offtime);
    Serial.print("  defumidifier_1_offtime = ");
    Serial.println(currentMillis - defumidifier_1_offtime);
    Serial.print("  min_Offtime = ");
    Serial.println(min_Offtime);
    Serial.print("in this order: D1_MinOffTimersDone =>>> defumidifier_1_offtime >= min_Offtime =>>> ");
    Serial.print(defumidifier_1_offtime);
    Serial.print(" >= ");
    Serial.println(min_Offtime);
    //these 2 lines will convert it into a readable timer
    Serial.print("  Dehumidifier 1 OFF Time: ");
    printableTimer(defumidifier_1_offtime);
  

  Serial.println();
  Serial.println();

/*
  //conditions for turning OFF D1
  D1_offHumidityLvl = currentHumidity <= (max_Humidity - humidity_deadband);   //this checks for the humidity level that D1 turns off at
  D1_MinRuntimersDone = defumidifier_1_runtime >= min_Runtime;  //this verifies that the minimum time has passed since the unit was last turned ON
*/


  Serial.println("D1 OFF CONDITIONS:");
  Serial.println();
  Serial.print("D1_offHumidityLvl = ");
  Serial.println(D1_offHumidityLvl);
  Serial.print("tooCold = ");
  Serial.println(tooCold);
  Serial.print("D1_MinRuntimersDone = ");
  Serial.println(D1_MinRuntimersDone);
  Serial.print("D1 OFF WHEN:  (D1_offHumidityLvl || tooCold) && D1_MinRuntimersDone =>>> (");
  Serial.print(D1_offHumidityLvl);
  Serial.print(" || ");
  Serial.print(tooCold);
  Serial.print(") && ");
  Serial.print(D1_MinRuntimersDone);
  Serial.println();
  Serial.println();

  //this is the OFF timer function
    Serial.println("  D1 minimum ON timer condition for turning OFF:");
    Serial.print("  D1_MinRuntimersDone = ");
    Serial.println(D1_MinRuntimersDone);
    Serial.print("  defumidifier_1_runtime = ");
    Serial.println(defumidifier_1_runtime);
    Serial.print("  min_Runtime = ");
    Serial.println(min_Runtime);
    Serial.print("  in this order: D1_MinRuntimersDone =>>> defumidifier_1_runtime >= min_Runtime =>>> ");
    Serial.print(defumidifier_1_runtime);
    Serial.print(" >= ");
    Serial.println(min_Runtime);
    //these 2 lines will convert it into a readable timer
    Serial.print("  Dehumidifier 1 ON Time: ");
    printableTimer(defumidifier_1_runtime); 
  
#endif







#ifdef DEBUG_D2

/*
  //conditions for the boolean vaues on D2
  extraHumid = currentHumidity >= overDrive_Humidity;
      digitalWrite(HH_faultPin, extraHumid);
  D2_MinOffTimersDone = currentMillis - defumidifier_2_offtime >= min_Offtime;
  D2_offHumidityLvl = currentHumidity <= (overDrive_Humidity - humidity_deadband);
  D2_MinRuntimersDone = currentMillis - defumidifier_2_runtime >= min_Runtime;

  ON WHEN: (extraHumid || D1_fault) && !tooCold && (D2_MinOffTimersDone || D2_firstPass)

  OFF WHEN: (D2_offHumidityLvl || tooCold) && D2_MinRuntimersDone

*/


//D2 ON CONDITIONS
    Serial.println();
    Serial.println();
  Serial.println("D2 ON CONDITIONS:");
    Serial.println();
  Serial.print("extraHumid = ");
  Serial.println(extraHumid);
  Serial.print("!tooCold = ");
  Serial.println(!tooCold);
  Serial.print("D2_MinOffTimersDone = ");
  Serial.println(D2_MinOffTimersDone);
  Serial.print("D2_firstPass = ");
  Serial.println(D2_firstPass);
  Serial.print("D1_fault = ");
  Serial.println(D1_fault);

  Serial.print("ON WHEN:  (extraHumid || D1_fault) && !tooCold && (D2_MinOffTimersDone || D2_firstPass) =>>> ");
  Serial.print("(");
  Serial.print(extraHumid);
  Serial.print(" || ");
  Serial.print(D1_fault);
  Serial.print(" ) ");
  Serial.print(" && ");
  Serial.print(!tooCold);
  Serial.print(" && (");
  Serial.print(D2_MinOffTimersDone);
  Serial.print(" || ");
  Serial.print(D2_firstPass);
  Serial.print(" ) ");
  Serial.println();
  Serial.print("  Dehumidifier 2 ON Time: ");
    printableTimer(defumidifier_2_runtime);
  Serial.println();
  Serial.println();


//D2 OFF CONDITIONS
  Serial.println("D2 OFF CONDITIONS:");
    Serial.println();
  Serial.print("D2_offHumidityLvl = ");
  Serial.println(D2_offHumidityLvl);
  Serial.print("tooCold = ");
  Serial.println(tooCold);
  Serial.print("D2_MinRuntimersDone = ");
  Serial.println(D2_MinRuntimersDone);
  
  Serial.print("OFF WHEN:  ((D2_offHumidityLvl && !D1_fault) || tooCold) && D2_MinRuntimersDone =>>> ");
  Serial.print(" (");
  Serial.print(" ( ");
  Serial.print(D2_offHumidityLvl);
  Serial.print(" && ");
  Serial.print(!D1_fault);
  Serial.print(" ) ");
  Serial.print(" || ");
  Serial.print(tooCold);
  Serial.print(") && ");
  Serial.print(D2_MinRuntimersDone);
  Serial.println();
  Serial.print("  Dehumidifier 2 OFF Time: ");
    printableTimer(defumidifier_2_offtime);
  Serial.println();
  Serial.println();
 

#endif

    Serial.println("///////////////////////////////////////////////////////////////////");
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//this will read the STATUS of dehumidifier 1
   transformer_VPP = getVPP();
   notSeeingVoltage = (transformer_VPP < minimumTransformerValue);
   FaultTimer_ranOut = currentMillis - D1_Fault_Timer >= D1_Fault_delay;

   #ifdef DEBUG_CURRENT_TRANSFORMER
      Serial.println();
      Serial.print("FaultTimer_ranout =>>> currentMillis - D1_Fault_Timer >= D1_Fault_delay =>>> ");
      Serial.print(currentMillis);
      Serial.print(" - ");
      Serial.print(D1_Fault_Timer);
      Serial.print(" >= ");
      Serial.print(D1_Fault_delay);
      Serial.println(" ) ");
      
      Serial.print("currentMillis - D1_Fault_Timer = ");
      Serial.print(currentMillis - D1_Fault_Timer);

      Serial.println();
      Serial.print("Volts Peak : ");
      Serial.println(transformer_VPP,3);
      Serial.print("notSeeingVoltage = ");
      Serial.println(notSeeingVoltage);
      Serial.print("FaultTimer_ranOut = ");
      Serial.println(FaultTimer_ranOut);
      Serial.print("fault status ON when: notSeeingVoltage && FaultTimer_ranOut && D1_outputStatus =>>> ");
      Serial.print(notSeeingVoltage);
      Serial.print(" && ");
      Serial.print(FaultTimer_ranOut);
      Serial.print(" && ");
      Serial.println(D1_outputStatus);
      Serial.print("fault status OFF when: (!notSeeingVoltage && D1_outputStatus) || !D1_outputStatus =>>> ");
      Serial.print(" ( ");
      Serial.print(!notSeeingVoltage);
      Serial.print(" && ");
      Serial.print(D1_outputStatus);
      Serial.print(") || ");
      Serial.println(!D1_outputStatus);
  #endif
   
  if(notSeeingVoltage && FaultTimer_ranOut && D1_outputStatus){
    D1_fault = 1;  
  }
  else if ((!notSeeingVoltage && D1_outputStatus) || !D1_outputStatus){
    D1_fault = 0;
    D1_Fault_Timer = currentMillis;
  }
  digitalWrite(D1_faultPin, D1_fault);



  #ifdef DEBUG_CURRENT_TRANSFORMER
   Serial.print("D1_fault = ");
   Serial.print(D1_fault);
   Serial.println();
  #endif



 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Dehumidifier 1 CONTROL
  tooHumid = currentHumidity >= max_Humidity;
  D1_MinOffTimersDone = currentMillis - defumidifier_1_offtime >= min_Offtime;
  D1_offHumidityLvl = currentHumidity <= (max_Humidity - humidity_deadband);
  D1_MinRuntimersDone = currentMillis - defumidifier_1_runtime >= min_Runtime;

  if (tooHumid && !tooCold && (D1_MinOffTimersDone || D1_firstPass)) {
    digitalWrite(defumidifier_1_Pin, RELAY_ON);
    if (!D1_outputStatus) {
      defumidifier_1_runtime = currentMillis;
      D1_outputStatus = 1;
    }
  } else if ((D1_offHumidityLvl || tooCold) && D1_MinRuntimersDone) {
    digitalWrite(defumidifier_1_Pin, RELAY_OFF);
    if (D1_outputStatus) {
      defumidifier_1_offtime = currentMillis;
      D1_firstPass = 0;
      D1_outputStatus = 0;
    }
  }

 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Dehumidifier 2 CONTROL
  extraHumid = currentHumidity >= overDrive_Humidity;
      digitalWrite(HH_faultPin, extraHumid);
  D2_MinOffTimersDone = currentMillis - defumidifier_2_offtime >= min_Offtime;
  D2_offHumidityLvl = currentHumidity <= (overDrive_Humidity - humidity_deadband);
  D2_MinRuntimersDone = currentMillis - defumidifier_2_runtime >= min_Runtime;

  if ((extraHumid || D1_fault) && !tooCold && (D2_MinOffTimersDone || D2_firstPass)) {
    digitalWrite(defumidifier_2_Pin, RELAY_ON);
    if (!D2_outputStatus) {
      defumidifier_2_runtime = currentMillis;
      D2_outputStatus = 1;
    }
  } else if (((D2_offHumidityLvl && !D1_fault) || tooCold) && D2_MinRuntimersDone) {
    digitalWrite(defumidifier_2_Pin, RELAY_OFF);
    if (D2_outputStatus) {
      defumidifier_2_offtime = currentMillis;
      D2_firstPass = 0;
      D2_outputStatus = 0;
    }
  }

 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Heater control (eventually)
}

float getVPP() {
  float result;
  int readValue; // value read from the sensor
  int maxValue = 0; // store max value here
  uint32_t start_time = millis();
  while ((millis() - start_time) < sampleSize) { // sample for 1 Sec
    readValue = analogRead(dehumidifier_Status_pin);
    // see if you have a new maxValue
    if (readValue > maxValue) {
      // record the maximum sensor value
      maxValue = readValue;
    }
  }

  // Convert the digital data to a voltage
  result = (maxValue * 5.0) / 1024.0;
  return result;
}

void printableTimer(unsigned long Timer) {
  unsigned long allSeconds = Timer / 1000;
  int runHours = allSeconds / 3600;
  int secsRemaining = allSeconds % 3600;
  int runMinutes = secsRemaining / 60;
  int runSeconds = secsRemaining % 60;

  char buf[14];
  sprintf(buf, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);
  Serial.println(buf);
}
