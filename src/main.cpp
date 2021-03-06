#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MCP4725.h>

//First Line
//0000000000000000
//REQ 50%  AT

 //"dacoutput","potsraw","throttleraw","throttlepercent","actualoutputpercent","outputvoltage"
String serialOutputType ="outputvoltage";

float accelerationRateBase = 0.05; //the higher the speed the higher aggression base
float decelerationRateBase = 0.09;  //deceleration speed.  Should be about double speed adjustment ratio
int loopRuns = 0;

float inputVoltage = 5.2;
float outputVoltage = 0;

int throttleRequestRaw = 0; //from hall (throttleMinValue - throttleMaxValue)
float throttleRequestPercent = 0; //0-100
float throttleRequestPercent_lastDisplayed;

float speedOutputPercent = 0; //0-100
float speedOutputPercent__lastDisplayed;

int speedMaxRaw = 0;             //default max speed (throttleMinValue - throttleMaxValue)
int speedMaxPercent = 0; //0-100
int speedMaxPercent_lastDisplayed;

int agroRaw = 0; //0-1024 mapped to pot2
int agroPercent = 0;
int agroPercent_lastDisplayed = 0;


int brakeSmoothing = 4; //0 - 1024, manually entered

int throttleOutputMinValue = map(0.2,0,inputVoltage,0,4096); //0-4096
int throttleOutputMaxValue = map(4.2,0,inputVoltage,0,4096); //0-4096

int throttleOutputCurrentValue = 0; //what is being written to dac (0-4096)
int actualOutputPercent;

//accelerator setup.  
int throttleMinValue = 180; //0-1024
int throttleMaxValue = 900; //0-1024

int throttlePin = A3; //pot

int aggresivePin = A0;

int maxSpeedPot = A1;


int loopCounter = 0;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 4); // Change to (0x27,16,2) for 16x2 LCD.
MCP4725 DAC(0x60);


void writeToSerial(bool headers=false) {
  if (serialOutputType == "dacoutput") {
    if (headers) {
      Serial.println("DAC_OUTPUT");
    } else {
      Serial.println(throttleOutputCurrentValue);
    }
  }

  if (serialOutputType == "potsraw") {
    if (headers) {
      Serial.println("Max,Agro");
    } else {
      Serial.println(String(speedMaxRaw) + "," + String(agroRaw));
    }
  }

  if (serialOutputType == "throttleraw") {
    if (headers) {
      Serial.println("ThrottleRaw");
    } else {
      Serial.println(String(throttleRequestRaw) + "," + String(agroRaw));
    }
  }

  if (serialOutputType == "throttlepercent") {
    if (headers) {
      Serial.println("ThrottlePercent");
    } else {
      Serial.println(String(throttleRequestPercent));
    }
  }

  if (serialOutputType == "actualoutputpercent") {
    if (headers) {
      Serial.println("actualoutputpercent,outputPercent,speedMaxPercent");
    } else {
      
      Serial.println(String(actualOutputPercent) + "," + String(speedOutputPercent) + "," + String(speedMaxPercent));
    }
  }

  if (serialOutputType == "outputvoltage") {
    if (headers) {
      Serial.println("outputVoltage");
    } else {
      
      Serial.println(String(outputVoltage));
    }
  }




}

void setup()
{
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  DAC.setValue(0); //set throttle output to minimum

  writeToSerial(true); //serial data header
  // Serial.println("Request,Output");
  //Serial.println("DAC Output");

}



void readThrottle()
{
  throttleRequestRaw = analogRead(throttlePin); // read the input pin
  throttleRequestPercent = map(throttleRequestRaw,throttleMinValue,throttleMaxValue,0,100);
  if (throttleRequestPercent<0) {throttleRequestPercent = 0;}

}

void readMaxSpeed()
{
  speedMaxRaw = analogRead(maxSpeedPot); // read the input pin
  speedMaxPercent = map(speedMaxRaw,0,1024,10,100);
}

void readAggression()
{
  agroRaw = analogRead(aggresivePin); // read the input pin
  agroPercent = 75; //JAM DEBUG Override
}

