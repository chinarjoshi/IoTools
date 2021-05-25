const uint8_t button = 33;

void setup() {
  Serial.begin(115200); // initialize serial
  pinMode(button,INPUT_PULLUP); // means the button == 0 when pressed
}

void loop() {
  if (digitalRead(button) == 0){
    Serial.println("Pressed");
  } else {
    Serial.println("Not Pressed");
  }
}
