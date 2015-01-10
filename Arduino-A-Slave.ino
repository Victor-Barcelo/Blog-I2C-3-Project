#include <Wire.h>
#include <Time.h>

#define SLAVE_ADDRESS 0x04
#define DEBUG true

const int PIN_LIGHTSENSOR = 0;
const int PIN_PUSHBUTTON = 7;

// Button debounce: (http://arduino.cc/en/Tutorial/Debounce)
int buttonState;
int lastButtonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 50; 
//-------------------------

enum Command {None, SendSensorData, SendBtnState};
Command command = None;

bool isButtonPressed = false;

void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(recieveCommand);
  Wire.onRequest(sendData);
  if(DEBUG) Serial.println("Ready!");
}

void loop() {
  delay(100);
  checkBtnState();
}

void recieveCommand(int byteCount){
  int cmd = Wire.read();
  if(cmd == 0) configureTime();
  if(cmd == 1) command = SendSensorData;
  if(cmd == 2) command = SendBtnState;
}

void sendData(){
  switch(command) {
    case SendSensorData: 
      sendLightData();
      command = None;
      break;
    case SendBtnState: 
      sendBtnState();
      command = None;
      break;
  }
}

void sendLightData(){
  byte data[2];
  int light = analogRead(PIN_LIGHTSENSOR);
  if(DEBUG){
    Serial.print("Light: ");
    Serial.println(light);
  }
  data[0] = lowByte(light); 
  data[1] = highByte(light);  
  byte dataSend[] = {data[0],data[1]};
  Wire.write(dataSend, 2);
}

void checkBtnState(){
  int reading = digitalRead(PIN_PUSHBUTTON);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  } 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        isButtonPressed = true;
      }
    }
  }
  lastButtonState = reading;
}

void sendBtnState(){
  if(isButtonPressed){
    Wire.write(1);
    isButtonPressed = false;
  } else{
    Wire.write(0);
  }
}

void configureTime(){
  unsigned long bytes[4];
  int i = 0;
  while(Wire.available()){
    bytes[i] = Wire.read();
    i++;
  }
  unsigned long result = bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
  setTime(result);
}