#include <PS4Controller.h>
#include <Stepper.h>

// TODO: Make the drills better adjusted

/**
 * Drill Stepper Motor: pins 15,2,0,4 
 * DC Motors: pins 13, 12, 14, 27, 26, 25
 * Drill Motor: pins 16 (Enable = PWM), 17, 5
 * Input Only (no pull-up resistor): 35, 34, 39, 36 
 */

#define IDLE 0
// States for the moving the rover:
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4
// States for the drill: 
#define CLOCKWISE 5
#define COUNTERCLOCKWISE 6
// States for the gantry:
#define EXTEND 7
#define RETRACT 8
// States for getting data:
#define COLLECT 9
// States for debouncing
#define DEBOUNCING 10

// Number of steps per output rotation
const int stepsPerRevolution = 200;

Stepper stepperMotors(stepsPerRevolution, 15, 2, 0, 4);

uint8_t previousFsmState; // keep track of the previous state for the finite state machine
uint8_t fsmState;
uint8_t futureState;
uint8_t ps4Status;
uint32_t timer; // keep track of the time for debouncing between states

const uint8_t debouncingTime = 20;

const uint8_t drillEnable = 16;
const uint8_t drillPin1 = 17;
const uint8_t drillPin2 = 5;
const uint8_t drillPwm = 0;

const uint8_t motorEnable = 13;
const uint8_t motorPin1 = 12;
const uint8_t motorPin2 = 14;
const uint8_t motorPin3 = 27;
const uint8_t motorPin4 = 25;
const uint8_t motorPwm = 1;

const int freq = 30000;
const int resolution = 8;

