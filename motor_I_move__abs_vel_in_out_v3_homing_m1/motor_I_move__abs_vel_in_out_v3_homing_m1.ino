 /*
 * Title: MovePositionAbsolute
 *
 * Objective:
 *    This example demonstrates control of a ClearPath motor in Step and
 *    Direction mode.
 *
 * Description:
 *    This example enables a ClearPath then commands a series of repeating
 *    absolute position moves to the motor.
 *
 * Requirements:
 * 1. A ClearPath motor must be connected to Connector M-0.
 * 2. The connected ClearPath motor must be configured through the MSP software
 *    for Step and Direction mode (In MSP select Mode>>Step and Direction).
 * 3. The ClearPath motor must be set to use the HLFB mode "ASG-Position
 *    w/Measured Torque" with a PWM carrier frequency of 482 Hz through the MSP
 *    software (select Advanced>>High Level Feedback [Mode]... then choose
 *    "ASG-Position w/Measured Torque" from the dropdown, make sure that 482 Hz
 *    is selected in the "PWM Carrier Frequency" dropdown, and hit the OK
 *    button).
 * 4. Set the Input Format in MSP for "Step + Direction".
 *
 * ** Note: Homing is optional, and not required in this operational mode or in
 *    this example. This example makes positive absolute position moves,
 *    assuming any homing move occurs in the negative direction.
 *
 * ** Note: Set the Input Resolution in MSP the same as your motor's Positioning
 *    Resolution spec if you'd like the pulses sent by ClearCore to command a
 *    move of the same number of Encoder Counts, a 1:1 ratio.
 *
 * Links:
 * ** ClearCore Documentation: https://teknic-inc.github.io/ClearCore-library/
 * ** ClearCore Manual: https://www.teknic.com/files/downloads/clearcore_user_manual.pdf
 * ** ClearPath Manual (DC Power): https://www.teknic.com/files/downloads/clearpath_user_manual.pdf
 * ** ClearPath Manual (AC Power): https://www.teknic.com/files/downloads/ac_clearpath-mc-sd_manual.pdf
 *
 * 
 * Copyright (c) 2020 Teknic Inc. This work is free to use, copy and distribute under the terms of
 * the standard MIT permissive software license which can be found at https://opensource.org/licenses/MIT
 */



 //////////////////////////////////////////////////////////////////////////////////////
 // Specify which input pin to read from.
// IO-0 through A-12 are all available as digital inputs.
#define inputPin6 DI6
#define inputPin7 DI7
#define inputPin8 DI8

int incomingByte = 0;   // for incoming serial data

///////////////////////////////////////////////////////////////////////////////////////

#include "ClearCore.h"

// Specifies which motor to move.
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
MotorDriver *const motors[MOTOR_CON_CNT] = {
  &ConnectorM0, &ConnectorM1, &ConnectorM2, &ConnectorM3
};


// Select the baud rate to match the target serial device
#define baudRate 9600

// This example has built-in functionality to automatically clear motor alerts, 
//	including motor shutdowns. Any uncleared alert will cancel and disallow motion.
// WARNING: enabling automatic alert handling will clear alerts immediately when 
//	encountered and return a motor to a state in which motion is allowed. Before 
//	enabling this functionality, be sure to understand this behavior and ensure 
//	your system will not enter an unsafe state. 
// To enable automatic alert handling, #define HANDLE_ALERTS (1)
// To disable automatic alert handling, #define HANDLE_ALERTS (0)
#define HANDLE_ALERTS (0)

// Define the velocity and acceleration limits to be used for each move
int velocityLimit = 10000; // pulses per sec
int accelerationLimit = 100000; // pulses per sec^2

// Declares user-defined helper functions.
// The definition/implementations of these functions are at the bottom of the sketch.
void PrintAlerts(int i);
void HandleAlerts(int i);

// Declares our user-defined helper function, which is used to command moves to
// the motor. The definition/implementation of this function is at the  bottom
// of the example
bool MoveAbsolutePosition(int i, int32_t position);
bool MoveAtVelocity(int i, int32_t velocity);

