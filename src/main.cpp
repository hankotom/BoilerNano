#include <Arduino.h>

/*
Kétkörös fűtés vezérlés.
Bemenet: A termosztát rövidzárja
Kimenetek: Radiátor kör keringerő szivattyú és a kazán rövidzárja

Állapotok: 
Stop - itt mindenki áll
Starting - jön egy indítójel, akkor először elindítja a radiátor szivattyút
Work - Késleltetés után elindítja a kazánt is
Stopping - leállítja a kazánt, de utána még x percig kerinteti a radiátorkört, majd azt is leállítja


*/

#define THERMOSTATON HIGH
#define THERMOSTATOFF LOW

#define PUMPON HIGH
#define PUMPOFF LOW

#define BOILERON HIGH
#define BOILEROFF LOW

// constants won't change. Used here to set a pin number:

const int ledPin =  13;      // the number of the LED pin
const int thermostatPin =  2;// the number of the LED pin
const int boilerPin =  10;// the number of the LED pin
const int pumpPin =  10;// the number of the LED pin

// Variables will change:
int thermostat = 0;         // variable for reading the thermostat status
int pump = PUMPOFF;         // variable for pump status
int boiler = BOILEROFF;         // variable for boiler status
enum states{Stop, Starting, Work, Stopping} HeatState;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long stateChangeTimestamp = 0; // Egyetlen időbélyeg elég az állapotváltásokhoz
unsigned long TimerForStart = 0;        // will store last time LED was updated
unsigned long TimerForStop = 0;        // will store last time LED was updated

// constants won't change:
const unsigned long StartDelay = 1000;           // 
const unsigned long StopDelay  = 2000;           // 

void setup() {
  // set the digital pin as output:
  pinMode(thermostatPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(boilerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Serial.begin(57600);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }
  // Serial.println("Goodnight moon!");

  boiler = BOILEROFF;
  pump = PUMPOFF;
  HeatState = Stop;

  stateChangeTimestamp = millis();

}

void loop() {

  unsigned long currentMillis = millis(); // Olvassuk ki egyszer a ciklus elején
  thermostat = digitalRead(thermostatPin);

  switch (HeatState){

    case Stop:
      boiler = BOILEROFF;
      pump = PUMPOFF;
      if(thermostat == THERMOSTATON){
        HeatState = Starting;
        stateChangeTimestamp = currentMillis;  // Rögzítsük az időpontot
      }
      break;

    case Starting:
      boiler = BOILEROFF;
      pump = PUMPON;
      if(thermostat == THERMOSTATOFF){
        HeatState = Stop;
      }
      if((thermostat == THERMOSTATON) && (currentMillis - stateChangeTimestamp >= StartDelay)){
        HeatState = Work;
      }
      break;

    case Work:
      boiler = BOILERON;
      pump = PUMPON;
      if(thermostat == THERMOSTATOFF){
        HeatState = Stopping;
        stateChangeTimestamp = currentMillis;  // Rögzítsük az időpontot
      }
      break;

    case Stopping:
      boiler = BOILEROFF;
      pump = PUMPON;

      if(thermostat == THERMOSTATON){
        HeatState = Work;
      }

      if((thermostat == THERMOSTATOFF) && (currentMillis - stateChangeTimestamp >= StopDelay)){
        HeatState = Stop;
      }

      break;

    default:
      boiler = BOILEROFF;
      pump = PUMPOFF;
      HeatState = Stop;
      break;

  digitalWrite(boilerPin, boiler);
  digitalWrite(pumpPin, pump);
    // if ((millis() % 400) == 0){
    //   Serial.println(millis());
    //   Serial.print("Kazan: ");Serial.print(boiler); Serial.print(" Szivattyu: ");Serial.println(pump);
    // }

  }
//  if ((millis() % 400) == 0)
//    digitalWrite(ledPin, !ledPin);
//  Serial.println(millis() % 400);  //   Serial.print("  ");   Serial.println(pump);

}