void applyMotorOutput(int outputPercent)
{
  

  //outputPercent is now 0-100
  //if the max speed percent is 100, it will remain 0-100
  //if the max speed is 75, it will be 0-75
  
  actualOutputPercent = outputPercent * speedMaxPercent / 100;


  throttleOutputCurrentValue = map(actualOutputPercent, 0, 100, throttleOutputMinValue, throttleOutputMaxValue);

  DAC.setValue(throttleOutputCurrentValue);
  //compute outputVoltage
  outputVoltage = map(throttleOutputCurrentValue,0,4096,0,inputVoltage * 100);


  // Serial.println(outputPercent + "," + actualOutputPercent);

  // int outputPercent = map(outputPercent,0,1024,0,100);
  // Serial.println("Output Request Percent of Max: " + String(outputPercent));

  //figure out the max of 4096 from speedMaxPercent
  //max value at 100% speed is throttleOutputMaxValue
  //minimum value is throttleOutputMinValue
  //get percent of dac's max
  int val = outputPercent * speedMaxPercent / 100;
  int val2 = map(val,0,val,throttleOutputMinValue,throttleOutputMaxValue);
// Serial.println("Sending to DAC: " +  String(val2));

  //this is where we take the maximum speed into account.
  //figure out 0-4096 value to send to DAC
  // int valueToSendDAC = throttleOutputMaxValue * outputPercent / 100;
  // Serial.println("throttleOutputMaxValue:"+String(throttleOutputMaxValue));
  
  // Serial.println("outputPercent:"+String(outputPercent));
  // Serial.println("Sending to DAC: " + String(valueToSendDAC));
}


void displayThrottleData()
{
  // if (speedMaxPercent != speedMaxPercent_lastDisplayed || agroPercent != agroPercent_lastDisplayed)
  // {
  //   lcd.setCursor(0, 0);
  //   lcd.print("                ");
  //   lcd.setCursor(0, 0);

  //   //MAX set Speed
  //   lcd.print("SPD=");
  //   lcd.print(speedMaxPercent);

  //   //ACCEL
  //   lcd.print(" ACC=");
  //   lcd.print(agroPercent);
  // }

  // if (throttleRequestPercent != throttleRequestPercent_lastDisplayed || speedOutputPercent != speedOutputPercent__lastDisplayed)
  // {
  //   lcd.setCursor(0, 1);
  //   lcd.print("                ");
  //   lcd.setCursor(0, 1);

  //   //THROTTLE REQUEST
  //   lcd.print(" RQ=");
  //   lcd.print(throttleRequestPercent);

  //   //THROTTLE OUTPUT
  //   lcd.print(" O=");
  //   lcd.print(speedOutputPercent);
  //   // lcd.print(speedOutput);
  // }

  speedMaxPercent_lastDisplayed = speedMaxPercent;
  agroPercent_lastDisplayed = agroPercent;
  throttleRequestPercent_lastDisplayed = throttleRequestPercent;
}

void adjustOutput()
{
  //figure out how many units to adjust based on aggression
  int diff = abs(throttleRequestPercent - speedOutputPercent); 

  int acceleerationPointsPerCycle = agroPercent * accelerationRateBase; //adjust me later
  int decelerationPointsPerCycle = agroPercent * decelerationRateBase; //adjust me later

  

  // Serial.println( String(throttleRequestPercent) + "," + String(speedOutputPercent));
  if (throttleRequestPercent > speedOutputPercent)
  {
    speedOutputPercent = speedOutputPercent + acceleerationPointsPerCycle;
    if (speedOutputPercent>throttleRequestPercent) {speedOutputPercent=throttleRequestPercent;}
  }
  else if (throttleRequestPercent < speedOutputPercent)
  {
    speedOutputPercent = speedOutputPercent - decelerationPointsPerCycle;
    if (speedOutputPercent<throttleRequestPercent){speedOutputPercent=throttleRequestPercent;}
  }
  

  applyMotorOutput(speedOutputPercent);
}

void loop()
{
  loopCounter++;

  if (loopCounter > 32001)
  {
    loopCounter = 1;
  }

  // put your main code here, to run repeatedly:
  readThrottle(); //sets throttleRequest
  readMaxSpeed(); //read max from pot
  readAggression();
  adjustOutput();

  if (loopCounter % 10 == 0)
  {
    displayThrottleData();
  }

  writeToSerial();
  delay(20);
}
