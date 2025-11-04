#include <Arduino.h>

/*
 * Dual-circuit heating controller.
 * This code implements a state machine to control a heating system with a boiler and a circulation pump.
 * 
 * Input: A signal from the thermostat (e.g., closing a contact).
 * Outputs: Relays for the radiator circulation pump and the boiler.
 *
 * --- State Machine Description ---
 *
 * Stop:     All outputs are off. The system is idle, waiting for a heating request.
 *
 * Starting: When a heating request is received from the thermostat, the system first starts the circulation pump
 *           to get the water moving. This prevents the boiler from heating a static body of water.
 *
 * Work:     After a predefined delay (StartDelay), the boiler is also turned on. Both pump and boiler are active.
 *
 * Stopping: When the thermostat signal turns off, the boiler is immediately shut down. The pump continues to run
 *           for a set period (StopDelay) to dissipate the residual heat from the boiler into the radiators.
 *           This is known as a pump overrun or after-run, which protects the boiler and improves efficiency.
 *
 */

// --- Thermostat Signal Definitions ---
#define THERMOSTATON HIGH // Signal state when heating is requested
#define THERMOSTATOFF LOW // Signal state when heating is not requested

// --- Pump Control Definitions ---
#define PUMPON HIGH
#define PUMPOFF LOW

// --- Boiler Control Definitions ---
#define BOILERON HIGH
#define BOILEROFF LOW

// --- Hardware Pin Definitions ---
// constants won't change. Used here to set a pin number:

const int ledPin = 13;      // The number of the built-in LED pin, can be used for status indication.
const int thermostatPin = 2; // The input pin for the thermostat signal.
const int boilerPin = 10;   // The output pin to control the boiler relay.
const int pumpPin = 10;     // The output pin to control the pump relay.
                            // NOTE: boilerPin and pumpPin are set to the same pin. This should be corrected for the logic to work as intended.

// --- Global Variables ---
// Variables will change:
int thermostat = 0;       // Stores the current status of the thermostat input.
int pump = PUMPOFF;       // Stores the desired state for the pump.
int boiler = BOILEROFF;   // Stores the desired state for the boiler.
enum states{Stop, Starting, Work, Stopping} HeatState; // The current state of the state machine.

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store.
unsigned long stateChangeTimestamp = 0; // Stores the timestamp (from millis()) of the last state change for delay calculations.
unsigned long TimerForStart = 0;        // Kept for legacy reference, not used in the new millis() logic.
unsigned long TimerForStop = 0;         // Kept for legacy reference, not used in the new millis() logic.

// --- Timing Constants ---
// constants won't change:
const unsigned long StartDelay = 1000;  // Delay in milliseconds before starting the boiler after the pump has started.
const unsigned long StopDelay  = 2000;  // Delay in milliseconds for the pump to run after the boiler has stopped (pump overrun).

void setup() {
  // set the digital pin modes:
  pinMode(thermostatPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(boilerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Serial communication for debugging (currently disabled)
  // Serial.begin(57600);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }
  // Serial.println("Heating controller starting up...");

  // Initialize system state
  boiler = BOILEROFF;
  pump = PUMPOFF;
  HeatState = Stop;

  stateChangeTimestamp = millis();
}

void loop() {
  
  // Cache the value of millis() at the start of the loop to ensure consistent timing throughout the cycle.
  unsigned long currentMillis = millis(); 
  thermostat = digitalRead(thermostatPin);

  // --- Main State Machine ---
  switch (HeatState){

    case Stop:
      boiler = BOILEROFF;
      pump = PUMPOFF;
      // If thermostat calls for heat, transition to the Starting state.
      if(thermostat == THERMOSTATON){
        HeatState = Starting;
        stateChangeTimestamp = currentMillis;  // Save the timestamp to measure the StartDelay.
      }
      break;

    case Starting:
      boiler = BOILEROFF;
      pump = PUMPON;
      // If thermostat is turned off during this phase, go back to Stop.
      if(thermostat == THERMOSTATOFF){
        HeatState = Stop;
      }
      // If the start delay has passed, transition to the Work state.
      if((thermostat == THERMOSTATON) && (currentMillis - stateChangeTimestamp >= StartDelay)){
        HeatState = Work;
      }
      break;

    case Work:
      boiler = BOILERON;
      pump = PUMPON;
      // If thermostat is satisfied (turns off), transition to Stopping state.
      if(thermostat == THERMOSTATOFF){
        HeatState = Stopping;
        stateChangeTimestamp = currentMillis;  // Save the timestamp to measure the StopDelay (pump overrun).
      }
      break;

    case Stopping:
      boiler = BOILEROFF;
      pump = PUMPON; // Keep pump on for after-run.

      // If thermostat calls for heat again during the after-run, go straight back to Work state.
      if(thermostat == THERMOSTATON){
        HeatState = Work;
      }

      // If the after-run delay has passed, transition to Stop state.
      if((thermostat == THERMOSTATOFF) && (currentMillis - stateChangeTimestamp >= StopDelay)){
        HeatState = Stop;
      }
      break;

    default:
      // Failsafe: if the state is ever unknown, go to Stop state.
      boiler = BOILEROFF;
      pump = PUMPOFF;
      HeatState = Stop;
      break;
  }
  
  // --- Update Physical Outputs ---
  // Set the output pins based on the state variables determined by the switch-case block.
  digitalWrite(boilerPin, boiler);
  digitalWrite(pumpPin, pump);

  // --- Debugging Output (currently disabled) ---
  // This block would print the status to the serial monitor every 400ms.
  // if ((millis() % 400) == 0){
  //   Serial.println(millis());
  //   Serial.print("Boiler: "); Serial.print(boiler); Serial.print(" Pump: "); Serial.println(pump);
  // }

}