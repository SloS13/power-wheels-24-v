#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MCP4725.h>

//First Line
//0000000000000000
//REQ 50%  AT

float speedAdjustmentStepRatio = 0.15; //the higher the speed the higher aggression base
float decelAdjustmentStepRatio = 0.4;  //deceleration speed.  Should be about double speed adjustment ratio
int loopRuns = 0;
int throttleRequest = 0; //from hall (throttleMinValue - throttleMaxValue)
int throttleRequest_lastDisplayed;
int speedOutput = 0; //Speed we are outputting (throttleMinValue - throttleMaxValue)
int speedOutput_lastDisplayed;
int speedMax = 0;             //default max speed (throttleMinValue - throttleMaxValue)
int acceleratorSmoothing = 5; //0-1024 mapped to pot2
int acceleratorSmoothing_lastDisplayed;
int brakeSmoothing = 4; //0 - 1024, manually entered

int throttleOutputMinValue = map(0.8,0,5,0,4096); //0-4096
int throttleOutputMaxValue = map(0.8,0,5,0,4096); //0-4096

//accelerator setup.  These will also define the min/max output
int throttleMinValue = 180; //0-1024
int throttleMaxValue = 890; //0-1024

int throttlePin = A3; //pot

int aggresivePin = A0;

int maxSpeedPot = A1;
int maxSpeedVal; //raw from potentiometer
int maxSpeedVal_lastDisplayed;

int loopCounter = 0;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 4); // Change to (0x27,16,2) for 16x2 LCD.
MCP4725 DAC(0x60);

void setup()
{
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  DAC.setValue(throttleOutputMinValue); //set throttle output to minimum

}

void readThrottle()
{
  throttleRequest = analogRead(throttlePin); // read the input pin
}

void readMaxSpeed()
{
  maxSpeedVal = analogRead(maxSpeedPot); // read the input pin
}

void readAggression()
{
  acceleratorSmoothing = analogRead(aggresivePin); // read the input pin
}

void applyMotorOutput(int outputPercent)
{
  //this is where we take the maximum speed into account.
  //figure out 0-4096 value to send to DAC
}

void displayThrottleData()
{
  if (maxSpeedVal != maxSpeedVal_lastDisplayed || acceleratorSmoothing != acceleratorSmoothing_lastDisplayed)
  {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);

    //MAX set Speed
    lcd.print("SPD=");
    lcd.print(map(maxSpeedVal, 0, 1024, 1, 100));

    //ACCEL
    lcd.print(" ACC=");
    lcd.print(map(acceleratorSmoothing, 0, 1024, 1, 100));
  }

  if (throttleRequest != throttleRequest_lastDisplayed || speedOutput != speedOutput_lastDisplayed)
  {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);

    //THROTTLE REQUEST
    lcd.print(" RQ=");
    lcd.print(map(throttleRequest, throttleMinValue, throttleMaxValue, 1, 100));

    //THROTTLE OUTPUT
    lcd.print(" O=");
    lcd.print(map(speedOutput, throttleMinValue, throttleMaxValue, 1, 100));
    // lcd.print(speedOutput);
  }

  maxSpeedVal_lastDisplayed = maxSpeedVal;
  acceleratorSmoothing_lastDisplayed = acceleratorSmoothing;
  throttleRequest_lastDisplayed = throttleRequest;
  speedOutput_lastDisplayed = speedOutput;
}

int adjustOutput()
{
  //figure out how many units to adjust based on aggression
  int diff = abs(throttleRequest - speedOutput); //diff is the difference and 100% of it is maximum aggression
  //we will want between probably 40% - 100% of aggression
  float aggressionPercent = map(acceleratorSmoothing, 0, 1024, 40, 100);

  Serial.println("aggression: " + String(aggressionPercent));
  if (throttleRequest > speedOutput)
  {
    float adjustAmount = (aggressionPercent / 100 * diff) * speedAdjustmentStepRatio;
    speedOutput = speedOutput + adjustAmount;
  }
  else if (throttleRequest < speedOutput)
  {
    float adjustAmount = (aggressionPercent / 100 * diff) * decelAdjustmentStepRatio;
    speedOutput = speedOutput - adjustAmount;
  }
  applyMotorOutput(speedOutput);
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

  // delay(1);
}
