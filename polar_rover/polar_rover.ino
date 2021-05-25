#include <PS4Controller.h> // used to connect to a PS4 controller
#include <Stepper.h> // used to control the stepper motors
#include <WiFi.h> // used to connect to WiFi Network
#include <string.h>  //used for some string handling and processing.

/*
 * Libraries needed for the DS18B20 sensor
 */
#include <OneWire.h>
#include <DallasTemperature.h>

/*
 * Libraries needed for the BME280 sensor
 */
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

/**
 * Drill Stepper Motor: pins 15,2,0,4
 * Top Limit Switch: pin 32
 * Bottom Limit Switch: pin 33
 * DC Motors: pins 13, 12, 14, 27, 26
 * Drill Motor: pins 16 (Enable = PWM), 17, 5
 * Temp Sensor: pin 25
 * Barometer: pins 21 and 22
 * 
 * Input Only (no pull-up resistor): 35, 34, 39, 36 
 */

#define SEALEVELPRESSURE_HPA (1013.25)

/*
 * States for the Finite State Machine (FSM):
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
#define PROBE 9
#define BME 10
// States for debouncing
#define DEBOUNCING 11

// Constants for the http requests:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int GETTING_PERIOD = 5000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

//char network[] = "Pineapple Master";
//char password[] = "12345678";
char network[] = "Rocket";//wifi username
char password[] = "13843180Sam";//wifi password
char key[] = "6,wD-Ak]^wzWe@G";

//Constants to control the rover
// Number of steps per output rotation
const uint8_t stepsPerRevolution = 200;

Stepper stepperMotors(stepsPerRevolution, 15, 2, 0, 4);

const uint8_t topLimitSwitch = 32;
const uint8_t bottomLimitSwitch = 33;

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
const uint8_t motorPin4 = 26;
const uint8_t motorPwm = 1;

OneWire oneWire(25); // connected through pin IO25
DallasTemperature sensors(&oneWire);
float probeTemp = 0;

Adafruit_BME280 bme; // I2C
float bmeTemp = 0;
float bmeHumid = 0;
float bmeAlt = 0;
float bmeAir = 0;

const int freq = 30000;
const uint8_t resolution = 8;

void setup(){
  Serial.begin(115200);
  delay(100);
  sensors.begin();
  delay(100);
  bool statusBME = bme.begin(0x77);
  delay(100);
  if (!statusBME) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  // Connect to WiFi:
  delay(100); //wait a bit (100 ms)
  WiFi.begin(network,password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

  // Initialize limit switches for the stepper motors
  pinMode(topLimitSwitch, INPUT_PULLUP);
  pinMode(bottomLimitSwitch, INPUT_PULLUP);
  
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
  
  stepperMotors.setSpeed(50); // initialize speed of the motor in RPM
  PS4.begin("24:62:ab:f9:7b:36"); //connect ps4 controller
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

void readTemp(){
  sensors.requestTemperatures();
  probeTemp = sensors.getTempCByIndex(0);
}

void readBarometer(){
  bmeTemp = bme.readTemperature(); // in Celsius
  bmeAir = bme.readPressure() / 100.0F; // in hPa
  bmeAlt = bme.readAltitude(SEALEVELPRESSURE_HPA); // Approx altitude in meters
  bmeHumid = bme.readHumidity(); // in %
}

uint8_t checkRover(){//checks if state is moving bkwrds or frwrds
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
    ledcWrite(motorPwm, 0);// turn off motor
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
  if (PS4.data.button.up && digitalRead(topLimitSwitch) != 0){
    state = RETRACT;
  } else if (PS4.data.button.down && digitalRead(bottomLimitSwitch) != 0){
      state = EXTEND;
  }
  return state;
}

uint8_t checkProbe(){
  uint8_t state = IDLE;
  if (PS4.data.button.square){
    state = PROBE;
  }
  return state;
}

uint8_t checkBME(){
  uint8_t state = IDLE;
  if (PS4.data.button.triangle){
    state = BME;
  }
  return state;
}

void main_fsm(){
  // Read the values from the PS4 controller
  uint8_t roverState = checkRover();
  uint8_t drillState = checkDrill();
  uint8_t gantryState = checkGantry();
  uint8_t probeState = checkProbe();
  uint8_t bmeState = checkBME();
  
  switch (fsmState){
    case IDLE:
      if (previousFsmState != fsmState){
        Serial.println("IDLE");
        previousFsmState = fsmState;
      }
      if (roverState != IDLE){
          updateDebounce(roverState);
      } else if (drillState != IDLE){
          updateDebounce(drillState);
      } else if (gantryState != IDLE){
          updateDebounce(gantryState);
      } else if (probeState != IDLE){
          updateDebounce(probeState);
      } else if (bmeState != IDLE){
          updateDebounce(bmeState);
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
          updateDebounce(IDLE);
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
          updateDebounce(IDLE);
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
          updateDebounce(IDLE);
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
          updateDebounce(IDLE);
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
        moveGantry(gantryState); // could move gantry
      } else{
          turnOffDrill();
          updateDebounce(IDLE);
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
        moveGantry(gantryState); // could move gantry
      } else{
          turnOffDrill();
          updateDebounce(IDLE);
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
          updateDebounce(IDLE);
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
          updateDebounce(IDLE);
      }
      break;
    case PROBE:
      if (probeState == PROBE){
        if (previousFsmState != fsmState){
          readTemp();
          Serial.println("PROBE");
          previousFsmState = fsmState;
        }
      } else{
          updateDebounce(IDLE);
      }
      break;
    case BME:
      if (bmeState == BME){
        if (previousFsmState != fsmState){
          readBarometer();
          sendData();
          Serial.println("BME");
          previousFsmState = fsmState;
        }
      } else{
          updateDebounce(IDLE);
      }
      break;
    case DEBOUNCING:
      if (millis() - timer > debouncingTime){
        fsmState = futureState;
      }
      break;
  } 
}

void updateDebounce(uint8_t future){
  previousFsmState = fsmState;
  fsmState = DEBOUNCING;
  futureState = future;
  timer = millis();
}

void moveGantry(uint8_t gantryState){
  if (gantryState == EXTEND){
    stepperMotors.step(1); // extend the drill
  } else if (gantryState == RETRACT) {
    stepperMotors.step(-1); // retract the drill
  }
}

void sendData() {
  char body[100]; //for body
  sprintf(body,"key=%s&probeTemp=%.3f&bmeTemp=%.3f&bmeHumid=%.3f&bmeAlt=%.3f&bmeAir=%.3f", key, probeTemp, bmeTemp, bmeHumid, bmeAlt, bmeAir); //generate body
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(request_buffer,"POST http://10.0.0.64/update HTTP/1.1\r\n");
  strcat(request_buffer,"Host: 10.0.0.64\r\n");
  strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer,"\r\n"); //new line from header to body
  strcat(request_buffer,body); //body
  strcat(request_buffer,"\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("10.0.0.64", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer); //viewable in Serial Terminal
}