void setup(){
  Serial.begin(115200);
  
  // Initialize pins for the drill motor
  pinMode(drillEnable, OUTPUT);
  pinMode(drillPin1, OUTPUT);
  pinMode(drillPin2, OUTPUT);
  ledcSetup(drillPwm, freq, resolution); // configure LED PWM functionalitites
  ledcAttachPin(drillEnable, drillPwm); // attach the channel to the GPIO to be controlled
  turnOffDrill(); // Turn off drill motor

   // Initialize pins for the drive motor
  pinMode(motorEnable, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  ledcSetup(motorPwm, freq, resolution); // configure LED PWM functionalitites
  ledcAttachPin(motorEnable, motorPwm); // attach the channel to the GPIO to be controlled
  turnOffMotors(); // Turn off drill motor
  
  stepperMotors.setSpeed(30); // initialize speed of the motor in RPM
  PS4.begin("24:62:ab:f9:7b:36"); 
  Serial.println("\nBluetooth setup.");
  previousFsmState = 1;
  fsmState = IDLE;
  ps4Status = 1;
}

void loop(){
  if (PS4.isConnected()){
    if (!ps4Status){
      Serial.println("PS4 controller connected!\n");
      ps4Status = 1;
    }
    main_fsm();
  } else if (ps4Status) {
      Serial.println("Waiting to connect PS4 controller...");
      ps4Status = 0;
  }
}

void turnOffDrill(){
  // Turn off drill motor
  digitalWrite(drillPin1, LOW);
  digitalWrite(drillPin2, LOW);
}

void turnOffMotors(){
  // Turn off the motors that drive the rover
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
}

void collectData(){
}

uint8_t checkRover(){
  uint8_t state = IDLE;
  if (PS4.event.analog_move.stick.lx) {
    int16_t x_value = PS4.data.analog.stick.lx;
    int16_t y_value = PS4.data.analog.stick.ly;
    if (x_value > 70){
        ledcWrite(motorPwm, x_value*2 - 1);
        state = RIGHT;
    } else if (x_value < -70){
        ledcWrite(motorPwm, x_value*-2 - 1);
        state = LEFT;
    } else if (PS4.event.analog_move.stick.ly) {
      if (y_value > 70){
          ledcWrite(motorPwm, y_value*2 - 1);
          state = FORWARD;
      } else if (y_value < -70) {
          ledcWrite(motorPwm, y_value*-2 - 1);
          state = BACKWARD;
      }
    }
  }
  if (state == IDLE){
    ledcWrite(motorPwm, 0);
  }
  return state;
}

uint8_t checkDrill(){
  uint8_t state = IDLE;
  if (PS4.event.analog_move.stick.ry){
    int16_t y_value = PS4.data.analog.stick.ry;
    if (y_value > 20){
        if (y_value*2 - 1 == 253){
          ledcWrite(drillPwm, 255);
        } else {
          ledcWrite(drillPwm, y_value*2 - 1);
        }
        Serial.println(y_value*2 - 1, DEC);
        state = CLOCKWISE;
    } else if (y_value < -20){
        ledcWrite(drillPwm, y_value*-2 - 1);
        state = COUNTERCLOCKWISE;
    } else {
        ledcWrite(drillPwm, 0);
    }
  }
  return state;
}

uint8_t checkGantry(){
  uint8_t state = IDLE;
  if (PS4.data.button.up){
    state = RETRACT;
  } else if (PS4.data.button.down){
      state = EXTEND;
  }
  return state;
}

uint8_t checkData(){
  uint8_t state = IDLE;
  
  return state;
}

void main_fsm(){
  // Read the values from the PS4 controller
  uint8_t roverState = checkRover();
  uint8_t drillState = checkDrill();
  uint8_t gantryState = checkGantry();
  uint8_t dataState = checkData();
  
  switch (fsmState){
    case IDLE:
      if (previousFsmState != fsmState){
        Serial.println("IDLE");
        previousFsmState = fsmState;
      }
      if (roverState != IDLE){
          update_debounce(roverState);
      } else if (drillState != IDLE){
          update_debounce(drillState);
      } else if (gantryState != IDLE){
          update_debounce(gantryState);
      } else if (dataState != IDLE){
          update_debounce(dataState);
      }
      break;
    case FORWARD:
      if (roverState == FORWARD){
        if (previousFsmState != fsmState){
          // Drive left side forward
          digitalWrite(motorPin1, HIGH);
          digitalWrite(motorPin2, LOW);
          // Drive right side forward
          digitalWrite(motorPin3, HIGH);
          digitalWrite(motorPin4, LOW);
          Serial.println("FORWARD");
          previousFsmState = fsmState;
        }
      } else{
          turnOffMotors();
          update_debounce(IDLE);
      }
      break;
    case BACKWARD:
      if (roverState == BACKWARD){
        if (previousFsmState != fsmState){
          // Drive left side backward
          digitalWrite(motorPin1, LOW);
          digitalWrite(motorPin2, HIGH);
          // Drive right side backward
          digitalWrite(motorPin3, LOW);
          digitalWrite(motorPin4, HIGH);
          Serial.println("BACKWARD");
          previousFsmState = fsmState;
        }
      } else{
          turnOffMotors();
          update_debounce(IDLE);
      }
      break;
    case RIGHT:
      if (roverState == RIGHT){
        if (previousFsmState != fsmState){
          // Drive left side forward
          digitalWrite(motorPin1, HIGH);
          digitalWrite(motorPin2, LOW);
          // Drive right side backward
          digitalWrite(motorPin3, LOW);
          digitalWrite(motorPin4, HIGH);
          Serial.println("RIGHT");
          previousFsmState = fsmState;
        }
      } else{
          turnOffMotors();
          update_debounce(IDLE);
      }
      break;
    case LEFT:
      if (roverState == LEFT){
        if (previousFsmState != fsmState){
          // Drive left side backward
          digitalWrite(motorPin1, LOW);
          digitalWrite(motorPin2, HIGH);
          // Drive right side forward
          digitalWrite(motorPin3, HIGH);
          digitalWrite(motorPin4, LOW);
          Serial.println("LEFT");
          previousFsmState = fsmState;
        }
      } else{
          turnOffMotors();
          update_debounce(IDLE);
      }
      break;
    case CLOCKWISE:
      if (drillState == CLOCKWISE){
        if (previousFsmState != fsmState){
          // Turn on drill motor CW
          digitalWrite(drillPin1, HIGH);
          digitalWrite(drillPin2, LOW);
          Serial.println("CLOCKWISE");
          previousFsmState = fsmState;
        }
      } else{
          turnOffDrill();
          update_debounce(IDLE);
      }
      break;
    case COUNTERCLOCKWISE:
      if (drillState == COUNTERCLOCKWISE){
        if (previousFsmState != fsmState){
          // Turn on drill motor CCW
          digitalWrite(drillPin1, LOW);
          digitalWrite(drillPin2, HIGH);
          Serial.println("COUNTERCLOCKWISE");
          previousFsmState = fsmState;
        }
      } else{
          turnOffDrill();
          update_debounce(IDLE);
      }
      break;
    case EXTEND:
      if (gantryState == EXTEND){
        if (previousFsmState != fsmState){
          Serial.println("EXTEND");
          previousFsmState = fsmState;
        }
        stepperMotors.step(1); // extend the drill
      } else{
          update_debounce(IDLE);
      }
      break;
    case RETRACT:
      if (gantryState == RETRACT){
        if (previousFsmState != fsmState){
          Serial.println("RETRACT");
          previousFsmState = fsmState;
        }
        stepperMotors.step(-1); // retract the drill
      } else{
          update_debounce(IDLE);
      }
      break;
    case COLLECT:
      if (dataState == COLLECT){
        if (previousFsmState != fsmState){
          collectData();
          Serial.println("COLLECT");
          previousFsmState = fsmState;
        }
      } else{
          update_debounce(IDLE);
      }
      break;
    case DEBOUNCING:
      if (millis() - timer > debouncingTime){
        fsmState = futureState;
      }
      break;
  } 
}

void update_debounce(uint8_t future){
  previousFsmState = fsmState;
  fsmState = DEBOUNCING;
  futureState = future;
  timer = millis();
}