void setup() {
    // Put your setup code here, it will only run once:

///////////////////////////////////////////////////////////////////////////////////////////////
    pinMode(inputPin6, INPUT); // Set the pin as an INPUT
    pinMode(inputPin7, INPUT); // Set the pin as an INPUT
    pinMode(inputPin8, INPUT); // Set the pin as an INPUT
 
// Configure pins IO-0 through IO-5 as digital outputs. These are the only
    // pins that support digital output mode.
    pinMode(IO0, OUTPUT);
    pinMode(IO1, OUTPUT);
    pinMode(IO2, OUTPUT);
    pinMode(IO3, OUTPUT);
    pinMode(IO4, OUTPUT);
    pinMode(IO5, OUTPUT);
   // The connectors are all set up; start the loop with turning them all on.
    

    
////////////////////////////////////////////////////////////////////////////////////////////
    // Sets the input clocking rate. This normal rate is ideal for ClearPath
    // step and direction applications.
    MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_NORMAL);

    // Sets all motor connectors into step and direction mode.
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                          Connector::CPM_MODE_STEP_AND_DIR);

    for (int i=0; i < 3; i++) {
      // Set the motor's HLFB mode to bipolar PWM
      motors[i]->HlfbMode(MotorDriver::HLFB_MODE_HAS_BIPOLAR_PWM);
      // Set the HFLB carrier frequency to 482 Hz
      motors[i]->HlfbCarrier(MotorDriver::HLFB_CARRIER_482_HZ);
  
      // Sets the maximum velocity for each move
      motors[i]->VelMax(velocityLimit);
  
      // Set the maximum acceleration for each move
      motors[i]->AccelMax(accelerationLimit);
    }

    // Sets up serial communication and waits up to 5 seconds for a port to open.
    // Serial communication is not required for this example to run.
    Serial.begin(baudRate);
    uint32_t timeout = 5000;
    uint32_t startTime = millis();
    while (!Serial && millis() - startTime < timeout) {
        continue;
    }

    for (int i=0; i < 3; i++) {
      // Enables the motor; homing will begin automatically if enabled
      motors[i]->EnableRequest(true);
      Serial.println("Motor Enabled");
  
      // Waits for HLFB to assert (waits for homing to complete if applicable)
      Serial.println("Waiting for HLFB...");
      while (motors[i]->HlfbState() != MotorDriver::HLFB_ASSERTED &&
  			!motors[i]->StatusReg().bit.AlertsPresent) {
          continue;
      }
  	// Check if motor alert occurred during enabling
  	// Clear alert if configured to do so 
      if (motors[i]->StatusReg().bit.AlertsPresent) {
  		Serial.println("Motor alert detected.");		
  		PrintAlerts(i);
  		if(HANDLE_ALERTS){
  			HandleAlerts(i);
  		} else {
  			Serial.println("Enable automatic alert handling by setting HANDLE_ALERTS to 1.");
  		}
  		Serial.println("Enabling may not have completed as expected. Proceed with caution.");		
   		Serial.println();
  	} else {
  		Serial.println("Motor Ready");	
  	}
  }
  //////////////////motor 1 homing
  
 if (!digitalRead(inputPin8)) 
 { 
    //move motor 1 with velocity -20000
    MoveAtVelocity(1, -2000);
    while (!digitalRead(inputPin8))
    { 
      delay(100);
    }
  MoveAtVelocity(1, 0);
  motors[1]->PositionRefSet(0);
   delay(1000);
MoveAbsolutePosition(1, 200);  
  
 } 
}




