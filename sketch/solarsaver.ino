#include "LowPower.h"
#include "RunningAverage.h"

#define SERIAL_BAUD      9600

// FSM definition
enum States {
  startup,
  poweron_sensing,
  poweroff_sensing,
  poweron_override,
  poweroff_override,
};

States state = startup;
boolean togglePressed = false;

// GPIO pins
const int lightPin   = 0;    // Analogue in
const int voltagePin = 1;    // Analogue in
const int powerPin   = 5;    // Digital out
const int togglePin  = 2;    // Digital in (2 or 3)
const int ledPin     = 12;   // Digital out

// ADC sensor levels
const int lightLevel   = 500;
const int voltageLevel = 850;

// Number of readings required
const int nLightReadings   = 32;
const int nVoltageReadings = 32;

// Control spurious switching
const int lightThreshold   = 10;
const int voltageThreshold = 10;

// Running averages
RunningAverage lightReadings(nLightReadings);
RunningAverage voltageReadings(nVoltageReadings);

void powerOn()
{
  digitalWrite(powerPin, HIGH);
}

void powerOff()
{
  digitalWrite(powerPin, LOW);
}

void toggleHook()
{
  // Dummy hook
}

void blinkLed(int count = 1)
{
  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, HIGH);
    delay(70);
    digitalWrite(ledPin, LOW);
    delay(180);
  }
}

void setState(States newState) {

  Serial.print("New state [");

  switch (newState) {

    case poweron_sensing:
      Serial.println("Power on: sensing]");
      powerOn();
      break;

    case poweroff_sensing:
      Serial.println("Power off: sensing]");
      powerOff();
      break;

    case poweron_override:
      Serial.println("Power on: override]");
      powerOn();
      break;

    case poweroff_override:
      Serial.println("Power off: override]");
      powerOff();
      break;

    default:
      Serial.println("Unknown]");
      newState = poweron_sensing;
      powerOn();
      break;
  }

  state = newState;
}

void setup() {
  pinMode(powerPin, INPUT_PULLUP);

  Serial.begin(SERIAL_BAUD);

  // Setup digital pin modes
  pinMode(powerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(togglePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(togglePin), toggleHook, RISING);
  setState(poweroff_sensing);
}

void loop() {

  Serial.flush();

  // Enter idle state
  // ATmega328P, ATmega168
  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);

  // ATmega2560
  //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);

  // Debounce button
  if (togglePressed == true) {
    togglePressed = false;
  }
  else if (digitalRead(togglePin) == HIGH) {
    Serial.println("Toggle switch pressed");
    togglePressed = true;
    delay(150);
  }

  // Determine state transitions
  switch (state) {

    case poweron_sensing:

      if (togglePressed) {
        togglePressed = false;
        setState(poweron_override);
      }
      else {
        int lReading = analogRead(lightPin);
        lightReadings.addValue(lReading);
        int lAvg = lightReadings.getFastAverage();
        Serial.print("Average light level: ");
        Serial.println(lAvg);

        int vReading = analogRead(voltagePin);
        voltageReadings.addValue(vReading);
        int vAvg = voltageReadings.getFastAverage();
        Serial.print("Average voltage level: ");
        Serial.println(vAvg);

        if ((lightLevel - lAvg) > lightThreshold || (voltageLevel - vAvg) > voltageThreshold) {
          setState(poweroff_sensing);
        }
        blinkLed(1);
      }
      break;

    case poweroff_sensing:

      if (togglePressed) {
        togglePressed = false;
        setState(poweron_override);
      }
      else {
        int lReading = analogRead(lightPin);
        lightReadings.addValue(lReading);
        int lAvg = lightReadings.getFastAverage();
        Serial.print("Average light level: ");
        Serial.println(lAvg);

        int vReading = analogRead(voltagePin);
        voltageReadings.addValue(vReading);
        int vAvg = voltageReadings.getFastAverage();
        Serial.print("Average voltage level: ");
        Serial.println(vAvg);

        if ((lAvg - lightLevel) > lightThreshold && (vAvg - voltageLevel) > voltageThreshold) {
          setState(poweron_sensing);
        }
        blinkLed(2);
      }
      break;

    case poweron_override:
      if (togglePressed) {
        togglePressed = false;
        setState(poweroff_override);
        delay(150);
      }
      else {
        blinkLed(3);
      }
      break;

    case poweroff_override:
      if (togglePressed) {
        togglePressed = false;
        setState(poweron_sensing);
        delay(150);
      }
      else {
        blinkLed(4);
      }
      break;
  }
}