void loop() {
    // Put your main code here, it will run repeatedly:
    
   
 incomingByte = Serial.read();
    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (digitalRead(inputPin6) && !digitalRead(inputPin8) ){
        digitalWrite(IO0, true);
        digitalWrite(IO1, true);
        digitalWrite(IO2, true);
        digitalWrite(IO3, true);
        digitalWrite(IO4, true);
        digitalWrite(IO5, true);
        MoveAbsolutePosition(1, 900);
      //MoveAtVelocity(1, 900);
       delay(200);
          
    }
   
    if (digitalRead(inputPin7) && !digitalRead(inputPin8)) {
        digitalWrite(IO0, false);
        digitalWrite(IO1, false);
        digitalWrite(IO2, false);
        digitalWrite(IO3, false);
        digitalWrite(IO4, false);
        digitalWrite(IO5, false);
        MoveAbsolutePosition(1, 400);
     //MoveAtVelocity(1, 0);
       delay(200);
    }
   
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*------------------------------------------------------------------------------
 * MoveAbsolutePosition
 *
 *    Command step pulses to move the motor's current position to the absolute
 *    position specified by "position"
 *    Prints the move status to the USB serial port
 *    Returns when HLFB asserts (indicating the motor has reached the commanded
 *    position)
 *
 * Parameters:
 *    int position  - The absolute position, in step pulses, to move to
 *
 * Returns: True/False depending on whether the move was successfully triggered.
 */
bool MoveAbsolutePosition(int i, int position) {
    // Check if a motor alert is currently preventing motion
	// Clear alert if configured to do so 
    if (motors[i]->StatusReg().bit.AlertsPresent) {
		Serial.println("Motor alert detected.");		
		PrintAlerts(i);
		if(HANDLE_ALERTS){
			HandleAlerts(i);
		} else {
			Serial.println("Enable automatic alert handling by setting HANDLE_ALERTS to 1.");
		}
		Serial.println("Move canceled.");		
		Serial.println();
        return false;
    }

    Serial.print("Moving to absolute position: ");
    Serial.println(position);

    // Command the move of absolute distance
    motors[i]->Move(position, MotorDriver::MOVE_TARGET_ABSOLUTE);

    // Waits for HLFB to assert (signaling the move has successfully completed)
    Serial.println("Moving.. Waiting for HLFB");
    while ( (!motors[i]->StepsComplete() || motors[i]->HlfbState() != MotorDriver::HLFB_ASSERTED) &&
			!motors[i]->StatusReg().bit.AlertsPresent) {
        continue;
    }
	// Check if motor alert occurred during move
	// Clear alert if configured to do so 
    if (motors[i]->StatusReg().bit.AlertsPresent) {
		Serial.println("Motor alert detected.");		
		PrintAlerts(i);
		if(HANDLE_ALERTS){
			HandleAlerts(i);
		} else {
			Serial.println("Enable automatic fault handling by setting HANDLE_ALERTS to 1.");
		}
		Serial.println("Motion may not have completed as expected. Proceed with caution.");
		Serial.println();
		return false;
    } else {
		Serial.println("Move Done");
		return true;
	}
}
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
 * MoveAtVelocity
 *
 *    Command the motor to move at the specified "velocity", in pulses/second.
 *    Prints the move status to the USB serial port
 *
 * Parameters:
 *    int velocity  - The velocity, in step pulses/sec, to command
 *
 * Returns: True/False depending on whether the move was successfully triggered.
 */
bool MoveAtVelocity(int i, int velocity) {
    // Check if a motor alert is currently preventing motion
  // Clear alert if configured to do so 
    if (motors[i]->StatusReg().bit.AlertsPresent) {
      Serial.println("Motor alert detected.");    
      PrintAlerts(i);
      if(HANDLE_ALERTS){
        HandleAlerts(i);
      } else {
        Serial.println("Enable automatic alert handling by setting HANDLE_ALERTS to 1.");
      }
      Serial.println("Move canceled.");   
      Serial.println();
      return false;
    }

    Serial.print("Moving at velocity: ");
    Serial.println(velocity);

    // Command the velocity move
    motors[i]->MoveVelocity(velocity);

    // Waits for the step command to ramp up/down to the commanded velocity. 
    // This time will depend on your Acceleration Limit.
    Serial.println("Ramping to speed...");
    while (!motors[i]->StatusReg().bit.AtTargetVelocity) {
        continue;
    }

    Serial.println("At Speed");
    return true; 
}
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
 * PrintAlerts
 *
 *    Prints active alerts.
 *
 * Parameters:
 *    requires "motor" to be defined as a ClearCore motor connector
 *
 * Returns: 
 *    none
 */
 void PrintAlerts(int i){
	// report status of alerts
 	Serial.println("Alerts present: ");
	if(motors[i]->AlertReg().bit.MotionCanceledInAlert){
		Serial.println("    MotionCanceledInAlert "); }
	if(motors[i]->AlertReg().bit.MotionCanceledPositiveLimit){
		Serial.println("    MotionCanceledPositiveLimit "); }
	if(motors[i]->AlertReg().bit.MotionCanceledNegativeLimit){
		Serial.println("    MotionCanceledNegativeLimit "); }
	if(motors[i]->AlertReg().bit.MotionCanceledSensorEStop){
		Serial.println("    MotionCanceledSensorEStop "); }
	if(motors[i]->AlertReg().bit.MotionCanceledMotorDisabled){
		Serial.println("    MotionCanceledMotorDisabled "); }
	if(motors[i]->AlertReg().bit.MotorFaulted){
		Serial.println("    MotorFaulted ");
	}
 }
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * HandleAlerts
 *
 *    Clears alerts, including motor faults. 
 *    Faults are cleared by cycling enable to the motor.
 *    Alerts are cleared by clearing the ClearCore alert register directly.
 *
 * Parameters:
 *    requires "motor" to be defined as a ClearCore motor connector
 *
 * Returns: 
 *    none
 */
 void HandleAlerts(int i){
	if(motors[i]->AlertReg().bit.MotorFaulted){
		// if a motor fault is present, clear it by cycling enable
		Serial.println("Faults present. Cycling enable signal to motor to clear faults.");
		motors[i]->EnableRequest(false);
		Delay_ms(10);
		motors[i]->EnableRequest(true);
	}
	// clear alerts
	Serial.println("Clearing alerts.");
	motors[i]->ClearAlerts();
 }
//------------------------------------------------------------------------------

 
